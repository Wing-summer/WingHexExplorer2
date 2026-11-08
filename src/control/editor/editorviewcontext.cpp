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

#include "editorviewcontext.h"

#include "control/editorview.h"

EditorViewContext::EditorViewContext(EditorView *view)
    : WingHex::HexEditorContext(view), m_view(view) {
    Q_ASSERT(view);
    m_hex = m_view->hexEditor();
    Q_ASSERT(m_hex);
}

QUrl EditorViewContext::docFileName() const { return m_view->fileNameUrl(); }

QFontMetricsF EditorViewContext::fontMetrics() const {
    return m_hex->fontMetrics();
}

QColor EditorViewContext::headerColor() const { return m_hex->headerColor(); }

QColor EditorViewContext::addressColor() const { return m_hex->addressColor(); }

QColor EditorViewContext::bytesBackground() const {
    return m_hex->bytesBackground();
}

QColor EditorViewContext::bytesAlterBackground() const {
    return m_hex->bytesAlterBackground();
}

QColor EditorViewContext::bytesColor() const { return m_hex->bytesColor(); }

QColor EditorViewContext::selectionColor() const {
    return m_hex->selectionColor();
}

QColor EditorViewContext::selBackgroundColor() const {
    return m_hex->selBackgroundColor();
}

bool EditorViewContext::stringAreaVisible() const {
    return m_hex->asciiVisible();
}

bool EditorViewContext::addressAreaVisible() const {
    return m_hex->addressVisible();
}

bool EditorViewContext::headerAreaVisible() const {
    return m_hex->headerVisible();
}

QColor EditorViewContext::borderColor() const { return m_hex->borderColor(); }

qsizetype EditorViewContext::documentLastLine() const {
    auto render = m_hex->renderer();
    return render->documentLastLine();
}

qsizetype EditorViewContext::documentLastColumn() const {
    auto render = m_hex->renderer();
    return render->documentLastColumn();
}

qsizetype EditorViewContext::documentLines() const {
    auto render = m_hex->renderer();
    return render->documentLines();
}

qreal EditorViewContext::documentWidth() const {
    auto render = m_hex->renderer();
    return render->documentWidth();
}

qreal EditorViewContext::lineHeight() const {
    auto render = m_hex->renderer();
    return render->lineHeight();
}

qreal EditorViewContext::borderSize() const {
    auto render = m_hex->renderer();
    return render->borderSize();
}

int EditorViewContext::hexLineWidth() const {
    auto render = m_hex->renderer();
    return render->hexLineWidth();
}

int EditorViewContext::areaIndent() const {
    auto doc = m_hex->document();
    return doc->areaIndent();
}

int EditorViewContext::addressWidth() const {
    auto render = m_hex->renderer();
    return render->getAddressWidth();
}

qreal EditorViewContext::headerHeight() const {
    auto render = m_hex->renderer();
    return render->headerLineCount() * render->lineHeight();
}

qreal EditorViewContext::hexColumnX() const {
    auto render = m_hex->renderer();
    return render->getHexColumnX();
}

qreal EditorViewContext::stringColumnX() const {
    auto render = m_hex->renderer();
    return render->getAsciiColumnX();
}

qreal EditorViewContext::endColumnX() const {
    auto render = m_hex->renderer();
    return render->getEndColumnX();
}

qreal EditorViewContext::cellWidth() const {
    auto render = m_hex->renderer();
    return render->getCellWidth();
}

qreal EditorViewContext::nCellsWidth(int n) const {
    auto render = m_hex->renderer();
    return render->getNCellsWidth(n);
}

QRectF EditorViewContext::lineRect(qsizetype line, qsizetype firstline) const {
    auto render = m_hex->renderer();
    return render->getLineRect(line, firstline);
}

WingHex::HexPosition EditorViewContext::position() const {
    WingHex::HexPosition pos;
    auto cursor = m_hex->cursor();
    auto p = cursor->position();
    pos.line = p.line;
    pos.lineWidth = p.lineWidth;
    pos.column = p.column;
    pos.nibbleindex = p.nibbleindex;
    return pos;
}

qsizetype EditorViewContext::selectionCount() const {
    auto cursor = m_hex->cursor();
    return cursor->selectionCount();
}

WingHex::HexPosition EditorViewContext::selectionStart(qsizetype index) const {
    WingHex::HexPosition pos;
    auto cursor = m_hex->cursor();
    auto p = cursor->selectionStart(index);
    pos.line = p.line;
    pos.lineWidth = p.lineWidth;
    pos.column = p.column;
    pos.nibbleindex = p.nibbleindex;
    return pos;
}

WingHex::HexPosition EditorViewContext::selectionEnd(qsizetype index) const {
    WingHex::HexPosition pos;
    auto cursor = m_hex->cursor();
    auto p = cursor->selectionEnd(index);
    pos.line = p.line;
    pos.lineWidth = p.lineWidth;
    pos.column = p.column;
    pos.nibbleindex = p.nibbleindex;
    return pos;
}

qsizetype EditorViewContext::selectionLength(qsizetype index) const {
    auto cursor = m_hex->cursor();
    return cursor->selectionLength(index);
}

bool EditorViewContext::isInInsertionMode() const {
    auto cursor = m_hex->cursor();
    return cursor->insertionMode() == QHexCursor::InsertMode;
}

qsizetype EditorViewContext::currentLine() const {
    auto cursor = m_hex->cursor();
    return cursor->currentLine();
}

int EditorViewContext::currentColumn() const {
    auto cursor = m_hex->cursor();
    return cursor->currentColumn();
}

int EditorViewContext::currentNibble() const {
    auto cursor = m_hex->cursor();
    return cursor->currentNibble();
}

qsizetype EditorViewContext::currentSelectionLength() const {
    auto cursor = m_hex->cursor();
    return cursor->currentSelectionLength();
}

bool EditorViewContext::isLineSelected(qsizetype line) const {
    auto cursor = m_hex->cursor();
    return cursor->isLineSelected(line);
}

bool EditorViewContext::isSelected(const WingHex::HexPosition &pos) const {
    auto cursor = m_hex->cursor();

    QHexPosition p;
    p.line = pos.line;
    p.lineWidth = pos.lineWidth;
    p.column = pos.column;
    p.nibbleindex = pos.nibbleindex;

    return cursor->isSelected(p);
}

bool EditorViewContext::hasSelection() const {
    auto cursor = m_hex->cursor();
    return cursor->hasSelection();
}

bool EditorViewContext::hasInternalSelection() const {
    auto cursor = m_hex->cursor();
    return cursor->hasInternalSelection();
}

qsizetype EditorViewContext::beginLine() const { return m_beginLine; }

qsizetype EditorViewContext::endLine() const { return m_endLine; }

qsizetype EditorViewContext::firstVisibleLine() const {
    return m_firstVisibleLine;
}

int EditorViewContext::currentHorizontalOffset() const {
    return m_horizontalOffset;
}

QByteArray EditorViewContext::read(qsizetype offset, qsizetype len) const {
    auto doc = m_hex->document();
    return doc->read(offset, len);
}

char EditorViewContext::readAt(qsizetype offset) const {
    auto doc = m_hex->document();
    return doc->at(offset);
}

quintptr EditorViewContext::baseAddress() const {
    auto doc = m_hex->document();
    return doc->baseAddress();
}

bool EditorViewContext::metafgVisible() const {
    auto doc = m_hex->document();
    return doc->metafgVisible();
}

bool EditorViewContext::metabgVisible() const {
    auto doc = m_hex->document();
    return doc->metabgVisible();
}

bool EditorViewContext::metaCommentVisible() const {
    auto doc = m_hex->document();
    return doc->metaCommentVisible();
}

bool EditorViewContext::isReadOnly() const {
    auto doc = m_hex->document();
    return doc->isReadOnly();
}

bool EditorViewContext::isKeepSize() const {
    auto doc = m_hex->document();
    return doc->isKeepSize();
}

bool EditorViewContext::isLocked() const {
    auto doc = m_hex->document();
    return doc->isLocked();
}

bool EditorViewContext::lockKeepSize() const {
    auto doc = m_hex->document();
    return doc->lockKeepSize();
}

void EditorViewContext::update() { m_hex->viewport()->update(); }

void EditorViewContext::update(const QRect &r) { m_hex->viewport()->update(r); }

void EditorViewContext::update(const QRegion &r) {
    m_hex->viewport()->update(r);
}

QList<WingHex::WingEditorViewWidget *>
EditorViewContext::currentPluginEditorWidgets() const {
    return m_currentPluginEditorWidgets;
}

void EditorViewContext::setBeginLine(qsizetype newBeginLine) {
    m_beginLine = newBeginLine;
}

void EditorViewContext::setEndLine(qsizetype newEndLine) {
    m_endLine = newEndLine;
}

void EditorViewContext::setFirstVisibleLine(qsizetype newFirstVisibleLine) {
    m_firstVisibleLine = newFirstVisibleLine;
}

void EditorViewContext::setCurrentHorizontalOffset(int horizontalOffset) {
    m_horizontalOffset = horizontalOffset;
}

void EditorViewContext::setCurrentPluginEditorWidget(const QStringList &ids) {
    m_currentPluginEditorWidgets = m_view->editorViewWidgets(ids);
}

EditorView *EditorViewContext::view() const { return m_view; }
