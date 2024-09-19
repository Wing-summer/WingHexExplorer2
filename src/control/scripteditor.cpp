#include "scripteditor.h"
#include "qeditor.h"

#include <QAction>
#include <QFile>
#include <QPixmap>

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
    connect(editor, &QEditor::titleChanged, this, &ScriptEditor::processTitle);
    connect(editor, &QEditor::contentModified, this,
            &ScriptEditor::processTitle);

    this->setWidget(editor);

    auto lineMark = new QLineMarkPanel(editor);
    m_editor->addPanel(lineMark, QCodeEdit::West, true);
    connect(lineMark, &QLineMarkPanel::onToggleMark, this,
            &ScriptEditor::onToggleMark);

    auto lineNum = new QLineNumberPanel(editor);
    lineNum->setVerboseMode(true);
    m_editor->addPanel(lineNum, QCodeEdit::West, true);

    m_editor->addPanel(QStringLiteral("Fold Panel"), QCodeEdit::West, true);
    m_editor->addPanel(QStringLiteral("Line Change Panel"), QCodeEdit::West,
                       true);
    m_editor->addPanel(QStringLiteral("Goto Line Panel"), QCodeEdit::South,
                       true);
    m_editor->addPanel(QStringLiteral("Search Replace Panel"), QCodeEdit::South,
                       true);
}

ScriptEditor::~ScriptEditor() { m_editor->editor()->document(); }

QString ScriptEditor::fileName() const {
    return m_editor->editor()->fileName();
}

bool ScriptEditor::openFile(const QString &filename) {
    auto e = m_editor->editor();
    return e->load(filename);
}

bool ScriptEditor::save(const QString &path) {
    auto e = m_editor->editor();
    if (path.isEmpty()) {
        return e->save();
    }
    return e->save(path);
}

bool ScriptEditor::reload() {
    auto e = m_editor->editor();
    return e->load(e->fileName());
}

void ScriptEditor::processTitle() {
    auto e = m_editor->editor();
    if (e->isContentModified()) {
        setWindowTitle(e->windowTitle());
    } else {
        setWindowTitle(e->name());
    }
}

QEditor *ScriptEditor::editor() const { return m_editor->editor(); }
