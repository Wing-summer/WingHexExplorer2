#include "recentfilemanager.h"
#include "../class/winginputdialog.h"
#include "../control/toast.h"
#include "../utilities.h"

#include <QFile>
#include <QMenu>

RecentFileManager::RecentFileManager(QMenu *menu) : QObject(), m_menu(menu) {}

void RecentFileManager::apply(QWidget *parent, const QStringList &files) {
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
        bool ok;
        auto d = WingInputDialog::getInt(nullptr, tr("Input"), tr("InputIndex"),
                                         0, 0, m_recents.count(), 1, &ok);
        if (ok) {
            m_menu->removeAction(hitems.at(d));
            m_recents.removeAt(d);
            for (auto it = hitems.begin() + d; it != hitems.end(); it++) {
                (*it)->setIconText(QString::number(d++));
            }
        }
    });
    m_menu->addAction(a);

    m_menu->addSeparator();

    int i = 0;
    for (auto &item : files) {
        if (QFile::exists(item)) {
            if (m_recents.count() > 10)
                break;
            m_recents << item;
            a = new QAction(m_menu);
            a->setText(QStringLiteral("%1 : %2").arg(i++).arg(item));
            a->setData(item);
            connect(a, &QAction::triggered, this, &RecentFileManager::trigger);
            hitems.push_back(a);
            m_menu->addAction(a);
        }
    }
}

RecentFileManager::~RecentFileManager() { emit onSaveRecent(m_recents); }

void RecentFileManager::addRecentFile(QString filename) {
    int o = 0;
    if (QFile::exists(filename) && (o = m_recents.indexOf(filename)) < 0) {

        while (m_recents.count() >= 10) {
            m_recents.pop_back();
        }

        auto a = new QAction(m_menu);
        a->setData(filename);
        connect(a, &QAction::triggered, this, &RecentFileManager::trigger);
        m_recents.push_front(filename);
        if (hitems.count())
            m_menu->insertAction(hitems.first(), a);
        else
            m_menu->addAction(a);
        hitems.push_front(a);
        auto i = 0;
        for (auto &item : hitems) {
            item->setText(QStringLiteral("%1 : %2").arg(i++).arg(
                item->data().toString()));
        }
    } else {
        if (hitems.count() > 1) {
            auto a = hitems.at(o);
            m_menu->removeAction(a);
            m_menu->insertAction(hitems.first(), a);
            hitems.move(o, 0);
            auto i = 0;
            for (auto &item : hitems) {
                item->setText(QStringLiteral("%1 : %2").arg(i++).arg(
                    item->data().toString()));
            }
        }
    }
}

void RecentFileManager::trigger() {
    auto send = qobject_cast<QAction *>(sender());
    if (send) {
        auto f = send->data().toString();
        if (QFile::exists(f)) {
            // AppManager::openFile(f);
            return;
        }
    }
    auto index = hitems.indexOf(send);
    if (index >= 0) {
        m_menu->removeAction(send);
        hitems.removeAt(index);
        m_recents.removeAt(index);
        Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                     tr("FileNotExistClean"));
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
    }
    m_recents.clear();
    hitems.clear();
    emit onSaveRecent({});
    Toast::toast(m_parent, NAMEICONRES(QStringLiteral("clearhis")),
                 tr("HistoryClearFinished"));
}
