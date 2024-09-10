#include "scripteditor.h"
#include "qeditor.h"

#include "class/skinmanager.h"

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

    switch (SkinManager::instance().currentTheme()) {
    case SkinManager::Theme::Dark:
        m_formats =
            new QFormatScheme(QStringLiteral(":/qcodeedit/as_dark.qxf"), this);
        break;
    case SkinManager::Theme::Light:
        m_formats =
            new QFormatScheme(QStringLiteral(":/qcodeedit/as_light.qxf"), this);
        break;
    }

    QDocument::setDefaultFormatScheme(m_formats);

    m_languages = new QLanguageFactory(m_formats, this);
    m_languages->addDefinitionPath(QStringLiteral(":/qcodeedit"));

    m_editor = new QCodeEdit(this);
    auto l = m_editor->panelLayout();
    l->setSizeConstraint(QLayout::SetMinimumSize);

    auto editor = m_editor->editor();
    connect(editor, &QEditor::titleChanged, this,
            &ads::CDockWidget::setWindowTitle);

    this->setWidget(editor);

    auto lineMark = new QLineMarkPanel(editor);
    m_editor->addPanel(lineMark, QCodeEdit::West, true);

    auto lineNum = new QLineNumberPanel(editor);
    lineNum->setVerboseMode(true);
    m_editor->addPanel(lineNum, QCodeEdit::West, true);

    m_editor->addPanel(QStringLiteral("Fold Panel"), QCodeEdit::West, true);
    m_editor->addPanel(QStringLiteral("Goto Line Panel"), QCodeEdit::South,
                       true);

    m_languages->setLanguage(editor, QStringLiteral("AngelScript"));
}

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

QEditor *ScriptEditor::editor() const { return m_editor->editor(); }
