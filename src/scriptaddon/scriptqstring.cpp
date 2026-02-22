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

#include "scriptqstring.h"
#include "AngelScript/sdk/add_on/autowrapper/aswrappedcall.h"
#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "angelscript.h"
#include "class/angelscripthelper.h"

#include <QMetaEnum>
#include <QString>

#include <unordered_map>

enum DoubleFmt { DFDecimal, DFExponent, DFSignificantDigits };

BEGIN_AS_NAMESPACE

class CQStringFactory : public asIStringFactory {
public:
    CQStringFactory() {}
    ~CQStringFactory() { Q_ASSERT(stringCache.empty()); }

    const void *GetStringConstant(const char *data, asUINT length) override {
        auto str = QString::fromUtf8(data, length);
        auto it = stringCache.find(str);
        if (it != stringCache.end())
            it->second++;
        else
            it = stringCache.emplace(str, 1).first;
        return reinterpret_cast<const void *>(&it->first);
    }

    int ReleaseStringConstant(const void *str) override {
        if (!str)
            return asERROR;

        const QString *qstr = reinterpret_cast<const QString *>(str);
        auto it = stringCache.find(*qstr);

        if (it != stringCache.end()) {
            it->second--;
            if (it->second == 0) {
                stringCache.erase(it);
            }
        }

        return asSUCCESS;
    }

    int GetRawStringData(const void *str, char *data,
                         asUINT *length) const override {
        if (!str)
            return asERROR;

        static const QString *qstr = nullptr;
        static QByteArray cache;
        auto nstr = reinterpret_cast<const QString *>(str);

        if (qstr != nstr) {
            qstr = nstr;
            cache = qstr->toUtf8();
        }

        if (length) {
            *length = cache.size();
        }
        if (data) {
            memcpy(data, cache.constData(), cache.size());
            qstr = nullptr;
            cache.clear();
            cache.squeeze();
        }

        return asSUCCESS;
    }

    std::unordered_map<QString, int> stringCache;
};

static CQStringFactory *stringFactory = nullptr;

// Make this public so the application can also use the string
// factory and share the string constants if so desired, or to
// monitor the size of the string factory cache.
CQStringFactory *GetQStringFactorySingleton() {
    if (stringFactory == nullptr) {
        // Make sure no other thread is creating the string factory at the same
        // time
        asAcquireExclusiveLock();
        if (stringFactory == nullptr) {
            // The following instance will be destroyed by the global
            // CStdStringFactoryCleaner instance upon application shutdown
            stringFactory = new CQStringFactory();
        }
        asReleaseExclusiveLock();
    }
    return stringFactory;
}

class CQStringFactoryCleaner {
public:
    ~CQStringFactoryCleaner() {
        if (stringFactory) {
            // Only delete the string factory if the stringCache is empty
            // If it is not empty, it means that someone might still attempt
            // to release string constants, so if we delete the string factory
            // the application might crash. Not deleting the cache would
            // lead to a memory leak, but since this is only happens when the
            // application is shutting down anyway, it is not important.
            if (stringFactory->stringCache.empty()) {
                delete stringFactory;
                stringFactory = nullptr;
            }
        }
    }
};

Q_GLOBAL_STATIC(CQStringFactoryCleaner, cleaner)

static void ConstructString(QString *thisPointer) {
    new (thisPointer) QString();
}

static void ConstructStringChar(const QChar &ch, QString *thisPointer) {
    new (thisPointer) QString(ch);
}

static void CopyConstructString(const QString &other, QString *thisPointer) {
    new (thisPointer) QString(other);
}

static void DestructString(QString *thisPointer) { thisPointer->~QString(); }

static QString &AddAssignStringToString(const QString &str, QString &dest) {
    // We don't register the method directly because some compilers
    // and standard libraries inline the definition, resulting in the
    // linker being unable to find the declaration.
    // Example: CLang/LLVM with XCode 4.3 on OSX 10.7
    dest += str;
    return dest;
}

// bool string::isEmpty()
// bool string::empty() // if AS_USE_STLNAMES == 1
static bool StringIsEmpty(const QString &str) {
    // We don't register the method directly because some compilers
    // and standard libraries inline the definition, resulting in the
    // linker being unable to find the declaration
    // Example: CLang/LLVM with XCode 4.3 on OSX 10.7
    return str.isEmpty();
}

#if AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE == 0
static QString &AssignUInt64ToString(asQWORD i, QString &dest) {
    dest = QString::number(i);
    return dest;
}

static QString &AddAssignUInt64ToString(asQWORD i, QString &dest) {
    dest += QString::number(i);
    return dest;
}

static QString AddStringUInt64(const QString &str, asQWORD i) {
    return str + QString::number(i);
}

static QString AddInt64String(asINT64 i, const QString &str) {
    return QString::number(i) + str;
}

static QString &AssignInt64ToString(asINT64 i, QString &dest) {
    dest = QString::number(i);
    return dest;
}

static QString &AddAssignInt64ToString(asINT64 i, QString &dest) {
    dest += QString::number(i);
    return dest;
}

static QString AddStringInt64(const QString &str, asINT64 i) {
    return str + QString::number(i);
}

static QString AddUInt64String(asQWORD i, const QString &str) {
    return QString::number(i) + str;
}

static QString &AssignDoubleToString(double f, QString &dest) {
    dest = QString::number(f);
    return dest;
}

static QString &AddAssignDoubleToString(double f, QString &dest) {
    dest += QString::number(f);
    return dest;
}

static QString &AssignFloatToString(float f, QString &dest) {
    dest = QString::number(f);
    return dest;
}

static QString &AddAssignFloatToString(float f, QString &dest) {
    dest += QString::number(f);
    return dest;
}

static QString &AssignCharToString(const QChar &ch, QString &dest) {
    dest = ch;
    return dest;
}

static QString &AddAssignCharToString(const QChar &ch, QString &dest) {
    dest += ch;
    return dest;
}

static QString &AssignBoolToString(bool b, QString &dest) {
    dest = b ? QStringLiteral("true") : QStringLiteral("false");
    return dest;
}

static QString &AddAssignBoolToString(bool b, QString &dest) {
    dest += b ? QStringLiteral("true") : QStringLiteral("false");
    return dest;
}

static QString AddStringDouble(const QString &str, double f) {
    return str + QString::number(f);
}

static QString AddDoubleString(double f, const QString &str) {
    return QString::number(f) + str;
}

static QString AddStringFloat(const QString &str, float f) {
    return str + QString::number(f);
}

static QString AddFloatString(float f, const QString &str) {
    return QString::number(f) + str;
}

static QString AddStringChar(const QString &str, const QChar &ch) {
    return str + ch;
}

static QString AddCharString(const QChar &ch, const QString &str) {
    return ch + str;
}

static QString AddStringBool(const QString &str, bool b) {
    return str + (b ? QStringLiteral("true") : QStringLiteral("false"));
}

static QString AddBoolString(bool b, const QString &str) {
    return (b ? QStringLiteral("true") : QStringLiteral("false")) + str;
}
#endif

static QChar *StringCharAt(unsigned int i, QString &str) {
    if (asDWORD(i) >= asDWORD(str.size())) {
        // Set a script exception
        asIScriptContext *ctx = asGetActiveContext();
        ctx->SetException("Out of range");

        // Return a null pointer
        return nullptr;
    }

    return str.data() + i;
}

// AngelScript signature:
// int string::opCmp(const string &in) const
static int StringCmp(const QString &a, const QString &b) {
    int cmp = 0;
    if (a < b)
        cmp = -1;
    else if (a > b)
        cmp = 1;
    return cmp;
}

static QString StringAdd(const QString &a, const QString &b) { return a + b; }

// This function returns the index of the first position where the one of the
// bytes in substring exists in the input string. If the characters in the
// substring doesn't exist in the input string -1 is returned.
//
// AngelScript signature:
// int string::findFirstOf(const string &in sub, uint start = 0) const
static int StringFindFirstOf(const QString &sub, asUINT start,
                             const QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    return (int)str.indexOf(sub, start);
}

// This function returns the index of the last position where the one of the
// bytes in substring exists in the input string. If the characters in the
// substring doesn't exist in the input string -1 is returned.
//
// AngelScript signature:
// int string::findLastOf(const string &in sub, uint start = -1) const
static int StringFindLastOf(const QString &sub, asUINT start,
                            const QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    return (int)str.lastIndexOf(sub, start);
}

// AngelScript signature:
// void string::insert(uint pos, const string &in other)
static void StringInsert(unsigned int pos, const QString &other, QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    str.insert(pos, other);
}

// AngelScript signature:
// void string::erase(uint pos, int count = -1)
static void StringErase(unsigned int pos, int count, QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    str.remove(pos, count);
}

// AngelScript signature:
// uint string::length() const
static asUINT StringLength(const QString &str) {
    // We don't register the method directly because the return type changes
    // between 32bit and 64bit platforms
    return (asUINT)str.length();
}

// AngelScript signature:
// void string::resize(uint l)
static void StringResize(asUINT l, QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    str.resize(l);
}

// AngelScript signature:
// string number(int32 val, int base=10)
static QString Int32toString(asINT32 value, int base) {
    return QString::number(value, base);
}

// AngelScript signature:
// string number(int32 val, int base=10)
static QString UInt32toString(asDWORD value, int base) {
    return QString::number(value, base);
}

// AngelScript signature:
// string number(int64 val, int base=10)
static QString Int64toString(asINT64 value, int base) {
    return QString::number(value, base);
}

// AngelScript signature:
// string number(uint64 val, int base=10)
static QString UInt64String(asQWORD value, int base) {
    return QString::number(value, base);
}

// AngelScript signature:
// string number(float, string::DoubleFmt format=
//          string::DoubleFmt::DFSignificantDigits, int precision=6)
static QString Float2String(float value, DoubleFmt format, int precision) {
    char fmt = 'g';
    switch (format) {
    case DFDecimal:
        fmt = 'f';
        break;
    case DFExponent:
        fmt = 'e';
        break;
    case DFSignificantDigits:
        fmt = 'g';
        break;
    }
    return QString::number(value, fmt, precision);
}

// AngelScript signature:
// string number(double, DoubleFmt format=
//          string::DoubleFmt::DFSignificantDigits, int precision=6)
static QString Double2String(double value, DoubleFmt format, int precision) {
    char fmt = 'g';
    switch (format) {
    case DFDecimal:
        fmt = 'f';
        break;
    case DFExponent:
        fmt = 'e';
        break;
    case DFSignificantDigits:
        fmt = 'g';
        break;
    }
    return QString::number(value, fmt, precision);
}

// AngelScript signature:
// int parseInt(const string &in val, int base = 10, bool &out ok = void)
static asINT32 parseInt(const QString &val, int base, bool *ok) {
    return val.toInt(ok, base);
}

// AngelScript signature:
// uint parseUInt(const string &in val, int base = 10, bool &out ok = void)
static asDWORD parseUInt(const QString &val, int base, bool *ok) {
    return val.toUInt(ok, base);
}

// AngelScript signature:
// int64 parseInt64(const string &in val, int base = 10, bool &out ok = void)
static asINT64 parseInt64(const QString &val, int base, bool *ok) {
    return val.toLongLong(ok, base);
}

// AngelScript signature:
// uint64 parseUInt64(const string &in val, int base = 10, bool &out ok = void)
static asQWORD parseUInt64(const QString &val, int base, bool *ok) {
    return val.toULongLong(ok, base);
}

// AngelScript signature:
// double parseFloat(const string &in val, bool &out ok = void)
float parseFloat(const QString &val, bool *ok) { return val.toFloat(ok); }

// AngelScript signature:
// double parseDouble(const string &in val, bool &out ok = void)
double parseDouble(const QString &val, bool *ok) { return val.toDouble(ok); }

// This function returns a string containing the substring of the input string
// determined by the starting index and count of characters.
//
// AngelScript signature:
// string string::substr(uint start = 0, int count = -1) const
static QString StringSubString(asUINT start, int count, const QString &str) {
    return str.mid(start, count);
}

// String equality comparison.
// Returns true iff lhs is equal to rhs.
//
// For some reason gcc 4.7 has difficulties resolving the
// asFUNCTIONPR(operator==, (const string &, const string &)
// makro, so this wrapper was introduced as work around.
static bool StringEquals(const QString &lhs, const QString &rhs) {
    return lhs == rhs;
}

//=================================================
// some functions ADDITIONALLY exported by wingsummer

// int compare(const string &in val, bool caseSensitive = true) const
static int stringCompare(const QString &other, bool caseSensitive,
                         const QString &str) {
    return str.compare(other,
                       caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

// bool contains(const string &in val, bool caseSensitive = true) const
static bool stringContains(const QString &lhs, bool caseSensitive,
                           const QString &str) {
    return str.contains(lhs, caseSensitive ? Qt::CaseSensitive
                                           : Qt::CaseInsensitive);
}

// bool endsWith(const string &in val, bool caseSensitive = true) const
static bool stringEndsWith(const QString &lhs, bool caseSensitive,
                           const QString &str) {
    return str.endsWith(lhs, caseSensitive ? Qt::CaseSensitive
                                           : Qt::CaseInsensitive);
}

// string repeated(qsizetype times)
static QString stringRepeated(qsizetype times, const QString &str) {
    return str.repeated(times);
}

// string replace(const string &in before,const string &in after,
//    bool caseSensitive = true)
static QString stringReplace(const QString &before, const QString &after,
                             bool caseSensitive, QString &str) {
    return str.replace(before, after,
                       caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

// string simplified()
static QString stringSimplified(const QString &str) { return str.simplified(); }

// bool startsWith(const string &in substr, bool caseSensitive = true)
static bool stringStartsWith(const QString &substr, bool caseSensitive,
                             const QString &str) {
    return str.startsWith(substr, caseSensitive ? Qt::CaseSensitive
                                                : Qt::CaseInsensitive);
}

// array<string>@ split(const QString &sep, bool skipEmpty = false,
//      bool caseSensitive = true)
static CScriptArray *stringSplit(const QString &sep, bool skipEmpty,
                                 bool caseSensitive, const QString &str) {
    return retarrayWrapperFunction(
        [&]() -> QStringList {
            return str.split(
                sep, skipEmpty ? Qt::SkipEmptyParts : Qt::KeepEmptyParts,
                caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        },
        "array<string>");
}

static QString fromAscii(CScriptArray *array) {
    return QString::fromLatin1(
        reinterpret_cast<const char *>(array->GetBuffer()), array->GetSize());
}

static CScriptArray *toAsciiArray(const QString &s) {
    return byteArrayWrapperFunction([s]() { return s.toLatin1(); });
}

static QString fromUtf8(CScriptArray *array) {
    return QString::fromUtf8(reinterpret_cast<const char *>(array->GetBuffer()),
                             array->GetSize());
}

static CScriptArray *toUtf8Array(const QString &s) {
    return byteArrayWrapperFunction([s]() { return s.toUtf8(); });
}

static QString fromUtf16(CScriptArray *array) {
    return QString::fromUtf16(
        reinterpret_cast<const char16_t *>(array->GetBuffer()),
        array->GetSize() / sizeof(char16_t));
}

static CScriptArray *toUtf16Array(const QString &s) {
    return byteArrayWrapperFunction([s]() {
        auto data = s.utf16();
        return QByteArray(reinterpret_cast<const char *>(data),
                          s.size() * sizeof(char16_t));
    });
}

static QString fromUcs4(CScriptArray *array) {
    return QString::fromUcs4(
        reinterpret_cast<const char32_t *>(array->GetBuffer()),
        array->GetSize() / sizeof(char32_t));
}

static CScriptArray *toUcs4Array(const QString &s) {
    return byteArrayWrapperFunction([s]() { return s.toUcs4(); });
}

static QString fromRawData(CScriptArray *array) {
    auto total = array->GetSize();
    QString buffer;

    buffer.reserve(total);
    for (asUINT i = 0; i < total; ++i) {
        auto ch = reinterpret_cast<const QChar *>(array->At(i));
        buffer.append(*ch);
    }

    return buffer;
}

static void appendStr(const QString &str, QString &s) { s.append(str); }

static void appendCh(const QChar &ch, QString &s) { s.append(ch); }

static void prependStr(const QString &str, QString &s) { s.prepend(str); }

static void prependCh(const QChar &ch, QString &s) { s.prepend(ch); }

static CScriptArray *toRawData(const QString &s) {
    return retarrayWrapperFunction(
        [s]() {
            QList<QChar> data;
            data.reserve(s.length());
            for (auto &ch : s) {
                data.append(ch);
            }
            return data;
        },
        "array<char>");
}

asUINT String_opForBegin(const QString *) { return 0; }

bool String_opForEnd(asUINT iter, const QString *arr) {
    if (arr == nullptr || arr->length() <= iter)
        return true;

    return false;
}

asUINT String_opForNext(asUINT iter, const QString *) { return iter + 1; }

asUINT String_opForValue1(asUINT iter, const QString *) { return iter; }

//=================================================

static void ConstructChar(QChar *thisPointer) { new (thisPointer) QChar(); }

static void ConstructCharInt(quint16 v, QChar *thisPointer) {
    new (thisPointer) QChar(v);
}

static void ConstructCharString(QString *v, QChar *thisPointer) {
    if (v->isEmpty()) {
        new (thisPointer) QChar();
    } else {
        new (thisPointer) QChar(v->at(0));
    }
}

static void CopyConstructChar(const QChar &other, QChar *thisPointer) {
    new (thisPointer) QChar(other);
}

static void DestructChar(QChar *thisPointer) { thisPointer->~QChar(); }

//=================================================

void RegisterQString_Native(asIScriptEngine *engine) {
    int r = 0;
    Q_UNUSED(r);

    // register the char type
    r = engine->RegisterObjectType("char", sizeof(QChar),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_ALLINTS |
                                       asGetTypeTraits<QChar>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("char", asBEHAVE_CONSTRUCT, "void f()",
                                        asFUNCTION(ConstructChar),
                                        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "char", asBEHAVE_CONSTRUCT, "void f(uint16)",
        asFUNCTION(ConstructCharInt), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "char", asBEHAVE_CONSTRUCT, "void f(const char &in)",
        asFUNCTION(CopyConstructChar), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour("char", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(DestructChar),
                                        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    r = engine->RegisterObjectMethod(
        "char", "int16 unicode() const",
        asMETHODPR(QChar, unicode, () const, ushort), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#else
    r = engine->RegisterObjectMethod(
        "char", "int16 unicode() const",
        asMETHODPR(QChar, unicode, () const, char16_t), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#endif

    r = engine->RegisterObjectMethod(
        "char", "char &opAssign(const char &in)",
        asMETHODPR(QChar, operator=, (const QChar &), QChar &),
        asCALL_THISCALL);
    Q_UNUSED(r);
    Q_ASSERT(r >= 0);

    // Register the string type
#if AS_CAN_USE_CPP11
    // With C++11 it is possible to use asGetTypeTraits to automatically
    // determine the correct flags to use
    r = engine->RegisterObjectType("string", sizeof(QString),
                                   asOBJ_VALUE | asGetTypeTraits<QString>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#else
    r = engine->RegisterObjectType("string", sizeof(QString),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#endif

    r = engine->RegisterObjectBehaviour(
        "char", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(ConstructCharString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterStringFactory("string", GetQStringFactorySingleton());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register the object operator overloads
    r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,
                                        "void f()", asFUNCTION(ConstructString),
                                        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "string", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(CopyConstructString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(DestructString),
                                        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "string", asBEHAVE_CONSTRUCT, "void f(const char &in)",
        asFUNCTION(ConstructStringChar), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string &opAssign(const string &in)",
        asMETHODPR(QString, operator=, (const QString &), QString &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM,
    // otherwise the linker fails
    r = engine->RegisterObjectMethod(
        "string", "string &opAddAssign(const string &in)",
        asFUNCTION(AddAssignStringToString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    //	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const
    // string &in)", asMETHODPR(string, operator+=, (const string&), string&),
    // asCALL_THISCALL); Q_ASSERT( r >= 0 );

    // Need to use a wrapper for operator== otherwise gcc 4.7 fails to compile
    r = engine->RegisterObjectMethod(
        "string", "bool opEquals(const string &in) const",
        asFUNCTIONPR(StringEquals, (const QString &, const QString &), bool),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "int opCmp(const string &in) const", asFUNCTION(StringCmp),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "string", "string opAdd(const string &in) const", asFUNCTION(StringAdd),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "uint length() const",
                                     asFUNCTION(StringLength),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "void resize(uint)",
                                     asFUNCTION(StringResize),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM,
    // otherwise the linker fails
    //	r = engine->RegisterObjectMethod("string", "bool isEmpty() const",
    // asMETHOD(string, empty), asCALL_THISCALL); Q_ASSERT( r >= 0 );
    r = engine->RegisterObjectMethod("string", "bool isEmpty() const",
                                     asFUNCTION(StringIsEmpty),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register the index operator, both as a mutator and as an inspector
    // Note that we don't register the operator[] directly, as it doesn't do
    // bounds checking
    r = engine->RegisterObjectMethod("string", "char &opIndex(uint)",
                                     asFUNCTION(StringCharAt),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "const char &opIndex(uint) const", asFUNCTION(StringCharAt),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

#if AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE == 0
    // Automatic conversion from values
    r = engine->RegisterObjectMethod("string", "string &opAssign(double)",
                                     asFUNCTION(AssignDoubleToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)",
                                     asFUNCTION(AddAssignDoubleToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(double) const",
                                     asFUNCTION(AddStringDouble),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const",
                                     asFUNCTION(AddDoubleString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "string", "string &opAssign(const char &in)",
        asFUNCTION(AssignCharToString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string &opAddAssign(const char &in)",
        asFUNCTION(AddAssignCharToString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string opAdd(const char &in) const",
        asFUNCTION(AddStringChar), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string opAdd_r(const char &in) const",
        asFUNCTION(AddCharString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(float)",
                                     asFUNCTION(AssignCharToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)",
                                     asFUNCTION(AddAssignFloatToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(float) const",
                                     asFUNCTION(AddStringFloat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const",
                                     asFUNCTION(AddFloatString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(int64)",
                                     asFUNCTION(AssignInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)",
                                     asFUNCTION(AddAssignInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(int64) const",
                                     asFUNCTION(AddStringInt64),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const",
                                     asFUNCTION(AddInt64String),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)",
                                     asFUNCTION(AssignUInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)",
                                     asFUNCTION(AddAssignUInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const",
                                     asFUNCTION(AddStringUInt64),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const",
                                     asFUNCTION(AddUInt64String),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(bool)",
                                     asFUNCTION(AssignBoolToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)",
                                     asFUNCTION(AddAssignBoolToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(bool) const",
                                     asFUNCTION(AddStringBool),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const",
                                     asFUNCTION(AddBoolString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#endif

    // Utilities
    r = engine->RegisterObjectMethod(
        "string", "void append(const string &in str)", asFUNCTION(appendStr),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "void append(const char &in ch)",
                                     asFUNCTION(appendCh),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "void prepend(const string &in str)", asFUNCTION(prependStr),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "void prepend(const char &in ch)", asFUNCTION(prependCh),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string substr(uint start = 0, int count = -1) const",
        asFUNCTION(StringSubString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "int findFirstOf(const string &in, uint start = 0) const",
        asFUNCTION(StringFindFirstOf), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "int findLastOf(const string &in, uint start = 0) const",
        asFUNCTION(StringFindLastOf), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "string", "void insert(uint pos, const string &in other)",
        asFUNCTION(StringInsert), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "void erase(uint pos, int count = -1)",
        asFUNCTION(StringErase), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "string",
        "int compare(const string &in val, bool caseSensitive = true) const",
        asFUNCTION(stringCompare), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "bool contains(const string &in val, bool caseSensitive = true) const",
        asFUNCTION(stringContains), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "bool endsWith(const string &in val, bool caseSensitive = true) const",
        asFUNCTION(stringEndsWith), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string repeated(" QSIZETYPE " times)",
        asFUNCTION(stringRepeated), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "string replace(const string &in before,const string &in after, "
        "bool caseSensitive = true)",
        asFUNCTION(stringReplace), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string simplified()",
                                     asFUNCTION(stringSimplified),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "bool startsWith(const string &in, bool caseSensitive = true)",
        asFUNCTION(stringStartsWith), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "array<string>@ split(const string &in, bool skipEmpty = false, "
        "bool caseSensitive = true)",
        asFUNCTION(stringSplit), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "string", "array<uint8>@ toAsciiArray() const",
        asFUNCTION(toAsciiArray), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "array<uint8>@ toUtf8Array() const", asFUNCTION(toUtf8Array),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "array<uint8>@ toUtf16Array() const",
        asFUNCTION(toUtf16Array), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "array<uint8>@ toUcs4Array() const", asFUNCTION(toUcs4Array),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "array<char>@ toRawData() const",
                                     asFUNCTION(toRawData),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Support for foreach
    r = engine->RegisterObjectMethod(
        "string", "uint opForBegin() const",
        asFUNCTIONPR(String_opForBegin, (const QString *), asUINT),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "bool opForEnd(uint) const",
        asFUNCTIONPR(String_opForEnd, (asUINT, const QString *), bool),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "uint opForNext(uint) const",
        asFUNCTIONPR(String_opForNext, (asUINT, const QString *), asUINT),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "const char opForValue0(uint index) const",
        asMETHODPR(QString, at, (qsizetype) const, const QChar),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "uint opForValue1(uint index) const",
        asFUNCTIONPR(String_opForValue1, (asUINT, const QString *), asUINT),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Global functions
    r = engine->SetDefaultNamespace("string");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterEnum("DoubleFmt");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("DoubleFmt", "DFDecimal",
                                  DoubleFmt::DFDecimal);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("DoubleFmt", "DFExponent",
                                  DoubleFmt::DFExponent);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("DoubleFmt", "DFSignificantDigits",
                                  DoubleFmt::DFSignificantDigits);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction("string number(int64 val, int base=10)",
                                       asFUNCTION(Int64toString), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("string number(uint64 val, int base=10)",
                                       asFUNCTION(UInt64String), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("string number(int32 val, int base=10)",
                                       asFUNCTION(Int32toString), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("string number(uint32 val, int base=10)",
                                       asFUNCTION(UInt32toString),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string number(float val, string::DoubleFmt "
        "format=string::DoubleFmt::DFSignificantDigits, int precision=6)",
        asFUNCTION(Float2String), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string number(double val, string::DoubleFmt "
        "format=string::DoubleFmt::DFSignificantDigits, int precision=6)",
        asFUNCTION(Double2String), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("int parseInt(const string &in, int "
                                       "base = 10, bool &out ok = void)",
                                       asFUNCTION(parseInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("uint parseUInt(const string &in, "
                                       "int base = 10, bool &out ok = void)",
                                       asFUNCTION(parseUInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("int64 parseInt64(const string &in, int "
                                       "base = 10, bool &out ok = void)",
                                       asFUNCTION(parseInt64), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("uint64 parseUInt64(const string &in, "
                                       "int base = 10, bool &out ok = void)",
                                       asFUNCTION(parseUInt64), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "float parseFloat(const string &in, bool &out ok = void)",
        asFUNCTION(parseFloat), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "double parseDouble(const string &in, bool &out ok = void)",
        asFUNCTION(parseDouble), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "string fromAscii(const array<uint8> &in array)", asFUNCTION(fromAscii),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromUtf8(const array<uint8> &in array)", asFUNCTION(fromUtf8),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromUtf16(const array<uint8> &in array)", asFUNCTION(fromUtf16),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromUcs4(const array<uint8> &in array)", asFUNCTION(fromUcs4),
        asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromRawData(const array<char> &in array)",
        asFUNCTION(fromRawData), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    engine->SetDefaultNamespace("");
}

static void ConstructStringGeneric(asIScriptGeneric *gen) {
    new (gen->GetObject()) QString();
}

static void CopyConstructStringGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetArgObject(0));
    new (gen->GetObject()) QString(*a);
}

static void DestructStringGeneric(asIScriptGeneric *gen) {
    QString *ptr = static_cast<QString *>(gen->GetObject());
    ptr->~QString();
}

static void AssignStringGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetArgObject(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self = *a;
    gen->SetReturnAddress(self);
}

static void AddAssignStringGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetArgObject(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self += *a;
    gen->SetReturnAddress(self);
}

static void StringEqualsGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    QString *b = static_cast<QString *>(gen->GetArgAddress(0));
    *(bool *)gen->GetAddressOfReturnLocation() = (*a == *b);
}

static void StringCmpGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    QString *b = static_cast<QString *>(gen->GetArgAddress(0));

    int cmp = 0;
    if (*a < *b)
        cmp = -1;
    else if (*a > *b)
        cmp = 1;

    *(int *)gen->GetAddressOfReturnLocation() = cmp;
}

static void StringAddGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    QString *b = static_cast<QString *>(gen->GetArgAddress(0));
    QString ret_val = *a + *b;
    gen->SetReturnObject(&ret_val);
}

static void StringLengthGeneric(asIScriptGeneric *gen) {
    QString *self = static_cast<QString *>(gen->GetObject());
    *static_cast<asUINT *>(gen->GetAddressOfReturnLocation()) =
        (asUINT)self->length();
}

static void StringIsEmptyGeneric(asIScriptGeneric *gen) {
    QString *self = reinterpret_cast<QString *>(gen->GetObject());
    *reinterpret_cast<bool *>(gen->GetAddressOfReturnLocation()) =
        StringIsEmpty(*self);
}

static void StringResizeGeneric(asIScriptGeneric *gen) {
    QString *self = static_cast<QString *>(gen->GetObject());
    self->resize(*static_cast<asUINT *>(gen->GetAddressOfArg(0)));
}

static void StringCharAtGeneric(asIScriptGeneric *gen) {
    asDWORD index = gen->GetArgDWord(0);
    QString *self = static_cast<QString *>(gen->GetObject());

    if (index >= asDWORD(self->size())) {
        // Set a script exception
        asIScriptContext *ctx = asGetActiveContext();
        ctx->SetException("Out of range");

        gen->SetReturnAddress(0);
    } else {
        gen->SetReturnAddress(&self[index]);
    }
}

#if AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE == 0
static void AssignInt2StringGeneric(asIScriptGeneric *gen) {
    asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self = QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AssignUInt2StringGeneric(asIScriptGeneric *gen) {
    asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self = QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AssignDouble2StringGeneric(asIScriptGeneric *gen) {
    double *a = static_cast<double *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self = QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AssignFloat2StringGeneric(asIScriptGeneric *gen) {
    float *a = static_cast<float *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self = QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AssignBool2StringGeneric(asIScriptGeneric *gen) {
    bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self = *a ? QStringLiteral("true") : QStringLiteral("false");
    gen->SetReturnAddress(self);
}

static void AddAssignDouble2StringGeneric(asIScriptGeneric *gen) {
    double *a = static_cast<double *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self += QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AddAssignFloat2StringGeneric(asIScriptGeneric *gen) {
    float *a = static_cast<float *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self += QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AddAssignInt2StringGeneric(asIScriptGeneric *gen) {
    asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self += QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AddAssignUInt2StringGeneric(asIScriptGeneric *gen) {
    asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self += QString::number(*a);
    gen->SetReturnAddress(self);
}

static void AddAssignBool2StringGeneric(asIScriptGeneric *gen) {
    bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
    QString *self = static_cast<QString *>(gen->GetObject());
    *self += *a ? QStringLiteral("true") : QStringLiteral("false");
    gen->SetReturnAddress(self);
}

static void AddString2DoubleGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    double *b = static_cast<double *>(gen->GetAddressOfArg(0));
    QString ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2FloatGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    float *b = static_cast<float *>(gen->GetAddressOfArg(0));
    QString ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2IntGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    asINT64 *b = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
    QString ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2UIntGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    asQWORD *b = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
    QString ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2BoolGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    bool *b = static_cast<bool *>(gen->GetAddressOfArg(0));
    QString ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddDouble2StringGeneric(asIScriptGeneric *gen) {
    double *a = static_cast<double *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    QString ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddFloat2StringGeneric(asIScriptGeneric *gen) {
    float *a = static_cast<float *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    QString ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddInt2StringGeneric(asIScriptGeneric *gen) {
    asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    QString ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddUInt2StringGeneric(asIScriptGeneric *gen) {
    asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    QString ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddBool2StringGeneric(asIScriptGeneric *gen) {
    bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    QString ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}
#endif

void RegisterQString_Generic(asIScriptEngine *engine) {
    int r = 0;
    Q_UNUSED(r);
    // QChar is wrapper of uint16 so...
    r = engine->RegisterTypedef("char", "uint16");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register the string type
    r = engine->RegisterObjectType("string", sizeof(QString),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterStringFactory("string", GetQStringFactorySingleton());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register the object operator overloads
    r = engine->RegisterObjectBehaviour(
        "string", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(ConstructStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "string", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(CopyConstructStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(DestructStringGeneric),
                                        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string &opAssign(const string &in)",
        asFUNCTION(AssignStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string &opAddAssign(const string &in)",
        asFUNCTION(AddAssignStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "bool opEquals(const string &in) const",
        asFUNCTION(StringEqualsGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "int opCmp(const string &in) const",
        asFUNCTION(StringCmpGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string opAdd(const string &in) const",
        asFUNCTION(StringAddGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register the object methods
    r = engine->RegisterObjectMethod("string", "uint length() const",
                                     asFUNCTION(StringLengthGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "void resize(uint)",
                                     asFUNCTION(StringResizeGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "bool isEmpty() const",
                                     asFUNCTION(StringIsEmptyGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register the index operator, both as a mutator and as an inspector
    r = engine->RegisterObjectMethod("string", "char &opIndex(uint)",
                                     asFUNCTION(StringCharAtGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "const char &opIndex(uint) const",
        asFUNCTION(StringCharAtGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

#if AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE == 0
    // Automatic conversion from values
    r = engine->RegisterObjectMethod("string", "string &opAssign(double)",
                                     asFUNCTION(AssignDouble2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)",
                                     asFUNCTION(AddAssignDouble2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(double) const",
                                     asFUNCTION(AddString2DoubleGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const",
                                     asFUNCTION(AddDouble2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(float)",
                                     asFUNCTION(AssignFloat2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)",
                                     asFUNCTION(AddAssignFloat2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(float) const",
                                     asFUNCTION(AddString2FloatGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const",
                                     asFUNCTION(AddFloat2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(int64)",
                                     asFUNCTION(AssignInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)",
                                     asFUNCTION(AddAssignInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(int64) const",
                                     asFUNCTION(AddString2IntGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const",
                                     asFUNCTION(AddInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)",
                                     asFUNCTION(AssignUInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)",
                                     asFUNCTION(AddAssignUInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const",
                                     asFUNCTION(AddString2UIntGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const",
                                     asFUNCTION(AddUInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string", "string &opAssign(bool)",
                                     asFUNCTION(AssignBool2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)",
                                     asFUNCTION(AddAssignBool2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd(bool) const",
                                     asFUNCTION(AddString2BoolGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const",
                                     asFUNCTION(AddBool2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#endif

    // Utilities
    r = engine->RegisterObjectMethod("string",
                                     "void append(const string &in str)",
                                     WRAP_FN(appendStr), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "void append(const char &in ch)",
                                     WRAP_FN(appendCh), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string",
                                     "void prepend(const string &in str)",
                                     WRAP_FN(prependStr), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string",
                                     "void prepend(const char &in ch)",
                                     WRAP_FN(prependCh), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "string substr(uint start = 0, int count = -1) const",
        WRAP_FN(StringSubString), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "int findFirstOf(const string &in, uint start = 0) const",
        WRAP_FN(StringFindFirstOf), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string", "int findLastOf(const string &in, uint start = 0) const",
        WRAP_FN(StringFindLastOf), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "string", "void insert(uint pos, const string &in other)",
        WRAP_FN(StringInsert), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string",
                                     "void erase(uint pos, int count = -1)",
                                     WRAP_FN(StringErase), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "string",
        "int compare(const string &in val, bool caseSensitive = true) const",
        WRAP_FN(stringCompare), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "bool contains(const string &in val, bool caseSensitive = true) const",
        WRAP_FN(stringContains), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "bool endsWith(const string &in val, bool caseSensitive = true) const",
        WRAP_FN(stringEndsWith), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string",
                                     "string repeated(" QSIZETYPE " times)",
                                     WRAP_FN(stringRepeated), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "string replace(const string &in before,const string &in after, "
        "bool caseSensitive = true)",
        WRAP_FN(stringReplace), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "string simplified()",
                                     asFUNCTION(stringSimplified),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "bool startsWith(const string &in, bool caseSensitive = true)",
        WRAP_FN(stringStartsWith), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "string",
        "array<string>@ split(const string &in, bool skipEmpty = false, "
        "bool caseSensitive = true)",
        WRAP_FN(stringSplit), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("string",
                                     "array<uint8>@ toAsciiArray() const",
                                     WRAP_FN(toAsciiArray), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string",
                                     "array<uint8>@ toUtf8Array() const",
                                     WRAP_FN(toUtf8Array), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string",
                                     "array<uint8>@ toUtf16Array() const",
                                     WRAP_FN(toUtf16Array), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string",
                                     "array<uint8>@ toUcs4Array() const",
                                     WRAP_FN(toUcs4Array), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("string", "array<char>@ toRawData() const",
                                     WRAP_FN(toRawData), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Global functions
    r = engine->SetDefaultNamespace("string");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterEnum("DoubleFmt");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("DoubleFmt", "DFDecimal",
                                  DoubleFmt::DFDecimal);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("DoubleFmt", "DFExponent",
                                  DoubleFmt::DFExponent);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("DoubleFmt", "DFSignificantDigits",
                                  DoubleFmt::DFSignificantDigits);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction("string number(int64 val, int base=10)",
                                       WRAP_FN(Int64toString), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("string number(uint64 val, int base=10)",
                                       WRAP_FN(UInt64String), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("string number(int32 val, int base=10)",
                                       WRAP_FN(Int32toString), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("string number(uint32 val, int base=10)",
                                       WRAP_FN(UInt32toString), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string number(float val, string::DoubleFmt "
        "format=string::DoubleFmt::DFSignificantDigits, int precision=6)",
        WRAP_FN(Float2String), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string number(double val, string::DoubleFmt "
        "format=string::DoubleFmt::DFSignificantDigits, int precision=6)",
        WRAP_FN(Double2String), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("int parseInt(const string &in, int "
                                       "base = 10, bool &out ok = void)",
                                       WRAP_FN(parseInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("uint parseUInt(const string &in, "
                                       "int base = 10, bool &out ok = void)",
                                       WRAP_FN(parseUInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("int64 parseInt64(const string &in, int "
                                       "base = 10, bool &out ok = void)",
                                       WRAP_FN(parseInt64), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("uint64 parseUInt64(const string &in, "
                                       "int base = 10, bool &out ok = void)",
                                       WRAP_FN(parseUInt64), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "float parseFloat(const string &in, bool &out ok = void)",
        WRAP_FN(parseFloat), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "double parseDouble(const string &in, bool &out ok = void)",
        WRAP_FN(parseDouble), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "string fromAscii(const array<uint8> &in array)", WRAP_FN(fromAscii),
        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromUtf8(const array<uint8> &in array)", WRAP_FN(fromUtf8),
        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromUtf16(const array<uint8> &in array)", WRAP_FN(fromUtf16),
        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromUcs4(const array<uint8> &in array)", WRAP_FN(fromUcs4),
        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction(
        "string fromRawData(const array<char> &in array)", WRAP_FN(fromRawData),
        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    engine->SetDefaultNamespace("");
}

void RegisterQString(asIScriptEngine *engine) {
    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
        RegisterQString_Generic(engine);
    else
        RegisterQString_Native(engine);
}

// This function takes as input an array of string handles as well as a
// delimiter and concatenates the array elements into one delimited string.
// Example:
//
// array<string> array = {"A", "B", "", "D"};
// string str = join(array, "|");
//
// The resulting string is:
//
// "A|B||D"
//
// AngelScript signature:
// string join(const array<string> &in array, const string &in delim)
static QString StringJoin(const CScriptArray &array, const QString &delim) {
    // Create the new string
    QString str;
    if (array.GetSize()) {
        int n;
        for (n = 0; n < (int)array.GetSize() - 1; n++) {
            str += *(const QString *)array.At(n);
            str += delim;
        }

        // Add the last part
        str += *(const QString *)array.At(n);
    }

    return str;
}

static void StringJoin_Generic(asIScriptGeneric *gen) {
    // Get the arguments
    CScriptArray *array = *(CScriptArray **)gen->GetAddressOfArg(0);
    QString *delim = *(QString **)gen->GetAddressOfArg(1);

    // Return the string
    new (gen->GetAddressOfReturnLocation()) QString(StringJoin(*array, *delim));
}

// This is where the utility functions are registered.
// The string type must have been registered first.
void RegisterQStringUtils(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("string");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY")) {
        r = engine->RegisterGlobalFunction(
            "string join(const array<string> &in, const string &in delim)",
            asFUNCTION(StringJoin_Generic), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    } else {
        r = engine->RegisterGlobalFunction(
            "string join(const array<string> &in, const string &in delim)",
            asFUNCTION(StringJoin), asCALL_CDECL);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }
    engine->SetDefaultNamespace("");
}

// This function returns the index of the first position where the one of the
// bytes in substring exists in the input string. If the characters in the
// substring doesn't exist in the input string -1 is returned.
//
// AngelScript signature:
// int string::findFirstOf(const regex::exp &in exp, uint start = 0) const
static int StringFindFirstOfReg(const QRegularExpression &exp, asUINT start,
                                const QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    return (int)str.indexOf(exp, start);
}

// This function returns the index of the last position where the one of the
// bytes in substring exists in the input string. If the characters in the
// substring doesn't exist in the input string -1 is returned.
//
// AngelScript signature:
// int string::findLastOf(const regex::exp &in exp, uint start = -1) const
static int StringFindLastOfReg(const QRegularExpression &exp, asUINT start,
                               const QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    return (int)str.lastIndexOf(exp, start);
}

// bool contains(const regex::exp &in exp) const
static bool stringContainsReg(const QRegularExpression &exp,
                              const QString &str) {
    return str.contains(exp);
}

// array<string>@ split(const regex::exp &exp, bool skipEmpty = false)
static CScriptArray *stringSplitReg(const QRegularExpression &exp,
                                    bool skipEmpty, const QString &str) {
    return retarrayWrapperFunction(
        [&]() -> QStringList {
            return str.split(exp, skipEmpty ? Qt::SkipEmptyParts
                                            : Qt::KeepEmptyParts);
        },
        "array<string>");
}

void RegisterQStringRegExSupport(asIScriptEngine *engine) {
    Q_ASSERT(engine->GetTypeInfoByName("regex::exp"));

    int r = engine->SetDefaultNamespace("string");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY")) {
        r = engine->RegisterObjectMethod(
            "string",
            "int findFirstOf(const regex::exp &in, uint start = 0) const",
            WRAP_FN(StringFindFirstOfReg), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterObjectMethod(
            "string",
            "int findLastOf(const regex::exp &in, int start = -1) const",
            WRAP_FN(StringFindLastOfReg), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterObjectMethod(
            "string", "bool contains(const regex::exp &in) const",
            WRAP_FN(stringContainsReg), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterObjectMethod(
            "string",
            "array<string>@ split(const regex::exp &in, "
            "bool skipEmpty = false)",
            WRAP_FN(stringSplitReg), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    } else {
        r = engine->RegisterObjectMethod(
            "string",
            "int findFirstOf(const regex::exp &in, uint start = 0) const",
            asFUNCTION(StringFindFirstOfReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterObjectMethod(
            "string",
            "int findLastOf(const regex::exp &in, int start = -1) const",
            asFUNCTION(StringFindLastOfReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterObjectMethod(
            "string", "bool contains(const regex::exp &in) const",
            asFUNCTION(stringContainsReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
        r = engine->RegisterObjectMethod(
            "string",
            "array<string>@ split(const regex::exp &in, "
            "bool skipEmpty = false)",
            asFUNCTION(stringSplitReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        Q_UNUSED(r);
    }
    engine->SetDefaultNamespace("");
}

END_AS_NAMESPACE
