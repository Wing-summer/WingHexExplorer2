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

#include "qhextextedit.h"

#include <QKeyEvent>
#include <QLineEdit>

QHexTextEdit::QHexTextEdit(QWidget *parent)
    : QTextEdit(parent), m_isHexMode(true) {
    setLineWrapMode(QTextEdit::NoWrap);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mCurserPositionPre = 0;
}

void QHexTextEdit::keyPressEvent(QKeyEvent *event) {
    if (!m_isHexMode) {
        QTextEdit::keyPressEvent(event);
        return;
    }

    auto pressedKey = event->key();
    QTextCursor cursor = textCursor();
    auto cursorPosition = cursor.position();
    auto newCursorPosition = cursorPosition;
    auto index = cursorPosition - cursorPosition / 3;
    if (((pressedKey >= Qt::Key_0) && (pressedKey <= Qt::Key_9)) ||
        ((pressedKey >= Qt::Key_A) && (pressedKey <= Qt::Key_F)) ||
        pressedKey == Qt::Key_Question) {
        mText.insert(index, event->text());
        newCursorPosition = cursorPosition + (cursorPosition % 3 + 1);
    } else if (pressedKey == Qt::Key_Backspace) {
        if (index != 0) {
            mText.remove(index - 1, 1);
            newCursorPosition = cursorPosition - 2 + (cursorPosition) % 3;
        }
    } else if (pressedKey == Qt::Key_Delete) {
        if (index != mText.length()) {
            mText.remove(index, 1);
            newCursorPosition = cursorPosition;
        }
    } else if (pressedKey == Qt::Key_Left) {
        if (index != 0) {
            newCursorPosition = cursorPosition - 2 + (cursorPosition) % 3;
        }
    } else if (pressedKey == Qt::Key_Right) {
        if (index != mText.length()) {
            newCursorPosition = cursorPosition + (cursorPosition + 1) % 3;
        }
    }
    // Allow only single-line editing
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->ignore(); // Ignore Enter key press to prevent new lines
    }

    QString temp;
    for (int i = 0; i < mText.length(); i++) {
        temp.append(mText.at(i).toUpper());
        if (i % 2 == 1)
            temp.append(' ');
    }

    setText(temp);
    cursor.setPosition(newCursorPosition);
    setTextCursor(cursor);
    mCurserPositionPre = newCursorPosition;
}

void QHexTextEdit::mousePressEvent(QMouseEvent *event) {
    if (!m_isHexMode) {
        QTextEdit::mousePressEvent(event);
        return;
    }

    QTextCursor cursor = cursorForPosition(event->pos());
    int cursorPosition = cursor.position();
    if (cursorPosition % 3 == 2) {
        cursorPosition++;
        cursor.setPosition(cursorPosition);
        mCurserPositionPre = cursorPosition;
    }
    QTextEdit::mousePressEvent(event);
    setTextCursor(cursor);
}

bool QHexTextEdit::isHexMode() const { return m_isHexMode; }

void QHexTextEdit::setIsHexMode(bool newIsHexMode) {
    m_isHexMode = newIsHexMode;
}
