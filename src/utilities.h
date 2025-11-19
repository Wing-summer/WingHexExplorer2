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
#include <QBuffer>
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
#include <QTreeView>
#include <QWidget>
#include <QtEndian>

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

Q_DECL_UNUSED static inline QString NAMEICONRES(const QString &name) {
    return ":/com.wingsummer.winghex/images/" + name + ".png";
}

Q_DECL_UNUSED static inline QIcon ICONRES(const QString &name) {
    static QHash<QString, QIcon> cache;
    auto picon = cache.find(name);
    if (picon == cache.end()) {
        QIcon icon(NAMEICONRES(name));
        cache.insert(name, icon);
        return icon;
    } else {
        return *picon;
    }
}

template <typename T>
class LinkedList : public std::list<T> {
    using std::list<T>::list;

public:
    inline bool contains(const T &var) const {
        return std::find(this->begin(), this->end(), var) != this->end();
    }

    inline void append(const T &var) { this->push_back(var); }
    inline void append(T &&var) { this->push_back(var); }

    inline bool isEmpty() const { return this->empty(); }

    inline void removeOne(const T &var) {
        auto r = std::find(this->begin(), this->end(), var);
        if (r != this->end()) {
            this->erase(r);
        }
    }
};

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

    inline static QString processBytesCount(qint64 bytescount) {
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

    inline static QStringList getEncodings() {
        static QStringList encodings;

        if (encodings.isEmpty()) {
            for (auto e = 0; e < int(QStringConverter::Encoding::LastEncoding);
                 ++e) {
                auto ee = QStringConverter::nameForEncoding(
                    QStringConverter::Encoding(e));
                if (ee == QStringLiteral("ISO-8859-1")) {
                    encodings << QStringLiteral("ASCII");
                } else {
                    encodings << QString::fromLatin1(ee);
                }
            }
        }

        return encodings;
    }

    inline static QList<QCryptographicHash::Algorithm>
    supportedHashAlgorithms() {
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

    inline static QStringList supportedHashAlgorithmStringList() {
        static QStringList hashNames;
        if (hashNames.isEmpty()) {
            auto hashes = Utilities::supportedHashAlgorithms();
            auto hashe = QMetaEnum::fromType<QCryptographicHash::Algorithm>();
            for (auto &hash : hashes) {
                hashNames << QString::fromLatin1(hashe.valueToKey(hash));
            }
        }
        return hashNames;
    }

    inline static QByteArray getMd5(const QString &filename) {
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

    inline static QByteArray getMd5(const QByteArray &data) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(data);
        return hash.result();
    }

    inline static constexpr bool checkIsLittleEndian() {
#if WING_LITTLE_ENDIAN
        return true;
#else
        return false;
#endif
    }

    template <typename T>
    static inline T qToBigEndian(T source) {
        return ::qToBigEndian(source);
    }

    template <typename T>
    static inline T qToLittleEndian(T source) {
        return ::qToLittleEndian(source);
    }

    template <typename T>
    static inline T processEndian(T source, bool isLitteEndian) {
        if (checkIsLittleEndian()) {
            if (!isLitteEndian) {
                return qToBigEndian(source);
            }
        } else {
            if (isLitteEndian) {
                return qToLittleEndian(source);
            }
        }
        return source;
    }

    inline static QIcon getIconFromFile(QStyle *style,
                                        const QString &filename) {
        QFileIconProvider prov;
        auto qicon = prov.icon(QFileInfo(filename));
        if (qicon.availableSizes().isEmpty()) {
            return style->standardIcon(QStyle::SP_FileIcon);
        }
        return qicon;
    }

    inline static void moveToCenter(QWidget *window) {
        if (window == nullptr)
            return;
        auto screen = qApp->primaryScreen()->availableSize();
        window->move((screen.width() - window->width()) / 2,
                     (screen.height() - window->height()) / 2);
    }

    inline static bool isTextFile(const QMimeType &type) {
        return type.inherits(QStringLiteral("text/plain"));
    }

    inline static bool isTextFile(const QFileInfo &info) {
        QMimeDatabase db;
        auto t = db.mimeTypeForFile(info, QMimeDatabase::MatchContent);
        return isTextFile(t);
    }

    inline static QString getAppDataPath() {
        return QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);
    }

    inline static void applyItemViewProperty(QAbstractItemView *view) {
        view->setEditTriggers(QTableView::EditTrigger::NoEditTriggers);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(
            QAbstractItemView::SelectionBehavior::SelectRows);
        view->setFocusPolicy(Qt::StrongFocus);
        view->setAlternatingRowColors(true);
    }

    inline static void applyTableViewProperty(QTableView *view) {
        applyItemViewProperty(view);
        view->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
        auto hheader = view->horizontalHeader();
        hheader->setDefaultAlignment(Qt::AlignCenter);
        hheader->setStretchLastSection(true);
        hheader->setHighlightSections(false);
        auto font = view->font();
        auto metric = QFontMetrics(font);
        hheader->setMinimumSectionSize(metric.horizontalAdvance(QChar::Space) *
                                       15);
    }

    inline static QString getAbsoluteDirPath(const QString &fileName) {
        return QFileInfo(fileName).absoluteDir().absolutePath();
    }

    inline static void applyTreeViewProperty(QTreeView *view) {
        applyItemViewProperty(view);
        view->setRootIsDecorated(true);
        view->setUniformRowHeights(true);
        auto hheader = view->header();
        hheader->setDefaultAlignment(Qt::AlignCenter);
        hheader->setStretchLastSection(true);
        hheader->setHighlightSections(false);
        auto font = view->font();
        auto metric = QFontMetrics(font);
        hheader->setMinimumSectionSize(metric.horizontalAdvance(QChar::Space) *
                                       15);
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

    inline static QByteArray encodingString(const QString &str,
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

    inline static QString realEncodingName(const QString &enc = {}) {
        if (enc.isEmpty() ||
            enc.compare(QStringLiteral("ASCII"), Qt::CaseInsensitive) == 0) {
            return QStringLiteral("ISO-8859-1");
        }
        return enc;
    }

    inline static QString decodingString(const QByteArray &buffer,
                                         const QString &enc = {}) {
        auto encoding = realEncodingName(enc);

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

    inline static bool isValidIdentifier(const QString &str) {
        if (str.isEmpty()) {
            return false;
        }
        auto pch = str.cbegin();
        if (pch->isDigit()) {
            return false;
        }
        pch++;
        for (; pch != str.cend(); pch++) {
            if (!pch->isLetterOrNumber() && *pch != '_') {
                return false;
            }
        }
        return true;
    }

    inline static QString getUrlString(const QString &fileName) {
        return QUrl::fromLocalFile(fileName).toString(QUrl::FullyEncoded);
    }

    inline static QString getASPredefPath() {
        QDir datap(Utilities::getAppDataPath());
        return datap.absoluteFilePath(QStringLiteral("as.predefined"));
    }

    inline static QString getDeviceFileName(const QString &ext,
                                            const QString &file) {
        return QStringLiteral("wdrv://") + ext + QStringLiteral("/") + file;
    }
};

#endif // UTILITIES_H
