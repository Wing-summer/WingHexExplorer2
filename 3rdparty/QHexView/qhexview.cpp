#include "qhexview.h"
#include "document/buffer/qmemorybuffer.h"
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

#include <cmath>

#define CURSOR_BLINK_INTERVAL 500 // ms
#define DOCUMENT_WHEEL_LINES 3

/*======================*/
// added by wingsummer

bool QHexView::isReadOnly() { return m_document->isReadOnly(); }
bool QHexView::isKeepSize() { return m_document->isKeepSize(); }
bool QHexView::isLocked() { return m_document->isLocked(); }
bool QHexView::setLockedFile(bool b) {
    auto ret = m_document->setLockedFile(b);
    return ret;
}
bool QHexView::setKeepSize(bool b) {
    auto ret = m_document->setKeepSize(b);
    if (b) {
        m_cursor->setInsertionMode(QHexCursor::OverwriteMode);
    }
    return ret;
}

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

void QHexView::setAsciiVisible(bool b) {
    m_renderer->setStringVisible(b);
    this->adjustScrollBars();
    this->viewport()->update();
}

void QHexView::setAddressVisible(bool b) {
    m_renderer->setAddressVisible(b);
    this->adjustScrollBars();
    this->viewport()->update();
}

void QHexView::setHeaderVisible(bool b) {
    m_renderer->setHeaderVisible(b);
    this->adjustScrollBars();
    this->viewport()->update();
}

quintptr QHexView::addressBase() { return m_document->baseAddress(); }

void QHexView::setAddressBase(quintptr base) {
    m_document->SetBaseAddress(base);
}

bool QHexView::isSaved() { return m_document->isDocSaved(); }

QFont QHexView::getHexeditorFont() {
    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (f.styleHint() != QFont::TypeWriter) {
        f.setFamily("Monospace"); // Force Monospaced font
        f.setStyleHint(QFont::TypeWriter);
    }
    return f;
}

void QHexView::getStatus() {
    emit canUndoChanged(m_document->canUndo());
    emit canRedoChanged(m_document->canRedo());
    emit cursorLocationChanged();
    emit documentSaved(m_document->isDocSaved());
    emit documentKeepSize(m_document->isKeepSize());
    emit documentLockedFile(m_document->isLocked());
    emit metafgVisibleChanged(m_document->metafgVisible());
    emit metabgVisibleChanged(m_document->metabgVisible());
    emit metaCommentVisibleChanged(m_document->metaCommentVisible());
}

void QHexView::establishSignal(QHexDocument *doc) {
    connect(doc, &QHexDocument::documentChanged, this, [this]() {
        this->adjustScrollBars();
        this->viewport()->update();
    });

    connect(m_cursor, &QHexCursor::positionChanged, this,
            &QHexView::moveToSelection);
    connect(m_cursor, &QHexCursor::selectionChanged, this, [this]() {
        this->viewport()->update();
        emit cursorSelectionChanged();
    });
    connect(m_cursor, &QHexCursor::insertionModeChanged, this,
            &QHexView::renderCurrentLine);
    connect(doc, &QHexDocument::canUndoChanged, this,
            &QHexView::canUndoChanged);
    connect(doc, &QHexDocument::canRedoChanged, this,
            &QHexView::canRedoChanged);
    connect(doc, &QHexDocument::documentSaved, this, &QHexView::documentSaved);
    connect(doc, &QHexDocument::metabgVisibleChanged, this, [=](bool b) {
        emit metabgVisibleChanged(b);
        emit metaStatusChanged();
    });
    connect(doc, &QHexDocument::metafgVisibleChanged, this, [=](bool b) {
        emit metafgVisibleChanged(b);
        emit metaStatusChanged();
    });
    connect(doc, &QHexDocument::metaCommentVisibleChanged, this, [=](bool b) {
        emit metaCommentVisibleChanged(b);
        emit metaStatusChanged();
    });
    connect(doc, &QHexDocument::metaDataChanged, this,
            [=] { this->viewport()->update(); });
    connect(doc, &QHexDocument::documentKeepSize, this,
            &QHexView::documentKeepSize);
    connect(doc, &QHexDocument::documentLockedFile, this,
            &QHexView::documentLockedFile);
    connect(doc, &QHexDocument::hexLineWidthChanged, this,
            [=] { m_cursor->setLineWidth(doc->hexLineWidth()); });
}

/*======================*/

QHexView::QHexView(QWidget *parent)
    : QAbstractScrollArea(parent), m_document(nullptr), m_renderer(nullptr) {
    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    if (f.styleHint() != QFont::TypeWriter) {
        f.setFamily("Monospace"); // Force Monospaced font
        f.setStyleHint(QFont::TypeWriter);
    }

    m_fontSize = f.pointSizeF();
    m_cursor = new QHexCursor(this);

    this->setFont(f);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setMouseTracking(true);
    this->verticalScrollBar()->setSingleStep(1);
    this->verticalScrollBar()->setPageStep(1);

    m_blinktimer = new QTimer(this);
    m_blinktimer->setInterval(CURSOR_BLINK_INTERVAL);

    connect(m_blinktimer, &QTimer::timeout, this, &QHexView::blinkCursor);

    this->setDocument(QSharedPointer<QHexDocument>(
        QHexDocument::fromMemory<QMemoryBuffer>(QByteArray(), false)));
}

QHexView::~QHexView() { m_blinktimer->stop(); }

QSharedPointer<QHexDocument> QHexView::document() { return m_document; }

QHexRenderer *QHexView::renderer() { return m_renderer; }

// modified by wingsummer
void QHexView::setDocument(const QSharedPointer<QHexDocument> &document,
                           QHexCursor *cursor) {
    // modified by wingsummer
    if (!document) {
        return;
    }

    if (m_document) {
        m_document->disconnect();
        m_document.clear();
    }

    m_document = document;

    if (m_renderer) {
        m_renderer->switchDoc(m_document.get());
        if (cursor) {
            m_cursor->disconnect();
            m_renderer->setCursor(cursor);
            m_cursor = cursor;
        }
    } else {
        if (cursor) {
            m_cursor->disconnect();
            m_cursor = cursor;
        }
        m_renderer = new QHexRenderer(m_document.data(), m_cursor,
                                      this->fontMetrics(), this);
    }

    establishSignal(m_document.data());
    emit documentChanged(document.data());

    this->adjustScrollBars();
    this->viewport()->update();
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
        if (m_renderer->hitTest(abspos, &position, this->firstVisibleLine())) {
            QString comments;

            auto mi = m_document->metadata()->get(position.offset());
            if (mi.has_value()) {
                QTextStream ss(&comments);

                if (mi->foreground.isValid() && mi->foreground.alpha()) {
                    auto color = mi->foreground.name();
                    auto bcolor =
                        QHexMetadata::generateContrastingColor(mi->foreground)
                            .name();
                    ss << QStringLiteral("<p>") << tr("Foreground:")
                       << QStringLiteral("<b><a style=\"color:") << color
                       << QStringLiteral(";background-color:") << bcolor
                       << QStringLiteral("\">") << color
                       << QStringLiteral("</a></b></p>");
                }
                if (mi->background.isValid() && mi->background.alpha()) {
                    auto color = mi->background.name();
                    auto bcolor =
                        QHexMetadata::generateContrastingColor(mi->background)
                            .name();
                    ss << QStringLiteral("<p>") << tr("Background:")
                       << QStringLiteral("<b><a style=\"color:") << color
                       << QStringLiteral(";background-color:") << bcolor
                       << QStringLiteral("\">") << color
                       << QStringLiteral("</a></b></p>");
                }
                if (!mi->comment.isEmpty()) {
                    ss << tr("Comment:") << mi->comment;
                }
            }

            if (!comments.isEmpty())
                QToolTip::showText(helpevent->globalPos(), comments, this);
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
    if (!handled)
        handled = this->processAction(cur, e);
    if (!handled)
        handled = this->processTextInput(cur, e);
    if (!handled)
        QAbstractScrollArea::keyPressEvent(e);

    m_blinktimer->start();
}

QPoint QHexView::absolutePosition(const QPoint &pos) const {
    QPoint shift(horizontalScrollBar()->value(), 0);
    return pos + shift;
}

QHexCursor *QHexView::cursor() const { return m_cursor; }

qsizetype QHexView::copyLimit() const { return m_copylimit; }

void QHexView::setCopyLimit(qsizetype newCopylimit) {
    m_copylimit = newCopylimit;
}

qreal QHexView::scaleRate() const { return m_scaleRate; }

qsizetype QHexView::searchForward(qsizetype begin, const QByteArray &ba) {
    if (begin < 0) {
        begin = m_cursor->position().offset();
    }
    return m_document->searchForward(begin, ba);
}

qsizetype QHexView::searchBackward(qsizetype begin, const QByteArray &ba) {
    qsizetype startPos;
    if (begin < 0) {
        startPos = m_cursor->position().offset() - 1;
    } else {
        startPos = begin;
    }
    return m_document->searchBackward(startPos, ba);
}

bool QHexView::RemoveSelection(int nibbleindex) {
    if (!m_cursor->hasSelection())
        return false;

    auto total = m_cursor->selectionCount();
    bool res = true;
    m_document->beginMarco(QStringLiteral(""));
    for (int i = 0; i < total; ++i) {
        res &=
            m_document->Remove(m_cursor, m_cursor->selectionStart(i).offset(),
                               m_cursor->selectionLength(i), nibbleindex);
    }
    m_document->endMarco();
    m_cursor->clearSelection();

    return res;
}

bool QHexView::removeSelection() {
    if (!m_cursor->hasSelection())
        return false;

    // We essure selections are ordered by desending
    // by selection-start, so it's safe to remove
    auto total = m_cursor->selectionCount();
    bool res = true;
    for (int i = 0; i < total; ++i) {
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
                            m_cursor->currentSelectionLength());
}

QByteArray QHexView::previewSelectedBytes() const {
    auto sel = m_cursor->previewSelection();
    return m_document->read(sel.begin.offset(), sel.length());
}

QByteArrayList QHexView::selectedBytes() const {
    if (!m_cursor->hasSelection())
        return {};

    QByteArrayList res;
    auto total = m_cursor->selectionCount();
    for (int i = 0; i < total; ++i) {
        res << m_document->read(m_cursor->selectionStart(i).offset(),
                                m_cursor->selectionLength(i));
    }
    return res;
}

void QHexView::paste(bool hex) {
    QClipboard *c = qApp->clipboard();
    QByteArray data = c->text().toUtf8();

    if (data.isEmpty())
        return;

    this->removeSelection();

    if (hex)
        data = QByteArray::fromHex(data);

    auto pos = m_cursor->position().offset();
    if (!m_document->isKeepSize()) {
        m_document->insert(pos, data);
        m_cursor->moveTo(pos + data.length()); // added by wingsummer
    } else
        m_document->replace(pos, data);
}

bool QHexView::Cut(bool hex, int nibbleindex) {
    if (!m_cursor->hasSelection())
        return true;

    if (m_document->isKeepSize())
        return false;

    auto res = this->copy(hex);
    if (res) {
        return this->RemoveSelection(nibbleindex);
    } else {
        emit copyLimitRaised();
        return res;
    }
}

void QHexView::Paste(int nibbleindex, bool hex) {
    QClipboard *c = qApp->clipboard();
    QByteArray data = c->mimeData()->data(
        QStringLiteral("application/octet-stream")); // don't use getText()

    if (data.isEmpty())
        return;

    this->RemoveSelection(nibbleindex);

    if (hex)
        data = QByteArray::fromHex(data);

    auto pos = m_cursor->position().offset();
    if (m_cursor->insertionMode() == QHexCursor::InsertionMode::InsertMode) {
        m_document->Insert(m_cursor, pos, data, nibbleindex);
        m_cursor->moveTo(pos + data.length()); // added by wingsummer
    } else {
        m_document->Replace(m_cursor, pos, data, nibbleindex);
    }
}

void QHexView::Replace(qsizetype offset, uchar b, int nibbleindex) {
    m_document->Replace(m_cursor, offset, b, nibbleindex);
}

void QHexView::Replace(qsizetype offset, const QByteArray &data,
                       int nibbleindex) {
    m_document->Replace(m_cursor, offset, data, nibbleindex);
}

bool QHexView::cut(bool hex) {
    if (!m_cursor->hasSelection() || m_document->isKeepSize())
        return false;

    auto res = this->copy(hex);
    if (res) {
        return this->removeSelection();
    } else {
        emit copyLimitRaised();
        return res;
    }
}

bool QHexView::copy(bool hex) {
    if (!m_cursor->hasSelection())
        return true;

    QClipboard *c = qApp->clipboard();

    auto len = currentSelectionLength();

    // 如果拷贝字节超过 ? MB 阻止
    if (len > 1024 * 1024 * m_copylimit) {
        emit copyLimitRaised();
        return false;
    }

    QByteArray bytes = this->selectedBytes().join();

    if (hex)
        bytes = bytes.toHex(' ').toUpper();

    auto mime = new QMimeData;
    mime->setData(QStringLiteral("application/octet-stream"),
                  bytes); // don't use setText()
    c->setMimeData(mime);

    // fix the bug by wingsummer
    return true;
}

qreal QHexView::fontSize() const { return m_fontSize; }

void QHexView::setScaleRate(qreal rate) {
    if (m_scaleRate > 0) {
        m_scaleRate = rate;
        setFontSize(fontSize());
        emit scaleRateChanged();
    }
}

QColor QHexView::selBackgroundColor() const {
    return m_renderer->selBackgroundColor();
}

void QHexView::setSelBackgroundColor(const QColor &newSelBackgroundColor) {
    m_renderer->setSelBackgroundColor(newSelBackgroundColor);
    emit selBackgroundColorChanged();
}

void QHexView::setFontSize(qreal size) {
    Q_ASSERT(size > 0);
    auto font = this->font();
    font.setPointSizeF(size * m_scaleRate);
    this->setFont(font);
    m_fontSize = size;
}

QColor QHexView::selectionColor() const { return m_renderer->selectionColor(); }

void QHexView::setSelectionColor(const QColor &newSelectionColor) {
    m_renderer->setSelectionColor(newSelectionColor);
    emit selectionColorChanged();
}

QColor QHexView::bytesAlterBackground() const {
    return m_renderer->bytesAlterBackground();
}

void QHexView::setBytesAlterBackground(const QColor &newBytesAlterBackground) {
    m_renderer->setBytesAlterBackground(newBytesAlterBackground);
    emit bytesAlterBackgroundChanged();
}

QColor QHexView::bytesColor() const { return m_renderer->bytesColor(); }

void QHexView::setBytesColor(const QColor &newBytesColor) {
    m_renderer->setBytesColor(newBytesColor);
    emit bytesColorChanged();
}

QColor QHexView::bytesBackground() const {
    return m_renderer->bytesBackground();
}

void QHexView::setBytesBackground(const QColor &newBytesBackground) {
    m_renderer->setBytesBackground(newBytesBackground);
    emit bytesBackgroundChanged();
}

QColor QHexView::addressColor() const { return m_renderer->addressColor(); }

void QHexView::setAddressColor(const QColor &newAddressColor) {
    m_renderer->setAddressColor(newAddressColor);
    emit addressColorChanged();
}

QColor QHexView::headerColor() const { return m_renderer->headerColor(); }

void QHexView::setHeaderColor(const QColor &newHeaderColor) {
    m_renderer->setHeaderColor(newHeaderColor);
    emit headerColorChanged();
}

void QHexView::mousePressEvent(QMouseEvent *e) {
    QAbstractScrollArea::mousePressEvent(e);

    if (!m_renderer || (e->buttons() != Qt::LeftButton))
        return;

    QHexPosition position;
    QPoint abspos = absolutePosition(e->pos());

    if (!m_renderer->hitTest(abspos, &position, this->firstVisibleLine()))
        return;

    m_renderer->selectArea(abspos);

    if (m_renderer->editableArea(m_renderer->selectedArea())) {
        auto m = getSelectionMode();
        bool clearSelection = false;
        if (m == QHexCursor::SelectionNormal) {
            clearSelection = m_cursor->selectionCount() <= 1 ||
                             e->modifiers().testFlag(Qt::ControlModifier);
        } else if (m == QHexCursor::SelectionSingle) {
            clearSelection = true;
        }

        m_cursor->moveTo(position, clearSelection);
    }

    e->accept();
}

void QHexView::mouseMoveEvent(QMouseEvent *e) {
    QAbstractScrollArea::mouseMoveEvent(e);
    if (!m_renderer || !m_document)
        return;

    QPoint abspos = absolutePosition(e->pos());

    if (e->buttons() == Qt::LeftButton) {
        if (m_blinktimer->isActive()) {
            m_blinktimer->stop();
            m_renderer->enableCursor(false);
        }

        QHexCursor *cursor = m_cursor;
        QHexPosition position;

        if (!m_renderer->hitTest(abspos, &position, this->firstVisibleLine()))
            return;

        cursor->select(position.line, position.column,
                       QHexCursor::SelectionModes(
                           getSelectionMode() | QHexCursor::SelectionPreview));
        e->accept();
    }

    if (e->buttons() != Qt::NoButton)
        return;

    int hittest = m_renderer->hitTestArea(abspos);

    if (m_renderer->editableArea(hittest))
        this->setCursor(Qt::IBeamCursor);
    else
        this->setCursor(Qt::ArrowCursor);
}

void QHexView::mouseReleaseEvent(QMouseEvent *e) {
    QAbstractScrollArea::mouseReleaseEvent(e);
    if (e->button() != Qt::LeftButton)
        return;
    if (!m_blinktimer->isActive())
        m_blinktimer->start();

    if (m_cursor->hasPreviewSelection()) {
        auto sel = m_cursor->previewSelection();
        m_cursor->mergePreviewSelection();
        m_cursor->moveTo(sel.end);
    }

    e->accept();
}

void QHexView::focusInEvent(QFocusEvent *e) {
    QAbstractScrollArea::focusInEvent(e);
    if (!m_renderer)
        return;

    m_renderer->enableCursor();
    m_blinktimer->start();
}

void QHexView::focusOutEvent(QFocusEvent *e) {
    QAbstractScrollArea::focusOutEvent(e);
    if (!m_renderer)
        return;

    m_blinktimer->stop();
    m_renderer->enableCursor(true); // modified by wingsummer : false -> true
}

void QHexView::wheelEvent(QWheelEvent *e) {
    if (qApp->keyboardModifiers() == Qt::ControlModifier) {
        auto dela = e->angleDelta().y() / 1200.0 / 2;
        setScaleRate(scaleRate() + dela);
        return;
    }

    if (e->angleDelta().y() == 0) {
        int value = this->verticalScrollBar()->value();

        if (e->angleDelta().x() < 0) // Scroll Down
            this->verticalScrollBar()->setValue(value + DOCUMENT_WHEEL_LINES);
        else if (e->angleDelta().x() > 0) // Scroll Up
            this->verticalScrollBar()->setValue(value - DOCUMENT_WHEEL_LINES);

        return;
    }

    QAbstractScrollArea::wheelEvent(e);
}

void QHexView::resizeEvent(QResizeEvent *e) {
    QAbstractScrollArea::resizeEvent(e);
    this->adjustScrollBars();
}

void QHexView::paintEvent(QPaintEvent *e) {
    if (!m_document)
        return;

    QPainter painter(this->viewport());
    painter.setFont(this->font());

    const QRect &r = e->rect();

    const qsizetype firstVisible = this->firstVisibleLine();
    const int lineHeight = m_renderer->lineHeight();
    const int headerCount = m_renderer->headerLineCount();

    // these are lines from the point of view of the visible rect
    // where the first "headerCount" are taken by the header
    const int first = (r.top() / lineHeight);              // included
    const int lastPlusOne = (r.bottom() / lineHeight) + 1; // excluded

    // compute document lines, adding firstVisible and removing the header
    // the max is necessary if the rect covers the header
    const qsizetype begin = firstVisible + std::max(first - headerCount, 0);
    const qsizetype end = firstVisible + std::max(lastPlusOne - headerCount, 0);

    painter.save();
    painter.translate(-this->horizontalScrollBar()->value(), 0);
    m_renderer->render(&painter, begin, end, firstVisible);
    m_renderer->renderFrame(&painter);
    painter.restore();
}

void QHexView::moveToSelection() {
    QHexCursor *cur = m_cursor;

    if (!this->isLineVisible(cur->currentLine())) {
        QScrollBar *vscrollbar = this->verticalScrollBar();
        int scrollPos = static_cast<int>(
            std::max(qsizetype(0),
                     cur->currentLine() - this->visibleLines() / 2) /
            documentSizeFactor());
        vscrollbar->setValue(scrollPos);
    } else
        this->viewport()->update();

    emit cursorLocationChanged(); // added by wingsummer
}

void QHexView::blinkCursor() {
    if (!m_renderer)
        return;
    m_renderer->blinkCursor();
    this->renderCurrentLine();
}

void QHexView::moveNext(bool select) {
    QHexCursor *cur = m_cursor;
    auto line = cur->currentLine();
    auto column = cur->currentColumn();
    bool lastcell = (line >= m_renderer->documentLastLine()) &&
                    (column >= m_renderer->documentLastColumn());

    if ((m_renderer->selectedArea() == QHexRenderer::AsciiArea) && lastcell)
        return;

    int nibbleindex = cur->currentNibble();

    if (m_renderer->selectedArea() == QHexRenderer::HexArea) {
        if (lastcell && !nibbleindex)
            return;

        if (cur->position().offset() >= m_document->length() && nibbleindex)
            return;
    }

    if ((m_renderer->selectedArea() == QHexRenderer::HexArea)) {
        nibbleindex++;
        nibbleindex %= 2;

        if (nibbleindex)
            column++;
    } else {
        nibbleindex = 1;
        column++;
    }

    if (column > m_renderer->hexLineWidth() - 1) {
        line = std::min(m_renderer->documentLastLine(), line + 1);
        column = 0;
        nibbleindex = 1;
    }

    if (select)
        cur->select(line, std::min(m_renderer->hexLineWidth() - 1, int(column)),
                    QHexCursor::SelectionAdd);

    cur->moveTo(line, std::min(m_renderer->hexLineWidth() - 1, int(column)),
                nibbleindex);
}

void QHexView::movePrevious(bool select) {
    QHexCursor *cur = m_cursor;
    auto line = cur->currentLine();
    auto column = cur->currentColumn();
    bool firstcell = !line && !column;

    if ((m_renderer->selectedArea() == QHexRenderer::AsciiArea) && firstcell)
        return;

    int nibbleindex = cur->currentNibble();

    if ((m_renderer->selectedArea() == QHexRenderer::HexArea) && firstcell &&
        nibbleindex)
        return;

    if ((m_renderer->selectedArea() == QHexRenderer::HexArea)) {
        nibbleindex--;
        nibbleindex %= 2;
        if (!nibbleindex)
            column--;
    } else {
        nibbleindex = 1;
        column--;
    }

    if (column < 0) {
        line = std::max(qsizetype(0), line - 1);
        column = m_renderer->hexLineWidth() - 1;
        nibbleindex = 0;
    }

    if (select)
        cur->select(line, std::max(0, column), QHexCursor::SelectionAdd);

    cur->moveTo(line, std::max(0, column), nibbleindex);
}

QHexCursor::SelectionMode QHexView::getSelectionMode() const {
    auto mods = qApp->keyboardModifiers();

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

void QHexView::renderCurrentLine() {
    if (m_document)
        this->renderLine(m_cursor->currentLine());
}

bool QHexView::processAction(QHexCursor *cur, QKeyEvent *e) {
    auto canFlag = !(isReadOnly() || isLocked());

    if (e->modifiers() != Qt::NoModifier) {
        if (e->matches(QKeySequence::SelectAll)) {
            m_cursor->moveTo(0, 0);
            m_cursor->select(m_renderer->documentLastLine(),
                             m_renderer->documentLastColumn() - 1);
        } else {
            if (canFlag) {
                if (e->matches(QKeySequence::Undo))
                    m_document->undo();
                else if (e->matches(QKeySequence::Redo))
                    m_document->redo();
                else if (e->matches(QKeySequence::Cut))
                    this->Cut(
                        (m_renderer->selectedArea() == QHexRenderer::HexArea));
                else if (e->matches(QKeySequence::Copy))
                    this->copy(
                        (m_renderer->selectedArea() == QHexRenderer::HexArea));
                else if (e->matches(QKeySequence::Paste))
                    this->Paste(
                        (m_renderer->selectedArea() == QHexRenderer::HexArea));
                else
                    return false;

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

            if (pos.offset() <= 0)
                return true;

            // modified by wingsummer
            if (isKeepSize()) {
                if (cur->insertionMode() == QHexCursor::OverwriteMode) {
                    if (e->key() == Qt::Key_Backspace)
                        m_document->Replace(m_cursor,
                                            cur->position().offset() - 1,
                                            uchar(0), 0);
                    else
                        m_document->Replace(m_cursor, cur->position().offset(),
                                            uchar(0), 0);
                }
            } else {
                if (e->key() == Qt::Key_Backspace)
                    m_document->Remove(m_cursor, cur->position().offset() - 1,
                                       1, 1);
                else
                    m_document->Remove(m_cursor, cur->position().offset(), 1,
                                       0);
            }

        } else {
            this->RemoveSelection();
        }

    } else if (e->key() == Qt::Key_Insert) {
        if (!isKeepSize()) {
            cur->switchInsertionMode();
        }
    } else
        return false;

    return true;
}

bool QHexView::processMove(QHexCursor *cur, QKeyEvent *e) {
    if (e->matches(QKeySequence::MoveToNextChar) ||
        e->matches(QKeySequence::SelectNextChar))
        this->moveNext(e->matches(QKeySequence::SelectNextChar));
    else if (e->matches(QKeySequence::MoveToPreviousChar) ||
             e->matches(QKeySequence::SelectPreviousChar))
        this->movePrevious(e->matches(QKeySequence::SelectPreviousChar));
    else if (e->matches(QKeySequence::MoveToNextLine) ||
             e->matches(QKeySequence::SelectNextLine)) {
        if (m_renderer->documentLastLine() == cur->currentLine())
            return true;

        auto nextline = cur->currentLine() + 1;

        if (e->matches(QKeySequence::MoveToNextLine))
            cur->moveTo(nextline, cur->currentColumn());
        else
            cur->select(nextline, cur->currentColumn());
    } else if (e->matches(QKeySequence::MoveToPreviousLine) ||
               e->matches(QKeySequence::SelectPreviousLine)) {
        if (!cur->currentLine())
            return true;

        auto prevline = cur->currentLine() - 1;

        if (e->matches(QKeySequence::MoveToPreviousLine))
            cur->moveTo(prevline, cur->currentColumn());
        else
            cur->select(prevline, cur->currentColumn());
    } else if (e->matches(QKeySequence::MoveToNextPage) ||
               e->matches(QKeySequence::SelectNextPage)) {
        if (m_renderer->documentLastLine() == cur->currentLine())
            return true;

        auto pageline = std::min(m_renderer->documentLastLine(),
                                 cur->currentLine() + this->visibleLines());

        if (e->matches(QKeySequence::MoveToNextPage))
            cur->moveTo(pageline, cur->currentColumn());
        else
            cur->select(pageline, cur->currentColumn());
    } else if (e->matches(QKeySequence::MoveToPreviousPage) ||
               e->matches(QKeySequence::SelectPreviousPage)) {
        if (!cur->currentLine())
            return true;

        auto pageline =
            std::max(qsizetype(0), cur->currentLine() - this->visibleLines());

        if (e->matches(QKeySequence::MoveToPreviousPage))
            cur->moveTo(pageline, cur->currentColumn());
        else
            cur->select(pageline, cur->currentColumn());
    } else if (e->matches(QKeySequence::MoveToStartOfDocument) ||
               e->matches(QKeySequence::SelectStartOfDocument)) {
        if (!cur->currentLine())
            return true;

        if (e->matches(QKeySequence::MoveToStartOfDocument))
            cur->moveTo(0, 0);
        else
            cur->select(0, 0);
    } else if (e->matches(QKeySequence::MoveToEndOfDocument) ||
               e->matches(QKeySequence::SelectEndOfDocument)) {
        if (m_renderer->documentLastLine() == cur->currentLine())
            return true;

        if (e->matches(QKeySequence::MoveToEndOfDocument))
            cur->moveTo(m_renderer->documentLastLine(),
                        int(m_renderer->documentLastColumn()));
        else
            cur->select(m_renderer->documentLastLine(),
                        m_renderer->documentLastColumn());
    } else if (e->matches(QKeySequence::MoveToStartOfLine) ||
               e->matches(QKeySequence::SelectStartOfLine)) {
        if (e->matches(QKeySequence::MoveToStartOfLine))
            cur->moveTo(cur->currentLine(), 0);
        else
            cur->select(cur->currentLine(), 0);
    } else if (e->matches(QKeySequence::MoveToEndOfLine) ||
               e->matches(QKeySequence::SelectEndOfLine)) {
        if (e->matches(QKeySequence::MoveToEndOfLine)) {
            if (cur->currentLine() == m_renderer->documentLastLine())
                cur->moveTo(cur->currentLine(),
                            int(m_renderer->documentLastColumn()));
            else
                cur->moveTo(cur->currentLine(), m_renderer->hexLineWidth() - 1,
                            0);
        } else {
            if (cur->currentLine() == m_renderer->documentLastLine())
                cur->select(cur->currentLine(),
                            m_renderer->documentLastColumn());
            else
                cur->select(cur->currentLine(), m_renderer->hexLineWidth() - 1);
        }
    } else
        return false;

    return true;
}

bool QHexView::processTextInput(QHexCursor *cur, QKeyEvent *e) {
    if (isReadOnly() || isLocked() || (e->modifiers() & Qt::ControlModifier))
        return false;

    auto text = e->text();
    if (text.isEmpty()) {
        return false;
    }

    uchar key = static_cast<uchar>(text[0].toLatin1());

    if ((m_renderer->selectedArea() == QHexRenderer::HexArea)) {
        if (!((key >= '0' && key <= '9') ||
              (key >= 'a' && key <= 'f'))) // Check if is a Hex Char
            return false;

        uchar val = uchar(QString(char(key)).toUInt(nullptr, 16));
        this->RemoveSelection();

        if (this->atEnd() || (cur->currentNibble() && !isKeepSize() &&
                              cur->insertionMode() == QHexCursor::InsertMode)) {
            m_document->Insert(cur, cur->position().offset(), uchar(val << 4),
                               1); // X0 byte
            return true;
        }

        uchar ch = uchar(m_document->at(int(cur->position().offset())));

        if (cur->currentNibble()) // X0
            val = uchar((ch & 0x0F) | (val << 4));
        else // 0X
            val = (ch & 0xF0) | val;

        m_document->Replace(cur, cur->position().offset(), val,
                            cur->currentNibble());
        return true;
    }

    if ((m_renderer->selectedArea() == QHexRenderer::AsciiArea)) {
        if (!(key >= 0x20 && key <= 0x7E)) // Check if is a Printable Char
            return false;

        this->RemoveSelection();

        if (!this->atEnd() &&
            (cur->insertionMode() == QHexCursor::OverwriteMode))
            m_document->Replace(cur, cur->position().offset(), key, 1);
        else
            m_document->Insert(cur, cur->position().offset(), key, 1);

        QKeyEvent keyevent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
        qApp->sendEvent(this, &keyevent);
        return true;
    }

    return false;
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
    int documentWidth = m_renderer->documentWidth();
    int viewportWidth = viewport()->width();

    if (documentWidth > viewportWidth) {
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        // +1 to see the rightmost vertical line, +2 seems more pleasant to the
        // eyes
        hscrollbar->setMaximum(documentWidth - viewportWidth + 2);
    } else {
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        hscrollbar->setValue(0);
        hscrollbar->setMaximum(documentWidth);
    }
}

void QHexView::renderLine(qsizetype line) {
    if (!this->isLineVisible(line))
        return;
    this->viewport()->update(
        /*m_renderer->getLineRect(line, this->firstVisibleLine())*/);
}

qsizetype QHexView::firstVisibleLine() const {
    return qsizetype(this->verticalScrollBar()->value()) * documentSizeFactor();
}
qsizetype QHexView::lastVisibleLine() const {
    return this->firstVisibleLine() + this->visibleLines() - 1;
}

qsizetype QHexView::visibleLines() const {
    auto visLines =
        qsizetype(std::ceil(this->height() / m_renderer->lineHeight() -
                            m_renderer->headerLineCount()));
    return std::min(visLines, m_renderer->documentLines());
}

bool QHexView::isLineVisible(qsizetype line) const {
    if (!m_document)
        return false;
    if (line < this->firstVisibleLine())
        return false;
    if (line > this->lastVisibleLine())
        return false;
    return true;
}

int QHexView::documentSizeFactor() const {
    int factor = 1;

    if (m_document) {
        auto docLines = m_renderer->documentLines();
        if (docLines >= INT_MAX)
            factor = int(docLines / INT_MAX) + 1;
    }

    return factor;
}
