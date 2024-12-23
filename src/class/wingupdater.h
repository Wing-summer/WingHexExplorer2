#ifndef WINGUPDATER_H
#define WINGUPDATER_H

#include <QString>

class WingUpdater {
public:
    static bool checkUpdate(bool *ok = nullptr);

private:
    static bool versionCompare(const QString &version);

    static int compareVersions(const QString &version1,
                               const QString &version2);

private:
    static void parseVersion(const QString &version, QString &mainPart,
                             QString &betaPart);

    static int compareMainVersions(const QString &main1, const QString &main2);

    static int compareBetaVersions(const QString &beta1, const QString &beta2);

    static int extractBetaNumber(const QString &beta);
};

#endif // WINGUPDATER_H
