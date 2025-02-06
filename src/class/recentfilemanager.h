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

#ifndef RECENTFILEMANAGER_H
#define RECENTFILEMANAGER_H

#include <QAction>
#include <QList>
#include <QMenu>
#include <QObject>

class RecentFileManager : public QObject {
    Q_OBJECT
public:
    struct RecentInfo {
        QString fileName;
        bool isWorkSpace = false;

        bool operator==(const RecentInfo &info) const {
            return
#ifdef Q_OS_WIN
                this->fileName.compare(info.fileName, Qt::CaseInsensitive) == 0
#else
                this->fileName == info.fileName
#endif
                && this->isWorkSpace == info.isWorkSpace;
        }
        bool operator!=(const RecentInfo &info) const {
            return
#ifdef Q_OS_WIN
                this->fileName.compare(info.fileName, Qt::CaseInsensitive)
#else
                this->fileName != info.fileName
#endif
                || this->isWorkSpace != info.isWorkSpace;
        }

        friend QDataStream &operator<<(QDataStream &arch,
                                       const RecentInfo &object) {
            arch << object.fileName;
            arch << object.isWorkSpace;
            return arch;
        }

        friend QDataStream &operator>>(QDataStream &arch, RecentInfo &object) {
            arch >> object.fileName;
            arch >> object.isWorkSpace;
            return arch;
        }
    };

public:
    explicit RecentFileManager(QMenu *menu, bool fileNameOnly);
    ~RecentFileManager();
    void addRecentFile(const RecentInfo &info);
    void clearFile();
    void apply(QWidget *parent, const QList<RecentInfo> &files);

    const QList<RecentInfo> &saveRecent() const;

    static QString getDisplayFileName(const RecentInfo &info);
    static QString getDisplayTooltip(const RecentInfo &info, bool fileNameOnly);

signals:
    void triggered(const RecentFileManager::RecentInfo &rinfo);

private:
    bool existsPath(const RecentInfo &info);

private:
    QMenu *m_menu;
    QWidget *m_parent;
    QList<RecentInfo> m_recents;
    QList<QAction *> hitems;

    bool _fileNameOnly;
};

Q_DECLARE_METATYPE(RecentFileManager::RecentInfo)

#endif // RECENTFILEMANAGER_H
