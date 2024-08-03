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
