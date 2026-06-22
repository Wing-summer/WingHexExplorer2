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
#include <QStringDecoder>
#include <QStyle>
#include <QTableView>
#include <QTreeView>
#include <QWidget>
#include <QtEndian>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#undef MessageBox // because of IWingPlugin
#else
#include <pwd.h>
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

Q_DECL_UNUSED static inline bool qEnvironmentVariableBool(const char *varName,
                                                          bool defaultValue) {
    bool ok;
    auto value = qEnvironmentVariableIntValue(varName, &ok);
    if (ok) {
        return !!value;
    } else {
        return defaultValue;
    }
}

template <class Key, class T>
class Cache {
    struct Chain {
        Chain() noexcept : prev(this), next(this) {}
        Chain *prev;
        Chain *next;
    };

    struct Node : public Chain {
        using KeyType = Key;
        using ValueType = T;

        Key key;
        T value;

        inline Node(Key k, T v) noexcept : Chain(), key(k), value(v) {}

        inline static void createInPlace(Node *n, Key k, T v) {
            new (n) Node(k, v);
        }

        inline void emplace(T v) { value = v; }

        inline Node(Node &&other)
            : Chain(other), key(std::move(other.key)),
              value(std::move(other.value)) {
            Q_ASSERT(this->prev);
            Q_ASSERT(this->next);
            this->prev->next = this;
            this->next->prev = this;
        }

    private:
        Q_DISABLE_COPY(Node)
    };

    using Data = QHashPrivate::Data<Node>;

    mutable Chain chain;
    Data d;
    qsizetype mx = 0;
    qsizetype total = 0;

    inline static void unlinkChain(Node *n) noexcept {
        n->prev->next = n->next;
        n->next->prev = n->prev;
    }

    inline void linkFront(Node *n) noexcept {
        n->prev = &chain;
        n->next = chain.next;
        chain.next->prev = n;
        chain.next = n;
    }

    inline void moveToFront(Node *n) const noexcept {
        if (chain.next == n)
            return;
        unlinkChain(n);
        n->prev = const_cast<Chain *>(&chain);
        n->next = chain.next;
        chain.next->prev = n;
        chain.next = n;
    }

    inline void unlink(Node *n) {
        unlinkChain(n);
        --total;
        auto it = d.findBucket(n->key);
        d.erase(it);
    }

    inline void trim(qsizetype m) {
        while (chain.prev != &chain && total > m) {
            Node *n = static_cast<Node *>(chain.prev);
            unlink(n);
        }
    }

    Q_DISABLE_COPY(Cache)

public:
    inline explicit Cache(qsizetype maxCost) noexcept : mx(maxCost) {}

    inline ~Cache() {
        static_assert(std::is_nothrow_destructible_v<Key>,
                      "Types with throwing destructors are not supported in Qt "
                      "containers.");
        static_assert(std::is_nothrow_destructible_v<T>,
                      "Types with throwing destructors are not supported in Qt "
                      "containers.");
        clear();
    }

    inline qsizetype maxCost() const noexcept { return mx; }

    inline void setMaxCost(qsizetype m) {
        mx = m;
        trim(mx);
    }

    inline qsizetype totalCost() const noexcept { return total; }

    inline qsizetype size() const noexcept { return qsizetype(d.size); }
    inline qsizetype count() const noexcept { return qsizetype(d.size); }
    inline bool isEmpty() const noexcept { return !d.size; }

    inline void clear() {
        d.clear();
        total = 0;
        chain.next = &chain;
        chain.prev = &chain;
    }

    inline bool insert(const Key &key, T object) {
        if (mx < 1) {
            remove(key);
            return false;
        }

        if (Node *existing = d.findNode(key)) {
            existing->emplace(object);
            moveToFront(existing);
            return true;
        }

        trim(mx - 1);

        auto result = d.findOrInsert(key);
        Node *n = result.it.node();
        Node::createInPlace(n, key, object);
        linkFront(n);
        ++total;
        return true;
    }

    inline T object(const Key &key) const {
        if (isEmpty())
            return T{};

        Node *n = d.findNode(key);
        if (!n)
            return T{};

        moveToFront(n);
        return n->value;
    }

    inline bool contains(const Key &key) const {
        return !isEmpty() && d.findNode(key) != nullptr;
    }

    inline bool remove(const Key &key) {
        if (isEmpty())
            return false;

        Node *n = d.findNode(key);
        if (!n)
            return false;

        unlink(n);
        return true;
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

    inline static QStringConverter::Encoding
    encodingForName(const QString &encoding) {
        auto en = QStringConverter::encodingForName(encoding.toUtf8());
        return en.value_or(QStringConverter::Encoding ::Latin1);
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
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
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

    inline static QByteArray
    encodingString(const QString &str,
                   const QStringConverter::Encoding &enc =
                       QStringConverter::Encoding ::Latin1) {
        QStringEncoder e(enc);
        return e.encode(str);
    }

    inline static QString
    decodingString(const QByteArray &buffer,
                   const QStringConverter::Encoding &enc =
                       QStringConverter::Encoding ::Latin1) {
        QStringDecoder dec(enc);
        return dec.decode(buffer);
    }

    static QString stringEncodingName(QStringConverter::Encoding encoding) {
        switch (encoding) {
        case QStringConverter::Encoding::Utf16:
            return QStringLiteral("UTF-16");
        case QStringConverter::Encoding::Utf16LE:
            return QStringLiteral("UTF-16LE");
        case QStringConverter::Encoding::Utf16BE:
            return QStringLiteral("UTF-16BE");
        case QStringConverter::Encoding::Utf32:
            return QStringLiteral("UTF-32");
        case QStringConverter::Encoding::Utf32LE:
            return QStringLiteral("UTF-32LE");
        case QStringConverter::Encoding::Utf32BE:
            return QStringLiteral("UTF-32BE");
        case QStringConverter::Encoding::Utf8:
            return QStringLiteral("UTF-8");
        case QStringConverter::Encoding::Latin1:
        default:
            return QStringLiteral("ASCII");
        }
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

    inline static QMap<QString, int> hexLineWidthMap() {
        static QMap<QString, int> caches{
            {"8", 8}, {"16", 16}, {"20", 20}, {"24", 24}, {"32", 32}};
        return caches;
    }

    inline static std::initializer_list<int> hexLineValues() {
        static auto hexline = std::initializer_list{8, 16, 20, 24, 32};
        return hexline;
    }

    inline static QStringList hexLineWidthStrings() {
        static QStringList caches{"8", "16", "20", "24", "32"};
        return caches;
    }

    inline static int hexLineWidthIdx(int value) {
        int idx;
        switch (value) {
        case 8:
            idx = 0;
            break;
        case 20:
            idx = 2;
            break;
        case 24:
            idx = 3;
            break;
        case 32:
            idx = 4;
            break;
        default:
            idx = 1;
            break;
        }
        return idx;
    }

    inline static int hexLineWidthValue(const QString &value) {
        return hexLineWidthMap().value(value, 16);
    }

    inline static int hexLineWidthIdxFromValue(const QString &value) {
        auto v = hexLineWidthValue(value);
        return hexLineWidthIdx(v);
    }

    inline static QString translateCharEscape(QStringView digits,
                                              int *advance) {
        Q_ASSERT(digits.size() > 0);
        Q_ASSERT(advance);

        *advance = 0;
        if (digits.at(0) == QLatin1Char('x')) {
            // We only support exactly 2 hex bytes with \x format
            if (digits.size() < 3)
                return QString();
            QByteArray number = digits.mid(1, 2).toLatin1();
            char *end;
            ulong ch = strtoul(number.constData(), &end, 16);
            if (*end != '\0')
                return QString();
            *advance = 2;
            return QString(QChar::fromLatin1(static_cast<char>(ch)));
        } else if (digits.at(0) == QLatin1Char('u')) {
            if (digits.size() < 5)
                return QString();
            QByteArray number = digits.mid(1, 4).toLatin1();
            char *end;
            ulong ch = strtoul(number.constData(), &end, 16);
            if (*end != '\0')
                return QString();
            *advance = 4;
            return QString(QChar(static_cast<ushort>(ch)));
        } else if (digits.at(0) == QLatin1Char('U')) {
            if (digits.size() < 9)
                return QString();
            QByteArray number = digits.mid(1, 8).toLatin1();
            char *end;
            ulong ch = strtoul(number.constData(), &end, 16);
            if (*end != '\0')
                return QString();

            *advance = 8;
            if (ch > 0xFFFFU) {
                const QChar utf16[2] = {
                    QChar::highSurrogate(ch),
                    QChar::lowSurrogate(ch),
                };
                return QString(utf16, 2);
            } else {
                return QString(QChar(static_cast<ushort>(ch)));
            }
        } else {
            // Octal no longer supported
            qFatal("Unsupported character escape prefix");
            return QString();
        }
    }

    inline static QString translateStringEscapes(const QString &text) {
        QString result;
        result.reserve(text.size());
        int start = 0;
        for (;;) {
            int pos = text.indexOf(QLatin1Char('\\'), start);
            if (pos < 0 || pos + 1 >= text.size())
                break;

            result.append(text.sliced(start, pos - start));
            QChar next = text.at(pos + 1);
            start = pos + 2;
            switch (next.unicode()) {
            case 'a':
                result.append(QLatin1Char('\a'));
                break;
            case 'b':
                result.append(QLatin1Char('\b'));
                break;
            case 'e':
                result.append(QLatin1Char('\x1b'));
                break;
            case 'f':
                result.append(QLatin1Char('\f'));
                break;
            case 'n':
                result.append(QLatin1Char('\n'));
                break;
            case 'r':
                result.append(QLatin1Char('\r'));
                break;
            case 't':
                result.append(QLatin1Char('\t'));
                break;
            case 'v':
                result.append(QLatin1Char('\v'));
                break;
            case '\\':
            case '?':
            case '\'':
            case '"':
                result.append(next);
                break;
            case 'x': // Hex byte
            case 'u': // Unicode character (16-bit)
            case 'U': // Unicode character (32-bit)
            {
                int advance;
                const QString chars = translateCharEscape(
                    QStringView(text).mid(pos + 1), &advance);
                if (chars.isEmpty()) {
                    // Translation failed
                    result.append(QLatin1Char('\\'));
                    result.append(next);
                } else {
                    result.append(chars);
                    start += advance;
                }
            } break;
            default:
                // Just keep unrecognized sequences untranslated
                result.append(QLatin1Char('\\'));
                result.append(next);
                break;
            }
        }

        result.append(text.sliced(start));
        return result;
    }
};

#endif // UTILITIES_H
