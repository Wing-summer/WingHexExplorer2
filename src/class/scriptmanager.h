/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QDir>
#include <QFileSystemWatcher>
#include <QHash>

#include "QWingRibbon/ribbonbuttongroup.h"
#include "utilities.h"

namespace ads {
class CDockWidgetTab;
} // namespace ads

class ScriptManager {
    Q_DECLARE_TR_FUNCTIONS(ScriptManager)

public:
    struct ScriptDirMeta {
        QString rawName; // a flag
        QString name;
        QString author;
        QString license;
        QString homepage;
        QString comment;
        bool isEmptyDir = false;
        bool isContextMenu = false;
        bool isSys; // a flag
    };

public:
    static ScriptManager &instance();

    QString userScriptPath() const;

    QString systemScriptPath() const;

    QStringList usrScriptsDbCats() const;

    QStringList sysScriptsDbCats() const;

    QStringList getUsrScriptFileNames(const QString &cat) const;

    QStringList getSysScriptFileNames(const QString &cat) const;

    ScriptDirMeta usrDirMeta(const QString &cat) const;
    ScriptDirMeta sysDirMeta(const QString &cat) const;

    static QList<QMenu *> buildUpScriptRunnerContext(RibbonButtonGroup *group,
                                                     QWidget *parent);

private:
    void refreshUsrScriptsDbCats();
    void refreshSysScriptsDbCats();

private:
    static QToolButton *addPannelAction(RibbonButtonGroup *pannel,
                                        const QString &iconName,
                                        const QString &title,
                                        QMenu *menu = nullptr) {
        return addPannelAction(pannel, ICONRES(iconName), title, menu);
    }

    static QToolButton *addPannelAction(RibbonButtonGroup *pannel,
                                        const QIcon &icon, const QString &title,
                                        QMenu *menu = nullptr) {
        Q_ASSERT(pannel);
        auto a = new QToolButton(pannel);
#if QT_VERSION <= QT_VERSION_CHECK(6, 6, 0)
        if (menu) {
            a->setText(title + QStringLiteral(" ▼"));
        } else
#endif
        {
            a->setText(title);
        }

        a->setIcon(icon);
        a->setMenu(menu);
        if (menu) {
#if QT_VERSION > QT_VERSION_CHECK(6, 6, 0)
            a->setArrowType(Qt::DownArrow);
#endif
            a->setPopupMode(QToolButton::InstantPopup);
        }
        pannel->addButton(a);
        return a;
    }

public:
    void runScript(const QString &filename);

    ads::CDockWidgetTab *indicator() const;
    void setIndicator(ads::CDockWidgetTab *newIndicator);

    static bool isScriptFile(const QString &file);

private:
    explicit ScriptManager();
    virtual ~ScriptManager();

    Q_DISABLE_COPY(ScriptManager)

    ScriptDirMeta ensureDirMeta(const QFileInfo &info);

    QStringList getScriptFileNames(const QDir &dir) const;

    QString readJsonObjString(const QJsonObject &jobj, const QString &key);

    bool readJsonObjBool(const QJsonObject &jobj, const QString &key);

    static QMenu *buildUpScriptDirMenu(QWidget *parent,
                                       const QStringList &files, bool isSys);

private:
    ads::CDockWidgetTab *m_indicator = nullptr;

    QString m_sysScriptsPath;
    QString m_usrScriptsPath;
    QStringList m_usrScriptsDbCats;
    QStringList m_sysScriptsDbCats;

    QHash<QString, ScriptDirMeta> _usrDirMetas;
    QHash<QString, ScriptDirMeta> _sysDirMetas;
};

Q_DECLARE_METATYPE(ScriptManager::ScriptDirMeta)

#endif // SCRIPTMANAGER_H
