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

#ifndef QHEXTEXTEDIT_H
#define QHEXTEXTEDIT_H

#include <QTextEdit>

class QHexTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit QHexTextEdit(QWidget *parent = nullptr);

    bool isHexMode() const;
    void setIsHexMode(bool newIsHexMode);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString mText;
    int mCurserPositionPre;
    bool m_isHexMode;
};

#endif // QHEXTEXTEDIT_H
