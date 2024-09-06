#include "scripteditor.h"
#include "qeditor.h"

#include "class/skinmanager.h"
#include "utilities.h"

#include <QAction>
#include <QFile>
#include <QPixmap>

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

    this->setWidget(editor);

    auto lineMark = new QLineMarkPanel(editor);
    m_editor->addPanel(lineMark, QCodeEdit::West, true)
        ->setShortcut(QKeySequence(Qt::Key_F6));

    auto lineNum = new QLineNumberPanel(editor);
    lineNum->setVerboseMode(true);
    m_editor->addPanel(lineNum, QCodeEdit::West, true)
        ->setShortcut(QKeySequence(Qt::Key_F11));

    auto fold = new QFoldPanel(editor);
    m_editor->addPanel(fold, QCodeEdit::West, true)
        ->setShortcut(QKeySequence(Qt::Key_F9));

    m_languages->setLanguage(editor, QStringLiteral("AngelScript"));

    // m_editor->markerDefine(QPixmap(RESNAME("bp")), BreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("conbp")), ConditionBreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("bpdis")), DisabledBreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("hitbp")), DbgRunHitBreakPoint);
    // m_editor->markerDefine(QPixmap(RESNAME("hitcur")), DbgRunCurrentLine);
}

QString ScriptEditor::fileName() const { return m_fileName; }

bool ScriptEditor::isNewFile() const { return m_isNewFile; }

void ScriptEditor::newFile(size_t index) {
    auto istr = QString::number(index);
    m_fileName = tr("Untitled") + istr;
    this->setWindowTitle(m_fileName);
    m_isNewFile = true;
}

bool ScriptEditor::openFile(const QString &filename) {
    QFileInfo finfo(filename);

    if (!finfo.exists() || !finfo.isFile()) {
        return false;
    }
    if (!Utilities::isTextFile(finfo)) {
        return false;
    }

    QFile file(finfo.absolutePath());
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }
    m_editor->editor()->setText(QString::fromUtf8(file.readAll()));
    file.close();
    m_isNewFile = false;
    return true;
}

bool ScriptEditor::save(const QString &path, bool isExport) {
    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }
    file.write(m_editor->editor()->text().toUtf8());
    file.close();

    if (!isExport) {
        m_fileName = path;
        m_isNewFile = false;
    }

    return true;
}

bool ScriptEditor::reload() { return openFile(m_fileName); }

QString ScriptEditor::RESNAME(const QString &name) {
    return QStringLiteral(":/com.wingsummer.winghex/images/scriptdbg/") + name +
           QStringLiteral(".png");
}

QEditor *ScriptEditor::editor() const { return m_editor->editor(); }
