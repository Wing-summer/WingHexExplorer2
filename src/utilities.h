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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QAbstractButton>
#include <QApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QMetaEnum>
#include <QMimeDatabase>
#include <QScreen>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QStyle>
#include <QTableView>
#include <QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

#ifdef Q_OS_WIN
#include <qt_windows.h>
#undef MessageBox // because of IWingPlugin
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#define PROEXT ".wingpro"

Q_DECL_UNUSED constexpr auto FIND_CONTEXT_SIZE = 3;
Q_DECL_UNUSED constexpr auto FIND_MAX_DISPLAY_FIND_CHARS = 8;

Q_DECL_UNUSED static inline QString NAMEICONRES(const QString &name) {
    return ":/com.wingsummer.winghex/images/" + name + ".png";
}

Q_DECL_UNUSED static inline QIcon ICONRES(const QString &name) {
    return QIcon(NAMEICONRES(name));
}

class Utilities {
public:
    static inline bool isRoot() {
#ifdef Q_OS_WIN
        BOOL isAdmin = FALSE;
        PSID adminGroup = NULL;

        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(
                &ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
            // Check whether the token of the current process is a member of the
            // Admin group.
            if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
                isAdmin = FALSE;
            }
            FreeSid(adminGroup);
        }
        return isAdmin;
#else
        return getuid() == 0;
#endif
    }

#ifdef Q_OS_LINUX
    static bool isFileOwnerRoot(const QString &filePath) {
        struct stat fileStat;
        if (stat(filePath.toUtf8(), &fileStat) != 0) {
            return false;
        }

        // Check if the owner UID is 0 (root)
        return fileStat.st_uid == 0;
    }

    static bool fixUpFilePermissions(const QString &filePath) {
        // Set permissions to 666 (rw-rw-rw-)
        if (chmod(filePath.toUtf8(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                         S_IROTH | S_IWOTH) != 0) {
            return false;
        }
        return true;
    }
#endif

    static QString processBytesCount(qint64 bytescount) {
        QStringList B{"B", "KB", "MB", "GB", "TB"};
        auto av = bytescount;
        auto r = av;

        for (int i = 0; i < 5; i++) {
            auto lld = lldiv(r, 1024);
            r = lld.quot;
            av = lld.rem;
            if (r == 0) {
                return QStringLiteral("%1 %2").arg(av).arg(B.at(i));
            }
        }

        return QStringLiteral("%1 TB").arg(av);
    }

    static QStringList getEncodings() {
        static QStringList encodings;

        if (encodings.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            for (auto e = 0; e < int(QStringConverter::Encoding::LastEncoding);
                 ++e) {
                auto ee = QStringConverter::nameForEncoding(
                    QStringConverter::Encoding(e));
                if (ee == QStringLiteral("ISO-8859-1")) {
                    encodings << QStringLiteral("ASCII");
                } else {
                    encodings << ee;
                }
            }
#else
            for (auto &e : QTextCodec::availableCodecs()) {
                if (e == QStringLiteral("ISO-8859-1")) {
                    encodings << QStringLiteral("ASCII");
                } else {
                    encodings << e;
                }
            }
#endif
        }

        return encodings;
    }

    static QList<QCryptographicHash::Algorithm> supportedHashAlgorithms() {
        static QList<QCryptographicHash::Algorithm> algorithms;
        if (algorithms.isEmpty()) {
            auto hashe = QMetaEnum::fromType<QCryptographicHash::Algorithm>();
            for (int i = 0; i < hashe.keyCount(); ++i) {
                auto e = hashe.value(i);
                if (e >= 0 && QCryptographicHash::hashLength(
                                  QCryptographicHash::Algorithm(e))) {
                    algorithms << QCryptographicHash::Algorithm(e);
                }
            }
        }
        return algorithms;
    }

    static QStringList supportedHashAlgorithmStringList() {
        static QStringList hashNames;
        if (hashNames.isEmpty()) {
            auto hashes = Utilities::supportedHashAlgorithms();
            auto hashe = QMetaEnum::fromType<QCryptographicHash::Algorithm>();
            for (auto &hash : hashes) {
                hashNames << hashe.valueToKey(hash);
            }
        }
        return hashNames;
    }

    static QByteArray getMd5(const QString &filename) {
        if (filename.isEmpty()) {
            return {};
        }

        QFile sourceFile(filename);
        if (sourceFile.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Md5);
            if (hash.addData(&sourceFile)) {
                return hash.result();
            }
        }
        return {};
    }

    static bool checkIsLittleEndian() {
        short s = 0x1122;
        auto l = *reinterpret_cast<char *>(&s);
        return l == 0x22;
    }

    static QIcon getIconFromFile(QStyle *style, const QString &filename) {
        QFileIconProvider prov;
        auto qicon = prov.icon(QFileInfo(filename));
        if (qicon.availableSizes().isEmpty()) {
            return style->standardIcon(QStyle::SP_FileIcon);
        }
        return qicon;
    }

    static bool fileCanWrite(QString path) {
        return QFileInfo(path).permission(QFile::WriteUser);
    }

    static void moveToCenter(QWidget *window) {
        if (window == nullptr)
            return;
        auto screen = qApp->primaryScreen()->availableSize();
        window->move((screen.width() - window->width()) / 2,
                     (screen.height() - window->height()) / 2);
    }

    static bool isTextFile(const QFileInfo &info) {
        QMimeDatabase db;
        auto t = db.mimeTypeForFile(info);
        return t.inherits(QStringLiteral("text/plain"));
    }

    static bool isStorageDevice(const QString &path) {
        auto sdns = QStorageInfo::mountedVolumes();
        return std::find_if(sdns.begin(), sdns.end(),
                            [path](const QStorageInfo &info) {
                                return info.device() == path;
                            }) != sdns.end();
    }

    static QStorageInfo getStorageDevice(const QString &path) {
        auto sdns = QStorageInfo::mountedVolumes();
        auto r = std::find_if(
            sdns.begin(), sdns.end(),
            [path](const QStorageInfo &info) { return info.device() == path; });
        if (r != sdns.end()) {
            return *r;
        }
        return QStorageInfo();
    }

    static QString getAppDataPath() {
        return QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);
    }

    static void applyTableViewProperty(QTableView *view) {
        view->setEditTriggers(QTableView::EditTrigger::NoEditTriggers);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(
            QAbstractItemView::SelectionBehavior::SelectRows);
        view->setFocusPolicy(Qt::StrongFocus);
        view->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
        view->horizontalHeader()->setStretchLastSection(true);
    }

    template <typename T>
    static inline void addSpecialMark(T *w) {
        if (w) {
            if constexpr (std::is_same_v<T, QLabel> ||
                          std::is_base_of_v<QLabel, T> ||
                          std::is_same_v<T, QAbstractButton> ||
                          std::is_base_of_v<QAbstractButton, T>) {
                w->setText(w->text() + QStringLiteral(" (*)"));
                w->setToolTip(QApplication::tr("OptionNeedRestart"));
            }
        }
    }

    static QByteArray encodingString(const QString &str,
                                     const QString &enc = {}) {
        auto encoding = enc;
        if (encoding.isEmpty() || encoding.compare(QStringLiteral("ASCII"),
                                                   Qt::CaseInsensitive) == 0) {
            encoding = QStringLiteral("ISO-8859-1");
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto en = QStringConverter::encodingForName(encoding.toUtf8());
        Q_ASSERT(en.has_value());
        QStringEncoder e(en.value());
        return e.encode(str);
#else
        auto en = QTextCodec::codecForName(encoding.toUtf8());
        auto e = en->makeEncoder();
        return e->fromUnicode(str);
#endif
    }

    static QString decodingString(const QByteArray &buffer,
                                  const QString &enc = {}) {
        auto encoding = enc;
        if (encoding.isEmpty() || encoding.compare(QStringLiteral("ASCII"),
                                                   Qt::CaseInsensitive) == 0) {
            encoding = QStringLiteral("ISO-8859-1");
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto en = QStringConverter::encodingForName(encoding.toUtf8());
        Q_ASSERT(en.has_value());
        QStringDecoder dec(en.value());

        return dec.decode(buffer);
#else
        auto en = QTextCodec::codecForName(encoding.toUtf8());
        auto dec = en->makeDecoder();
        return dec->toUnicode(buffer);
#endif
    }
};

#endif // UTILITIES_H
