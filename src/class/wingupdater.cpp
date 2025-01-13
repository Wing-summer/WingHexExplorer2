#include "wingupdater.h"

#include "languagemanager.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVersionNumber>

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
        auto ret = versionCompare(version, WINGHEX_VERSION);
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

bool WingUpdater::versionCompare(const QString &remote, const QString &local) {
    // only stable version updating notification
    auto rversion = QVersionNumber::fromString(remote);
    auto lversion = QVersionNumber::fromString(local);
    return rversion <= lversion;
}
