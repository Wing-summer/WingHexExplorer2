#ifndef RECENTFILEMANAGER_H
#define RECENTFILEMANAGER_H

#include <QAction>
#include <QList>
#include <QMenu>
#include <QObject>

class RecentFileManager : public QObject {
    Q_OBJECT
public:
    explicit RecentFileManager(QMenu *menu);
    ~RecentFileManager();
    void addRecentFile(QString filename);
    void clearFile();
    void apply(QWidget *parent);

private:
    QMenu *m_menu;
    QWidget *m_parent;
    QStringList m_recents;
    QList<QAction *> hitems;

    void trigger();
};

#endif // RECENTFILEMANAGER_H
