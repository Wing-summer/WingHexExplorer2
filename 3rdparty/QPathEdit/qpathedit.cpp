#include "qpathedit.h"

#include <QAction>
#include <QApplication>
#include <QCompleter>
#include <QDropEvent>
#include <QEvent>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QPainter>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QValidator>

// HELPER CLASSES

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wpadded")
class PathValidator : public QValidator {
public:
    PathValidator(QObject *parent);
    void setMode(QPathEdit::PathMode mode);
    void setAllowEmpty(bool allow);
    State validate(QString &text, int &) const override;

private:
    QPathEdit::PathMode mode;
    bool allowEmpty;
};
QT_WARNING_POP

// QPATHEDIT IMPLEMENTATION

QPathEdit::QPathEdit(QWidget *parent, QPathEdit::Style style)
    : QPathEdit(ExistingFile, parent, style) {}

QPathEdit::QPathEdit(QPathEdit::PathMode pathMode, QWidget *parent,
                     QPathEdit::Style style)
    : QWidget(parent), edit(new QLineEdit(this)),
      pathCompleter(new QCompleter(this)),
      completerModel(new QFileSystemModel(this)),
      pathValidator(new PathValidator(this)), dialog(new QFileDialog(this)),
      currentValidPath(), wasPathValid(true), uiStyle(style),
      mode(ExistingFile), defaultDir(QStandardPaths::writableLocation(
                              QStandardPaths::HomeLocation)),
      allowEmpty(true), toolButton(new QToolButton(this)),
      dialogAction(new QAction(getDefaultIcon(), tr("Open FileDialog"), this)),
      hasCustomIcon(false) {
    // setup dialog
    dialog->setOptions(QFileDialog::Option(0));
    setPathMode(pathMode);
    connect(dialog, &QFileDialog::fileSelected, this,
            &QPathEdit::dialogFileSelected);

    // setup completer
    completerModel->setRootPath(QString());
    completerModel->setNameFilterDisables(false);
    connect(completerModel, &QFileSystemModel::directoryLoaded, pathCompleter,
            [this](const QString &) { pathCompleter->complete(); });
    pathCompleter->setModel(completerModel);

    wingdialog = new FramelessDialogBase(this);
    dialog->setWindowFlag(Qt::Widget);
    wingdialog->buildUpContent(dialog);
    wingdialog->setWindowTitle(qAppName());

    QObject::connect(dialog, &QFileDialog::finished, wingdialog,
                     &FramelessDialogBase::done);

    // setup this
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    layout->addWidget(edit);
    layout->addWidget(toolButton);
    setLayout(layout);
    // setup lineedit
    edit->installEventFilter(this);
    edit->setCompleter(pathCompleter);
    edit->setValidator(pathValidator);
    edit->setDragEnabled(true);
    edit->setReadOnly(true);
    connect(edit, &QLineEdit::editingFinished, this,
            &QPathEdit::editTextUpdate);
    connect(edit, &QLineEdit::textChanged, this, &QPathEdit::updateValidInfo);
    // setup "button"
    connect(dialogAction, &QAction::triggered, this, &QPathEdit::showDialog);
    toolButton->setDefaultAction(dialogAction);
    int height = edit->sizeHint().height();
#ifdef Q_OS_WIN
    height += 2;
#endif
    toolButton->setFixedSize(height, height);
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wimplicit-fallthrough")
    QT_WARNING_DISABLE_CLANG("-Wimplicit-fallthrough")
    switch (style) {
    case JoinedButton:
        edit->addAction(dialogAction, QLineEdit::TrailingPosition);
    case NoButton:
        toolButton->setVisible(false);
        break;
    default:
        break;
    }
    QT_WARNING_POP

    QWidget::setTabOrder(edit, toolButton);
    setFocusPolicy(edit->focusPolicy());
    setFocusProxy(edit);
    setAcceptDrops(true);
}

QPathEdit::QPathEdit(QPathEdit::PathMode pathMode,
                     const QString &defaultDirectory, QWidget *parent,
                     QPathEdit::Style style)
    : QPathEdit(pathMode, parent, style) {
    setDefaultDirectory(defaultDirectory);
}

QPathEdit::PathMode QPathEdit::pathMode() const { return mode; }

void QPathEdit::setPathMode(PathMode pathMode) {
    mode = pathMode;
    pathValidator->setMode(pathMode);
    currentValidPath.clear();
    emit pathChanged(QString());
    edit->clear();
    switch (pathMode) {
    case ExistingFile:
        dialog->setAcceptMode(QFileDialog::AcceptOpen);
        dialog->setFileMode(QFileDialog::ExistingFile);
        completerModel->setFilter(QDir::AllEntries | QDir::AllDirs |
                                  QDir::NoDotAndDotDot);
        break;
    case ExistingFolder:
        dialog->setAcceptMode(QFileDialog::AcceptOpen);
        dialog->setFileMode(QFileDialog::Directory);
        completerModel->setFilter(QDir::Drives | QDir::Dirs |
                                  QDir::NoDotAndDotDot);
        break;
    case AnyFile:
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setFileMode(QFileDialog::AnyFile);
        completerModel->setFilter(QDir::AllEntries | QDir::AllDirs |
                                  QDir::NoDotAndDotDot);
        break;
    }
}

QFileDialog::Options QPathEdit::dialogOptions() const {
    return dialog->options();
}

void QPathEdit::setDialogOptions(QFileDialog::Options dialogOptions) {
    dialog->setOptions(dialogOptions);
}

bool QPathEdit::isEmptyPathAllowed() const { return allowEmpty; }

void QPathEdit::setAllowEmptyPath(bool allowEmptyPath) {
    allowEmpty = allowEmptyPath;
    pathValidator->setAllowEmpty(allowEmptyPath);
}

QString QPathEdit::defaultDirectory() const { return defaultDir; }

void QPathEdit::setDefaultDirectory(const QString &defaultDirectory) {
    defaultDir = defaultDirectory;
}

QString QPathEdit::path() const { return currentValidPath; }

QString QPathEdit::editPath() const { return edit->text(); }

QUrl QPathEdit::pathUrl() const {
    return QUrl::fromLocalFile(currentValidPath);
}

bool QPathEdit::hasAcceptableInput() const { return wasPathValid; }

bool QPathEdit::setPath(QString path, bool allowInvalid) {
    if (edit->text() == path)
        return true;

    if (allowInvalid)
        edit->setText(path);

    int pseudo = 0;
    if (pathValidator->validate(path, pseudo) == QValidator::Acceptable) {
        currentValidPath =
            path.replace(QStringLiteral("\\"), QStringLiteral("/"));
        if (!allowInvalid)
            edit->setText(path);
        emit pathChanged(path);
        return true;
    } else
        return false;
}

void QPathEdit::clear() {
    edit->clear();
    currentValidPath.clear();
    emit pathChanged(QString());
}

QString QPathEdit::placeholder() const { return edit->placeholderText(); }

void QPathEdit::setPlaceholder(const QString &placeholder) {
    edit->setPlaceholderText(placeholder);
}

QStringList QPathEdit::nameFilters() const { return dialog->nameFilters(); }

void QPathEdit::setNameFilters(const QStringList &nameFilters) {
    dialog->setNameFilters(nameFilters);
    QStringList tmp = modelFilters(nameFilters);
    completerModel->setNameFilters(tmp);
}

QStringList QPathEdit::mimeTypeFilters() const {
    return dialog->mimeTypeFilters();
}

void QPathEdit::setMimeTypeFilters(const QStringList &mimeFilters) {
    dialog->setMimeTypeFilters(mimeFilters);
    QStringList tmp = modelFilters(dialog->nameFilters());
    completerModel->setNameFilters(tmp);
}

bool QPathEdit::isEditable() const { return !edit->isReadOnly(); }

void QPathEdit::setEditable(bool editable) { edit->setReadOnly(!editable); }

bool QPathEdit::useCompleter() const { return edit->completer(); }

void QPathEdit::setUseCompleter(bool useCompleter) {
    edit->setCompleter(useCompleter ? pathCompleter : nullptr);
}

QPathEdit::Style QPathEdit::style() const { return uiStyle; }

void QPathEdit::setStyle(QPathEdit::Style style,
                         QLineEdit::ActionPosition position) {
    if (uiStyle == style)
        return;

    switch (style) {
    case SeperatedButton:
        edit->removeAction(dialogAction);
        toolButton->setVisible(true);
        break;
    case JoinedButton:
        edit->addAction(dialogAction, position);
        toolButton->setVisible(false);
        break;
    case NoButton:
        edit->removeAction(dialogAction);
        toolButton->setVisible(false);
        break;
    }

    uiStyle = style;
    if (!hasCustomIcon)
        dialogAction->setIcon(getDefaultIcon());
}

QIcon QPathEdit::dialogButtonIcon() const { return dialogAction->icon(); }

void QPathEdit::setDialogButtonIcon(const QIcon &icon) {
    dialogAction->setIcon(icon);
    hasCustomIcon = true;
}

void QPathEdit::resetDialogButtonIcon() {
    dialogAction->setIcon(QPathEdit::getDefaultIcon());
    hasCustomIcon = false;
}

void QPathEdit::showDialog() {
    if (wingdialog->isVisible()) {
        wingdialog->raise();
        wingdialog->activateWindow();
        return;
    }

    QString oldPath = edit->text();
    if (oldPath.isEmpty())
        dialog->setDirectory(defaultDir);
    else {
        if (mode == ExistingFolder)
            dialog->setDirectory(oldPath);
        else {
            QFileInfo info(oldPath);
            if (info.isDir())
                dialog->setDirectory(oldPath);
            else {
                dialog->setDirectory(info.dir());
                dialog->selectFile(info.fileName());
            }
        }
    }

    wingdialog->open();
}

void QPathEdit::updateValidInfo(const QString &path) {
    emit editPathChanged(path);
    completerModel->index(
        QFileInfo(path).dir().absolutePath()); // enforce "directory loading"
    if (edit->hasAcceptableInput()) {
        if (!wasPathValid) {
            wasPathValid = true;
            edit->setPalette(palette());
            emit acceptableInputChanged(wasPathValid);
        }
    } else {
        if (wasPathValid) {
            wasPathValid = false;
            QPalette pal = palette();
            pal.setColor(QPalette::Text, QColor(QStringLiteral("#B40404")));
            edit->setPalette(pal);
            emit acceptableInputChanged(wasPathValid);
        }
    }
}

void QPathEdit::editTextUpdate() {
    if (edit->hasAcceptableInput()) {
        QString newPath =
            edit->text().replace(QStringLiteral("\\"), QStringLiteral("/"));
        if (currentValidPath != newPath) {
            currentValidPath = newPath;
            emit pathChanged(currentValidPath);
        }
    }
}

void QPathEdit::dialogFileSelected(const QString &file) {
    if (!file.isEmpty()) {
        auto sels = dialog->selectedFiles();
        edit->setText(
            sels.first().replace(QStringLiteral("\\"), QStringLiteral("/")));
        editTextUpdate();
    }
}

QStringList QPathEdit::modelFilters(const QStringList &normalFilters) {
    QStringList res;
    foreach (QString filter, normalFilters) {
        QRegularExpressionMatch match =
            QRegularExpression(QStringLiteral("^.*\\((.*)\\)$")).match(filter);
        if (match.hasMatch())
            res.append(match.captured(1).split(
                QRegularExpression(QStringLiteral("\\s"))));
    }
    return res;
}

QIcon QPathEdit::getDefaultIcon() {
    switch (uiStyle) {
    case SeperatedButton: {
        QImage image(16, 16, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setFont(font());
        painter.setPen(palette().color(QPalette::ButtonText));
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.drawText(QRect(0, 0, 16, 16), Qt::AlignCenter, tr("..."));
        return QPixmap::fromImage(image);
    }
    case JoinedButton:
        return QIcon::fromTheme(
            QStringLiteral("view-choose"),
            QIcon(QStringLiteral(":/qpathedit/icons/dialog.ico")));
    case NoButton:
        return QIcon();
    }
    return QIcon();
}

bool QPathEdit::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Space &&
            keyEvent->modifiers() == Qt::ControlModifier) {
            pathCompleter->complete();
            return true;
        } else
            return QObject::eventFilter(watched, event);
    } else if (event->type() == QEvent::Drop) {
        QDropEvent *dropEvent = static_cast<QDropEvent *>(event);
        if (dropEvent->mimeData()->hasUrls() &&
            dropEvent->mimeData()->urls().count() == 1 &&
            dropEvent->mimeData()->urls().first().isLocalFile()) {

            QString filePath =
                dropEvent->mimeData()->urls().first().toLocalFile();
            bool matched = true;

            QFileInfo fi(filePath);
            if (!dialog->nameFilters().isEmpty() && fi.isFile() &&
                mode == ExistingFile) {
                const QString fileNameSuffix = fi.suffix();
                if (!fileNameSuffix.isEmpty()) {
                    // regexp copied from Qt sources
                    // QPlatformFileDialogHelper::filterRegExp
                    static QRegularExpression regexp(
                        "^(.*)\\(([a-zA-Z0-9_.,*? "
                        "+;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$");

                    // Makes a ["*.png", "*.jpg", "*.bmp"] formatted list of
                    // filters from the extension filters in format ["Image
                    // Files (*.png *.jpg)",
                    // ""Bitmaps (*.bmp)]
                    QStringList extensionsFilters;
                    foreach (const QString &filter, dialog->nameFilters()) {
                        QString f = filter;
                        QRegularExpressionMatch match;
                        Q_UNUSED(filter.indexOf(regexp, 0, &match));
                        if (match.hasMatch())
                            f = match.captured(2);
#if QT_DEPRECATED_SINCE(5, 15)
                        extensionsFilters.append(
                            f.split(' ', Qt::SkipEmptyParts));
#elif
                        extensionsFilters.append(
                            f.split(' ', QString::SkipEmptyParts));
#endif
                    }

                    matched = false;
                    foreach (const QString &extensionFilter,
                             extensionsFilters) {
                        if (extensionFilter == QStringLiteral("*.*")) {
                            matched = true;
                            break;
                        }

                        const QString filterSuffix =
                            QFileInfo(extensionFilter).suffix();
                        if (!filterSuffix.isEmpty()) {
                            if (fileNameSuffix == filterSuffix) {
                                matched = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (matched)
                setPath(filePath);
            return true;
        }
        return false;
    } else
        return QObject::eventFilter(watched, event);
}

// HELPER CLASSES IMPLEMENTATION

PathValidator::PathValidator(QObject *parent)
    : QValidator(parent), mode(QPathEdit::ExistingFile), allowEmpty(true) {}

void PathValidator::setMode(QPathEdit::PathMode mode) { this->mode = mode; }

void PathValidator::setAllowEmpty(bool allow) { allowEmpty = allow; }

QValidator::State PathValidator::validate(QString &text, int &) const {
    // check if empty is accepted
    if (text.isEmpty())
        return allowEmpty ? QValidator::Acceptable : QValidator::Intermediate;

    // nonexisting parent dir is not possible
    QFileInfo pathInfo(text);
    if (!pathInfo.dir().exists())
        return QValidator::Invalid;

    switch (mode) {
    case QPathEdit::AnyFile: // acceptable, as long as it's not an directoy
        if (pathInfo.isDir())
            return QValidator::Intermediate;
        else
            return QValidator::Acceptable;
    case QPathEdit::ExistingFile: // must be an existing file
        if (pathInfo.exists() && pathInfo.isFile())
            return QValidator::Acceptable;
        else
            return QValidator::Intermediate;
    case QPathEdit::ExistingFolder: // must be an existing folder
        if (pathInfo.exists() && pathInfo.isDir())
            return QValidator::Acceptable;
        else
            return QValidator::Intermediate;
    }

    return QValidator::Invalid;
}
