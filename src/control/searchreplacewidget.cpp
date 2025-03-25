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

#include "searchreplacewidget.h"

#include "class/skinmanager.h"
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

QString translateEscapes(const QString &text) {
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

SearchReplaceWidget::SearchReplaceWidget(CodeEdit *editor)
    : CodeEditControlWidget(editor), m_editor(editor) {
    auto &skin = SkinManager::instance();

    auto closeBtn = new QToolButton(this);
    closeBtn->setIcon(skin.themeIcon(QStringLiteral("dialog_close")));
    closeBtn->setToolTip(tr("Close"));
    closeBtn->setIconSize(QSize(12, 12));
    closeBtn->setStyleSheet(QStringLiteral("border:none;"));
    connect(closeBtn, &QToolButton::clicked, this,
            &SearchReplaceWidget::cancel);

    auto tbMenu = new QToolButton(this);
    tbMenu->setAutoRaise(true);
    tbMenu->setIcon(skin.themeIcon(QStringLiteral("menu")));
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
    m_wrapSearch->setChecked(true);
    m_replaceSel = settingsMenu->addAction(tr("Replace in selection"));
    m_replaceSel->setCheckable(true);
    tbMenu->setMenu(settingsMenu);
    tbMenu->setPopupMode(QToolButton::InstantPopup);

    connect(m_caseSensitive, &QAction::triggered, this,
            &SearchReplaceWidget::updateSettings);
    connect(m_wholeWord, &QAction::triggered, this,
            &SearchReplaceWidget::updateSettings);
    connect(m_regex, &QAction::triggered, this,
            &SearchReplaceWidget::updateSettings);
    connect(m_escapes, &QAction::triggered, this,
            &SearchReplaceWidget::updateSettings);
    connect(m_wrapSearch, &QAction::triggered, this,
            &SearchReplaceWidget::updateSettings);

    m_searchText = new QLineEdit(this);
    m_searchText->setClearButtonEnabled(true);
    setFocusProxy(m_searchText);

    auto tbNext = new QToolButton(this);
    tbNext->setAutoRaise(true);
    tbNext->setIcon(skin.themeIcon(QStringLiteral("right_arrow")));
    tbNext->setToolTip(tr("Find Next"));

    auto tbPrev = new QToolButton(this);
    tbPrev->setAutoRaise(true);
    tbPrev->setIcon(skin.themeIcon(QStringLiteral("left_arrow")));
    tbPrev->setToolTip(tr("Find Previous"));

    auto mlayout = new QVBoxLayout(this);
    mlayout->setContentsMargins(5, 5, 5, 5);
    mlayout->setSpacing(3);

    m_replaceText = new QLineEdit(this);
    m_replaceText->setClearButtonEnabled(true);

    auto tbReplaceCur = new QToolButton(this);
    tbReplaceCur->setAutoRaise(true);
    tbReplaceCur->setIcon(skin.themeIcon(QStringLiteral("dialog_apply")));
    tbReplaceCur->setToolTip(tr("Replace Current"));

    auto tbReplaceAll = new QToolButton(this);
    tbReplaceAll->setAutoRaise(true);
    tbReplaceAll->setIcon(skin.themeIcon(QStringLiteral("dialog_yes_to_all")));
    tbReplaceAll->setToolTip(tr("Replace All"));

    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->addWidget(closeBtn);
    layout->addWidget(tbMenu);
    layout->addWidget(m_searchText);
    layout->addWidget(tbPrev);
    layout->addWidget(tbNext);
    mlayout->addLayout(layout);

    m_replaceWidgets = new QWidget(this);
    layout = new QHBoxLayout(m_replaceWidgets);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    layout->addWidget(new QLabel(tr("Replace:"), this));
    layout->addWidget(m_replaceText);
    layout->addWidget(tbReplaceCur);
    layout->addWidget(tbReplaceAll);

    mlayout->addWidget(m_replaceWidgets);
    setLayout(mlayout);

    connect(m_searchText, &QLineEdit::textChanged, this,
            [this](const QString &text) {
                m_searchParams.searchText =
                    m_escapes->isChecked() ? translateEscapes(text) : text;
                m_editor->setLiveSearch(m_searchParams);
            });
    connect(m_searchText, &QLineEdit::returnPressed, this,
            &SearchReplaceWidget::searchForward);
    connect(tbNext, &QToolButton::clicked, this,
            &SearchReplaceWidget::searchForward);
    connect(tbPrev, &QToolButton::clicked, this,
            &SearchReplaceWidget::searchBackward);
    connect(tbReplaceCur, &QToolButton::clicked, this,
            &SearchReplaceWidget::replaceCurrent);
    connect(tbReplaceAll, &QToolButton::clicked, this, [this]() {
        performReplaceAll(m_replaceSel->isChecked()
                              ? ReplaceAllMode::InSelection
                              : ReplaceAllMode::WholeDocument);
    });
}

void SearchReplaceWidget::setSearchText(const QString &text) {
    m_searchText->setText(text);
}

void SearchReplaceWidget::setReplaceMode(bool b) {
    m_replaceWidgets->setVisible(b);
    m_replaceWidgets->setEnabled(b);
    m_replaceSel->setVisible(b);
    m_replaceSel->setEnabled(b);
}

void SearchReplaceWidget::activate() {
    m_searchParams.caseSensitive = m_caseSensitive->isChecked();
    m_searchParams.wholeWord = m_wholeWord->isChecked();
    m_searchParams.regex = m_regex->isChecked();

    setFocus(Qt::OtherFocusReason);
    m_searchText->selectAll();
    m_editor->setLiveSearch(m_searchParams);
}

void SearchReplaceWidget::cancel() {
    m_editor->clearLiveSearch();
    CodeEditControlWidget::cancel();
}

void SearchReplaceWidget::searchForward() {
    updateSettings();
    m_replaceCursor = searchNext(false);
}

void SearchReplaceWidget::searchBackward() {
    updateSettings();
    m_replaceCursor = searchNext(true);
}

void SearchReplaceWidget::replaceCurrent() {
    updateSettings();
    Q_ASSERT(m_editor);

    const QString searchText = m_searchText->text();
    if (searchText.isEmpty())
        return;

    if (m_replaceCursor.isNull() || m_replaceCursor != m_editor->textCursor()) {
        m_replaceCursor = searchNext(false);
        return;
    }

    QString replaceText = m_replaceText->text();
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

void SearchReplaceWidget::performReplaceAll(ReplaceAllMode mode) {
    updateSettings();
    Q_ASSERT(m_editor);

    const QString searchText = m_searchText->text();
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

    QString replaceText = m_replaceText->text();
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

QTextCursor SearchReplaceWidget::searchNext(bool reverse) {
    if (!isVisible()) {
        setVisible(true);
        setEnabled(true);
        activate();
    }
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

void SearchReplaceWidget::updateSettings() {
    m_searchParams.caseSensitive = m_caseSensitive->isChecked();
    m_searchParams.wholeWord = m_wholeWord->isChecked();
    m_searchParams.regex = m_regex->isChecked();

    m_editor->setLiveSearch(m_searchParams);
}

QString
SearchReplaceWidget::regexReplace(const QString &text,
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
