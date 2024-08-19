#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "qcodeedit.h"
#include "qeditsession.h"

class ScriptEditor : public ads::CDockWidget {
    Q_OBJECT

public:
    enum Symbols {
        BreakPoint,
        ConditionBreakPoint,
        DisabledBreakPoint,
        DbgRunCurrentLine,
        DbgRunHitBreakPoint,
    };

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

private:
    QString RESNAME(const QString &name);

private:
    QCodeEdit *m_editor = nullptr;
    QEditSession *m_session = nullptr;
};

#endif // SCRIPTEDITOR_H
