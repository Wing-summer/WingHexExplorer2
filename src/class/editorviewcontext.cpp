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

EditorViewContext::EditorViewContext(QHexView *view)
    : WingHex::HexEditorContext(view), _view(view) {
    Q_ASSERT(view);
}

QString EditorViewContext::docFileName() const {
    return _view->windowFilePath();
}

QFontMetricsF EditorViewContext::fontMetrics() const {
    return _view->fontMetrics();
}

QColor EditorViewContext::headerColor() const { return _view->headerColor(); }

QColor EditorViewContext::addressColor() const { return _view->addressColor(); }

QColor EditorViewContext::bytesBackground() const {
    return _view->bytesBackground();
}

QColor EditorViewContext::bytesAlterBackground() const {
    return _view->bytesAlterBackground();
}

QColor EditorViewContext::bytesColor() const { return _view->bytesColor(); }

QColor EditorViewContext::selectionColor() const {
    return _view->selectionColor();
}

QColor EditorViewContext::selBackgroundColor() const {
    return _view->selBackgroundColor();
}

bool EditorViewContext::stringAreaVisible() const {
    return _view->asciiVisible();
}

bool EditorViewContext::addressAreaVisible() const {
    return _view->addressVisible();
}

bool EditorViewContext::headerAreaVisible() const {
    return _view->headerVisible();
}

QColor EditorViewContext::borderColor() const { return _view->borderColor(); }

qsizetype EditorViewContext::documentLastLine() const {
    auto render = _view->renderer();
    return render->documentLastLine();
}

qsizetype EditorViewContext::documentLastColumn() const {
    auto render = _view->renderer();
    return render->documentLastColumn();
}

qsizetype EditorViewContext::documentLines() const {
    auto render = _view->renderer();
    return render->documentLines();
}

int EditorViewContext::documentWidth() const {
    auto render = _view->renderer();
    return render->documentWidth();
}

int EditorViewContext::lineHeight() const {
    auto render = _view->renderer();
    return render->lineHeight();
}

int EditorViewContext::borderSize() const {
    auto render = _view->renderer();
    return render->borderSize();
}

int EditorViewContext::hexLineWidth() const {
    auto render = _view->renderer();
    return render->hexLineWidth();
}

int EditorViewContext::areaIndent() const {
    auto doc = _view->document();
    return doc->areaIndent();
}

int EditorViewContext::addressWidth() const {
    auto render = _view->renderer();
    return render->getAddressWidth();
}

int EditorViewContext::headerHeight() const {
    auto render = _view->renderer();
    return render->headerLineCount() * render->lineHeight();
}

int EditorViewContext::hexColumnX() const {
    auto render = _view->renderer();
    return render->getHexColumnX();
}

int EditorViewContext::stringColumnX() const {
    auto render = _view->renderer();
    return render->getAsciiColumnX();
}

int EditorViewContext::endColumnX() const {
    auto render = _view->renderer();
    return render->getEndColumnX();
}

qreal EditorViewContext::cellWidth() const {
    auto render = _view->renderer();
    return render->getCellWidth();
}

int EditorViewContext::nCellsWidth(int n) const {
    auto render = _view->renderer();
    return render->getNCellsWidth(n);
}

QRect EditorViewContext::lineRect(qsizetype line, qsizetype firstline) const {
    auto render = _view->renderer();
    return render->getLineRect(line, firstline);
}

WingHex::HexPosition EditorViewContext::position() const {
    WingHex::HexPosition pos;
    auto cursor = _view->cursor();
    auto p = cursor->position();
    pos.line = p.line;
    pos.lineWidth = p.lineWidth;
    pos.column = p.column;
    pos.nibbleindex = p.nibbleindex;
    return pos;
}

qsizetype EditorViewContext::selectionCount() const {
    auto cursor = _view->cursor();
    return cursor->selectionCount();
}

WingHex::HexPosition EditorViewContext::selectionStart(qsizetype index) const {
    WingHex::HexPosition pos;
    auto cursor = _view->cursor();
    auto p = cursor->selectionStart(index);
    pos.line = p.line;
    pos.lineWidth = p.lineWidth;
    pos.column = p.column;
    pos.nibbleindex = p.nibbleindex;
    return pos;
}

WingHex::HexPosition EditorViewContext::selectionEnd(qsizetype index) const {
    WingHex::HexPosition pos;
    auto cursor = _view->cursor();
    auto p = cursor->selectionEnd(index);
    pos.line = p.line;
    pos.lineWidth = p.lineWidth;
    pos.column = p.column;
    pos.nibbleindex = p.nibbleindex;
    return pos;
}

qsizetype EditorViewContext::selectionLength(qsizetype index) const {
    auto cursor = _view->cursor();
    return cursor->selectionLength(index);
}

bool EditorViewContext::isInInsertionMode() const {
    auto cursor = _view->cursor();
    return cursor->insertionMode() == QHexCursor::InsertMode;
}

qsizetype EditorViewContext::currentLine() const {
    auto cursor = _view->cursor();
    return cursor->currentLine();
}

int EditorViewContext::currentColumn() const {
    auto cursor = _view->cursor();
    return cursor->currentColumn();
}

int EditorViewContext::currentNibble() const {
    auto cursor = _view->cursor();
    return cursor->currentNibble();
}

qsizetype EditorViewContext::currentSelectionLength() const {
    auto cursor = _view->cursor();
    return cursor->currentSelectionLength();
}

bool EditorViewContext::isLineSelected(qsizetype line) const {
    auto cursor = _view->cursor();
    return cursor->isLineSelected(line);
}

bool EditorViewContext::isSelected(const WingHex::HexPosition &pos) const {
    auto cursor = _view->cursor();

    QHexPosition p;
    p.line = pos.line;
    p.lineWidth = pos.lineWidth;
    p.column = pos.column;
    p.nibbleindex = pos.nibbleindex;

    return cursor->isSelected(p);
}

bool EditorViewContext::hasSelection() const {
    auto cursor = _view->cursor();
    return cursor->hasSelection();
}

bool EditorViewContext::hasInternalSelection() const {
    auto cursor = _view->cursor();
    return cursor->hasInternalSelection();
}

qsizetype EditorViewContext::beginLine() const { return _beginLine; }

qsizetype EditorViewContext::endLine() const { return _endLine; }

qsizetype EditorViewContext::firstVisibleLine() const {
    return _firstVisibleLine;
}

int EditorViewContext::currentHorizontalOffset() const {
    return _horizontalOffset;
}

QByteArray EditorViewContext::read(qsizetype offset, qsizetype len) const {
    auto doc = _view->document();
    return doc->read(offset, len);
}

char EditorViewContext::readAt(qsizetype offset) const {
    auto doc = _view->document();
    return doc->at(offset);
}

quintptr EditorViewContext::baseAddress() const {
    auto doc = _view->document();
    return doc->baseAddress();
}

bool EditorViewContext::metafgVisible() const {
    auto doc = _view->document();
    return doc->metafgVisible();
}

bool EditorViewContext::metabgVisible() const {
    auto doc = _view->document();
    return doc->metabgVisible();
}

bool EditorViewContext::metaCommentVisible() const {
    auto doc = _view->document();
    return doc->metaCommentVisible();
}

bool EditorViewContext::isReadOnly() const {
    auto doc = _view->document();
    return doc->isReadOnly();
}

bool EditorViewContext::isKeepSize() const {
    auto doc = _view->document();
    return doc->isKeepSize();
}

bool EditorViewContext::isLocked() const {
    auto doc = _view->document();
    return doc->isLocked();
}

bool EditorViewContext::lockKeepSize() const {
    auto doc = _view->document();
    return doc->lockKeepSize();
}

void EditorViewContext::update() { _view->viewport()->update(); }

void EditorViewContext::update(const QRect &r) { _view->viewport()->update(r); }

void EditorViewContext::update(const QRegion &r) {
    _view->viewport()->update(r);
}

void EditorViewContext::setBeginLine(qsizetype newBeginLine) {
    _beginLine = newBeginLine;
}

void EditorViewContext::setEndLine(qsizetype newEndLine) {
    _endLine = newEndLine;
}

void EditorViewContext::setFirstVisibleLine(qsizetype newFirstVisibleLine) {
    _firstVisibleLine = newFirstVisibleLine;
}

void EditorViewContext::setCurrentHorizontalOffset(int horizontalOffset) {
    _horizontalOffset = horizontalOffset;
}
