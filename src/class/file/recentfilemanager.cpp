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

#include "recentfilemanager.h"

#include <QFile>
#include <QMenu>

#include "class/plugin/pluginsystem.h"
#include "class/settings/thememanager.h"
#include "control/toast.h"
#include "dialog/historydeldialog.h"
#include "luau/runtime/scriptmanager.h"
#include "utilities.h"

RecentFileManager::RecentFileManager(QMenu *menu, bool isScriptFile)
    : QObject(), m_menu(menu), m_isScriptFile(isScriptFile) {
    Q_ASSERT(menu);
    menu->setToolTipsVisible(true);
}

void RecentFileManager::apply(QWidget *parent, const QList<RecentInfo> &files) {
    Q_ASSERT(parent);
    this->setParent(parent);
    m_parent = parent;

    auto &skin = ThemeManager::instance();

    QAction *a;
    a = new QAction(m_menu);
    a->setText(tr("Clear history"));
    a->setIcon(skin.standardIcon(ThemeManager::StandardIcon::ClearHistory));
    connect(a, &QAction::triggered, this, &RecentFileManager::clearFile);
    m_menu->addAction(a);

    a = new QAction(m_menu);
    a->setText(tr("Remove item"));
    a->setIcon(skin.standardIcon(ThemeManager::StandardIcon::Delete));
    connect(a, &QAction::triggered, this, [this] {
        auto &skin = ThemeManager::instance();
        if (m_hitems.count() == 0) {
            Toast::toast(
                m_parent,
                skin.standardPixmap(ThemeManager::StandardIcon::ClearHistory),
                tr("No history entries to remove"));
            return;
        }

        HistoryDelDialog hisdlg(m_recents, m_isScriptFile);
        if (hisdlg.exec()) {
            auto idxs = hisdlg.getResult();

            if (idxs.isEmpty()) {
                Toast::toast(m_parent,
                             skin.standardPixmap(
                                 ThemeManager::StandardIcon::ClearHistory),
                             tr("No history entries to remove"));
            } else {
                // indices are already ordered
                for (auto pidx = idxs.rbegin(); pidx != idxs.rend(); ++pidx) {
                    auto ind = *pidx;
                    m_recents.removeAt(ind);
                    auto action = m_hitems.takeAt(ind);
                    m_menu->removeAction(action);
                    action->deleteLater();
                    m_dirty = true;
                }
                Toast::toast(m_parent,
                             skin.standardPixmap(
                                 ThemeManager::StandardIcon::ClearHistory),
                             tr("History entries removed"));
            }
        }
    });
    m_menu->addAction(a);

    m_menu->addSeparator();

    for (auto pitem = files.rbegin(); pitem != files.rend(); ++pitem) {
        addRecentFile(*pitem);
    }
}

const QList<RecentFileManager::RecentInfo> &
RecentFileManager::saveRecent() const {
    return m_recents;
}

bool RecentFileManager::existsPath(const RecentInfo &info) {
    const auto &url = info.url;
    if (url.isLocalFile()) {
        auto f = url.toLocalFile();
        if (m_isScriptFile) {
            if (!ScriptManager::isScriptFile(f)) {
                return false;
            }
        }
        return QFile::exists(f);
    } else {
        if (m_isScriptFile) {
            return false;
        }
        auto scheme = url.scheme();
        if (scheme.compare(QStringLiteral("wdrv"), Qt::CaseInsensitive) == 0) {
            // plugin extension
            auto &plgsys = PluginSystem::instance();
            auto plgID = url.authority();
            return plgsys.ext2Device(plgID) != nullptr;
        }
    }
    return false;
}

bool RecentFileManager::isDirtyInfo(const RecentInfo &old,
                                    const RecentInfo &info) {
    return old.cursorRow != info.cursorRow || old.cursorCol != info.cursorCol ||
           old.scroll != info.scroll || old.view != info.view ||
           old.lineWidth != info.lineWidth ||
           old.lastUseEncoding != info.lastUseEncoding;
}

bool RecentFileManager::isDirty() const { return m_dirty; }

QString RecentFileManager::getDisplayFileName(const RecentInfo &info,
                                              bool isScriptFile) {
    auto url = info.url;
    QString displayName;

    if (isScriptFile || url.isLocalFile()) {
        QFileInfo finfo(url.toLocalFile());
        displayName = finfo.fileName();
    } else {
        auto ext = url.authority();
        auto file = url.path();
        if (file.front() == '/') {
            file.removeFirst();
        }
        displayName = QStringLiteral("[") + ext + QStringLiteral("] ") + file;
    }

    return displayName;
}

QString RecentFileManager::getDisplayTooltip(const RecentInfo &info,
                                             bool isScriptFile) {
    QString tt;
    if (isScriptFile) {
        tt = info.url.toLocalFile();
    } else {
        tt = QStringLiteral("<p>") + tr("[File]") + info.url.toDisplayString() +
             QStringLiteral("</p>");

        tt += QStringLiteral("<p>") + tr("[Workspace]") +
              (info.isWorkSpace ? tr("True") : tr("False")) +
              QStringLiteral("</p>");
    }
    return tt;
}

RecentFileManager::~RecentFileManager() {}

void RecentFileManager::addRecentFile(const RecentInfo &info) {
    if (!existsPath(info)) {
        return;
    }
    int o = m_recents.indexOf(info);
    if (o < 0) {
        while (m_recents.count() >= 10) {
            m_recents.removeLast();
            auto a = m_hitems.takeLast();
            m_menu->removeAction(a);
            delete a;
            m_dirty = true;
        }
        auto a = new QAction(m_menu);

        const auto &url = info.url;
        a->setToolTip(getDisplayTooltip(info, m_isScriptFile));
        if (url.isLocalFile()) {
            QMimeDatabase db;
            auto fileName = info.url.toLocalFile();
            auto mt = db.mimeTypeForFile(fileName);
            auto title = getDisplayFileName(info, m_isScriptFile);
            if (m_isScriptFile) {
                a->setText(title);
                a->setIcon(Utilities::getIconFromFile(qApp->style(), fileName));
            } else {
                if (info.isWorkSpace) {
                    a->setText(title);
                    a->setIcon(ThemeManager::instance().standardIcon(
                        ThemeManager::StandardIcon::Project));
                    auto font = a->font();
                    font.setUnderline(true);
                    a->setFont(font);
                } else {
                    a->setText(title + QStringLiteral(" (") + mt.name() +
                               QStringLiteral(")"));
                    a->setIcon(
                        Utilities::getIconFromFile(qApp->style(), fileName));
                }
            }
        } else {
            // plugin extension
            auto &plgsys = PluginSystem::instance();
            auto plgID = url.authority();
            a->setText(getDisplayFileName(info, false));
            auto r = plgsys.ext2Device(plgID);
            if (r) {
                a->setIcon(r->supportedFileIcon());
            } else {
                a->setIcon(ThemeManager::instance().standardIcon(
                    ThemeManager::StandardIcon::Device));
            }
        }
        connect(a, &QAction::triggered, this, [this, a] {
            auto send = qobject_cast<QAction *>(sender());
            if (send) {
                auto idx = m_hitems.indexOf(send);
                if (idx < 0) {
                    return;
                }
                auto f = m_recents.at(idx);
                if (existsPath(f)) {
                    auto idx = m_hitems.indexOf(a);
                    if (idx > 0) {
                        m_menu->removeAction(a);
                        m_menu->insertAction(m_hitems.first(), a);
                        m_hitems.move(idx, 0);
                        m_recents.move(idx, 0);
                        m_dirty = true;
                    }
                    Q_EMIT sigOnRecentFileTriggered(f);
                } else {
                    auto index = m_hitems.indexOf(send);
                    if (index >= 0) {
                        m_menu->removeAction(send);
                        m_hitems.removeAt(index);
                        send->deleteLater();
                        m_recents.removeAt(index);
                        m_dirty = true;
                        Toast::toast(
                            m_parent,
                            ThemeManager::instance().standardPixmap(
                                ThemeManager::StandardIcon::ClearHistory),
                            tr("The file does not exist; this entry has been "
                               "removed"));
                    }
                }
            }
        });

        if (m_hitems.count()) {
            m_menu->insertAction(m_hitems.first(), a);
        } else {
            m_menu->addAction(a);
        }
        m_recents.prepend(info);
        m_hitems.prepend(a);
    } else {
        if (m_hitems.count() > 1) {
            auto a = m_hitems.at(o);
            m_menu->removeAction(a);
            m_menu->insertAction(m_hitems.first(), a);
            m_hitems.move(o, 0);
            m_recents.move(o, 0);
            m_dirty = true;
        }
    }
}

void RecentFileManager::updateRecentFile(const RecentInfo &info) {
    int o = m_recents.indexOf(info);
    if (o < 0) {
        return;
    }
    auto &ninfo = m_recents[o];
    if (isDirtyInfo(ninfo, info)) {
        ninfo = info;
        m_dirty = true;
    }
}

void RecentFileManager::clearFile() {
    if (m_hitems.count() == 0) {
        Toast::toast(m_parent,
                     ThemeManager::instance().standardPixmap(
                         ThemeManager::StandardIcon::ClearHistory),
                     tr("No history entries to remove"));
        return;
    }
    for (const auto &item : std::as_const(m_hitems)) {
        m_menu->removeAction(item);
        delete item;
    }
    m_recents.clear();
    m_hitems.clear();
    m_dirty = true;
    Toast::toast(m_parent,
                 ThemeManager::instance().standardPixmap(
                     ThemeManager::StandardIcon::ClearHistory),
                 tr("History cleared"));
}
