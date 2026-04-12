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

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionSlider>

#include "qcolorpicker_slider.hpp"

QColorPickerSlider::QColorPickerSlider(Qt::Orientation orientation,
                                       QWidget *parent)
    : QSlider(orientation, parent) {}

QColorPickerSlider::QColorPickerSlider(QWidget *parent)
    : QColorPickerSlider(Qt::Vertical, parent) {}

QColorPickerSlider::~QColorPickerSlider() = default;

void QColorPickerSlider::setGradientStops(QGradientStops gradientStops) {
    _gradientStops = gradientStops;
    update();
}

void QColorPickerSlider::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF groove = rect().adjusted(1, 1, -1, -1);
    qreal r = orientation() == Qt::Horizontal ? groove.height() / 2.0
                                              : groove.width() / 2.0;

    QLinearGradient gradient;
    if (orientation() == Qt::Horizontal) {
        gradient = QLinearGradient(groove.left() + r, 0, groove.right() - r, 0);
    } else {
        gradient = QLinearGradient(0, groove.top() + r, 0, groove.bottom() - r);
    }
    gradient.setStops(_gradientStops);

    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawRoundedRect(groove, r, r);

    QColor leftColor =
        _gradientStops.isEmpty() ? QColor() : _gradientStops.first().second;
    QColor rightColor =
        _gradientStops.isEmpty() ? QColor() : _gradientStops.last().second;
    float leftPos = 0.0f, rightPos = 1.0f;

    const float valueFloat =
        (maximum() == minimum())
            ? 0.0f
            : float(value() - minimum()) / float(maximum() - minimum());

    for (int i = 0; i < _gradientStops.size() - 1; ++i) {
        if (_gradientStops[i].first <= valueFloat &&
            _gradientStops[i + 1].first >= valueFloat) {
            leftColor = _gradientStops[i].second;
            rightColor = _gradientStops[i + 1].second;
            leftPos = _gradientStops[i].first;
            rightPos = _gradientStops[i + 1].first;
            break;
        }
    }

    QColor fill = leftColor;
    if (rightPos > leftPos) {
        const float t = (valueFloat - leftPos) / (rightPos - leftPos);
        fill = QColor::fromRgbF(
            leftColor.redF() + (rightColor.redF() - leftColor.redF()) * t,
            leftColor.greenF() + (rightColor.greenF() - leftColor.greenF()) * t,
            leftColor.blueF() + (rightColor.blueF() - leftColor.blueF()) * t,
            leftColor.alphaF() +
                (rightColor.alphaF() - leftColor.alphaF()) * t);
    }

    const QPointF c = handleCenter();
    const qreal outerRadius = qMin(width(), height()) / 2.0 - 1.0;
    const qreal borderWidth = 2.0;
    const qreal innerRadius = outerRadius - borderWidth;

    QColor borderColor = palette().color(QPalette::Text);
    painter.setPen(Qt::NoPen);
    painter.setBrush(borderColor);
    painter.drawEllipse(c, outerRadius, outerRadius);

    painter.setBrush(fill);
    painter.drawEllipse(c, innerRadius, innerRadius);
}

QRectF QColorPickerSlider::trackRect() const {
    const qreal r = qMin(width(), height()) / 2.0;

    if (orientation() == Qt::Horizontal) {
        return QRectF(r, 0.0, width() - 2.0 * r, height());
    } else {
        return QRectF(0.0, r, width(), height() - 2.0 * r);
    }
}

QPointF QColorPickerSlider::handleCenter() const {
    QRectF groove = rect().adjusted(1, 1, -1, -1);

    const qreal r = orientation() == Qt::Horizontal ? groove.height() / 2.0
                                                    : groove.width() / 2.0;

    const qreal t = (maximum() == minimum()) ? 0.0
                                             : qreal(value() - minimum()) /
                                                   qreal(maximum() - minimum());

    if (orientation() == Qt::Horizontal) {
        const qreal x = groove.left() + r + t * (groove.width() - 2.0 * r);
        return QPointF(x, groove.center().y());
    } else {
        const qreal y = groove.top() + r + t * (groove.height() - 2.0 * r);
        return QPointF(groove.center().x(), y);
    }
}

void QColorPickerSlider::setValueFromPosition(const QPoint &pos) {
    QRectF groove = rect().adjusted(1, 1, -1, -1);

    const qreal r = orientation() == Qt::Horizontal ? groove.height() / 2.0
                                                    : groove.width() / 2.0;

    qreal t = 0.0;

    if (orientation() == Qt::Horizontal) {
        const qreal span = groove.width() - 2.0 * r;
        const qreal x =
            qBound(groove.left() + r, qreal(pos.x()), groove.right() - r);
        t = (x - (groove.left() + r)) / span;
    } else {
        const qreal span = groove.height() - 2.0 * r;
        const qreal y =
            qBound(groove.top() + r, qreal(pos.y()), groove.bottom() - r);
        t = (y - (groove.top() + r)) / span;
    }

    const int v = minimum() + int(std::round(t * (maximum() - minimum())));
    setValue(v);
}

bool QColorPickerSlider::hitHandle(const QPoint &p) const {
    const QPointF c = handleCenter();
    const qreal r = qMin(width(), height()) / 2.0 - 1.0;
    return QLineF(c, p).length() <= r;
}

void QColorPickerSlider::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && hitHandle(event->pos())) {
        setSliderDown(true);
        event->accept();
        return;
    }

    QSlider::mousePressEvent(event);
}

void QColorPickerSlider::mouseMoveEvent(QMouseEvent *event) {
    if (isSliderDown() && (event->buttons() & Qt::LeftButton)) {
        setValueFromPosition(event->pos());
        event->accept();
        return;
    }

    QSlider::mouseMoveEvent(event);
}

void QColorPickerSlider::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        setSliderDown(false);
        setValueFromPosition(event->pos());
        event->accept();
        return;
    }

    QSlider::mouseReleaseEvent(event);
}
