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
        qsizetype start = -1;
        qsizetype stop = -1;

        bool operator==(const RecentInfo &info) const {
            return
#ifdef Q_OS_WIN
                this->fileName.compare(info.fileName, Qt::CaseInsensitive) == 0
#else
                this->fileName == info.fileName
#endif
                && this->isWorkSpace == info.isWorkSpace &&
                this->start == info.start && this->stop == info.stop;
        }
        bool operator!=(const RecentInfo &info) const {
            return
#ifdef Q_OS_WIN
                this->fileName.compare(info.fileName, Qt::CaseInsensitive)
#else
                this->fileName != info.fileName
#endif
                || this->isWorkSpace != info.isWorkSpace ||
                this->start != info.start || this->stop != info.stop;
        }

        friend QDataStream &operator<<(QDataStream &arch,
                                       const RecentInfo &object) {
            arch << object.fileName;
            arch << object.isWorkSpace;
            arch << object.start;
            arch << object.stop;
            return arch;
        }

        friend QDataStream &operator>>(QDataStream &arch, RecentInfo &object) {
            arch >> object.fileName;
            arch >> object.isWorkSpace;
            arch >> object.start;
            arch >> object.stop;
            return arch;
        }
    };

public:
    explicit RecentFileManager(QMenu *menu);
    ~RecentFileManager();
    void addRecentFile(const RecentInfo &info);
    void clearFile();
    void apply(QWidget *parent, const QList<RecentInfo> &files);

    const QList<RecentInfo> &saveRecent() const;

signals:
    void triggered(const RecentInfo &rinfo);

private:
    bool existsPath(const RecentInfo &info);
    QString getDisplayFileName(const RecentInfo &info);
    QString getDisplayTooltip(const RecentInfo &info);

private:
    QMenu *m_menu;
    QWidget *m_parent;
    QList<RecentInfo> m_recents;
    QList<QAction *> hitems;
};

Q_DECLARE_METATYPE(RecentFileManager::RecentInfo)

#endif // RECENTFILEMANAGER_H
