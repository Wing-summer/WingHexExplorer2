#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "qcodeedit.h"
#include "qformatscheme.h"
#include "qlanguagefactory.h"

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

    QString fileName() const;
    bool isNewFile() const;

    QEditor *editor() const;

public slots:
    void newFile(size_t index);
    bool openFile(const QString &filename);

    bool save(const QString &path = QString(), bool isExport = false);
    bool reload();

private:
    QString RESNAME(const QString &name);

private:
    QCodeEdit *m_editor = nullptr;
    QFormatScheme *m_formats = nullptr;
    QLanguageFactory *m_languages = nullptr;

    QString m_fileName;
    bool m_isNewFile = true;
};

#endif // SCRIPTEDITOR_H
