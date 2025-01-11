#ifndef WINGUPDATER_H
#define WINGUPDATER_H

#include <QString>
#include <QStringList>
#include <QVersionNumber>

class WingUpdater {
public:
    static bool checkUpdate(bool *ok = nullptr);

private:
    static bool versionCompare(const QString &remote, const QString &local);
};

#endif // WINGUPDATER_H
