#include "colorpickerdialog.h"
#include "ui_colorpickerdialog.h"

#include "src/utilities.h"

#include <QPushButton>

ColorPickerDialog::ColorPickerDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::ColorPickerDialog) {
    ui->setupUi(this);

    ui->hueSlider->setGradientStops({{0.0 / 360.0, Qt::red},
                                     {60.0 / 360.0, Qt::yellow},
                                     {120.0 / 360.0, Qt::green},
                                     {180.0 / 360.0, Qt::cyan},
                                     {240.0 / 360.0, Qt::blue},
                                     {300.0 / 360.0, Qt::magenta},
                                     {359.0 / 360.0, Qt::red}});
    ui->saturationSlider->setGradientStops(
        {{0.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 0, 255)},
         {255.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 255)}});
    ui->brightnessSlider->setGradientStops(
        {{0.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 0)},
         {255.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 255)}});

    _oldColor.setHsv(180, 255, 255);
    updateColor(_oldColor);

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());

    Utilities::moveToCenter(_dialog);
}

ColorPickerDialog::~ColorPickerDialog() { delete ui; }

void ColorPickerDialog::on_hueSlider_valueChanged(int value) {
    Q_UNUSED(value);
    updateColor();
}

void ColorPickerDialog::on_saturationSlider_valueChanged(int value) {
    Q_UNUSED(value);
    updateColor();
}

void ColorPickerDialog::on_brightnessSlider_valueChanged(int value) {
    Q_UNUSED(value);
    updateColor();
}

void ColorPickerDialog::updateColor() {
    _color.setHsv(ui->hueSlider->value(), ui->saturationSlider->value(),
                  ui->brightnessSlider->value());
    updateColor(_color);
}

void ColorPickerDialog::updateColor(QColor color) {
    ui->hueSlider->setValue(color.hsvHue());
    ui->saturationSlider->setValue(color.hsvSaturation());
    ui->brightnessSlider->setValue(color.value());
    ui->leColor->setText(color.name());
    ui->wcolor->setStyleSheet(QStringLiteral("background-color: ") +
                              color.name());
    emit colorChanged();
}

QColor ColorPickerDialog::color() const { return _color; }

void ColorPickerDialog::setColor(const QColor &newColor) {
    if (_color == newColor)
        return;
    _oldColor = newColor;
    updateColor(newColor);
}

int ColorPickerDialog::exec() { return 0; }

void ColorPickerDialog::on_leColor_editingFinished() {
    QColor color(ui->leColor->text());
    if (color.isValid()) {
        setColor(color);
    }
}

void ColorPickerDialog::on_buttonBox_clicked(QAbstractButton *button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        // _dialog->done(1);
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
        //_dialog->done(0);
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Reset)) {
        _color = _oldColor;
        updateColor(_oldColor);
    } else {
        Q_ASSERT(false);
    }
}
