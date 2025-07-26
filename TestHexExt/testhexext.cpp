/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
** =============================================================================
*/

#include "testhexext.h"

#include <QAction>
#include <QMenu>
#include <QPainter>

TestHexExt::TestHexExt() : WingHex::IWingHexEditorPlugin() {
    m_context = new QMenu(QStringLiteral("TestHexExt"));
    auto a = new QAction(QStringLiteral("LineCol"), m_context);
    a->setCheckable(true);
    a->setChecked(true);
    connect(a, &QAction::toggled, this, &TestHexExt::setColVisible);
    m_context->addAction(a);
    m_aVisCol = a;
}

bool TestHexExt::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    return true;
}

void TestHexExt::unload(std::unique_ptr<QSettings> &set) { Q_UNUSED(set); }

const QString TestHexExt::comment() const {
    return QStringLiteral("A simple line pannel");
}

QMenu *TestHexExt::registeredHexContextMenu() const { return m_context; }

QList<WingHex::WingRibbonToolBoxInfo>
TestHexExt::registeredRibbonTools() const {
    return {};
}

QMargins TestHexExt::contentMargins(WingHex::HexEditorContext *context) const {
    auto visCol = context->property("TestHexExt.colVis").toBool();
    if (!visCol) {
        return {};
    }
    auto lines = context->documentLines();
    auto str = QString::number(lines);
    auto fm = context->fontMetrics();
    constexpr auto padding = 4;
    auto header = QStringLiteral("Line");
    auto minLen = fm.horizontalAdvance(header) + padding;
    auto colLen = qMax(fm.horizontalAdvance(str) + padding, minLen);
    return {int(colLen) + 1, 0, 0, 0};
}

void TestHexExt::prepareCallEditorContext(WingHex::HexEditorContext *context) {
    _curContext = context;
    auto b = isShowLinePannel(context);
    m_aVisCol->blockSignals(true);
    m_aVisCol->setChecked(b);
    m_aVisCol->blockSignals(false);
}

void TestHexExt::finishCallEditorContext(WingHex::HexEditorContext *context) {
    Q_UNUSED(context);
    _curContext = nullptr;
}

void TestHexExt::onPaintEvent(QPainter *painter, const QWidget *w,
                              WingHex::HexEditorContext *context) {
    auto visCol = isShowLinePannel(context);
    if (!visCol) {
        return;
    }
    painter->save();
    painter->translate(-context->currentHorizontalOffset(), 0);
    auto lines = context->documentLines();
    auto str = QString::number(lines);
    auto fm = context->fontMetrics();
    constexpr auto padding = 4;
    auto header = QStringLiteral("Line");
    auto minLen = fm.horizontalAdvance(header) + padding;
    auto colLen = qMax(fm.horizontalAdvance(str) + padding, minLen);
    auto border = context->borderColor();
    painter->setPen(border);
    if (context->headerAreaVisible()) {
        auto headerHeight = context->headerHeight();
        painter->drawLine(0, headerHeight, colLen, headerHeight);
        painter->save();
        painter->setPen(context->headerColor());
        QRectF rect(0, 0, colLen, headerHeight);
        painter->drawText(rect, header, QTextOption(Qt::AlignCenter));
        painter->restore();
        context->renderHexBackground(painter, {0, 0}, colLen);
    }

    // draw Line Numbers
    painter->setPen(context->addressColor());
    context->renderContent(
        painter, {0, 0}, colLen,
        [](QPainter *painter, const QRect &lineRect, qsizetype curLine) {
            painter->drawText(lineRect, QString::number(curLine),
                              QTextOption(Qt::AlignCenter));
        });

    painter->restore();
}

bool TestHexExt::isShowLinePannel(WingHex::HexEditorContext *context) {
    auto pp = context->property("TestHexExt.colVis");
    if (pp.isNull()) {
        context->setProperty("TestHexExt.colVis", true);
        return true;
    }
    return pp.toBool();
}

void TestHexExt::setColVisible(bool b) {
    if (_curContext) {
        _curContext->setProperty("TestHexExt.colVis", b);
    }
}
