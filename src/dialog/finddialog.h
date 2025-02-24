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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "control/qhextextedit.h"
#include "control/qtlonglongspinbox.h"
#include "framelessdialogbase.h"

#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QRadioButton>
#include <QTextEdit>

enum class SearchDirection { None, Region, Foreword, Backword, Selection };

class FindDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    struct Result {
        SearchDirection dir = SearchDirection::None;
        qsizetype start = 0;
        qsizetype stop = 0;

        // for searching info
        bool isStringFind;
        QString encoding;
        QString str;
    };

    struct FindInfo {
        bool isBigFile;
        bool isStringFind;
        qlonglong start;
        qlonglong stop;
        bool isSel;

        // for searching info
        QByteArray buffer;
        QString encoding;
        QString str;
    };

public:
    explicit FindDialog(const FindInfo &info, QWidget *parent = nullptr);
    Result getResult() const;

private:
    void on_accept();
    void on_reject();

private:
    QHexTextEdit *m_lineeditor;
    QComboBox *m_findMode;
    QTextEdit *m_preview;

    QtLongLongSpinBox *m_regionStart;
    QtLongLongSpinBox *m_regionStop;

    Result _result;
};

#endif // FINDDIALOG_H
