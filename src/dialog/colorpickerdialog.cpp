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

#include "colorpickerdialog.h"
#include "ui_colorpickerdialog.h"

#include <QPushButton>

#include "class/settings/thememanager.h"
#include "utilities.h"

ColorPickerDialog::ColorPickerDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::ColorPickerDialog) {
    ui->setupUi(this);
    setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::ColorPalette));

    ui->hueSlider->setGradientStops({{0.0 / 360.0, Qt::red},
                                     {60.0 / 360.0, Qt::yellow},
                                     {120.0 / 360.0, Qt::green},
                                     {180.0 / 360.0, Qt::cyan},
                                     {240.0 / 360.0, Qt::blue},
                                     {300.0 / 360.0, Qt::magenta},
                                     {359.0 / 360.0, Qt::red}});

    m_oldColor.setHsv(180, 255, 255);
    updateColor(m_oldColor);
    m_color = m_oldColor;

    m_dialog = new FramelessDialogBase(parent);
    m_dialog->buildUpContent(this);

    Utilities::moveToCenter(m_dialog);
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
    m_color.setHsv(ui->hueSlider->value(), ui->saturationSlider->value(),
                   ui->brightnessSlider->value());
    updateColor(m_color);
}

void ColorPickerDialog::updateHueSlider() {
    ui->saturationSlider->setGradientStops(
        {{0.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 0, 255)},
         {255.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 255)}});
    ui->brightnessSlider->setGradientStops(
        {{0.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 0)},
         {255.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 255)}});
}

void ColorPickerDialog::updateColor(QColor color) {
    ui->hueSlider->setValue(color.hsvHue());
    ui->saturationSlider->setGradientStops(
        {{0.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 0, 255)},
         {255.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 255)}});
    ui->brightnessSlider->setGradientStops(
        {{0.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 0)},
         {255.0 / 255.0, QColor::fromHsv(ui->hueSlider->value(), 255, 255)}});
    ui->saturationSlider->setValue(color.hsvSaturation());
    ui->brightnessSlider->setValue(color.value());
    ui->leColor->setText(color.name());
    ui->wcolor->setStyleSheet(QStringLiteral("background-color: ") +
                              color.name());
    Q_EMIT sigOnColorChanged();
}

QColor ColorPickerDialog::color() const { return m_color; }

void ColorPickerDialog::setColor(const QColor &newColor) {
    if (m_color == newColor) {
        return;
    }
    m_oldColor = newColor;
    updateColor(newColor);
}

int ColorPickerDialog::exec() { return m_dialog->exec(); }

void ColorPickerDialog::on_leColor_editingFinished() {
    QColor color(ui->leColor->text());
    if (color.isValid()) {
        setColor(color);
    }
}

void ColorPickerDialog::on_buttonBox_clicked(QAbstractButton *button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        m_dialog->done(1);
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
        m_dialog->done(0);
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Reset)) {
        m_color = m_oldColor;
        updateColor(m_oldColor);
    } else {
        Q_ASSERT(false);
    }
}
