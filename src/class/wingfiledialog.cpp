#include "wingfiledialog.h"
#include "../dialog/framelessdialog.h"

QString WingFileDialog::getExistingDirectory(QWidget *parent,
                                             const QString &caption,
                                             const QString &dir,
                                             QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl = getExistingDirectoryUrl(
        parent, caption, QUrl::fromLocalFile(dir), options, schemes);
    if (selectedUrl.isLocalFile() || selectedUrl.isEmpty())
        return selectedUrl.toLocalFile();
    else
        return selectedUrl.toString();
}

QUrl WingFileDialog::getExistingDirectoryUrl(
    QWidget *parent, const QString &caption, const QUrl &dir,
    QFileDialog::Options options, const QStringList &supportedSchemes) {

    auto filedialog = new QFileDialog(parent);
    filedialog->setDirectoryUrl(dir);
    filedialog->setOptions(options);
    filedialog->setFileMode(QFileDialog::Directory);
    filedialog->setSupportedSchemes(supportedSchemes);

    filedialog->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.setResizeable(false, d.contentsMargins());
    d.buildUpContent(filedialog);
    d.setWindowTitle(caption);
    d.resize(filedialog->width(), filedialog->height() + d.titleHeight());

    QObject::connect(filedialog, &QFileDialog::finished, &d,
                     &FramelessDialog::done);

    if (d.exec() == QDialog::Accepted)
        return filedialog->selectedUrls().value(0);
    return QUrl();
}

void WingFileDialog::getOpenFileContent(
    const QString &nameFilter,
    const std::function<void(const QString &, const QByteArray &)>
        &fileOpenCompleted,
    const QString &caption, QWidget *parent) {

    QFileDialog *dialog = new QFileDialog();
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->selectNameFilter(nameFilter);

    dialog->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.setResizeable(true, d.contentsMargins());
    d.buildUpContent(dialog);
    d.setWindowTitle(caption);
    d.resize(dialog->width(), dialog->height() + d.titleHeight());

    auto fileSelected = [=](const QString &fileName) {
        QByteArray fileContent;
        if (!fileName.isNull()) {
            QFile selectedFile(fileName);
            if (selectedFile.open(QIODevice::ReadOnly))
                fileContent = selectedFile.readAll();
        }
        fileOpenCompleted(fileName, fileContent);
    };
    auto dialogClosed = [dialog, &d](int code) {
        d.done(code);
        delete dialog;
    };
    QObject::connect(dialog, &QFileDialog::fileSelected, fileSelected);
    QObject::connect(dialog, &QFileDialog::finished, dialogClosed);
    dialog->show();
}

QString WingFileDialog::getOpenFileName(QWidget *parent, const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl =
        getOpenFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter,
                       selectedFilter, options, schemes);
    if (selectedUrl.isLocalFile() || selectedUrl.isEmpty())
        return selectedUrl.toLocalFile();
    else
        return selectedUrl.toString();
}

QStringList WingFileDialog::getOpenFileNames(QWidget *parent,
                                             const QString &caption,
                                             const QString &dir,
                                             const QString &filter,
                                             QString *selectedFilter,
                                             QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QList<QUrl> selectedUrls =
        getOpenFileUrls(parent, caption, QUrl::fromLocalFile(dir), filter,
                        selectedFilter, options, schemes);
    QStringList fileNames;
    fileNames.reserve(selectedUrls.size());
    for (const QUrl &url : selectedUrls) {
        if (url.isLocalFile() || url.isEmpty())
            fileNames << url.toLocalFile();
        else
            fileNames << url.toString();
    }
    return fileNames;
}

QUrl WingFileDialog::getOpenFileUrl(QWidget *parent, const QString &caption,
                                    const QUrl &dir, const QString &filter,
                                    QString *selectedFilter,
                                    QFileDialog::Options options,
                                    const QStringList &supportedSchemes) {

    QFileDialog *dialog = new QFileDialog(parent);
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->selectNameFilter(filter);
    dialog->setSupportedSchemes(supportedSchemes);
    dialog->setOptions(options);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.setResizeable(true, d.contentsMargins());
    d.buildUpContent(dialog);
    d.setWindowTitle(caption);
    d.resize(dialog->width(), dialog->height() + d.titleHeight());

    QObject::connect(dialog, &QFileDialog::finished, &d,
                     &FramelessDialog::done);

    if (selectedFilter && !selectedFilter->isEmpty())
        dialog->selectNameFilter(*selectedFilter);
    if (d.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog->selectedNameFilter();
        return dialog->selectedUrls().value(0);
    }
    return QUrl();
}

QList<QUrl> WingFileDialog::getOpenFileUrls(
    QWidget *parent, const QString &caption, const QUrl &dir,
    const QString &filter, QString *selectedFilter,
    QFileDialog::Options options, const QStringList &supportedSchemes) {
    QFileDialog *dialog = new QFileDialog(parent);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    dialog->selectNameFilter(filter);
    dialog->setSupportedSchemes(supportedSchemes);
    dialog->setOptions(options);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.setResizeable(true, d.contentsMargins());
    d.buildUpContent(dialog);
    d.setWindowTitle(caption);
    d.resize(dialog->width(), dialog->height() + d.titleHeight());

    QObject::connect(dialog, &QFileDialog::finished, &d,
                     &FramelessDialog::done);

    if (selectedFilter && !selectedFilter->isEmpty())
        dialog->selectNameFilter(*selectedFilter);
    if (d.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog->selectedNameFilter();
        return dialog->selectedUrls();
    }
    return {};
}

QString WingFileDialog::getSaveFileName(QWidget *parent, const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl =
        getSaveFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter,
                       selectedFilter, options, schemes);
    if (selectedUrl.isLocalFile() || selectedUrl.isEmpty())
        return selectedUrl.toLocalFile();
    else
        return selectedUrl.toString();
}

QUrl WingFileDialog::getSaveFileUrl(QWidget *parent, const QString &caption,
                                    const QUrl &dir, const QString &filter,
                                    QString *selectedFilter,
                                    QFileDialog::Options options,
                                    const QStringList &supportedSchemes) {

    QFileDialog *dialog = new QFileDialog(parent);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->selectNameFilter(filter);
    dialog->setSupportedSchemes(supportedSchemes);
    dialog->setOptions(options);
    dialog->setWindowFlag(Qt::Widget);
    dialog->setAcceptMode(QFileDialog::AcceptSave);

    FramelessDialog d(parent);
    d.setResizeable(true, d.contentsMargins());
    d.buildUpContent(dialog);
    d.setWindowTitle(caption);
    d.resize(dialog->width(), dialog->height() + d.titleHeight());

    QObject::connect(dialog, &QFileDialog::finished, &d,
                     &FramelessDialog::done);

    if (selectedFilter && !selectedFilter->isEmpty())
        dialog->selectNameFilter(*selectedFilter);
    if (d.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog->selectedNameFilter();
        return dialog->selectedUrls().value(0);
    }
    return QUrl();
}

void WingFileDialog::saveFileContent(const QByteArray &fileContent,
                                     const QString &fileNameHint,
                                     const QString &caption, QWidget *parent) {

    QFileDialog *dialog = new QFileDialog();
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->selectFile(fileNameHint);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.setResizeable(true, d.contentsMargins());
    d.buildUpContent(dialog);
    d.setWindowTitle(caption);
    d.resize(dialog->width(), dialog->height() + d.titleHeight());

    auto fileSelected = [=](const QString &fileName) {
        if (!fileName.isNull()) {
            QFile selectedFile(fileName);
            if (selectedFile.open(QIODevice::WriteOnly))
                selectedFile.write(fileContent);
        }
    };
    auto dialogClosed = [dialog, &d](int code) {
        d.done(code);
        delete dialog;
    };
    QObject::connect(dialog, &QFileDialog::fileSelected, fileSelected);
    QObject::connect(dialog, &QFileDialog::finished, dialogClosed);
    dialog->show();
}

WingFileDialog::WingFileDialog() {}
