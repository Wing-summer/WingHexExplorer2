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

#ifndef COLORPICKERDIALOG_H
#define COLORPICKERDIALOG_H

#include "framelessdialogbase.h"

#include <QAbstractButton>

namespace Ui {
class ColorPickerDialog;
}

class ColorPickerDialog : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)

public:
    explicit ColorPickerDialog(QWidget *parent = nullptr);
    virtual ~ColorPickerDialog();

    QColor color() const;
    void setColor(const QColor &newColor);

public slots:
    int exec();

signals:
    void colorChanged();

private slots:
    void on_hueSlider_valueChanged(int value);

    void on_saturationSlider_valueChanged(int value);

    void on_brightnessSlider_valueChanged(int value);

    void on_leColor_editingFinished();

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    void updateColor();
    void updateHueSlider();
    void updateColor(QColor color);

private:
    Ui::ColorPickerDialog *ui;

    FramelessDialogBase *_dialog = nullptr;

    QColor _color;
    QColor _oldColor;
};

#endif // COLORPICKERDIALOG_H
