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
    plugin()->emit warn(
        __FUNCTION__ + QStringLiteral(": ") +
        (isVisible ? QStringLiteral("true") : QStringLiteral("false")));
}

WingHex::WingEditorViewWidget *TestWingEditorViewWidget::clone() {
    return nullptr;
}

void TestWingEditorViewWidget::loadState(const QByteArray &state) {
    plugin()->emit warn(__FUNCTION__ + QStringLiteral(": ") +
                        QString::fromUtf8(state));
}

bool TestWingEditorViewWidget::hasUnsavedState() { return m_unSaved; }

QByteArray TestWingEditorViewWidget::saveState() {
    return WingHex::WINGSUMMER.toUtf8();
}

void TestWingEditorViewWidget::onWorkSpaceNotify(bool isWorkSpace) {
    plugin()->emit warn(
        __FUNCTION__ + QStringLiteral(": ") +
        (isWorkSpace ? QStringLiteral("true") : QStringLiteral("false")));
}

QList<TestWingEditorViewWidget *> TestWingEditorViewWidget::instances() {
    return m_instances;
}
