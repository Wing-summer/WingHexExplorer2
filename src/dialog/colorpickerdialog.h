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

    void updateColor(QColor color);

private:
    Ui::ColorPickerDialog *ui;

    FramelessDialogBase *_dialog = nullptr;

    QColor _color;
    QColor _oldColor;
};

#endif // COLORPICKERDIALOG_H
