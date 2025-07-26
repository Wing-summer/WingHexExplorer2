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
#ifndef CONSOLEHIGHLIGHANIM_H
#define CONSOLEHIGHLIGHANIM_H

#include <QColor>
#include <QObject>
#include <QPropertyAnimation>

class ConsoleHighlighAnim : public QObject {
    Q_OBJECT

    Q_PROPERTY(int alpha READ getAlpha WRITE setAlpha NOTIFY alphaChanged FINAL)

public:
    explicit ConsoleHighlighAnim(QObject *parent = nullptr);

public:
    int getAlpha() const;
    void setAlpha(int newAlpha);

    QWidget *widget() const;
    void setWidget(QWidget *newW);

public:
    void start();

    // 停止动画并清除样式
    void stop();

signals:
    void alphaChanged();

private:
    QColor m_highlightColor;
    int m_alpha;
    QWidget *_w;
    QPropertyAnimation *_anim;
};

#endif // CONSOLEHIGHLIGHANIM_H
