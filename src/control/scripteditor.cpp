#include "scripteditor.h"
#include "qeditor.h"

#include <QAction>
#include <QPixmap>
#include <cmath>

#include "qfoldpanel.h"
#include "qlinemarkpanel.h"
#include "qlinenumberpanel.h"
#include "qpanellayout.h"

ScriptEditor::ScriptEditor(QWidget *parent)
    : ads::CDockWidget(QString(), parent) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setObjectName(QStringLiteral("ScriptEditor"));

    m_editor = new QCodeEdit(this);
    auto l = m_editor->panelLayout();
    l->setSizeConstraint(QLayout::SetMinimumSize);

    auto editor = m_editor->editor();

    this->setWidget(editor);

    // auto lineMark = new QLineMarkPanel(editor);
    // m_editor->addPanel(lineMark, QCodeEdit::West, true)
    //     ->setShortcut(QKeySequence("F6"));

    auto lineNum = new QLineNumberPanel(editor);
    lineNum->setVerboseMode(true);
    m_editor->addPanel(lineNum, QCodeEdit::West, true)
        ->setShortcut(QKeySequence("F11"));

    // auto fold = new QFoldPanel(editor);
    // m_editor->addPanel(fold, QCodeEdit::West, true)
    //     ->setShortcut(QKeySequence("F9"));

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
