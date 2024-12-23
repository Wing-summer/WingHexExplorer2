#include "testwingeditorviewwidget.h"

#include <QVBoxLayout>

TestWingEditorViewWidget::TestWingEditorViewWidget(QWidget *parent)
    : WingHex::WingEditorViewWidget(parent) {
    auto layout = new QVBoxLayout(this);
    auto lbl = new QLabel(QStringLiteral("TestWingEditorView"), this);
    lbl->setAlignment(Qt::AlignCenter);
    layout->addWidget(lbl);
}

QIcon TestWingEditorViewWidget::icon() const {
    return QIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));
}

QString TestWingEditorViewWidget::name() const {
    return tr("TestWingEditorView");
}

QString TestWingEditorViewWidget::id() const {
    return QStringLiteral("TestWingEditorView");
}

void TestWingEditorViewWidget::toggled(bool isVisible) {}

WingHex::WingEditorViewWidget *TestWingEditorViewWidget::clone() {
    return nullptr;
}
