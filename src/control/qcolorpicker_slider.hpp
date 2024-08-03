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
