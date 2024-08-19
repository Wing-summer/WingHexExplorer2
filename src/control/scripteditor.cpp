#include "scripteditor.h"
#include "qeditor.h"

#include <QAction>
#include <QPixmap>
#include <cmath>

ScriptEditor::ScriptEditor(QWidget *parent)
    : ads::CDockWidget(QString(), parent) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setObjectName(QStringLiteral("ScriptEditor"));

    m_session = new QEditSession("session", this);
    m_editor = new QCodeEdit(this);

    auto editor = m_editor->editor();
    m_session->addEditor(editor);

    this->setWidget(editor);

    m_editor->addPanel("Line Mark Panel", QCodeEdit::West, true)
        ->setShortcut(QKeySequence("F6"));

    m_editor->addPanel("Line Number Panel", QCodeEdit::West, true)
        ->setShortcut(QKeySequence("F11"));

    m_editor->addPanel("Fold Panel", QCodeEdit::West, true)
        ->setShortcut(QKeySequence("F9"));

    // m_editor->markerDefine(QPixmap(RESNAME("bp")), BreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("conbp")), ConditionBreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("bpdis")), DisabledBreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("hitbp")), DbgRunHitBreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("hitcur")), DbgRunCurrentLine);
}

QString ScriptEditor::RESNAME(const QString &name) {
    return QStringLiteral(":/com.wingsummer.winghex/images/scriptdbg/") + name +
           QStringLiteral(".png");
}
