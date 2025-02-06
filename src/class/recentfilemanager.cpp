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
#include "control/toast.h"
#include "dialog/historydeldialog.h"
#include "utilities.h"

#include <QFile>
#include <QMenu>

RecentFileManager::RecentFileManager(QMenu *menu, bool fileNameOnly)
    : QObject(), m_menu(menu), _fileNameOnly(fileNameOnly) {
    Q_ASSERT(menu);
    menu->setToolTipsVisible(true);
}

void RecentFileManager::apply(QWidget *parent, const QList<RecentInfo> &files) {
    Q_ASSERT(parent);
    this->setParent(parent);
    m_parent = parent;

    QAction *a;
    a = new QAction(m_menu);
    a->setText(tr("ClearHistory"));
    a->setIcon(ICONRES("clearhis"));
    connect(a, &QAction::triggered, this, &RecentFileManager::clearFile);
    m_menu->addAction(a);

    a = new QAction(m_menu);
    a->setText(tr("RemoveItem"));
    a->setIcon(ICONRES("del"));
    connect(a, &QAction::triggered, this, [this] {
        if (hitems.count() == 0) {
            Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                         tr("NoHistoryDel"));
            return;
        }

        HistoryDelDialog hisdlg(m_recents, _fileNameOnly);
        if (hisdlg.exec()) {
            auto idxs = hisdlg.getResult();

            if (idxs.isEmpty()) {
                Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                             tr("NoHistoryDel"));
            } else {
                // indices are already ordered
                for (auto pidx = idxs.rbegin(); pidx != idxs.rend(); ++pidx) {
                    auto ind = *pidx;
                    m_recents.removeAt(ind);
                    auto action = hitems.takeAt(ind);
                    m_menu->removeAction(action);
                    action->deleteLater();
                }
                Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                             tr("HistoryDelSuccess"));
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
    if (Utilities::isStorageDevice(info.fileName)) {
        return true;
    } else {
        return QFile::exists(info.fileName);
    }
}

QString RecentFileManager::getDisplayFileName(const RecentInfo &info) {
    auto fileName = info.fileName;
    QString displayName;

    auto driver = Utilities::getStorageDevice(fileName);
    if (driver.isValid()) {
        displayName = driver.displayName();
        if (displayName.isEmpty()) {
            displayName = driver.device();
        }
    } else {
        QFileInfo finfo(fileName);
        displayName = finfo.fileName();
    }

    return displayName;
}

QString RecentFileManager::getDisplayTooltip(const RecentInfo &info,
                                             bool fileNameOnly) {
    QString tt;
    if (fileNameOnly) {
        tt = info.fileName;
    } else {
        tt = QStringLiteral("<p>") + tr("[file]") + info.fileName +
             QStringLiteral("</p>");

        tt += QStringLiteral("<p>") + tr("[isWorkSpace]") +
              (info.isWorkSpace ? tr("True") : tr("False")) +
              QStringLiteral("</p>");
    }
    return tt;
}

RecentFileManager::~RecentFileManager() {}

void RecentFileManager::addRecentFile(const RecentInfo &info) {
    int o = 0;
    if (existsPath(info) && (o = m_recents.indexOf(info)) < 0) {
        while (m_recents.count() >= 10) {
            m_recents.pop_back();
        }
        auto a = new QAction(m_menu);
        a->setData(QVariant::fromValue(info));

        QMimeDatabase db;
        auto mt = db.mimeTypeForFile(info.fileName);

        a->setText(getDisplayFileName(info) + QStringLiteral(" (") + mt.name() +
                   QStringLiteral(")"));
        a->setToolTip(getDisplayTooltip(info, _fileNameOnly));
        if (info.isWorkSpace) {
            a->setIcon(ICONRES(QStringLiteral("pro")));
            auto font = a->font();
            font.setUnderline(true);
            a->setFont(font);
        } else {
            a->setIcon(
                Utilities::getIconFromFile(qApp->style(), info.fileName));
        }
        connect(a, &QAction::triggered, this, [=] {
            auto send = qobject_cast<QAction *>(sender());
            if (send) {
                auto f = send->data().value<RecentInfo>();
                if (existsPath(f)) {
                    auto idx = hitems.indexOf(a);
                    if (idx > 0) {
                        m_menu->removeAction(a);

                        if (hitems.count())
                            m_menu->insertAction(hitems.first(), a);
                        else
                            m_menu->addAction(a);

                        hitems.move(idx, 0);
                    }
                    emit triggered(f);
                } else {
                    auto index = hitems.indexOf(send);
                    if (index < 0) {
                        m_menu->removeAction(send);
                        hitems.removeAt(index);
                        m_recents.removeAt(index);
                        Toast::toast(m_parent,
                                     NAMEICONRES(QStringLiteral("clearhis")),
                                     tr("FileNotExistClean"));
                    }
                }
            }
        });
        m_recents.push_front(info);
        if (hitems.count())
            m_menu->insertAction(hitems.first(), a);
        else
            m_menu->addAction(a);
        hitems.push_front(a);
    } else {
        if (hitems.count() > 1) {
            auto a = hitems.at(o);
            m_menu->removeAction(a);
            m_menu->insertAction(hitems.first(), a);
            hitems.move(o, 0);
        }
    }
}

void RecentFileManager::clearFile() {
    if (hitems.count() == 0) {
        Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                     tr("NoHistoryDel"));
        return;
    }
    for (auto &item : hitems) {
        m_menu->removeAction(item);
        item->deleteLater();
    }
    m_recents.clear();
    hitems.clear();
    Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                 tr("HistoryClearFinished"));
}
