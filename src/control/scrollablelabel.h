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

#ifndef SCROLLABLELABEL_H
#define SCROLLABLELABEL_H

#include <QLabel>
#include <QScrollArea>
#include <QTimer>

class ScrollableLabel : public QScrollArea {
    Q_OBJECT

public:
    explicit ScrollableLabel(QWidget *parent = nullptr);

    void setText(const QString &text);

    QSize sizeHint() const override;

    QSize minimumSizeHint() const override;

protected:
    void wheelEvent(QWheelEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    QLabel label;

    void setupUI();

    void updateLabelSize();

    bool shouldScroll() const;

    void adjustDisplayLogic();
};

#endif // MARQUEELABEL_H
