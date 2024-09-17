#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "qcodeedit.h"

class ScriptEditor : public ads::CDockWidget {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);
    virtual ~ScriptEditor();

    QString fileName() const;

    QEditor *editor() const;

public slots:
    bool openFile(const QString &filename);

    bool save(const QString &path = QString());
    bool reload();

private:
    void processTitle();

private:
    QCodeEdit *m_editor = nullptr;
};

#endif // SCRIPTEDITOR_H
