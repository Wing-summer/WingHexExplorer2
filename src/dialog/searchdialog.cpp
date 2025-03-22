/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "searchdialog.h"
#include "utilities.h"

#include "class/wingmessagebox.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStringView>
#include <QToolButton>
#include <QVBoxLayout>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QGuiApplication>
#include <QStyleHints>
#endif

static SearchDialog *s_instance = Q_NULLPTR;

SearchWidget::SearchWidget(WingCodeEdit *editor, QWidget *parent)
    : QWidget(parent), m_editor(editor) {

    auto tbMenu = new QToolButton(this);
    tbMenu->setAutoRaise(true);
    tbMenu->setIcon(QIcon::fromTheme(QStringLiteral("theme")));
    tbMenu->setToolTip(tr("Search Settings"));
    tbMenu->setStyleSheet(
        QStringLiteral("QToolButton::menu-indicator { image: none; }"));

    auto settingsMenu = new QMenu(this);
    m_caseSensitive = settingsMenu->addAction(tr("Match ca&se"));
    m_caseSensitive->setCheckable(true);
    m_wholeWord = settingsMenu->addAction(tr("Match &whole words"));
    m_wholeWord->setCheckable(true);
    m_regex = settingsMenu->addAction(tr("Regular e&xpressions"));
    m_regex->setCheckable(true);
    m_escapes = settingsMenu->addAction(tr("&Escape sequences"));
    m_escapes->setCheckable(true);
    m_wrapSearch = settingsMenu->addAction(tr("Wrap Aro&und"));
    m_wrapSearch->setCheckable(true);
    tbMenu->setMenu(settingsMenu);
    tbMenu->setPopupMode(QToolButton::InstantPopup);

    connect(m_caseSensitive, &QAction::triggered, this,
            &SearchWidget::updateSettings);
    connect(m_wholeWord, &QAction::triggered, this,
            &SearchWidget::updateSettings);
    connect(m_regex, &QAction::triggered, this, &SearchWidget::updateSettings);
    connect(m_escapes, &QAction::triggered, this,
            &SearchWidget::updateSettings);
    connect(m_wrapSearch, &QAction::triggered, this,
            &SearchWidget::updateSettings);

    m_searchText = new QLineEdit(this);
    m_searchText->setClearButtonEnabled(true);
    setFocusProxy(m_searchText);

    auto tbNext = new QToolButton(this);
    tbNext->setAutoRaise(true);
    tbNext->setIcon(ICONRES(""));
    tbNext->setToolTip(tr("Find Next"));

    auto tbPrev = new QToolButton(this);
    tbPrev->setAutoRaise(true);
    tbPrev->setIcon(ICONRES(""));
    tbPrev->setToolTip(tr("Find Previous"));

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    layout->addWidget(tbMenu);
    layout->addWidget(m_searchText);
    layout->addWidget(tbNext);
    layout->addWidget(tbPrev);
    setLayout(layout);

    connect(m_searchText, &QLineEdit::textChanged, this,
            [this](const QString &text) {
                m_searchParams.searchText =
                    m_escapes->isChecked()
                        ? SearchDialog::translateEscapes(text)
                        : text;
                m_editor->setLiveSearch(m_searchParams);
            });
    connect(m_searchText, &QLineEdit::returnPressed, this,
            [this] { searchNext(false); });
    connect(tbNext, &QToolButton::clicked, this, [this] { searchNext(false); });
    connect(tbPrev, &QToolButton::clicked, this, [this] { searchNext(true); });
}

void SearchWidget::setSearchText(const QString &text) {
    m_searchText->setText(text);
}

void SearchWidget::activate() {
    m_searchParams.caseSensitive = m_caseSensitive->isChecked();
    m_searchParams.wholeWord = m_wholeWord->isChecked();
    m_searchParams.regex = m_regex->isChecked();

    setFocus(Qt::OtherFocusReason);
    m_searchText->selectAll();
    m_editor->setLiveSearch(m_searchParams);
}

void SearchWidget::searchNext(bool reverse) {
    if (!isVisible()) {
        setVisible(true);
        setEnabled(true);
        activate();
    }
    if (m_searchParams.searchText.isEmpty())
        return;

    auto searchCursor = m_editor->textSearch(m_editor->textCursor(),
                                             m_searchParams, false, reverse);
    if (searchCursor.isNull() && m_wrapSearch->isChecked()) {
        QTextCursor wrapCursor = m_editor->textCursor();
        wrapCursor.movePosition(reverse ? QTextCursor::End
                                        : QTextCursor::Start);
        searchCursor =
            m_editor->textSearch(wrapCursor, m_searchParams, true, reverse);
    }

    if (searchCursor.isNull())
        QMessageBox::information(this, QString(),
                                 tr("The specified text was not found"));
    else
        m_editor->setTextCursor(searchCursor);
}

void SearchWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    // Round the corners of this widget.  Not strictly necessary, but
    // it looks nicer...
    const int arc = 4;
    const int circ = 8;
    const int h = height() - 1;
    const int w = width() - 1;
    painter.setPen(palette().color(QPalette::Mid));
    const QColor windowColor = palette().color(QPalette::Window);
    painter.setBrush(windowColor);
    painter.drawEllipse(0, h - circ, circ, circ);
    painter.drawEllipse(w - circ, h - circ, circ, circ);
    painter.fillRect(0, 0, arc, h - arc, windowColor);
    painter.fillRect(w - arc, 0, arc, h - arc, windowColor);
    painter.fillRect(arc, h - arc, w - circ, arc, windowColor);
    painter.fillRect(arc, 0, w - circ, h - arc, windowColor);
    painter.drawLine(0, 0, 0, h - arc);
    painter.drawLine(arc, h, w - arc, h);
    painter.drawLine(w, 0, w, h - arc);
}

void SearchWidget::updateSettings() {
    m_searchParams.caseSensitive = m_caseSensitive->isChecked();
    m_searchParams.wholeWord = m_wholeWord->isChecked();
    m_searchParams.regex = m_regex->isChecked();

    m_editor->setLiveSearch(m_searchParams);
}

/* Just sets some more sane defaults for QComboBox:
 * - Don't auto-insert items (we handle that manually)
 * - Disable the completer, since it insists on changing the typed
 *   item to match another item in the list that differs only in case.
 */
class SearchComboBox : public QComboBox {
public:
    explicit SearchComboBox(QWidget *parent) : QComboBox(parent) {
        setEditable(true);
        setInsertPolicy(QComboBox::NoInsert);
        setDuplicatesEnabled(true);
        setCompleter(Q_NULLPTR);
    }
};

SearchDialog::SearchDialog(QWidget *parent)
    : FramelessDialogBase(parent), m_editor() {
    s_instance = this;
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Find and Replace..."));
    setWindowIcon(ICONRES("edit-find-replace"));

    m_searchText = new SearchComboBox(this);
    m_replaceText = new SearchComboBox(this);

    m_caseSensitive = new QCheckBox(tr("Match ca&se"), this);
    m_caseSensitive->setChecked(false);
    m_wholeWord = new QCheckBox(tr("Match &whole words"), this);
    m_wholeWord->setChecked(false);
    m_regex = new QCheckBox(tr("Regular e&xpressions"), this);
    m_regex->setChecked(false);
    m_escapes = new QCheckBox(tr("&Escape sequences"), this);
    m_escapes->setChecked(false);
    m_wrapSearch = new QCheckBox(tr("Wrap Aro&und"), this);
    m_wrapSearch->setChecked(false);

    // QDialogButtonBox insists on rearranging buttons depending on your
    // platform, which would be fine if we only had standard actions, but most
    // of our action buttons here are custom.
    auto buttonBox = new QWidget(this);
    buttonBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    auto buttonLayout = new QVBoxLayout(buttonBox);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(5);
    auto findNext = new QPushButton(tr("Find &Next"), this);
    buttonLayout->addWidget(findNext);
    auto findPrev = new QPushButton(tr("Find &Previous"), this);
    buttonLayout->addWidget(findPrev);
    auto replaceNext = new QPushButton(tr("&Replace"), this);
    buttonLayout->addWidget(replaceNext);
    auto replaceAll = new QPushButton(tr("Replace &All"), this);
    buttonLayout->addWidget(replaceAll);
    m_replaceSelectionButton = new QPushButton(tr("&In Selection"), this);
    buttonLayout->addWidget(m_replaceSelectionButton);
    buttonLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                          QSizePolicy::MinimumExpanding));
    auto closeButton = new QPushButton(tr("&Close"), this);
    buttonLayout->addWidget(closeButton);

    connect(findNext, &QPushButton::clicked, this,
            &SearchDialog::searchForward);
    connect(findPrev, &QPushButton::clicked, this,
            &SearchDialog::searchBackward);
    connect(replaceNext, &QPushButton::clicked, this,
            &SearchDialog::replaceCurrent);
    connect(replaceAll, &QPushButton::clicked, this,
            [this] { performReplaceAll(WholeDocument); });
    connect(m_replaceSelectionButton, &QPushButton::clicked, this,
            [this] { performReplaceAll(InSelection); });
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    auto w = new QWidget(this);
    auto layout = new QGridLayout(w);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setVerticalSpacing(5);
    layout->setHorizontalSpacing(10);
    auto searchLabel = new QLabel(tr("&Find:"), this);
    searchLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    searchLabel->setBuddy(m_searchText);
    layout->addWidget(searchLabel, 0, 0);
    layout->addWidget(m_searchText, 0, 1, 1, 2);
    auto replaceLabel = new QLabel(tr("Replace wit&h:"), this);
    replaceLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    replaceLabel->setBuddy(m_replaceText);
    layout->addWidget(replaceLabel, 1, 0);
    layout->addWidget(m_replaceText, 1, 1, 1, 2);
    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::MinimumExpanding,
                                    QSizePolicy::Fixed),
                    2, 0, 1, 3);
    layout->addWidget(m_caseSensitive, 3, 1);
    layout->addWidget(m_wholeWord, 4, 1);
    layout->addWidget(m_regex, 5, 1);
    layout->addWidget(m_escapes, 6, 1);
    layout->addWidget(m_wrapSearch, 3, 2);
    layout->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
        layout->rowCount(), 0, 1, 3);
    layout->addWidget(buttonBox, 0, layout->columnCount(), layout->rowCount(),
                      1);
    buildUpContent(w);
}

SearchDialog::~SearchDialog() {
    syncSearchSettings(false);
    s_instance = Q_NULLPTR;
}

SearchDialog *SearchDialog::create(WingCodeEdit *editor, QWidget *parent) {
    if (s_instance) {
        s_instance->raise();
    } else {
        Q_ASSERT(parent);
        s_instance = new SearchDialog(parent);
        s_instance->show();
        s_instance->raise();
        s_instance->activateWindow();

        s_instance->m_editor = editor;
        connect(s_instance->m_editor, &WingCodeEdit::selectionChanged,
                s_instance, [] {
                    bool hasSelection =
                        s_instance->m_editor->textCursor().hasSelection();
                    s_instance->m_replaceSelectionButton->setEnabled(
                        hasSelection);
                });
    }

    const QTextCursor cursor = s_instance->m_editor->textCursor();
    if (cursor.hasSelection()) {
        s_instance->m_searchText->setCurrentText(cursor.selectedText());
        s_instance->m_searchText->lineEdit()->selectAll();
    }
    s_instance->m_replaceSelectionButton->setEnabled(cursor.hasSelection());

    return s_instance;
}

static QString translateCharEscape(QStringView digits, int *advance) {
    Q_ASSERT(digits.size() > 0);
    Q_ASSERT(advance);

    *advance = 0;
    if (digits.at(0) == QLatin1Char('x')) {
        // We only support exactly 2 hex bytes with \x format
        if (digits.size() < 3)
            return QString();
        QByteArray number = digits.mid(1, 2).toLatin1();
        char *end;
        ulong ch = strtoul(number.constData(), &end, 16);
        if (*end != '\0')
            return QString();
        *advance = 2;
        return QString(QChar::fromLatin1(static_cast<char>(ch)));
    } else if (digits.at(0) == QLatin1Char('u')) {
        if (digits.size() < 5)
            return QString();
        QByteArray number = digits.mid(1, 4).toLatin1();
        char *end;
        ulong ch = strtoul(number.constData(), &end, 16);
        if (*end != '\0')
            return QString();
        *advance = 4;
        return QString(QChar(static_cast<ushort>(ch)));
    } else if (digits.at(0) == QLatin1Char('U')) {
        if (digits.size() < 9)
            return QString();
        QByteArray number = digits.mid(1, 8).toLatin1();
        char *end;
        ulong ch = strtoul(number.constData(), &end, 16);
        if (*end != '\0')
            return QString();

        *advance = 8;
        if (ch > 0xFFFFU) {
            const QChar utf16[2] = {
                QChar::highSurrogate(ch),
                QChar::lowSurrogate(ch),
            };
            return QString(utf16, 2);
        } else {
            return QString(QChar(static_cast<ushort>(ch)));
        }
    } else {
        // Octal no longer supported
        qFatal("Unsupported character escape prefix");
        return QString();
    }
}

QString SearchDialog::translateEscapes(const QString &text) {
    QString result;
    result.reserve(text.size());
    int start = 0;
    for (;;) {
        int pos = text.indexOf(QLatin1Char('\\'), start);
        if (pos < 0 || pos + 1 >= text.size())
            break;

        result.append(text.sliced(start, pos - start));
        QChar next = text.at(pos + 1);
        start = pos + 2;
        switch (next.unicode()) {
        case 'a':
            result.append(QLatin1Char('\a'));
            break;
        case 'b':
            result.append(QLatin1Char('\b'));
            break;
        case 'e':
            result.append(QLatin1Char('\x1b'));
            break;
        case 'f':
            result.append(QLatin1Char('\f'));
            break;
        case 'n':
            result.append(QLatin1Char('\n'));
            break;
        case 'r':
            result.append(QLatin1Char('\r'));
            break;
        case 't':
            result.append(QLatin1Char('\t'));
            break;
        case 'v':
            result.append(QLatin1Char('\v'));
            break;
        case '\\':
        case '?':
        case '\'':
        case '"':
            result.append(next);
            break;
        case 'x': // Hex byte
        case 'u': // Unicode character (16-bit)
        case 'U': // Unicode character (32-bit)
        {
            int advance;
            const QString chars =
                translateCharEscape(QStringView(text).mid(pos + 1), &advance);
            if (chars.isEmpty()) {
                // Translation failed
                result.append(QLatin1Char('\\'));
                result.append(next);
            } else {
                result.append(chars);
                start += advance;
            }
        } break;
        default:
            // Just keep unrecognized sequences untranslated
            result.append(QLatin1Char('\\'));
            result.append(next);
            break;
        }
    }

    result.append(text.sliced(start));
    return result;
}

QString SearchDialog::regexReplace(const QString &text,
                                   const QRegularExpressionMatch &regexMatch) {
    QString result;
    result.reserve(text.size());
    int start = 0;
    for (;;) {
        int pos = text.indexOf(QLatin1Char('\\'), start);
        if (pos < 0 || pos + 1 >= text.size())
            break;

        result.append(text.sliced(start, pos - start));
        QChar next = text.at(pos + 1);
        if (next.unicode() >= '0' && next.unicode() <= '9') {
            // We support up to 99 replacements...
            QByteArray number = QStringView(text).mid(pos + 1, 2).toLatin1();
            char *end;
            ulong ref = strtoul(number.constData(), &end, 10);
            result.append(regexMatch.captured(ref));
            start = pos + 1 + (end - number.constData());
        } else {
            result.append(QLatin1Char('\\'));
            result.append(next);
            start = pos + 2;
        }
    }

    result.append(text.sliced(start));
    return result;
}

void SearchDialog::syncSearchSettings(bool saveRecent) {
    const QString searchText = m_searchText->currentText();
    if (!searchText.isEmpty() && saveRecent) {
        if (m_searchText->count() == 0 ||
            m_searchText->itemText(0) != searchText) {
            m_searchText->insertItem(0, searchText);
        }
    }
    m_searchParams.searchText =
        m_escapes->isChecked() ? translateEscapes(searchText) : searchText;

    const QString replaceText = m_replaceText->currentText();
    if (!replaceText.isEmpty() && saveRecent) {
        if (m_replaceText->count() == 0 ||
            m_replaceText->itemText(0) != replaceText) {
            m_replaceText->insertItem(0, replaceText);
        }
    }

    m_searchParams.caseSensitive = m_caseSensitive->isChecked();
    m_searchParams.wholeWord = m_wholeWord->isChecked();
    m_searchParams.regex = m_regex->isChecked();
}

QTextCursor SearchDialog::searchNext(bool reverse) {
    Q_ASSERT(m_editor);

    if (m_searchParams.searchText.isEmpty())
        return QTextCursor();

    auto searchCursor = m_editor->textSearch(
        m_editor->textCursor(), m_searchParams, false, reverse, &m_regexMatch);
    if (searchCursor.isNull() && m_wrapSearch->isChecked()) {
        QTextCursor wrapCursor = m_editor->textCursor();
        wrapCursor.movePosition(reverse ? QTextCursor::End
                                        : QTextCursor::Start);
        searchCursor = m_editor->textSearch(wrapCursor, m_searchParams, true,
                                            reverse, &m_regexMatch);
    }

    if (searchCursor.isNull())
        WingMessageBox::information(this, QString(),
                                    tr("The specified text was not found"));
    else
        m_editor->setTextCursor(searchCursor);
    return searchCursor;
}

void SearchDialog::searchForward() {
    syncSearchSettings(true);
    m_replaceCursor = searchNext(false);
}

void SearchDialog::searchBackward() {
    syncSearchSettings(true);
    m_replaceCursor = searchNext(true);
}

void SearchDialog::replaceCurrent() {
    syncSearchSettings(true);
    Q_ASSERT(m_editor);

    const QString searchText = m_searchText->currentText();
    if (searchText.isEmpty())
        return;

    if (m_replaceCursor.isNull() || m_replaceCursor != m_editor->textCursor()) {
        m_replaceCursor = searchNext(false);
        return;
    }

    QString replaceText = m_replaceText->currentText();
    if (m_escapes->isChecked())
        replaceText = translateEscapes(replaceText);

    m_replaceCursor.beginEditBlock();
    m_replaceCursor.removeSelectedText();
    if (m_regex->isChecked())
        m_replaceCursor.insertText(regexReplace(replaceText, m_regexMatch));
    else
        m_replaceCursor.insertText(replaceText);
    m_replaceCursor.endEditBlock();

    m_replaceCursor = searchNext(false);
}

void SearchDialog::performReplaceAll(ReplaceAllMode mode) {
    syncSearchSettings(true);
    Q_ASSERT(m_editor);

    const QString searchText = m_searchText->currentText();
    if (searchText.isEmpty())
        return;

    auto searchCursor = m_editor->textCursor();
    if (mode == InSelection)
        searchCursor.setPosition(m_editor->textCursor().selectionStart());
    else
        searchCursor.movePosition(QTextCursor::Start);
    searchCursor = m_editor->textSearch(searchCursor, m_searchParams, true,
                                        false, &m_regexMatch);
    if (searchCursor.isNull()) {
        QMessageBox::information(this, QString(),
                                 tr("The specified text was not found"));
        return;
    } else if (mode == InSelection &&
               searchCursor.selectionEnd() >
                   m_editor->textCursor().selectionEnd()) {
        WingMessageBox::information(
            this, QString(),
            tr("The specified text was not found in the selection"));
        return;
    }

    QString replaceText = m_replaceText->currentText();
    if (m_escapes->isChecked())
        replaceText = translateEscapes(replaceText);

    searchCursor.beginEditBlock();
    auto replaceCursor = searchCursor;
    int replacements = 0;
    while (!replaceCursor.isNull()) {
        if (mode == InSelection && replaceCursor.selectionEnd() >
                                       m_editor->textCursor().selectionEnd())
            break;

        replaceCursor.removeSelectedText();
        if (m_regex->isChecked())
            replaceCursor.insertText(regexReplace(replaceText, m_regexMatch));
        else
            replaceCursor.insertText(replaceText);
        replaceCursor = m_editor->textSearch(replaceCursor, m_searchParams,
                                             false, false, &m_regexMatch);
        ++replacements;
    }
    searchCursor.endEditBlock();

    WingMessageBox::information(
        this, QString(),
        tr("Successfully replaced %1 matches").arg(replacements));
}
