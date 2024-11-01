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

#include "QHexView/qhexview.h"
#include "framelessdialogbase.h"

#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QRadioButton>
#include <QSpinBox>

enum class SearchDirection { None, Region, Foreword, Backword, Selection };

class FindDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    struct Result {
        SearchDirection dir = SearchDirection::None;
        qsizetype start = 0;
        qsizetype stop = 0;
    };

public:
    FindDialog(bool isBigFile, int start, int stop, bool sel = true,
               QWidget *parent = nullptr);
    QByteArray getResult(Result &result);

private:
    void on_accept();
    void on_reject();

private:
    QHexView *m_hexeditor;
    QLineEdit *m_lineeditor;
    QRadioButton *m_string;
    QRadioButton *m_hex;
    QComboBox *m_encodings;
    QByteArray _findarr;

    QSpinBox *m_regionStart;
    QSpinBox *m_regionStop;

    Result _result;
};

#endif // FINDDIALOG_H
