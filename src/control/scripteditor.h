#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qsci/qsciscintilla.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"

class ScriptEditor : public ads::CDockWidget {
    Q_OBJECT
public:
    explicit ScriptEditor(QWidget *parent = nullptr);

signals:

private:
    QsciScintilla *m_editor;
};

#endif // SCRIPTEDITOR_H
