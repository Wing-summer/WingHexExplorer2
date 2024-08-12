#include "scripteditor.h"
#include "class/qscilexeras.h"

#include <QPixmap>

ScriptEditor::ScriptEditor(QWidget *parent)
    : ads::CDockWidget(QString(), parent) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setObjectName(QStringLiteral("ScriptEditor"));

    m_editor = new QsciScintilla(this);
    this->setWidget(m_editor);

    m_editor->setUtf8(true);
    m_editor->setEolMode(QsciScintilla::EolUnix);
    m_editor->setTabWidth(4);
    m_editor->setIndentationsUseTabs(false);
    m_editor->setIndentationGuides(true);
    m_editor->setAutoIndent(true);
    m_editor->setLexer(new QsciLexerAs(this));

    m_editor->markerDefine(QPixmap(RESNAME("bp")), BreakPoint);
    m_editor->markerDefine(QPixmap(RESNAME("conbp")), ConditionBreakPoint);
    m_editor->markerDefine(QPixmap(RESNAME("bpdis")), DisabledBreakPoint);
    m_editor->markerDefine(QPixmap(RESNAME("hitbp")), DbgRunHitBreakPoint);
    m_editor->markerDefine(QPixmap(RESNAME("hitcur")), DbgRunCurrentLine);

    m_editor->setMarginType(0, QsciScintilla::SymbolMargin); // breakpoint
    m_editor->setMarginType(1, QsciScintilla::NumberMargin); // line number
    m_editor->setMarginType(2, QsciScintilla::SymbolMargin); // folding

    m_editor->setFolding(QsciScintilla::BoxedTreeFoldStyle, 2);
    m_editor->SendScintilla(QsciScintilla::SCI_SETMARGINMASKN, 2, long(0));

    connect(m_editor, &QsciScintilla::marginClicked, this,
            &ScriptEditor::on_marginClicked);
    connect(m_editor, &QsciScintilla::linesChanged, this,
            &ScriptEditor::on_linesChanged);
    connect(m_editor, &QsciScintilla::indicatorClicked, this,
            &ScriptEditor::on_indicatorClicked);
    connect(m_editor, &QsciScintilla::cursorPositionChanged, this,
            &ScriptEditor::on_cursorPositionChanged);
}

QColor ScriptEditor::caretColor() const { return m_caretColor; }

void ScriptEditor::setCaretColor(const QColor &newCaretColor) {
    if (m_caretColor == newCaretColor)
        return;
    m_editor->setCaretForegroundColor(newCaretColor);
    m_caretColor = newCaretColor;
    emit caretColorChanged();
}

QColor ScriptEditor::caretHighlightColor() const {
    return m_caretHighlightColor;
}

void ScriptEditor::setCaretHighlightColor(
    const QColor &newCaretHighlightColor) {
    if (m_caretHighlightColor == newCaretHighlightColor)
        return;
    m_editor->setCaretLineBackgroundColor(newCaretHighlightColor);
    m_caretHighlightColor = newCaretHighlightColor;
    emit caretHighlightColorChanged();
}

bool ScriptEditor::isHightlightCurrentLine() const {
    return m_editor->SendScintilla(QsciScintillaBase::SCI_GETCARETLINEVISIBLE);
}

void ScriptEditor::setHightlightCurrentLine(bool enabled) {
    m_editor->setCaretLineVisible(enabled);
}

QColor ScriptEditor::marginsForegroundColor() const {
    return m_marginsForegroundColor;
}

void ScriptEditor::setMarginsForegroundColor(
    const QColor &newMarginsForegroundColor) {
    if (m_marginsForegroundColor == newMarginsForegroundColor)
        return;
    m_editor->setMarginsForegroundColor(newMarginsForegroundColor);
    m_editor->setFoldMarginColors(newMarginsForegroundColor,
                                  m_marginsBackgroundColor);
    m_marginsForegroundColor = newMarginsForegroundColor;
    emit marginsForegroundColorChanged();
}

QColor ScriptEditor::marginsBackgroundColor() const {
    return m_marginsBackgroundColor;
}

void ScriptEditor::setMarginsBackgroundColor(
    const QColor &newMarginsBackgroundColor) {
    if (m_marginsBackgroundColor == newMarginsBackgroundColor)
        return;
    m_editor->setMarginsBackgroundColor(newMarginsBackgroundColor);
    m_editor->setFoldMarginColors(m_marginsForegroundColor,
                                  newMarginsBackgroundColor);
    m_marginsBackgroundColor = newMarginsBackgroundColor;
    emit marginsBackgroundColorChanged();
}

QString ScriptEditor::RESNAME(const QString &name) {
    return QStringLiteral(":/com.wingsummer.winghex/images/scriptdbg/") + name +
           QStringLiteral(".png");
}

void ScriptEditor::on_marginClicked(int margin, int line,
                                    Qt::KeyboardModifiers state) {}

void ScriptEditor::on_indicatorClicked(int line, int index,
                                       Qt::KeyboardModifiers state) {}

void ScriptEditor::on_cursorPositionChanged(int line, int index) {}

void ScriptEditor::on_linesChanged() {}

QColor ScriptEditor::paperColor() const { return m_paperColor; }

void ScriptEditor::setPaperColor(const QColor &newPaperColor) {
    if (m_paperColor == newPaperColor)
        return;
    m_editor->setPaper(newPaperColor);
    m_paperColor = newPaperColor;
    emit paperColorChanged();
}
