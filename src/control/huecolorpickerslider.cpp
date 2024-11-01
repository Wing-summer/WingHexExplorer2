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

#include "huecolorpickerslider.h"
#include "dialog/colorpickerdialog.h"

#include <QAction>
#include <QMessageBox>
#include <QMouseEvent>

HueColorPickerSlider::HueColorPickerSlider(Qt::Orientation orientation,
                                           QWidget *parent)
    : QColorPickerSlider(orientation, parent) {
    this->setRange(0, 359);
    this->setGradientStops({{0.0 / 360.0, Qt::red},
                            {60.0 / 360.0, Qt::yellow},
                            {120.0 / 360.0, Qt::green},
                            {180.0 / 360.0, Qt::cyan},
                            {240.0 / 360.0, Qt::blue},
                            {300.0 / 360.0, Qt::magenta},
                            {359.0 / 360.0, Qt::red}});

    _color.setHsv(180, 255, 255);
    connect(this, &HueColorPickerSlider::valueChanged, this, [this](int v) {
        _color.setHsv(v, _color.hsvSaturation(), _color.value());
        this->setToolTip(_color.name());
        emit colorChanged(_color);
    });

    this->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto a = new QAction(tr("ChooseColor"), this);
    connect(a, &QAction::triggered, this, [=] {
        ColorPickerDialog d;
        d.setColor(_color);
        if (d.exec()) {
            setColor(d.color());
        }
    });
    this->addAction(a);

    this->setValue(180);
}

QColor HueColorPickerSlider::color() const { return _color; }

void HueColorPickerSlider::setColor(const QColor &newColor) {
    if (_color != newColor) {
        _color = newColor;

        auto colorS = newColor.hsvSaturation();
        auto colorV = newColor.value();
        this->setGradientStops({{0.0 / 360.0, redColor(colorS, colorV)},
                                {60.0 / 360.0, yellowColor(colorS, colorV)},
                                {120.0 / 360.0, greenColor(colorS, colorV)},
                                {180.0 / 360.0, cyanColor(colorS, colorV)},
                                {240.0 / 360.0, blueColor(colorS, colorV)},
                                {300.0 / 360.0, magentaColor(colorS, colorV)},
                                {359.0 / 360.0, redColor(colorS, colorV)}});

        this->setValue(_color.hsvHue());
    }
}

QColor HueColorPickerSlider::redColor(int s, int v) {
    return color(Qt::red, s, v);
}

QColor HueColorPickerSlider::yellowColor(int s, int v) {
    return color(Qt::yellow, s, v);
}

QColor HueColorPickerSlider::greenColor(int s, int v) {
    return color(Qt::green, s, v);
}

QColor HueColorPickerSlider::cyanColor(int s, int v) {
    return color(Qt::cyan, s, v);
}

QColor HueColorPickerSlider::blueColor(int s, int v) {
    return color(Qt::blue, s, v);
}

QColor HueColorPickerSlider::magentaColor(int s, int v) {
    return color(Qt::magenta, s, v);
}

QColor HueColorPickerSlider::color(Qt::GlobalColor color, int s, int v) {
    QColor gcolor(color);
    gcolor.setHsv(gcolor.hsvHue(), s, v);
    return gcolor;
}
