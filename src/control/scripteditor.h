#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qsci/qsciscintilla.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"

class ScriptEditor : public ads::CDockWidget {
    Q_OBJECT

    Q_PROPERTY(QColor caretColor READ caretColor WRITE setCaretColor NOTIFY
                   caretColorChanged FINAL)
    Q_PROPERTY(
        QColor caretHighlightColor READ caretHighlightColor WRITE
            setCaretHighlightColor NOTIFY caretHighlightColorChanged FINAL)
    Q_PROPERTY(QColor marginsForegroundColor READ marginsForegroundColor WRITE
                   setMarginsForegroundColor NOTIFY
                       marginsForegroundColorChanged FINAL)
    Q_PROPERTY(QColor marginsBackgroundColor READ marginsBackgroundColor WRITE
                   setMarginsBackgroundColor NOTIFY
                       marginsBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor paperColor READ paperColor WRITE setPaperColor NOTIFY
                   paperColorChanged FINAL)

private:
    enum Symbols {
        BreakPoint,
        ConditionBreakPoint,
        DisabledBreakPoint,
        DbgRunCurrentLine,
        DbgRunHitBreakPoint,
    };

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    QColor caretColor() const;
    void setCaretColor(const QColor &newCaretColor);

    QColor caretHighlightColor() const;
    void setCaretHighlightColor(const QColor &newCaretHighlightColor);

    bool isHightlightCurrentLine() const;
    void setHightlightCurrentLine(bool enabled);

    QColor marginsForegroundColor() const;
    void setMarginsForegroundColor(const QColor &newMarginsForegroundColor);

    QColor marginsBackgroundColor() const;
    void setMarginsBackgroundColor(const QColor &newMarginsBackgroundColor);

    QColor paperColor() const;
    void setPaperColor(const QColor &newPaperColor);

signals:
    void caretColorChanged();

    void caretHighlightColorChanged();

    void marginsForegroundColorChanged();

    void marginsBackgroundColorChanged();

    void paperColorChanged();

private:
    QString RESNAME(const QString &name);

private slots:
    void on_marginClicked(int margin, int line, Qt::KeyboardModifiers state);
    void on_indicatorClicked(int line, int index, Qt::KeyboardModifiers state);
    void on_cursorPositionChanged(int line, int index);
    void on_linesChanged();

private:
    QsciScintilla *m_editor = nullptr;

private:
    QColor m_caretColor;
    QColor m_caretHighlightColor;
    QColor m_marginsForegroundColor;
    QColor m_marginsBackgroundColor;
    QColor m_paperColor;
};

#endif // SCRIPTEDITOR_H
