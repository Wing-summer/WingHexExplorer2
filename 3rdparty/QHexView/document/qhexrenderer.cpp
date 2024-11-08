#include "qhexrenderer.h"
#include "document/commands/encodingchangecommand.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QTextCursor>
#include <QWidget>
#include <cctype>
#include <cwctype>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

#define HEX_UNPRINTABLE_CHAR '.'

/*===================================*/
// added by wingsummer

bool QHexRenderer::stringVisible() { return m_asciiVisible; }

void QHexRenderer::setStringVisible(bool b) {
    m_asciiVisible = b;
    emit m_document->documentChanged();
}

bool QHexRenderer::headerVisible() { return m_headerVisible; }

void QHexRenderer::setHeaderVisible(bool b) {
    m_headerVisible = b;
    emit m_document->documentChanged();
}

bool QHexRenderer::addressVisible() { return m_addressVisible; }

void QHexRenderer::setAddressVisible(bool b) {
    m_addressVisible = b;
    emit m_document->documentChanged();
}

QString QHexRenderer::encoding() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (m_encoding.compare(QStringLiteral("ASCII"), Qt::CaseInsensitive))
        return m_encoding;
    return QStringLiteral("ISO-8859-1");
#else
    return m_encoding;
#endif
}

void QHexRenderer::SetEncoding(QString encoding) {
    m_document->addUndoCommand(
        new EncodingChangeCommand(this, m_encoding, encoding));
}

void QHexRenderer::switchDoc(QHexDocument *doc) {
    if (doc)
        m_document = doc;
}

bool QHexRenderer::setEncoding(const QString &encoding) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    auto enc = encoding;
    if (encoding.compare(QStringLiteral("ISO-8859-1"), Qt::CaseInsensitive) ==
        0) {
        m_encoding = QStringLiteral("ASCII");
        emit m_document->documentChanged();
        return true;
    }
    if (QStringConverter::encodingForName(enc.toUtf8())) {
#else
    if (QTextCodec::codecForName(encoding.toUtf8())) {
#endif
        m_encoding = encoding;
        emit m_document->documentChanged();
        return true;
    }
    return false;
}
/*===================================*/

QHexRenderer::QHexRenderer(QHexDocument *document, QHexCursor *cursor,
                           const QFontMetricsF &fontmetrics, QObject *parent)
    : QObject(parent), m_document(document), m_cursor(cursor),
      m_fontmetrics(fontmetrics), m_encoding("ASCII") {
    m_selectedarea = QHexRenderer::HexArea;
    m_cursorenabled = false;

    /*===================================*/
    // added by wingsummer

    m_asciiVisible = true;
    m_addressVisible = true;
    m_headerVisible = true;

    /*===================================*/
}

// modified by wingsummer
void QHexRenderer::renderFrame(QPainter *painter) {
    QRect rect = painter->window();
    int hexx = this->getHexColumnX();
    int asciix = this->getAsciiColumnX();
    int endx = this->getEndColumnX();

    // x coordinates are in absolute space
    // y coordinates are in viewport space
    // see QHexView::paintEvent where the painter has been shifted horizontally

    if (m_headerVisible)
        painter->drawLine(0, this->headerLineCount() * this->lineHeight() - 1,
                          endx,
                          this->headerLineCount() * this->lineHeight() - 1);
    if (m_addressVisible)
        painter->drawLine(hexx, rect.top(), hexx, rect.bottom());

    painter->drawLine(asciix, rect.top(), asciix, rect.bottom());

    if (m_asciiVisible)
        painter->drawLine(endx, rect.top(), endx, rect.bottom());
}

// modified by wingsummer
void QHexRenderer::render(QPainter *painter, qsizetype begin, qsizetype end,
                          qsizetype firstline) {
    if (m_headerVisible)
        this->drawHeader(painter);

    auto documentLines = this->documentLines();
    for (qsizetype line = begin; line < std::min(end, documentLines); line++) {
        QRect linerect = this->getLineRect(line, firstline);
        if (line % 2)
            painter->fillRect(linerect, m_bytesBackground);
        else
            painter->fillRect(linerect, m_bytesAlterBackground);

        if (m_addressVisible)
            this->drawAddress(painter, linerect, line);

        this->drawHex(painter, linerect, line);

        if (m_asciiVisible)
            this->drawString(painter, linerect, line, m_encoding);
    }
}

void QHexRenderer::updateMetrics(const QFontMetricsF &fm) {
    m_fontmetrics = fm;
}
void QHexRenderer::enableCursor(bool b) { m_cursorenabled = b; }

void QHexRenderer::selectArea(const QPoint &pt) {
    int area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return;

    m_selectedarea = area;
}

bool QHexRenderer::hitTest(const QPoint &pt, QHexPosition *position,
                           qsizetype firstline) const {
    int area = this->hitTestArea(pt);
    if (!this->editableArea(area))
        return false;

    position->line =
        std::min(firstline + pt.y() / this->lineHeight() - headerLineCount(),
                 this->documentLastLine());
    position->lineWidth = quint8(this->hexLineWidth());

    if (area == QHexRenderer::HexArea) {
        int relx = pt.x() - this->getHexColumnX() - this->borderSize();
        int column = int(relx / this->getCellWidth());
        position->column = column / 3;
        // first char is nibble 1, 2nd and space are 0
        position->nibbleindex = (column % 3 == 0) ? 1 : 0;
    } else {
        int relx = pt.x() - this->getAsciiColumnX() - this->borderSize();
        position->column = int(relx / this->getCellWidth());
        position->nibbleindex = 1;
    }

    if (position->line == this->documentLastLine()) // Check last line's columns
    {
        QByteArray ba = this->getLine(position->line);
        position->column = std::min(position->column, int(ba.length()));
    } else
        position->column = std::min(position->column, int(hexLineWidth() - 1));

    return true;
}

int QHexRenderer::hitTestArea(const QPoint &pt) const {
    if (pt.y() < headerLineCount() * lineHeight())
        return QHexRenderer::HeaderArea;

    if ((pt.x() >= this->borderSize()) &&
        (pt.x() <= (this->getHexColumnX() - this->borderSize())))
        return QHexRenderer::AddressArea;

    if ((pt.x() > (this->getHexColumnX() + this->borderSize())) &&
        (pt.x() < (this->getAsciiColumnX() - this->borderSize())))
        return QHexRenderer::HexArea;

    if ((pt.x() > (this->getAsciiColumnX() + this->borderSize())) &&
        (pt.x() < (this->getEndColumnX() - this->borderSize())))
        return QHexRenderer::AsciiArea;

    return QHexRenderer::ExtraArea;
}

int QHexRenderer::selectedArea() const { return m_selectedarea; }
bool QHexRenderer::editableArea(int area) const {
    return (area == QHexRenderer::HexArea || area == QHexRenderer::AsciiArea);
}
qsizetype QHexRenderer::documentLastLine() const {
    return this->documentLines() - 1;
}
qsizetype QHexRenderer::documentLastColumn() const {
    return this->getLine(this->documentLastLine()).length();
}
qsizetype QHexRenderer::documentLines() const {
    auto r = std::lldiv(this->rendererLength(), hexLineWidth());
    return r.quot + (r.rem > 0 ? 1 : 0);
}
int QHexRenderer::documentWidth() const { return this->getEndColumnX(); }
int QHexRenderer::lineHeight() const { return qRound(m_fontmetrics.height()); }

QRect QHexRenderer::getLineRect(qsizetype line, qsizetype firstline) const {
    return QRect(0, int((line - firstline + headerLineCount()) * lineHeight()),
                 this->getEndColumnX(), lineHeight());
}

// modified by wingsummer
int QHexRenderer::headerLineCount() const { return m_headerVisible ? 1 : 0; }

int QHexRenderer::borderSize() const {
    if (m_document)
        return this->getNCellsWidth(m_document->areaIndent());
    return this->getNCellsWidth(DEFAULT_AREA_IDENTATION);
}

int QHexRenderer::hexLineWidth() const {
    if (m_document)
        return m_document->hexLineWidth();
    return DEFAULT_HEX_LINE_LENGTH;
}

QString QHexRenderer::hexString(qsizetype line, QByteArray *rawline) const {
    QByteArray lrawline = this->getLine(line);
    if (rawline)
        *rawline = lrawline;

    return lrawline.toHex(' ').toUpper() + QStringLiteral(" ");
}

// modified by wingsummer
QString QHexRenderer::decodeString(qsizetype line, QString encoding,
                                   QByteArray *rawline) const {
    QByteArray lrawline = this->getLine(line);
    if (rawline)
        *rawline = lrawline;

    if (encoding.toLower() == "ascii") {
        QByteArray ascii = lrawline;
        this->unprintableChars(ascii);
        return ascii;
    } else {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto enc = QStringConverter::encodingForName(encoding.toUtf8());
        if (enc) {
            QStringDecoder decoder(enc.value());
            auto unicode = decoder.decode(lrawline);
            QString data = unicode.data;
            this->unprintableWChars(data);
            return data;
        } else {
            // Handle the case where the encoding is not recognized
            auto unicode = QString::fromUtf8(lrawline); // Fallback to UTF-8
            this->unprintableWChars(unicode);
            return unicode;
        }
#else
        auto enc = QTextCodec::codecForName(encoding.toUtf8());
        auto d = enc->makeDecoder();
        auto unicode = d->toUnicode(lrawline);
        this->unprintableWChars(unicode);
        return unicode;
#endif
    }
}

QByteArray QHexRenderer::getLine(qsizetype line) const {
    return m_document->read(line * hexLineWidth(), hexLineWidth());
}

void QHexRenderer::blinkCursor() { m_cursorenabled = !m_cursorenabled; }

qsizetype QHexRenderer::rendererLength() const {
    return m_document->length() + 1;
}

// modified by wingsummer
int QHexRenderer::getAddressWidth() const {
    auto base = m_document->baseAddress();
    quint64 maxAddr = base + quint64(this->rendererLength());
    if (base <= 0xFFFFFFFF && maxAddr <= 0xFFFFFFFF)
        return 8;
    else
        return 16;
    return QString::number(maxAddr, 16).length();
}

int QHexRenderer::getHexColumnX() const {
    if (m_addressVisible) {
        return this->getNCellsWidth(this->getAddressWidth()) +
               2 * this->borderSize();
    }
    return 0;
}
int QHexRenderer::getAsciiColumnX() const {
    return this->getHexColumnX() + this->getNCellsWidth(hexLineWidth() * 3) +
           2 * this->borderSize();
}

int QHexRenderer::getEndColumnX() const {
    if (m_asciiVisible) {
        return this->getAsciiColumnX() + this->getNCellsWidth(hexLineWidth()) +
               2 * this->borderSize();
    }
    return this->getAsciiColumnX();
}

qreal QHexRenderer::getCellWidth() const {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return m_fontmetrics.horizontalAdvance(" ");
#else
    return m_fontmetrics.width(" ");
#endif
}

int QHexRenderer::getNCellsWidth(int n) const {
    return qRound(n * getCellWidth());
}

void QHexRenderer::unprintableChars(QByteArray &ascii) const {
    for (char &ch : ascii) {
        if (std::isprint(static_cast<unsigned char>(ch)))
            continue;

        ch = HEX_UNPRINTABLE_CHAR;
    }
}

// added by wingsummer
void QHexRenderer::unprintableWChars(QString &unicode) const {
    for (QChar &ch : unicode) {
        if (std::iswprint(ch.unicode()))
            continue;
        ch = HEX_UNPRINTABLE_CHAR;
    }
}

void QHexRenderer::applyDocumentStyles(QPainter *painter,
                                       QTextDocument *textdocument) const {
    textdocument->setDocumentMargin(0);
    textdocument->setUndoRedoEnabled(false);
    textdocument->setDefaultFont(painter->font());
}

void QHexRenderer::applyBasicStyle(QTextCursor &textcursor,
                                   const QByteArray &rawline,
                                   Factor factor) const {
    QColor color = m_bytesColor;

    if (color.lightness() < 50) {
        if (color == Qt::black)
            color = Qt::gray;
        else
            color = color.darker();
    } else
        color = color.lighter();

    QTextCharFormat charformat;
    charformat.setForeground(color);

    for (int i = 0; i < rawline.length(); i++) {
        if ((rawline[i] != 0x00) && (uchar(rawline[i]) != 0xFF))
            continue;

        textcursor.setPosition(i * factor);
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                factor);
        textcursor.mergeCharFormat(charformat);
    }
}

void QHexRenderer::applyMetadata(QTextCursor &textcursor, qsizetype line,
                                 Factor factor) const {
    QHexMetadata *metadata = m_document->metadata();

    if (!metadata->lineHasMetadata(line))
        return;

    const QHexLineMetadata &linemetadata = metadata->get(line);
    for (auto &mi : linemetadata) {
        QTextCharFormat charformat;
        if (m_document->metabgVisible() && mi.background.isValid() &&
            mi.background.rgba())
            charformat.setBackground(mi.background);
        if (m_document->metafgVisible() && mi.foreground.isValid() &&
            mi.foreground.rgba())
            charformat.setForeground(mi.foreground);
        if (m_document->metaCommentVisible() && !mi.comment.isEmpty())
            charformat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

        textcursor.setPosition(int(mi.start * factor));
        textcursor.movePosition(
            QTextCursor::Right, QTextCursor::KeepAnchor,
            int((mi.length * factor) - (factor > 1 ? 1 : 0)));
        textcursor.mergeCharFormat(charformat);
    }
}

void QHexRenderer::applySelection(QTextCursor &textcursor, qsizetype line,
                                  Factor factor) const {
    if (!m_cursor->isLineSelected(line))
        return;

    const QHexPosition &startsel = m_cursor->selectionStart();
    const QHexPosition &endsel = m_cursor->selectionEnd();

    if (startsel.line == endsel.line) {
        textcursor.setPosition(startsel.column * factor);
        textcursor.movePosition(
            QTextCursor::Right, QTextCursor::KeepAnchor,
            ((endsel.column - startsel.column + 1) * factor));
    } else {
        if (line == startsel.line)
            textcursor.setPosition(startsel.column * factor);
        else
            textcursor.setPosition(0);

        if (line == endsel.line)
            textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                    ((endsel.column + 1) * factor));
        else
            textcursor.movePosition(QTextCursor::EndOfLine,
                                    QTextCursor::KeepAnchor);
    }

    if (factor == Hex)
        textcursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);

    QTextCharFormat charformat;
    charformat.setBackground(m_selBackgroundColor);
    charformat.setForeground(m_selectionColor);
    textcursor.mergeCharFormat(charformat);
}

void QHexRenderer::applyCursorAscii(QTextCursor &textcursor,
                                    qsizetype line) const {
    if ((line != m_cursor->currentLine()) || !m_cursorenabled)
        return;

    textcursor.clearSelection();
    textcursor.setPosition(m_cursor->currentColumn());
    textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

    QTextCharFormat charformat;

    if ((m_cursor->insertionMode() == QHexCursor::OverwriteMode) ||
        (m_selectedarea != QHexRenderer::AsciiArea)) {
        charformat.setForeground(m_bytesBackground);
        if (m_selectedarea == QHexRenderer::AsciiArea)
            charformat.setBackground(m_bytesColor);
        else
            charformat.setBackground(m_bytesColor.lighter(250));
    } else
        charformat.setUnderlineStyle(
            QTextCharFormat::UnderlineStyle::SingleUnderline);

    textcursor.mergeCharFormat(charformat);
}

void QHexRenderer::applyCursorHex(QTextCursor &textcursor,
                                  qsizetype line) const {
    if ((line != m_cursor->currentLine()) || !m_cursorenabled)
        return;

    textcursor.clearSelection();
    textcursor.setPosition(m_cursor->currentColumn() * 3);

    if ((m_selectedarea == QHexRenderer::HexArea) && !m_cursor->currentNibble())
        textcursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);

    textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);

    if (m_selectedarea == QHexRenderer::AsciiArea)
        textcursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    QTextCharFormat charformat;

    if ((m_cursor->insertionMode() == QHexCursor::OverwriteMode) ||
        (m_selectedarea != QHexRenderer::HexArea)) {
        charformat.setForeground(m_bytesBackground);
        if (m_selectedarea == QHexRenderer::HexArea)
            charformat.setBackground(m_bytesColor);
        else
            charformat.setBackground(m_bytesColor.lighter(250));
    } else
        charformat.setUnderlineStyle(
            QTextCharFormat::UnderlineStyle::SingleUnderline);

    textcursor.setCharFormat(charformat);
}

void QHexRenderer::drawAddress(QPainter *painter, const QRect &linerect,
                               qsizetype line) {
    auto addr = quintptr(line * hexLineWidth()) + m_document->baseAddress();
    QString addrStr =
        QString::number(addr, 16)
            .rightJustified(this->getAddressWidth(), QLatin1Char('0'))
            .toUpper();

    QRect addressrect = linerect;
    addressrect.setWidth(this->getHexColumnX());

    painter->save();
    painter->setPen(m_addressColor);
    painter->drawText(addressrect, Qt::AlignHCenter | Qt::AlignVCenter,
                      addrStr);
    painter->restore();
}

void QHexRenderer::drawHex(QPainter *painter, const QRect &linerect,
                           qsizetype line) {
    QTextDocument textdocument;
    QTextCursor textcursor(&textdocument);
    QByteArray rawline;

    textcursor.insertText(this->hexString(line, &rawline));

    if (line == this->documentLastLine())
        textcursor.insertText(" ");

    QRect hexrect = linerect;
    hexrect.setX(this->getHexColumnX() + this->borderSize());

    this->applyDocumentStyles(painter, &textdocument);
    this->applyBasicStyle(textcursor, rawline, Hex);

    auto dis = !m_document->metabgVisible() && !m_document->metafgVisible() &&
               !m_document->metaCommentVisible();
    if (!dis)
        this->applyMetadata(textcursor, line, Hex);

    this->applyBookMark(textcursor, line, Hex);
    this->applySelection(textcursor, line, Hex);
    this->applyCursorHex(textcursor, line);

    painter->save();
    painter->translate(hexrect.topLeft());

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette.setColor(QPalette::Text, m_bytesColor);
    textdocument.documentLayout()->draw(painter, ctx);

    painter->restore();
}

void QHexRenderer::applyBookMark(QTextCursor &textcursor, qsizetype line,
                                 Factor factor) {

    if (!m_document->lineHasBookMark(line))
        return;

    auto pos = m_document->getLineBookmarksPos(line);
    for (auto item : pos) {
        textcursor.setPosition(int((item % hexLineWidth()) * factor) + 2);
        auto charformat = textcursor.charFormat();
        textcursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor,
                                factor - 1);
        charformat.setFontOverline(true);

        if (charformat.underlineStyle() ==
            QTextCharFormat::UnderlineStyle::NoUnderline) {
            charformat.setUnderlineStyle(
                QTextCharFormat::UnderlineStyle::DashUnderline);
        } else {
            charformat.setUnderlineStyle(
                QTextCharFormat::UnderlineStyle::WaveUnderline);
        }

        textcursor.setCharFormat(charformat);
    }
}

void QHexRenderer::drawString(QPainter *painter, const QRect &linerect,
                              qsizetype line, QString encoding) {
    QTextDocument textdocument;
    QTextCursor textcursor(&textdocument);
    QByteArray rawline;
    // modified by wingsummer
    textcursor.insertText(this->decodeString(line, encoding, &rawline));

    if (line == this->documentLastLine())
        textcursor.insertText(" ");

    QRect asciirect = linerect;
    asciirect.setX(this->getAsciiColumnX() + this->borderSize());

    this->applyDocumentStyles(painter, &textdocument);
    this->applyBasicStyle(textcursor, rawline, String);

    auto dis = !m_document->metabgVisible() && !m_document->metafgVisible() &&
               !m_document->metaCommentVisible();
    if (!dis)
        this->applyMetadata(textcursor, line, String);

    this->applySelection(textcursor, line, String);
    this->applyCursorAscii(textcursor, line);

    painter->save();
    painter->translate(asciirect.topLeft());

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette.setColor(QPalette::Text, m_bytesColor);
    textdocument.documentLayout()->draw(painter, ctx);

    painter->restore();
}

void QHexRenderer::drawHeader(QPainter *painter) {
    QRect rect = QRect(0, 0, this->getEndColumnX(),
                       this->headerLineCount() * this->lineHeight());
    QString hexheader;

    for (quint8 i = 0; i < this->hexLineWidth(); i++)
        hexheader.append(
            QString("%1 ")
                .arg(QString::number(i, 16).rightJustified(2, QChar('0')))
                .toUpper());

    QRect addressrect = rect;
    addressrect.setWidth(this->getHexColumnX());

    QRect hexrect = rect;

    hexrect.setX(m_addressVisible ? this->getHexColumnX() + this->borderSize()
                                  : this->borderSize());
    hexrect.setWidth(this->getNCellsWidth(hexLineWidth() * 3));

    QRect asciirect = rect;
    asciirect.setX(this->getAsciiColumnX());
    asciirect.setWidth(this->getEndColumnX() - this->getAsciiColumnX());

    painter->save();
    painter->setPen(m_headerColor);

    if (m_addressVisible)
        painter->drawText(addressrect, Qt::AlignHCenter | Qt::AlignVCenter,
                          QString("Offset"));

    // align left for maximum consistency with drawHex() which prints from the
    // left. so hex and positions are aligned vertically
    painter->drawText(hexrect, Qt::AlignLeft | Qt::AlignVCenter, hexheader);

    if (m_asciiVisible)
        painter->drawText(asciirect, Qt::AlignHCenter | Qt::AlignVCenter,
                          m_encoding);
    painter->restore();
}

QHexCursor *QHexRenderer::cursor() const { return m_cursor; }

void QHexRenderer::setCursor(QHexCursor *newCursor) { m_cursor = newCursor; }

QColor QHexRenderer::bytesColor() const { return m_bytesColor; }

void QHexRenderer::setBytesColor(const QColor &newBytesColor) {
    m_bytesColor = newBytesColor;
}

QColor QHexRenderer::selectionColor() const { return m_selectionColor; }

void QHexRenderer::setSelectionColor(const QColor &newSelectionColor) {
    m_selectionColor = newSelectionColor;
}

QColor QHexRenderer::selBackgroundColor() const { return m_selBackgroundColor; }

void QHexRenderer::setSelBackgroundColor(const QColor &newSelBackgroundColor) {
    m_selBackgroundColor = newSelBackgroundColor;
}

QColor QHexRenderer::bytesAlterBackground() const {
    return m_bytesAlterBackground;
}

void QHexRenderer::setBytesAlterBackground(
    const QColor &newBytesAlterBackground) {
    m_bytesAlterBackground = newBytesAlterBackground;
}

QColor QHexRenderer::bytesBackground() const { return m_bytesBackground; }

void QHexRenderer::setBytesBackground(const QColor &newBytesBackground) {
    m_bytesBackground = newBytesBackground;
}

QColor QHexRenderer::addressColor() const { return m_addressColor; }

void QHexRenderer::setAddressColor(const QColor &newAddressColor) {
    m_addressColor = newAddressColor;
}

QColor QHexRenderer::headerColor() const { return m_headerColor; }

void QHexRenderer::setHeaderColor(const QColor &newHeaderColor) {
    m_headerColor = newHeaderColor;
}
