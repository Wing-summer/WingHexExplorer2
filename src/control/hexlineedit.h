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

#ifndef HEXLINEEDIT_H
#define HEXLINEEDIT_H

#include <QDateTime>
#include <QLineEdit>

class HexLineEdit : public QLineEdit {
    Q_OBJECT

public:
    struct HexNibble {
        bool hasValue = false;
        quint8 value = 0;
        bool isWildcard = false;
    };
    struct HexByte {
        HexNibble high;
        HexNibble low;
        bool isComplete() const { return high.hasValue && low.hasValue; }
    };

    struct UndoRecord {
        enum OperationType { Insert, Delete, Replace, Paste, ModeChange };

        OperationType type;
        int position;
        QString removedText;
        QString insertedText;
        int cursorBefore;
        int cursorAfter;
        bool hasSelection;
        int selectionStart;
        int selectionEnd;
        qint64 timestamp;

        inline explicit UndoRecord(OperationType t, const QString &fullText,
                                   int cb, int ca)
            : type(t), position(0), removedText(""), insertedText(fullText),
              cursorBefore(cb), cursorAfter(ca), hasSelection(false),
              selectionStart(-1), selectionEnd(-1),
              timestamp(QDateTime::currentMSecsSinceEpoch()) {}

        inline explicit UndoRecord(OperationType t, int pos,
                                   const QString &removed,
                                   const QString &inserted, int cb, int ca,
                                   bool sel = false, int selStart = -1,
                                   int selEnd = -1)
            : type(t), position(pos), removedText(removed),
              insertedText(inserted), cursorBefore(cb), cursorAfter(ca),
              hasSelection(sel), selectionStart(selStart), selectionEnd(selEnd),
              timestamp(QDateTime::currentMSecsSinceEpoch()) {}

        inline bool canMergeWith(const UndoRecord &other) const {
            if (type != other.type)
                return false;

            if (qAbs(timestamp - other.timestamp) > 1000)
                return false;

            if (type == Insert) {
                return (!hasSelection && !other.hasSelection &&
                        other.cursorBefore == cursorAfter &&
                        position + insertedText.length() == other.position);
            }

            if (type == Delete) {
                return (!hasSelection && !other.hasSelection) &&
                       ((position == other.position &&
                         removedText.length() == 1) ||
                        (position == other.position + 1 &&
                         removedText.length() == 1));
            }

            return false;
        }

        inline void mergeWith(const UndoRecord &other) {
            if (type == Insert) {
                insertedText += other.insertedText;
                cursorAfter = other.cursorAfter;
            } else if (type == Delete) {
                if (position == other.position) {
                    removedText += other.removedText;
                } else {
                    removedText = other.removedText + removedText;
                    position = other.position;
                }
                cursorBefore = other.cursorBefore;
            }
            timestamp = other.timestamp;
        }

        inline QString applyTo(const QString &text) const {
            QString result = text;

            if (type == Insert || type == Paste) {
                result.insert(position, insertedText);
            } else if (type == Delete) {
                result.remove(position, removedText.length());
            } else if (type == Replace) {
                result.remove(position, removedText.length());
                result.insert(position, insertedText);
            }

            return result;
        }

        inline UndoRecord reverse() const {
            OperationType reverseType = type;
            if (type == Insert)
                reverseType = Delete;
            else if (type == Delete)
                reverseType = Insert;

            return UndoRecord(reverseType, position, insertedText, removedText,
                              cursorAfter, cursorBefore, hasSelection,
                              selectionStart, selectionEnd);
        }
    };

    enum Mode { TextMode, HexMode };

public:
    explicit HexLineEdit(QWidget *parent = nullptr);

public:
    Mode mode() const;
    QString rawHexText() const;
    QVector<HexByte> hexBytes() const;

    bool isValid() const;
    qsizetype undoLimit() const;

public slots:
    void setMode(Mode m);
    void setHexText(const QString &hexish);

    void undo();
    void redo();

    void clearUndoHistory();

    void setUndoLimit(qsizetype limit);

signals:
    void modeChanged(HexLineEdit::Mode);
    void undoAvailable(bool available);
    void redoAvailable(bool available);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    void copySelectedHex();

    void cutSelectedHex();

    // Intercept key presses for hex mode (including Ctrl+V)
    void keyPressEvent(QKeyEvent *ev) override;

    void mousePressEvent(QMouseEvent *ev) override;

    void inputMethodEvent(QInputMethodEvent *event) override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

private:
    Mode m_mode = TextMode;
    QString m_raw; // only [0-9A-F?], uppercase, no spaces
    bool m_guardPaste = false;

    QVector<UndoRecord> m_undoStack;
    qsizetype m_undoLimit;
    qsizetype m_undoIndex;
    bool m_isUndoing;
    bool m_isRedoing;
    bool m_mergeEnabled;

private:
    void saveUndoRecord(UndoRecord::OperationType type, const QString &fullText,
                        int cursorBefore, int cursorAfter);

    void saveUndoRecord(UndoRecord::OperationType type, int position,
                        const QString &removed, const QString &inserted,
                        int cursorBefore, int cursorAfter,
                        bool hasSelection = false, int selStart = -1,
                        int selEnd = -1);

    void saveUndoRecord(const UndoRecord &newRecord);

    static bool isHexOrWildcard(QChar c);

    static quint8 hexCharToValue(QChar c);

    QString filteredRaw(const QString &s) const;

    QString formatRawToDisplay(const QString &raw) const;

    qsizetype logicalPosFromDisplayPos(qsizetype displayPos) const;

    qsizetype displayPosFromLogicalPos(qsizetype logicalPos) const;

    void insertAtLogicalPos(const QString &hexChars, qsizetype logicalPos);

    void removeLogicalRange(qsizetype logicalFrom, qsizetype logicalLen);

    void refreshDisplayAndSetCursor(qsizetype logicalCursorPos);

    bool hasSelection() const;

    void replaceSelectionWithRaw(const QString &newRawChunk);

    // central paste handler: filter then insert at logical cursor or replace
    // selection
    void handlePasteText(const QString &txt);
};

#endif // HEXLINEEDIT_H
