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

#ifndef QCOLORPICKERSLIDER_H
#define QCOLORPICKERSLIDER_H

#include <QGradientStops>
#include <QSlider>

class QColorPickerSlider : public QSlider {
    Q_OBJECT

public:
    explicit QColorPickerSlider(Qt::Orientation orientation,
                                QWidget *parent = nullptr);
    explicit QColorPickerSlider(QWidget *parent = nullptr);

    ~QColorPickerSlider();

    void setGradientStops(QGradientStops gradientStops);
    void setRenderCheckerboard(bool renderCheckerboard);

    void paintEvent(QPaintEvent *event) override;

private:
    QGradientStops _gradientStops;
    bool _renderCheckerboard = false;
    QPixmap _checkerboardPixmap;
};

#endif
