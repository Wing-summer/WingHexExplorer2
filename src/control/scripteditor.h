#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "qcodeedit.h"
#include "qformatscheme.h"
#include "qlanguagefactory.h"

class ScriptEditor : public ads::CDockWidget {
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = nullptr);

    QString fileName() const;

    QEditor *editor() const;

public slots:
    bool openFile(const QString &filename);

    bool save(const QString &path = QString());
    bool reload();

private:
    QString RESNAME(const QString &name);

private:
    QCodeEdit *m_editor = nullptr;
    QFormatScheme *m_formats = nullptr;
    QLanguageFactory *m_languages = nullptr;
};

#endif // SCRIPTEDITOR_H
