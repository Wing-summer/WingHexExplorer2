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

#ifndef EDITORVIEWCONTEXT_H
#define EDITORVIEWCONTEXT_H

#include "QHexView/qhexview.h"
#include "WingPlugin/hexeditorcontext.h"

class EditorViewContext : public WingHex::HexEditorContext {
public:
    explicit EditorViewContext(QHexView *view);

    // HexEditorPalette interface
public:
    virtual QString docFileName() const override;
    virtual QFontMetricsF fontMetrics() const override;
    virtual QColor headerColor() const override;
    virtual QColor addressColor() const override;
    virtual QColor bytesBackground() const override;
    virtual QColor bytesAlterBackground() const override;
    virtual QColor bytesColor() const override;
    virtual QColor selectionColor() const override;
    virtual QColor selBackgroundColor() const override;
    virtual bool stringAreaVisible() const override;
    virtual bool addressAreaVisible() const override;
    virtual bool headerAreaVisible() const override;
    virtual QColor borderColor() const override;

public:
    virtual qsizetype documentLastLine() const override;
    virtual qsizetype documentLastColumn() const override;
    virtual qsizetype documentLines() const override;
    virtual int documentWidth() const override;
    virtual int lineHeight() const override;
    virtual int borderSize() const override;
    virtual int hexLineWidth() const override;
    virtual int areaIndent() const override;
    virtual int addressWidth() const override;

public:
    virtual int headerHeight() const override;
    virtual int hexColumnX() const override;
    virtual int stringColumnX() const override;
    virtual int endColumnX() const override;
    virtual qreal cellWidth() const override;
    virtual int nCellsWidth(int n) const override;
    virtual QRect lineRect(qsizetype line, qsizetype firstline) const override;

public:
    virtual WingHex::HexPosition position() const override;
    virtual qsizetype selectionCount() const override;
    virtual WingHex::HexPosition selectionStart(qsizetype index) const override;
    virtual WingHex::HexPosition selectionEnd(qsizetype index) const override;
    virtual qsizetype selectionLength(qsizetype index) const override;
    virtual bool isInInsertionMode() const override;
    virtual qsizetype currentLine() const override;
    virtual int currentColumn() const override;
    virtual int currentNibble() const override;
    virtual qsizetype currentSelectionLength() const override;
    virtual bool isLineSelected(qsizetype line) const override;
    virtual bool isSelected(const WingHex::HexPosition &pos) const override;
    virtual bool hasSelection() const override;
    virtual bool hasInternalSelection() const override;
    virtual qsizetype beginLine() const override;
    virtual qsizetype endLine() const override;
    virtual qsizetype firstVisibleLine() const override;
    virtual int currentHorizontalOffset() const override;

public:
    virtual QByteArray read(qsizetype offset, qsizetype len) const override;
    virtual char readAt(qsizetype offset) const override;
    virtual quintptr baseAddress() const override;
    virtual bool metafgVisible() const override;
    virtual bool metabgVisible() const override;
    virtual bool metaCommentVisible() const override;
    virtual bool isReadOnly() const override;
    virtual bool isKeepSize() const override;
    virtual bool isLocked() const override;
    virtual bool lockKeepSize() const override;

public slots:
    virtual void update() override;

public:
    void setBeginLine(qsizetype newBeginLine);
    void setEndLine(qsizetype newEndLine);
    void setFirstVisibleLine(qsizetype newFirstVisibleLine);
    void setCurrentHorizontalOffset(int horizontalOffset);

private:
    QHexView *_view;

    qsizetype _beginLine = 0;
    qsizetype _endLine = 0;
    qsizetype _firstVisibleLine = 0;
    int _horizontalOffset = 0;
};

#endif // EDITORVIEWCONTEXT_H
