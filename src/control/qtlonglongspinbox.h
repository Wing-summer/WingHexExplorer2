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

#ifndef QTLONGLONGSPINBOX_H
#define QTLONGLONGSPINBOX_H

#include <QAbstractSpinBox>
#include <QtGlobal>

class QtLongLongSpinBox : public QAbstractSpinBox {
    Q_OBJECT
public:
    explicit QtLongLongSpinBox(QWidget *parent = 0);

    qlonglong value() const;

    QString prefix() const;
    void setPrefix(const QString &prefix);

    QString suffix() const;
    void setSuffix(const QString &suffix);

    QString cleanText() const;

    qlonglong singleStep() const;
    void setSingleStep(qlonglong val);

    qlonglong minimum() const;
    void setMinimum(qlonglong min);

    qlonglong maximum() const;
    void setMaximum(qlonglong max);

    void setRange(qlonglong min, qlonglong max);

    void setDisplayIntegerBase(int base);

    int displayIntegerBase() const;

public slots:
    void setValue(qlonglong value);

signals:
    void valueChanged(qlonglong i);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void stepBy(int steps);
    virtual StepEnabled stepEnabled() const;
    virtual QValidator::State validate(QString &input, int &pos) const;

private:
    void lineEditEditingFinalize();
    void selectCleanText();

private:
    QString m_prefix;
    QString m_suffix;
    qlonglong m_singleStep;
    qlonglong m_minimum;
    qlonglong m_maximum;
    qlonglong m_value;

    int m_base;

private:
    Q_DISABLE_COPY(QtLongLongSpinBox)
};

#endif // QTLONGLONGSPINBOX_H
