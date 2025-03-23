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

#ifndef QTEXTPAD_SEARCHDIALOG_H
#define QTEXTPAD_SEARCHDIALOG_H

#include <QList>
#include <QRegularExpressionMatch>
#include <QTextCursor>

#include "codeeditcontrolwidget.h"
#include "control/codeedit.h"

class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QTextCursor;

class SearchReplaceWidget : public CodeEditControlWidget {
    Q_OBJECT

public:
    explicit SearchReplaceWidget(CodeEdit *editor);

public:
    void setSearchText(const QString &text);
    void setReplaceMode(bool b);
    void activate();

    virtual void cancel() override;

private:
    void updateSettings();

    QTextCursor searchNext(bool reverse);

    QString regexReplace(const QString &text,
                         const QRegularExpressionMatch &regexMatch);

    enum ReplaceAllMode { WholeDocument, InSelection };
    void replaceCurrent();
    void performReplaceAll(ReplaceAllMode mode);

private slots:
    void searchForward();
    void searchBackward();

private:
    QLineEdit *m_searchText;
    QLineEdit *m_replaceText;

    QAction *m_caseSensitive;
    QAction *m_wholeWord;
    QAction *m_regex;
    QAction *m_escapes;
    QAction *m_wrapSearch;
    QAction *m_replaceSel;

    CodeEdit *m_editor;
    CodeEdit::SearchParams m_searchParams;

    QWidget *m_replaceWidgets;
    QTextCursor m_replaceCursor;
    QRegularExpressionMatch m_regexMatch;
};

#endif // QTEXTPAD_SEARCHDIALOG_H
