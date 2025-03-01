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

#include "testwingeditorviewwidget.h"

#include <QVBoxLayout>

QList<TestWingEditorViewWidget *> TestWingEditorViewWidget::m_instances;

TestWingEditorViewWidget::TestWingEditorViewWidget(WingHex::IWingPlugin *plg,
                                                   QWidget *parent)
    : WingHex::WingEditorViewWidget(plg, parent) {
    auto layout = new QVBoxLayout(this);
    auto lbl = new QLabel(QStringLiteral("TestWingEditorView"), this);
    lbl->setAlignment(Qt::AlignCenter);
    layout->addWidget(lbl);
    m_instances << this;
}

TestWingEditorViewWidget::~TestWingEditorViewWidget() {
    m_instances.removeOne(this);
}

void TestWingEditorViewWidget::setEnableMeta(bool b) { m_unSaved = b; }

QIcon TestWingEditorViewWidget::Creator::icon() const {
    return QIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));
}

QString TestWingEditorViewWidget::Creator::name() const {
    return tr("TestWingEditorView");
}

QString TestWingEditorViewWidget::Creator::id() const {
    return QStringLiteral("TestWingEditorView");
}

void TestWingEditorViewWidget::toggled(bool isVisible) {
    logWarn(__FUNCTION__ + QStringLiteral(": ") +
            (isVisible ? QStringLiteral("true") : QStringLiteral("false")));
}

WingHex::WingEditorViewWidget *TestWingEditorViewWidget::clone() {
    return nullptr;
}

void TestWingEditorViewWidget::loadState(const QByteArray &state) {
    logWarn(__FUNCTION__ + QStringLiteral(": ") + QString::fromUtf8(state));
}

bool TestWingEditorViewWidget::hasUnsavedState() { return m_unSaved; }

QByteArray TestWingEditorViewWidget::saveState() { return "wingsummer"; }

void TestWingEditorViewWidget::onWorkSpaceNotify(bool isWorkSpace) {
    logWarn(__FUNCTION__ + QStringLiteral(": ") +
            (isWorkSpace ? QStringLiteral("true") : QStringLiteral("false")));
}

QList<TestWingEditorViewWidget *> TestWingEditorViewWidget::instances() {
    return m_instances;
}
