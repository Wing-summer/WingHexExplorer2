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

#ifndef TOAST_H
#define TOAST_H

#include <QDialog>
#include <QFont>
#include <QIcon>
#include <QSize>
#include <QString>

class Toast : public QDialog {
    Q_OBJECT

    Q_PROPERTY(int fontPointSize READ fontPointSize WRITE setFontPointSize)
    Q_PROPERTY(QPixmap icon READ icon WRITE setIcon)
    Q_PROPERTY(QColor backcolor READ backColor WRITE setBackColor)
    Q_PROPERTY(QColor textcolor READ textColor WRITE setTextColor)

public:
    static int LENGTH_LONG;
    static int LENGTH_SHORT;
    enum TOAST_POS { TOP, CENTER, BOTTOM };

    static void toast(QWidget *parent, const QPixmap &icon,
                      const QString &strContent, int fontPointSize = 20,
                      int nToastInterval = 1500);

    virtual ~Toast();

    void setTextFont(const QFont &font);
    const QFont &textFont();

    void setFontPointSize(int fontSize);
    int fontPointSize() const;

    void setToastPos(TOAST_POS pos);
    TOAST_POS lastToastPos() const;

    QPixmap icon() const;
    void setIcon(const QPixmap &icon);

    QColor backColor() const;
    void setBackColor(const QColor &backColor);

    QColor textColor() const;
    void setTextColor(const QColor &textColor);

protected:
    QSize calculateTextSize();
    void init();
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    QString m_strContent;
    int m_nToastInterval;
    int m_nCurrentWindowOpacity;
    int m_nCurrentStayTime;
    int m_nStatus;
    QFont m_drawFont;

    QColor m_textColor = Qt::white;
    QColor m_backColor = Qt::black;
    QPixmap m_icon;

    QWidget *_parent = nullptr;

    TOAST_POS m_pos = TOAST_POS::BOTTOM;

    Toast(const QString &strContent, const QPixmap &icon, int nToastInterval,
          QWidget *parent);
};

#endif // TOAST_H
