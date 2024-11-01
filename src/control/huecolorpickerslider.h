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

#ifndef HUECOLORPICKERSLIDER_H
#define HUECOLORPICKERSLIDER_H

#include <QWidget>

#include "qcolorpicker_slider.hpp"

class HueColorPickerSlider : public QColorPickerSlider {
    Q_OBJECT
public:
    explicit HueColorPickerSlider(Qt::Orientation orientation,
                                  QWidget *parent = nullptr);

    QColor color() const;
    void setColor(const QColor &newColor);

signals:
    void colorChanged(QColor color);

private:
    QColor redColor(int s, int v);
    QColor yellowColor(int s, int v);
    QColor greenColor(int s, int v);
    QColor cyanColor(int s, int v);
    QColor blueColor(int s, int v);
    QColor magentaColor(int s, int v);

    QColor color(Qt::GlobalColor color, int s, int v);

private:
    QColor _color;
};

#endif // HUECOLORPICKERSLIDER_H
