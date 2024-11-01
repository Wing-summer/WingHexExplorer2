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

#ifndef METADIALOG_H
#define METADIALOG_H

#include "framelessdialogbase.h"

#include <QCheckBox>
#include <QColor>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPalette>

#include "control/huecolorpickerslider.h"

class MetaDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit MetaDialog(QWidget *parent);
    QColor foreGroundColor();
    QColor backGroundColor();
    QString comment();

    void setForeGroundColor(QColor color);
    void setBackGroundColor(QColor color);
    void setComment(QString comment);

private:
    void on_accept();
    void on_reject();

private:
    QCheckBox *cforeground, *cbackground, *ccomment;
    QLineEdit *m_comment;
    HueColorPickerSlider *iforeground, *ibackground;

    QColor _foreground;
    QColor _background;
    QString _comment;
};

#endif // METADIALOG_H
