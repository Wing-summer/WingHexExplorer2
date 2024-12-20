/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
** =============================================================================
*/

#ifndef CTLTESTFORM_H
#define CTLTESTFORM_H

#include <QTextBrowser>
#include <QWidget>

#include "iwingplugin.h"

namespace Ui {
class CtlTestForm;
}

class CtlTestForm : public QWidget {
    Q_OBJECT

public:
    explicit CtlTestForm(WingHex::IWingPlugin *plg, QTextBrowser *br,
                         QWidget *parent = nullptr);
    ~CtlTestForm();

private slots:
    void on_btnInt8_clicked();

    void on_btnInt16_clicked();

    void on_btnInt32_clicked();

    void on_btnInt64_clicked();

    void on_btnFloat_clicked();

    void on_btnDouble_clicked();

    void on_btnString_clicked();

    void on_btnByteArray_clicked();

private:
    template <typename T>
    bool writeContent(qsizetype offset, const T &value) {
        Q_ASSERT(_plg);
        if constexpr (std::is_integral_v<T>) {
            if constexpr (sizeof(T) == sizeof(qint8)) {
                return emit _plg->controller.writeInt8(offset, value);
            } else if constexpr (sizeof(T) == sizeof(qint16)) {
                return emit _plg->controller.writeInt16(offset, value);
            } else if constexpr (sizeof(T) == sizeof(qint32)) {
                return emit _plg->controller.writeInt32(offset, value);
            } else if constexpr (sizeof(T) == sizeof(qint64)) {
                return emit _plg->controller.writeInt64(offset, value);
            } else {
                static_assert(false, "unsupported writeContent");
                return false;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if constexpr (std::is_same_v<T, float>) {
                return emit _plg->controller.writeFloat(offset, value);
            } else {
                return emit _plg->controller.writeDouble(offset, value);
            }
        } else if constexpr (std::is_same_v<T, QString>) {
            return emit _plg->controller.writeString(offset, value);
        } else if constexpr (std::is_same_v<T, QByteArray>) {
            return emit _plg->controller.writeBytes(offset, value);
        } else {
            static_assert(false, "unsupported writeContent");
            return false;
        }
    }

    template <typename T>
    bool insertContent(qsizetype offset, const T &value) {
        Q_ASSERT(_plg);
        if constexpr (std::is_integral_v<T>) {
            if constexpr (sizeof(T) == sizeof(qint8)) {
                return emit _plg->controller.insertInt8(offset, value);
            } else if constexpr (sizeof(T) == sizeof(qint16)) {
                return emit _plg->controller.insertInt16(offset, value);
            } else if constexpr (sizeof(T) == sizeof(qint32)) {
                return emit _plg->controller.insertInt32(offset, value);
            } else if constexpr (sizeof(T) == sizeof(qint64)) {
                return emit _plg->controller.insertInt64(offset, value);
            } else {
                static_assert(false, "unsupported insertContent");
                return false;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if constexpr (std::is_same_v<T, float>) {
                return emit _plg->controller.insertFloat(offset, value);
            } else {
                return emit _plg->controller.insertDouble(offset, value);
            }
        } else if constexpr (std::is_same_v<T, QString>) {
            return emit _plg->controller.insertString(offset, value);
        } else if constexpr (std::is_same_v<T, QByteArray>) {
            return emit _plg->controller.insertBytes(offset, value);
        } else {
            static_assert(false, "unsupported insertContent");
            return false;
        }
    }

    template <typename T>
    bool appendContent(const T &value) {
        Q_ASSERT(_plg);
        if constexpr (std::is_integral_v<T>) {
            if constexpr (sizeof(T) == sizeof(qint8)) {
                return emit _plg->controller.appendInt8(value);
            } else if constexpr (sizeof(T) == sizeof(qint16)) {
                return emit _plg->controller.appendInt16(value);
            } else if constexpr (sizeof(T) == sizeof(qint32)) {
                return emit _plg->controller.appendInt32(value);
            } else if constexpr (sizeof(T) == sizeof(qint64)) {
                return emit _plg->controller.appendInt64(value);
            } else {
                static_assert(false, "unsupported appendContent");
                return false;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if constexpr (std::is_same_v<T, float>) {
                return emit _plg->controller.appendFloat(value);
            } else {
                return emit _plg->controller.appendDouble(value);
            }
        } else if constexpr (std::is_same_v<T, QString>) {
            return emit _plg->controller.appendString(value);
        } else if constexpr (std::is_same_v<T, QByteArray>) {
            return emit _plg->controller.appendBytes(value);
        } else {
            static_assert(false, "unsupported appendContent");
            return false;
        }
    }

private:
    Ui::CtlTestForm *ui;

    WingHex::IWingPlugin *_plg;
    QTextBrowser *_br;
};

#endif // CTLTESTFORM_H
