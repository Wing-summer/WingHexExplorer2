/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#include "qhexview.h"

#include <cmath>

#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QHelpEvent>
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QScreen>
#include <QScrollBar>
#include <QTextCursor>
#include <QToolTip>

#define CURSOR_BLINK_INTERVAL 500 // ms
#define DOCUMENT_WHEEL_LINES 3

/*======================*/
// added by wingsummer

bool QHexView::isReadOnly() {
    return m_viewReadOnly || m_document->isReadOnly();
}

void QHexView::setReadOnly(bool readOnly) { m_viewReadOnly = readOnly; }

bool QHexView::viewReadOnly() const { return m_viewReadOnly; }

bool QHexView::isKeepSize() { return m_document->isKeepSize(); }

bool QHexView::isLocked() { return m_document->isLocked(); }

bool QHexView::setLockedFile(bool b) { return m_document->setLockedFile(b); }

bool QHexView::setKeepSize(bool b) { return m_document->setKeepSize(b); }

void QHexView::setLockKeepSize(bool b) { m_document->setLockKeepSize(b); }

bool QHexView::lockKeepSize() const { return m_document->lockKeepSize(); }

qsizetype QHexView::documentLines() { return m_renderer->documentLines(); }

qsizetype QHexView::documentBytes() { return m_document->length(); }

qsizetype QHexView::currentRow() { return m_cursor->currentLine(); }

qsizetype QHexView::currentColumn() { return m_cursor->currentColumn(); }

qsizetype QHexView::currentOffset() { return m_cursor->position().offset(); }

qsizetype QHexView::currentSelectionLength() {
    return m_cursor->currentSelectionLength();
}

qsizetype QHexView::selectionCount() { return m_cursor->selectionCount(); }

bool QHexView::hasSelection() { return m_cursor->hasSelection(); }

bool QHexView::asciiVisible() { return m_renderer->stringVisible(); }

bool QHexView::headerVisible() { return m_renderer->headerVisible(); }

bool QHexView::addressVisible() { return m_renderer->addressVisible(); }

void QHexView::setAsciiVisible(bool b) { m_renderer->setStringVisible(b); }

void QHexView::setAddressVisible(bool b) { m_renderer->setAddressVisible(b); }

void QHexView::setHeaderVisible(bool b) { m_renderer->setHeaderVisible(b); }

quintptr QHexView::addressBase() { return m_document->baseAddress(); }

void QHexView::setAddressBase(quintptr base) {
    m_document->SetBaseAddress(base);
}

bool QHexView::isSaved() { return m_document->isDocSaved(); }

QFont QHexView::getHexeditorFont() {
    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (f.styleHint() != QFont::TypeWriter) {
        f.setFamily(QStringLiteral("Monospace")); // Force Monospaced font
        f.setStyleHint(QFont::TypeWriter);
    }
    return f;
}

void QHexView::getStatus() {
    Q_EMIT sigOnCanUndoChanged(m_document->canUndo());
    Q_EMIT sigOnCanRedoChanged(m_document->canRedo());
    Q_EMIT sigOnCursorLocationChanged();
    Q_EMIT sigOnDocumentSaved(m_document->isDocSaved());
    Q_EMIT sigOnDocumentKeepSize(m_document->isKeepSize());
    Q_EMIT sigOnDocumentLockedFile(m_document->isLocked());
    Q_EMIT sigOnMetaFgVisibleChanged(m_document->metafgVisible());
    Q_EMIT sigOnMetaBgVisibleChanged(m_document->metabgVisible());
    Q_EMIT sigOnMetaCommentVisibleChanged(m_document->metaCommentVisible());
    Q_EMIT sigOnEditableAreaClicked(m_renderer->selectedArea());
}

void QHexView::establishSignal(QHexDocument *doc) {
    connect(doc, &QHexDocument::sigOnDocumentChanged, this, &QHexView::refresh);

    connect(doc, &QHexDocument::sigOnCanUndoChanged, this,
            &QHexView::sigOnCanUndoChanged);
    connect(doc, &QHexDocument::sigOnCanRedoChanged, this,
            &QHexView::sigOnCanRedoChanged);
    connect(doc, &QHexDocument::sigOnDocumentSaved, this,
            &QHexView::sigOnDocumentSaved);
    connect(doc, &QHexDocument::sigOnMetaBgVisibleChanged, this, [=](bool b) {
        Q_EMIT sigOnMetaBgVisibleChanged(b);
        Q_EMIT sigOnMetaStatusChanged();
    });
    connect(doc, &QHexDocument::sigOnMetaFgVisibleChanged, this, [=](bool b) {
        Q_EMIT sigOnMetaFgVisibleChanged(b);
        Q_EMIT sigOnMetaStatusChanged();
    });
    connect(doc, &QHexDocument::sigOnMetaCommentVisibleChanged, this,
            [=](bool b) {
                Q_EMIT sigOnMetaCommentVisibleChanged(b);
                Q_EMIT sigOnMetaStatusChanged();
            });
    connect(doc, &QHexDocument::sigOnMetaDataChanged, this,
            [this] { viewport()->update(); });
    connect(doc, &QHexDocument::sigOnDocumentKeepSize, this,
            &QHexView::sigOnDocumentKeepSize);
    connect(doc, &QHexDocument::sigOnDocumentLockedFile, this,
            &QHexView::sigOnDocumentLockedFile);
    connect(doc, &QHexDocument::sigOnHexLineWidthChanged, this, [this, doc] {
        m_cursor->setLineWidth(doc->hexLineWidth());
        refresh();
    });
}

void QHexView::adjustLineVisible(qsizetype line) {
    if (line < 0) {
        return;
    }

    qsizetype pos = 0;
    constexpr auto padding = 2;
    if (line < firstVisibleLine()) {
        pos = qMax(0, line - padding);
    } else if (line > lastVisibleLine()) {
        pos = qMax(0, line - visibleLines() + 1 + padding);
    } else {
        return;
    }
    QScrollBar *vscrollbar = this->verticalScrollBar();
    auto scrollPos = int(pos / documentSizeFactor());
    vscrollbar->setValue(scrollPos);
}

/*======================*/

QHexView::QHexView(QWidget *parent)
    : QAbstractScrollArea(parent), m_document(nullptr), m_renderer(nullptr) {
    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    if (f.styleHint() != QFont::TypeWriter) {
        f.setFamily("Monospace"); // Force Monospaced font
        f.setStyleHint(QFont::TypeWriter);
        f.setStyleHint(QFont::Monospace);
    }

    m_fontSize = qApp->font().pointSizeF();
    f.setPointSizeF(m_fontSize);
    m_cursor = new QHexCursor(this);

    this->setFont(f);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setMouseTracking(true);
    this->verticalScrollBar()->setSingleStep(1);
    this->verticalScrollBar()->setPageStep(1);

    m_blinktimer = new QTimer(this);
    m_blinktimer->setInterval(CURSOR_BLINK_INTERVAL);

    connect(m_blinktimer, &QTimer::timeout, this, &QHexView::blinkCursor);

    resetDocument();
}

QHexView::~QHexView() { m_blinktimer->stop(); }

QSharedPointer<QHexDocument> QHexView::document() { return m_document; }

QHexRenderer *QHexView::renderer() { return m_renderer; }

bool QHexView::setRenderer(QHexRenderer *renderer) {
    if (!renderer || renderer->thread() != thread() ||
        (renderer->parent() && renderer->parent() != this)) {
        qWarning("QHexView: invalid renderer ownership or thread");
        return false;
    }
    if (renderer == m_renderer) {
        return true;
    }
    auto previous = m_renderer;
    if (previous) {
        renderer->copyVisualState(*previous);
        disconnect(previous, nullptr, this, nullptr);
    }
    renderer->setParent(this);
    renderer->switchDoc(m_document.data());
    renderer->setCursor(m_cursor);
    renderer->updateMetrics(QFontMetricsF(font()));
    m_renderer = renderer;
    connect(renderer, &QHexRenderer::sigOnLayoutChanged, this,
            &QHexView::refresh);
    connect(renderer, &QHexRenderer::sigOnRepaintRequested, this,
            [this] { viewport()->update(); });

    if (previous) {
        previous->deleteLater();
    }

    refresh();
    return true;
}

// modified by wingsummer
void QHexView::setDocument(const QSharedPointer<QHexDocument> &document,
                           QHexCursor *cursor) {
    // modified by wingsummer
    if (!document) {
        return;
    }

    // Only remove this view's connections. Cursors may be shared by views.
    if (m_document) {
        disconnect(m_document.data(), nullptr, this, nullptr);
    }
    if (m_cursor) {
        disconnect(m_cursor, nullptr, this, nullptr);
    }

    m_document = document;
    if (cursor) {
        m_cursor = cursor;
    }
    m_cursor->setLineWidth(m_document->hexLineWidth());

    if (m_renderer) {
        m_renderer->switchDoc(m_document.data());
        m_renderer->setCursor(m_cursor);
    } else {
        setRenderer(new QHexRenderer(m_document.data(), m_cursor,
                                     QFontMetricsF(font()), this));
    }

    establishSignal(m_document.data());

    connect(m_cursor, &QHexCursor::sigOnPositionChanged, this,
            &QHexView::processPosChanged);
    connect(m_cursor, &QHexCursor::sigOnSelectionChanged, this, [this] {
        viewport()->update();
        processPosChanged();
        Q_EMIT sigOnCursorSelectionChanged();
    });
    connect(m_cursor, &QHexCursor::sigOnInsertionModeChanged, this, [this]() {
        renderCurrentLine();
        Q_EMIT sigOnInsertionModeChanged();
    });

    refresh();
}

void QHexView::resetDocument() {
    this->setDocument(
        QSharedPointer<QHexDocument>(QHexDocument::fromInternalBuffer(false)));
}

bool QHexView::event(QEvent *e) {
    if (m_renderer && (e->type() == QEvent::FontChange)) {
        m_renderer->updateMetrics(QFontMetricsF(this->font()));
        return true;
    }

    if (m_document && m_renderer && (e->type() == QEvent::ToolTip)) {
        QHelpEvent *helpevent = static_cast<QHelpEvent *>(e);
        QHexPosition position;

        QPoint abspos = absolutePosition(helpevent->pos());
        if (m_renderer->hitTest(abspos, &position, firstVisibleLine(),
                                lastVisibleLine())) {
            QString comments;

            auto m = m_document->metadata();
            auto idx = m->getIndex(position.offset());
            if (idx >= 0) {
                QTextStream ss(&comments);
                auto mi = m->at(idx);

                if (mi.foreground.isValid() && mi.foreground.alpha()) {
                    auto color = mi.foreground.name();
                    auto bcolor =
                        QHexMetadata::generateContrastingColor(mi.foreground)
                            .name();
                    ss << QStringLiteral("<p>") << tr("Foreground:")
                       << QStringLiteral("<b><a style=\"color:") << color
                       << QStringLiteral(";background-color:") << bcolor
                       << QStringLiteral("\">") << color
                       << QStringLiteral("</a></b></p>");
                }
                if (mi.background.isValid() && mi.background.alpha()) {
                    auto color = mi.background.name();
                    auto bcolor =
                        QHexMetadata::generateContrastingColor(mi.background)
                            .name();
                    ss << QStringLiteral("<p>") << tr("Background:")
                       << QStringLiteral("<b><a style=\"color:") << color
                       << QStringLiteral(";background-color:") << bcolor
                       << QStringLiteral("\">") << color
                       << QStringLiteral("</a></b></p>");
                }
                if (!mi.comment.isEmpty()) {
                    ss << tr("Comment:") << mi.comment;
                }
            }

            if (!comments.isEmpty()) {
                QToolTip::showText(helpevent->globalPos(), comments, this);
            }
        }

        return true;
    }

    return QAbstractScrollArea::event(e);
}

void QHexView::keyPressEvent(QKeyEvent *e) {
    if (!m_renderer || !m_document) {
        QAbstractScrollArea::keyPressEvent(e);
        return;
    }

    QHexCursor *cur = m_cursor;

    m_blinktimer->stop();
    m_renderer->enableCursor();

    bool handled = this->processMove(cur, e);
    if (!handled) {
        handled = this->processAction(cur, e);
    }
    if (!handled) {
        handled = this->processTextInput(cur, e);
    }
    if (!handled) {
        QAbstractScrollArea::keyPressEvent(e);
    }

    m_blinktimer->start();
}

void QHexView::keyReleaseEvent(QKeyEvent *e) {
    if (!m_renderer || !m_document) {
        QAbstractScrollArea::keyReleaseEvent(e);
        return;
    }

    if (!e->isAutoRepeat()) {
        auto key = e->key();
        if (key == Qt::Key_Shift || key == Qt::Key_Alt ||
            key == Qt::Key_Control) {
            if (m_cursor->hasPreviewSelection()) {
                auto sel = m_cursor->position();
                m_cursor->mergePreviewSelection();
                m_cursor->moveTo(sel);
            }
        }
    }

    QAbstractScrollArea::keyReleaseEvent(e);
}

QPoint QHexView::absolutePosition(const QPoint &pos) const {
    auto margins = viewport()->contentsMargins();
    QPoint shift(horizontalScrollBar()->value() - margins.left(),
                 -margins.top());
    return pos + shift;
}

QHexCursor *QHexView::cursor() const { return m_cursor; }

bool QHexView::cursorSync() const { return m_renderer->cursorSync(); }

qsizetype QHexView::copyLimit() const { return m_copylimit; }

void QHexView::setCopyLimit(qsizetype newCopylimit) {
    m_copylimit = newCopylimit;
}

void QHexView::setCursorSync(bool newCursorSync) {
    m_renderer->setCursorSync(newCursorSync);
}

qreal QHexView::scaleRate() const { return m_scaleRate; }

qsizetype QHexView::findNext(qsizetype begin, const QByteArray &ba) {
    if (begin < 0) {
        begin = m_cursor->position().offset();
    }
    return m_document->findNext(begin, ba);
}

qsizetype QHexView::findPrevious(qsizetype begin, const QByteArray &ba) {
    qsizetype startPos;
    if (begin < 0) {
        startPos = m_cursor->position().offset() - 1;
    } else {
        startPos = begin;
    }
    return m_document->findPrevious(startPos, ba);
}

bool QHexView::RemoveSelection(int nibbleindex) {
    if (!m_cursor->hasSelection()) {
        return true;
    }

    if (isReadOnly() || isLocked() || isKeepSize()) {
        return false;
    }

    auto total = m_cursor->selectionCount();
    bool success = true;
    auto msg = QStringLiteral("[H-G] {cnt: %1}").arg(total);
    m_document->beginMarco(msg);
    for (qsizetype i = total; i-- > 0;) {
        auto res =
            m_document->Remove(m_cursor, m_cursor->selectionStart(i).offset(),
                               m_cursor->selectionLength(i), nibbleindex);
        if (!res) {
            success = false;
            break;
        }
    }
    m_document->endMarco();
    m_cursor->clearSelection();

    return success;
}

bool QHexView::removeSelection() {
    if (isReadOnly()) {
        return false;
    }
    if (!m_cursor->hasSelection()) {
        return true;
    }

    // Selections are sorted by start offset; remove from the end so earlier
    // ranges retain their original offsets.
    auto total = m_cursor->selectionCount();
    bool res = true;
    for (qsizetype i = total; i-- > 0;) {
        res &= m_document->remove(m_cursor->selectionStart(i).offset(),
                                  m_cursor->selectionLength(i));
    }

    m_cursor->clearSelection();
    return res;
}

bool QHexView::atEnd() const {
    return m_cursor->position().offset() >= m_document->length();
}

QByteArray QHexView::selectedBytes(qsizetype index) const {
    return m_document->read(m_cursor->selectionStart(index).offset(),
                            m_cursor->selectionLength(index));
}

QByteArray QHexView::previewSelectedBytes() const {
    auto sel = m_cursor->previewSelection();
    return m_document->read(sel.begin.offset(), sel.length());
}

QByteArrayList QHexView::selectedBytes() const {
    if (!m_cursor->hasSelection()) {
        return {};
    }

    QByteArrayList res;
    auto total = m_cursor->selectionCount();
    for (int i = 0; i < total; ++i) {
        res.append(selectedBytes(i));
    }
    return res;
}

QByteArray QHexView::clipboardBytes(bool hex) {
    const auto *clipboard = qApp->clipboard();
    if (hex) {
        return QByteArray::fromHex(clipboard->text().toUtf8());
    }

    const auto *mime = clipboard->mimeData();
    auto data = mime->data(QStringLiteral("application/octet-stream"));
    return data.isEmpty() ? mime->text().toUtf8() : data;
}

bool QHexView::paste(bool hex) {
    if (isReadOnly()) {
        return false;
    }
    const auto data = clipboardBytes(hex);

    if (data.isEmpty()) {
        return true;
    }

    auto ret = this->removeSelection();
    if (!ret) {
        return false;
    }

    auto pos = m_cursor->position().offset();
    if (!m_document->isKeepSize()) {
        bool ret = m_document->insert(pos, data);
        if (ret) {
            m_cursor->moveTo(pos + data.length());
        }
        return ret;
    } else {
        return m_document->replace(pos, data);
    }
}

bool QHexView::Cut(bool hex, int nibbleindex) {
    if (isReadOnly()) {
        return false;
    }
    if (!m_cursor->hasSelection()) {
        return true;
    }

    if (m_document->isKeepSize()) {
        return false;
    }

    auto res = this->copy(hex);
    if (res) {
        return this->RemoveSelection(nibbleindex);
    }
    return false;
}

bool QHexView::Paste(bool hex, int nibbleindex) {
    if (isReadOnly()) {
        return false;
    }
    const auto data = clipboardBytes(hex);

    if (data.isEmpty()) {
        return true;
    }

    auto ret = this->RemoveSelection(nibbleindex);
    if (!ret) {
        return false;
    }

    auto pos = m_cursor->position().offset();
    if (m_cursor->insertionMode() == QHexCursor::InsertionMode::InsertMode) {
        auto ret = m_document->Insert(m_cursor, pos, data, nibbleindex);
        if (ret) {
            m_cursor->moveTo(pos + data.length());
        }
        return ret;
    } else {
        return m_document->Replace(m_cursor, pos, data, nibbleindex);
    }
}

void QHexView::Replace(qsizetype offset, uchar b, int nibbleindex) {
    if (isReadOnly()) {
        return;
    }
    m_document->Replace(m_cursor, offset, b, nibbleindex);
}

void QHexView::Replace(qsizetype offset, const QByteArray &data,
                       int nibbleindex) {
    if (isReadOnly()) {
        return;
    }
    m_document->Replace(m_cursor, offset, data, nibbleindex);
}

bool QHexView::cut(bool hex) {
    if (isReadOnly()) {
        return false;
    }
    if (!m_cursor->hasSelection() || m_document->isKeepSize()) {
        return false;
    }

    auto res = this->copy(hex);
    if (res) {
        return this->removeSelection();
    }
    return false;
}

bool QHexView::copy(bool hex) {
    if (!m_cursor->hasSelection()) {
        return true;
    }

    QClipboard *c = qApp->clipboard();

    auto len = currentSelectionLength();
    if (len > 1024 * 1024 * m_copylimit) {
        Q_EMIT sigOnCopyLimitRaised();
        return false;
    }

    QByteArray bytes = this->selectedBytes().join();
    if (hex) {
        bytes = bytes.toHex(' ').toUpper();
        c->setText(bytes);
    } else {
        auto mime = new QMimeData;
        mime->setData(QStringLiteral("application/octet-stream"),
                      bytes); // don't use setText()
        c->setMimeData(mime);
    }

    // fix the bug by wingsummer
    return true;
}

qreal QHexView::fontSize() const { return m_fontSize; }

void QHexView::setScaleRate(qreal rate) {
    if (rate >= 0.2 && rate < 3.01) {
        m_scaleRate = rate;
        setFontSize(fontSize());
        adjustScrollBars();
        update();
        Q_EMIT sigOnScaleRateChanged();
    }
}

QColor QHexView::selBackgroundColor() const {
    return m_renderer->selBackgroundColor();
}

QColor QHexView::borderColor() const { return m_renderer->borderColor(); }

void QHexView::setSelBackgroundColor(const QColor &newSelBackgroundColor) {
    m_renderer->setSelBackgroundColor(newSelBackgroundColor);
    Q_EMIT sigOnSelBackgroundColorChanged();
    this->viewport()->update();
}

void QHexView::setBorderColor(const QColor &newBorderColor) {
    m_renderer->setBorderColor(newBorderColor);
    Q_EMIT sigOnBorderColorChanged();
    this->viewport()->update();
}

void QHexView::setFontSize(qreal size) {
    Q_ASSERT(size > 0);
    auto font = this->font();
    font.setPointSizeF(size * m_scaleRate);
    this->setFont(font);
    this->viewport()->update();
}

QColor QHexView::selectionColor() const { return m_renderer->selectionColor(); }

void QHexView::setSelectionColor(const QColor &newSelectionColor) {
    m_renderer->setSelectionColor(newSelectionColor);
    Q_EMIT sigOnSelectionColorChanged();
    this->viewport()->update();
}

QColor QHexView::bytesAlterBackground() const {
    return m_renderer->bytesAlterBackground();
}

void QHexView::setBytesAlterBackground(const QColor &newBytesAlterBackground) {
    m_renderer->setBytesAlterBackground(newBytesAlterBackground);
    Q_EMIT sigOnBytesAlterBackgroundChanged();
    this->viewport()->update();
}

QColor QHexView::bytesColor() const { return m_renderer->bytesColor(); }

void QHexView::setBytesColor(const QColor &newBytesColor) {
    m_renderer->setBytesColor(newBytesColor);
    Q_EMIT sigOnBytesColorChanged();
    this->viewport()->update();
}

QColor QHexView::bytesBackground() const {
    return m_renderer->bytesBackground();
}

void QHexView::setBytesBackground(const QColor &newBytesBackground) {
    m_renderer->setBytesBackground(newBytesBackground);
    Q_EMIT sigOnBytesBackgroundChanged();
    this->viewport()->update();
}

QColor QHexView::addressColor() const { return m_renderer->addressColor(); }

void QHexView::setAddressColor(const QColor &newAddressColor) {
    m_renderer->setAddressColor(newAddressColor);
    Q_EMIT sigOnAddressColorChanged();
    this->viewport()->update();
}

QColor QHexView::headerColor() const { return m_renderer->headerColor(); }

void QHexView::setHeaderColor(const QColor &newHeaderColor) {
    m_renderer->setHeaderColor(newHeaderColor);
    Q_EMIT sigOnHeaderColorChanged();
    this->viewport()->update();
}

void QHexView::mousePressEvent(QMouseEvent *e) {
    QAbstractScrollArea::mousePressEvent(e);

    if (!m_renderer || (e->buttons() != Qt::LeftButton)) {
        return;
    }

    QHexPosition position;
    QPoint abspos = absolutePosition(e->pos());

    if (!m_renderer->hitTest(abspos, &position, firstVisibleLine(),
                             lastVisibleLine())) {
        return;
    }

    m_renderer->selectArea(abspos);
    auto area = m_renderer->selectedArea();
    if (m_renderer->editableArea(area)) {
        auto m = getSelectionMode();
        bool clearSelection = false;
        if (m == QHexCursor::SelectionNormal) {
            clearSelection = m_cursor->selectionCount() <= 1 ||
                             e->modifiers().testFlag(Qt::ControlModifier);
        } else if (m == QHexCursor::SelectionSingle) {
            clearSelection = true;
        }

        m_cursor->moveTo(position, clearSelection);
        Q_EMIT sigOnEditableAreaClicked(area);
    }

    e->accept();
}

void QHexView::mouseMoveEvent(QMouseEvent *e) {
    QAbstractScrollArea::mouseMoveEvent(e);
    if (!m_renderer || !m_document) {
        return;
    }

    QPoint abspos = absolutePosition(e->pos());

    if (e->buttons() == Qt::LeftButton) {
        if (m_blinktimer->isActive()) {
            m_blinktimer->stop();
            m_renderer->enableCursor(false);
        }

        QHexCursor *cursor = m_cursor;
        QHexPosition position;

        if (!m_renderer->hitTest(abspos, &position, firstVisibleLine(),
                                 lastVisibleLine())) {
            return;
        }

        cursor->select(position,
                       getSelectionMode() | QHexCursor::SelectionPreview);
        e->accept();
    }

    if (e->buttons() != Qt::NoButton) {
        return;
    }

    auto hittest = m_renderer->hitTestArea(abspos);

    if (m_renderer->editableArea(hittest)) {
        this->setCursor(Qt::IBeamCursor);
    } else {
        if (hittest == QHexRenderer::Areas::HeaderArea) {
            this->setCursor(Qt::PointingHandCursor);
        } else {
            this->setCursor(Qt::ArrowCursor);
        }
    }
}

void QHexView::mouseReleaseEvent(QMouseEvent *e) {
    QAbstractScrollArea::mouseReleaseEvent(e);
    if (e->button() != Qt::LeftButton) {
        return;
    }
    if (!m_blinktimer->isActive()) {
        m_blinktimer->start();
    }

    if (m_cursor->hasPreviewSelection()) {
        auto sel = m_cursor->previewSelection();
        m_cursor->mergePreviewSelection();
        m_cursor->moveTo(sel.end);
    }

    e->accept();
}

void QHexView::mouseDoubleClickEvent(QMouseEvent *e) {
    QAbstractScrollArea::mouseDoubleClickEvent(e);

    QPoint abspos = absolutePosition(e->pos());
    if (m_renderer->hitTestArea(abspos) == QHexRenderer::Areas::HeaderArea) {
        auto r = m_renderer->hitTestColArea(abspos);
        emit sigOnHeaderAreaClicked(r);
    }
}

void QHexView::focusInEvent(QFocusEvent *e) {
    QAbstractScrollArea::focusInEvent(e);
    if (!m_renderer) {
        return;
    }

    m_renderer->enableCursor();
    m_blinktimer->start();
}

void QHexView::focusOutEvent(QFocusEvent *e) {
    QAbstractScrollArea::focusOutEvent(e);
    if (!m_renderer) {
        return;
    }

    m_blinktimer->stop();
    m_renderer->enableCursor(true); // modified by wingsummer : false -> true
}

void QHexView::wheelEvent(QWheelEvent *e) {
    if (qApp->keyboardModifiers() == Qt::ControlModifier) {
        auto dela = e->angleDelta().y() / 1200.0;
        setScaleRate(scaleRate() + dela);
        return;
    }

    if (e->angleDelta().y() == 0) {
        int value = this->verticalScrollBar()->value();

        if (e->angleDelta().x() < 0) { // Scroll Down
            this->verticalScrollBar()->setValue(value + DOCUMENT_WHEEL_LINES);
        } else if (e->angleDelta().x() > 0) { // Scroll Up
            this->verticalScrollBar()->setValue(value - DOCUMENT_WHEEL_LINES);
        }

        return;
    }

    QAbstractScrollArea::wheelEvent(e);
}

void QHexView::resizeEvent(QResizeEvent *e) {
    QAbstractScrollArea::resizeEvent(e);
    this->adjustScrollBars();
}

void QHexView::paintEvent(QPaintEvent *e) {
    if (!m_document) {
        return;
    }

    QPainter painter(this->viewport());
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(this->font());

    const QRect &r = e->rect();

    const qsizetype firstVisible = this->firstVisibleLine();
    const int lineHeight = m_renderer->lineHeight();
    const int headerCount = m_renderer->headerLineCount();

    auto m = viewport()->contentsMargins();

    // these are lines from the point of view of the visible rect
    // where the first "headerCount" are taken by the header
    const int first = (r.top() - m.top()) / lineHeight; // included
    const int lastPlusOne =
        ((r.bottom() - m.top() - m.bottom()) / lineHeight) + 1; // excluded

    // compute document lines, adding firstVisible and removing the header
    // the max is necessary if the rect covers the header
    const qsizetype begin = firstVisible + std::max(first - headerCount, 0);
    const qsizetype end = firstVisible + std::max(lastPlusOne - headerCount, 0);

    auto xOff = this->horizontalScrollBar()->value();
    painter.save();
    painter.translate(-xOff + m.left(), m.top());
    m_renderer->render(&painter, begin, end, firstVisible);
    m_renderer->renderFrame(&painter);
    m_renderer->renderAdditonalFrame(&painter, m.top() > 0, m.left() > 0);
    painter.restore();
    Q_EMIT sigOnPaintCustomEvent(xOff, firstVisible, begin, end);
}

void QHexView::ensureCurrentLineVisible() {
    adjustLineVisible(m_cursor->currentLine());
}

void QHexView::toggleArea() {
    if (!m_renderer) {
        return;
    }
    m_renderer->toggleArea();
    emit sigOnEditableAreaClicked(m_renderer->selectedArea());
}

void QHexView::blinkCursor() {
    if (!m_renderer) {
        return;
    }
    m_renderer->blinkCursor();
    this->renderCurrentLine();
}

void QHexView::moveNext(QHexCursor::SelectionModes modes) {
    QHexCursor *cur = m_cursor;
    auto line = cur->currentLine();
    auto column = cur->currentColumn();
    const bool lastcell = (line >= m_renderer->documentLastLine()) &&
                          (column >= m_renderer->documentLastColumn());
    const auto area = m_renderer->selectedArea();
    const bool isAscii = area == QHexRenderer::Areas::AsciiArea;
    const bool isHex = area == QHexRenderer::Areas::HexArea;

    if (lastcell) {
        return;
    }

    int nibbleindex = cur->currentNibble();

    if (isHex) {
        if (lastcell && !nibbleindex) {
            return;
        }

        if (cur->position().offset() >= m_document->length() && nibbleindex) {
            return;
        }

        nibbleindex++;
        nibbleindex %= 2;

        if (nibbleindex) {
            column++;
        }
    } else if (isAscii) {
        nibbleindex = 1;
        column++;
    }

    if (column > m_renderer->hexLineWidth() - 1) {
        line = std::min(m_renderer->documentLastLine(), line + 1);
        column = 0;
        nibbleindex = 1;
    }

    const int targetColumn = std::min(m_renderer->hexLineWidth() - 1, column);
    if (isInSelection(modes)) {
        cur->select(line, targetColumn, nibbleindex,
                    modes | QHexCursor::SelectionPreview);
    } else {
        cur->moveTo(line, targetColumn, nibbleindex);
    }
}

void QHexView::movePrevious(QHexCursor::SelectionModes modes) {
    QHexCursor *cur = m_cursor;
    auto line = cur->currentLine();
    auto column = cur->currentColumn();
    const bool firstcell = !line && !column;
    const auto area = m_renderer->selectedArea();
    const bool isAscii = area == QHexRenderer::Areas::AsciiArea;
    const bool isHex = area == QHexRenderer::Areas::HexArea;

    if (firstcell) {
        return;
    }

    int nibbleindex = cur->currentNibble();

    if (isHex) {
        if (nibbleindex == 0) {
            nibbleindex = 1;
        } else {
            nibbleindex = 0;
            column--;
        }
    } else if (isAscii) {
        nibbleindex = 1;
        column--;
    }

    if (column < 0) {
        line = std::max(qsizetype(0), line - 1);
        column = m_renderer->hexLineWidth() - 1;
        nibbleindex = 0;
    }

    const int targetColumn = std::max(0, column);
    if (isInSelection(modes)) {
        cur->select(line, targetColumn, nibbleindex,
                    modes | QHexCursor::SelectionPreview);
    } else {
        cur->moveTo(line, targetColumn, nibbleindex);
    }
}

QHexCursor::SelectionModes QHexView::getSelectionMode() const {
    auto mods = qApp->keyboardModifiers();
    return getSelectionMode(mods);
}

QHexCursor::SelectionModes
QHexView::getSelectionMode(Qt::KeyboardModifiers mods) {
    bool pressedShift = mods.testFlag(Qt::ShiftModifier);
    bool pressedAlt = mods.testFlag(Qt::AltModifier);
    bool pressedControl = mods.testFlag(Qt::ControlModifier);

    if (pressedAlt && pressedShift) {
        pressedShift = false;
        pressedAlt = false;
        pressedControl = true;
    }

    if (pressedControl) {
        return QHexCursor::SelectionSingle;
    }

    if (pressedShift) {
        return QHexCursor::SelectionAdd;
    }

    if (pressedAlt) {
        return QHexCursor::SelectionRemove;
    }

    return QHexCursor::SelectionNormal;
}

bool QHexView::isInSelection(QHexCursor::SelectionModes modes) {
    return modes == QHexCursor::SelectionMode::SelectionAdd ||
           modes == QHexCursor::SelectionMode::SelectionRemove;
}

void QHexView::renderCurrentLine() {
    if (m_document) {
        this->renderLine(m_cursor->currentLine());
    }
}

void QHexView::processPosChanged() {
    adjustLineVisible(m_cursor->currentLine());
    renderCurrentLine();
    Q_EMIT sigOnCursorLocationChanged();
}

bool QHexView::processAction(QHexCursor *cur, QKeyEvent *e) {
    auto canFlag = !(isReadOnly() || isLocked());

    if (e->matches(QKeySequence::Copy)) {
        copy(m_renderer->selectedArea() == QHexRenderer::Areas::HexArea);
        return true;
    }
    if (e->modifiers() != Qt::NoModifier) {
        if (e->matches(QKeySequence::SelectAll)) {
            m_cursor->moveTo(0, 0);
            m_cursor->select(m_renderer->documentLastLine(),
                             m_renderer->documentLastColumn() - 1);
        } else {
            if (canFlag) {
                if (e->matches(QKeySequence::Undo)) {
                    m_document->undo();
                } else if (e->matches(QKeySequence::Redo)) {
                    m_document->redo();
                } else if (e->matches(QKeySequence::Cut)) {
                    this->Cut((m_renderer->selectedArea() ==
                               QHexRenderer::Areas::HexArea));
                } else if (e->matches(QKeySequence::Copy)) {
                    this->copy((m_renderer->selectedArea() ==
                                QHexRenderer::Areas::HexArea));
                } else if (e->matches(QKeySequence::Paste)) {
                    this->Paste((m_renderer->selectedArea() ==
                                 QHexRenderer::Areas::HexArea));
                } else {
                    return false;
                }

            } else {
                return false;
            }
        }
        return true;
    }

    if (canFlag &&
        ((e->key() == Qt::Key_Backspace) || (e->key() == Qt::Key_Delete))) {
        if (!cur->hasSelection()) {
            const QHexPosition &pos = cur->position();

            if (pos.offset() <= 0) {
                return true;
            }

            // modified by wingsummer
            if (!isKeepSize()) {
                if (e->key() == Qt::Key_Backspace) {
                    m_document->Remove(m_cursor, pos.offset() - 1, 1, 1);
                } else {
                    m_document->Remove(m_cursor, pos.offset(), 1, 0);
                }
            }
        } else {
            this->RemoveSelection();
        }

    } else if (e->key() == Qt::Key_Insert) {
        cur->switchInsertionMode();
    } else {
        return false;
    }

    return true;
}

bool QHexView::processMove(QHexCursor *cur, QKeyEvent *e) {
    const auto modifiers = e->modifiers();
    auto sel = getSelectionMode(modifiers);
    int nibble = m_renderer->selectedArea() == QHexRenderer::Areas::HexArea
                     ? cur->currentNibble()
                     : 1;

    switch (e->key()) {
    case Qt::Key_Right:
        this->moveNext(sel);
        return true;
    case Qt::Key_Left:
        this->movePrevious(sel);
        return true;
    case Qt::Key_Down: {
        if (m_renderer->documentLastLine() == cur->currentLine()) {
            return true;
        }
        nibble = cur->currentNibble();
        if (isInSelection(sel)) {
            auto nextline = cur->currentLine() + 1;
            cur->select(nextline, cur->currentColumn(), nibble,
                        sel | QHexCursor::SelectionPreview);
        } else {
            auto nextline = cur->currentLine() + 1;
            cur->moveTo(nextline, cur->currentColumn(), nibble);
        }
        return true;
    }
    case Qt::Key_Up: {
        if (!cur->currentLine()) {
            return true;
        }

        auto prevline = cur->currentLine() - 1;
        nibble = cur->currentNibble();
        if (isInSelection(sel)) {
            cur->select(prevline, cur->currentColumn(), nibble,
                        sel | QHexCursor::SelectionPreview);
        } else {
            cur->moveTo(prevline, cur->currentColumn(), nibble);
        }
        return true;
    }
    case Qt::Key_PageDown: {
        if (m_renderer->documentLastLine() == cur->currentLine()) {
            return true;
        }

        auto pageline = std::min(m_renderer->documentLastLine(),
                                 cur->currentLine() + this->visibleLines());
        nibble = cur->currentNibble();
        if (isInSelection(sel)) {
            cur->select(pageline, cur->currentColumn(), nibble,
                        sel | QHexCursor::SelectionPreview);
        } else {
            cur->moveTo(pageline, cur->currentColumn(), nibble);
        }
        return true;
    }
    case Qt::Key_PageUp: {
        if (!cur->currentLine()) {
            return true;
        }

        auto pageline =
            std::max(qsizetype(0), cur->currentLine() - this->visibleLines());
        nibble = cur->currentNibble();
        if (isInSelection(sel)) {
            cur->select(pageline, cur->currentColumn(), nibble,
                        sel | QHexCursor::SelectionPreview);
        } else {
            cur->moveTo(pageline, cur->currentColumn(), nibble);
        }
        return true;
    }
    case Qt::Key_Home:
        if (!cur->currentLine()) {
            return true;
        }
        if (isInSelection(sel)) {
            cur->select(cur->currentLine(), 0, nibble,
                        sel | QHexCursor::SelectionPreview);
        } else {
            cur->moveTo(cur->currentLine(), 0);
        }
        return true;
    case Qt::Key_End:
        if (m_renderer->documentLastLine() == cur->currentLine()) {
            return true;
        }
        if (isInSelection(sel)) {
            const auto targetColumn =
                (cur->currentLine() == m_renderer->documentLastLine())
                    ? m_renderer->documentLastColumn()
                    : m_renderer->hexLineWidth() - 1;
            cur->select(cur->currentLine(), targetColumn, nibble,
                        sel | QHexCursor::SelectionPreview);
        } else {
            if (cur->currentLine() == m_renderer->documentLastLine()) {
                cur->moveTo(cur->currentLine(),
                            m_renderer->documentLastColumn(), nibble);
            } else {
                cur->moveTo(cur->currentLine(), m_renderer->hexLineWidth() - 1,
                            nibble);
            }
        }
        return true;
    default:
        return false;
    }
}

bool QHexView::processTextInput(QHexCursor *cur, QKeyEvent *e) {
    if (e->modifiers() & Qt::ControlModifier) {
        return false;
    }
    if (isReadOnly() || isLocked()) {
        emit sigOnInputTextHitLimit();
        return false;
    }

    auto text = e->text();
    if (text.isEmpty()) {
        return false;
    }

    QChar key = text.front();
    if ((m_renderer->selectedArea() == QHexRenderer::Areas::HexArea)) {
        uchar val = hexChar2Value(key);
        if (val == 0xFF) {
            return false;
        }

        auto r = this->RemoveSelection();
        if (!r) {
            emit sigOnInputTextHitLimit();
            return false;
        }
        if (cur->currentNibble() &&
            cur->insertionMode() == QHexCursor::InsertMode) {
            r = m_document->Insert(cur, cur->position().offset(),
                                   uchar(val << 4),
                                   1); // X0 byte
            if (!r) {
                emit sigOnInputTextHitLimit();
                return false;
            }
            return true;
        }

        uchar ch = uchar(m_document->at(cur->position().offset()));

        if (cur->currentNibble()) { // X0
            val = uchar((ch & 0x0F) | (val << 4));
        } else { // 0X
            val = (ch & 0xF0) | val;
        }

        r = m_document->Replace(cur, cur->position().offset(), val,
                                cur->currentNibble());
        if (!r) {
            emit sigOnInputTextHitLimit();
            return false;
        }
        return true;
    }

    if ((m_renderer->selectedArea() == QHexRenderer::Areas::AsciiArea)) {
        if (!key.isPrint()) { // Check if is a Printable Char
            return false;
        }

        auto r = this->RemoveSelection();
        if (!r) {
            emit sigOnInputTextHitLimit();
            return false;
        }
        if (cur->insertionMode() == QHexCursor::OverwriteMode) {
            r = m_document->Replace(cur, cur->position().offset(),
                                    key.toLatin1(), 1);
        } else {
            r = m_document->Insert(cur, cur->position().offset(),
                                   key.toLatin1(), 1);
        }
        if (!r) {
            emit sigOnInputTextHitLimit();
            return false;
        }
        QKeyEvent keyevent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
        qApp->sendEvent(this, &keyevent);
        return true;
    }

    return false;
}

void QHexView::refresh() {
    adjustScrollBars();
    viewport()->update();
}

void QHexView::adjustScrollBars() {
    QScrollBar *vscrollbar = this->verticalScrollBar();
    int sizeFactor = this->documentSizeFactor();
    vscrollbar->setSingleStep(sizeFactor);

    auto docLines = m_renderer->documentLines();
    auto visLines = this->visibleLines();

    // modified by wingsummer,fix the scrollbar bug
    if (docLines > visLines && !m_document->isEmpty()) {
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        vscrollbar->setMaximum(
            int((docLines - visLines) / uint(sizeFactor) + 1));
    } else {
        this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        vscrollbar->setValue(0);
        vscrollbar->setMaximum(0);
    }

    QScrollBar *hscrollbar = this->horizontalScrollBar();
    auto documentWidth = m_renderer->documentWidth();
    auto viewportWidth = viewport()->width();
    auto margins = viewport()->contentsMargins();

    if (documentWidth > viewportWidth) {
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        // +1 to see the rightmost vertical line, +2 seems more pleasant to
        // the eyes
        hscrollbar->setMaximum(documentWidth - viewportWidth + 2 +
                               margins.left() + margins.right());
    } else {
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        hscrollbar->setValue(0);
        hscrollbar->setMaximum(documentWidth + margins.left() +
                               margins.right());
    }

    m_renderer->setCacheLineCount(visLines + 8);
}

void QHexView::renderLine(qsizetype line) {
    if (!this->isLineVisible(line)) {
        return;
    }
    this->viewport()->update();
}

qsizetype QHexView::firstVisibleLine() const {
    return qsizetype(this->verticalScrollBar()->value()) * documentSizeFactor();
}

qsizetype QHexView::lastVisibleLine() const {
    return this->firstVisibleLine() + this->visibleLines() - 1;
}

qsizetype QHexView::visibleLines() const {
    auto margins = viewport()->contentsMargins();
    auto visLines = qsizetype(
        std::ceil((this->height() - margins.top() - margins.bottom()) /
                      m_renderer->lineHeight() -
                  m_renderer->headerLineCount()));
    return std::min(visLines, m_renderer->documentLines());
}

bool QHexView::isLineVisible(qsizetype line) const {
    if (!m_document) {
        return false;
    }
    if (line < this->firstVisibleLine()) {
        return false;
    }
    if (line > this->lastVisibleLine()) {
        return false;
    }
    return true;
}

uchar QHexView::hexChar2Value(const QChar &ch) {
    auto c = ch.toLatin1();
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return 0xFF;
    }
}

int QHexView::documentSizeFactor() const {
    int factor = 1;

    if (m_document) {
        auto docLines = m_renderer->documentLines();
        if (docLines >= INT_MAX) {
            factor = int(docLines / INT_MAX) + 1;
        }
    }

    return factor;
}
