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

#ifndef _QCALL_TIP_H_
#define _QCALL_TIP_H_

#include "qce-config.h"

/*!
        \file qcalltip.h
        \brief Definition of the QCallTip class
*/

#include <QWidget>

class QCE_EXPORT QCallTip : public QWidget {
    Q_OBJECT

    Q_PROPERTY(QColor background READ background WRITE setBackground NOTIFY
                   backgroundChanged FINAL)
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground NOTIFY
                   foregroundChanged FINAL)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY
                   borderColorChanged FINAL)
    Q_PROPERTY(
        qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged FINAL)

public:
    QCallTip(QWidget *p = nullptr);
    virtual ~QCallTip();

    QStringList tips() const;
    void setTips(const QStringList &l);

    QColor background() const;
    void setBackground(const QColor &newBackground);

    QColor foreground() const;
    void setForeground(const QColor &newForeground);

    QColor borderColor() const;
    void setBorderColor(const QColor &newBorderColor);

    qreal opacity() const;
    void setOpacity(qreal newOpacity);

signals:
    void backgroundChanged();
    void foregroundChanged();
    void borderColorChanged();

    void opacityChanged();

protected:
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void keyPressEvent(QKeyEvent *e) override;
    virtual void focusInEvent(QFocusEvent *e) override;
    virtual void focusOutEvent(QFocusEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

private:
    int m_index;
    QStringList m_tips;
    QRect m_up, m_down;

    QColor _background;
    QColor _foreground;
    QColor _borderColor;

    qreal _opacity = 1;
};

#endif
