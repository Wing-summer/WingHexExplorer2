#ifndef COMMANDHISTORYMANAGER_H
#define COMMANDHISTORYMANAGER_H

#include <QStringList>

class CommandHistoryManager {
public:
    static QStringList load();

    static void save(const QStringList &strings);

private:
    static QString getHistoryPath();
};

#endif // COMMANDHISTORYMANAGER_H
