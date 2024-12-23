#include "wingupdater.h"

#include "languagemanager.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>

bool WingUpdater::checkUpdate(bool *ok) {
    QNetworkAccessManager manager;
    QUrl url;

    // Github is not easy to access for Chinese people,
    // Gitee mirror instead
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    if (LanguageManager::instance().defaultLocale().territory() ==
#else
    if (LanguageManager::instance().defaultLocale().country() ==
#endif
        QLocale::China) {
        url =
            QUrl(QStringLiteral("https://gitee.com/wing-cloud/WingHexExplorer2/"
                                "raw/updater/WINGHEX_VERSION"));
    } else {
        url = QUrl(QStringLiteral(
            "https://raw.githubusercontent.com/Wing-summer/WingHexExplorer2/"
            "updater/WINGHEX_VERSION"));
    }

    QNetworkRequest request(url);

#ifdef Q_OS_WIN
    request.setRawHeader(
        "User-Agent",
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
        "like Gecko) Chrome/91.0.4472.124 Safari/537.36 Edg/91.0.864.59");
#else
    request.setRawHeader(
        "User-Agent",
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, "
        "like Gecko) Chrome/91.0.4472.124 Safari/537.36 Edg/91.0.864.59");
#endif

    QEventLoop loop;

    auto *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        // Successfully received the response
        auto version = reply->readAll();
        if (ok) {
            *ok = true;
        }
        auto ret = versionCompare(version);
        return ret;
    } else {
        // Handle errors
        if (ok) {
            *ok = false;
        }
    }

    reply->deleteLater();

    return true;
}

bool WingUpdater::versionCompare(const QString &version) {
    return compareVersions(WINGHEX_VERSION, version) >= 0;
}

int WingUpdater::compareVersions(const QString &version1,
                                 const QString &version2) {
    // Extract main version and beta part
    QString main1, beta1;
    parseVersion(version1, main1, beta1);
    QString main2, beta2;
    parseVersion(version2, main2, beta2);

    // Compare main version
    int mainComparison = compareMainVersions(main1, main2);
    if (mainComparison != 0) {
        return mainComparison;
    }

    // Compare beta part
    return compareBetaVersions(beta1, beta2);
}

void WingUpdater::parseVersion(const QString &version, QString &mainPart,
                               QString &betaPart) {
    static QRegularExpression regex(R"((.*?)(-beta(\d*)?)?$)");
    QRegularExpressionMatch match = regex.match(version);

    if (match.hasMatch()) {
        mainPart = match.captured(1);
        betaPart = match.captured(2); // Full beta string, e.g., "-beta2"
    } else {
        mainPart = version;
        betaPart.clear();
    }
}

int WingUpdater::compareMainVersions(const QString &main1,
                                     const QString &main2) {
    QStringList parts1 = main1.split('.', Qt::SkipEmptyParts);
    QStringList parts2 = main2.split('.', Qt::SkipEmptyParts);

    int maxParts = qMax(parts1.size(), parts2.size());
    for (int i = 0; i < maxParts; ++i) {
        int v1 = i < parts1.size() ? parts1[i].toInt() : 0;
        int v2 = i < parts2.size() ? parts2[i].toInt() : 0;

        if (v1 < v2)
            return -1;
        else if (v1 > v2)
            return 1;
    }
    return 0;
}

int WingUpdater::compareBetaVersions(const QString &beta1,
                                     const QString &beta2) {
    if (beta1.isEmpty() && beta2.isEmpty()) {
        return 0; // Both are not beta versions
    }
    if (beta1.isEmpty()) {
        return 1; // Non-beta is greater
    }
    if (beta2.isEmpty()) {
        return -1; // Beta is less
    }

    // Extract numeric beta suffix
    int betaNum1 = extractBetaNumber(beta1);
    int betaNum2 = extractBetaNumber(beta2);

    if (betaNum1 < betaNum2) {
        return -1;
    } else if (betaNum1 > betaNum2) {
        return 1;
    }
    return 0; // Both beta versions are equal
}

int WingUpdater::extractBetaNumber(const QString &beta) {
    static QRegularExpression regex(R"(-beta(\d*))");
    QRegularExpressionMatch match = regex.match(beta);

    if (match.hasMatch()) {
        QString number = match.captured(1);
        return number.isEmpty() ? 0 : number.toInt();
    }
    return 0; // No numeric suffix
}
