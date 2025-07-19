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
    // TODO
    return {};
}

QMargins TestHexExt::contentMargins(WingHex::HexEditorContext *context) const {
    if (!_visCol) {
        return {};
    }
    auto lines = context->documentLines();
    auto str = QString::number(lines);
    auto fm = context->fontMetrics();
    constexpr auto padding = 4;
    auto len = fm.horizontalAdvance(str) + padding;
    return {int(len), 0, 0, 1};
}

void TestHexExt::onPaintEvent(QPainter *painter, const QWidget *w,
                              WingHex::HexEditorContext *context) {
    if (!_visCol) {
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
    painter->drawLine(colLen, 0, colLen, w->height());

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

void TestHexExt::setColVisible(bool b) { _visCol = b; }
