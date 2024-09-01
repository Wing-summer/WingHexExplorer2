#include "scriptqstring.h"
#include "AngelScript/add_on/scriptarray/scriptarray.h"
#include "angelscript.h"
#include "class/angelscripthelper.h"

#include <QHash>
#include <QMetaEnum>
#include <QString>

BEGIN_AS_NAMESPACE
typedef std::unordered_map<QString, int> Map_t;
END_AS_NAMESPACE

BEGIN_AS_NAMESPACE
class CQStringFactory : public asIStringFactory {
public:
    CQStringFactory() {}
    ~CQStringFactory() {
        // The script engine must release each string
        // constant that it has requested
        Q_ASSERT(stringCache.size() == 0);
    }

    const void *GetStringConstant(const char *data, asUINT length) {
        // The string factory might be modified from multiple
        // threads, so it is necessary to use a mutex.
        asAcquireExclusiveLock();

        QString str = QString::fromUtf8(data, length);

        Map_t::iterator it = stringCache.find(str);
        if (it != stringCache.end())
            it->second++;
        else
            it = stringCache.insert(Map_t::value_type(str, 1)).first;

        asReleaseExclusiveLock();

        return reinterpret_cast<const void *>(&it->first);
    }

    int ReleaseStringConstant(const void *str) {
        if (str == 0)
            return asERROR;

        int ret = asSUCCESS;

        // The string factory might be modified from multiple
        // threads, so it is necessary to use a mutex.
        asAcquireExclusiveLock();
        auto strv = *reinterpret_cast<const QString *>(str);

        Map_t::iterator it =
            stringCache.find(*reinterpret_cast<const QString *>(str));
        if (it == stringCache.end())
            ret = asERROR;
        else {
            it->second--;
            if (it->second == 0)
                stringCache.erase(it);
        }

        asReleaseExclusiveLock();

        return ret;
    }

    int GetRawStringData(const void *str, char *data, asUINT *length) const {
        if (str == 0)
            return asERROR;

        auto buffer = reinterpret_cast<const QString *>(str)->toUtf8();

        if (length)
            *length = buffer.length();

        if (data)
            memcpy(data, buffer.data(), buffer.length());

        return asSUCCESS;
    }

    // THe access to the string cache is protected with the common mutex
    // provided by AngelScript
    Map_t stringCache;
};

static CQStringFactory *stringFactory = nullptr;

// TODO: Make this public so the application can also use the string
//       factory and share the string constants if so desired, or to
//       monitor the size of the string factory cache.
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

static CQStringFactoryCleaner cleaner;

static void ConstructString(QString *thisPointer) {
    new (thisPointer) QString();
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

static QString AddStringBool(const QString &str, bool b) {
    return str + (b ? QStringLiteral("true") : QStringLiteral("false"));
}

static QString AddBoolString(bool b, const QString &str) {
    return (b ? QStringLiteral("true") : QStringLiteral("false")) + str;
}
#endif

static char *StringCharAt(unsigned int i, QString &str) {
    if (asDWORD(i) >= asDWORD(str.size())) {
        // Set a script exception
        asIScriptContext *ctx = asGetActiveContext();
        ctx->SetException("Out of range");

        // Return a null pointer
        return 0;
    }

    return reinterpret_cast<char *>(str.data() + i);
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

// This function returns the index of the first position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findFirst(const string &in sub, uint start = 0) const
static int StringFindFirst(const QString &sub, asUINT start,
                           const QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    return (int)str.indexOf(sub, start);
}

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
// string formatInt(int64 val, const string &in options)
static QString formatInt(asINT64 value, const QString &options) {
    return QString::asprintf(options.toLatin1(), value);
}

// AngelScript signature:
// string formatUInt(uint64 val, const string &in options)
static QString formatUInt(asQWORD value, const QString &options) {
    return QString::asprintf(options.toLatin1(), value);
}

// AngelScript signature:
// string formatFloat(double val, const string &in options, uint width, uint
// precision)
static QString formatFloat(double value, const QString &options) {
    return QString::asprintf(options.toLatin1(), value);
}

// AngelScript signature:
// int64 parseInt(const string &in val, uint base = 10, uint &out byteCount = 0)
static asINT64 parseInt(const QString &val, asUINT base, asUINT *byteCount) {
    return val.toInt(nullptr, base);
}

// AngelScript signature:
// uint64 parseUInt(const string &in val, uint base = 10, bool &out ok =
// 0)
static asQWORD parseUInt(const QString &val, asUINT base, bool *ok) {
    return val.toUInt(ok, base);
}

// AngelScript signature:
// double parseFloat(const string &in val, uint &out byteCount = 0)
double parseFloat(const QString &val, bool *ok) { return val.toFloat(ok); }

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

//=================================================

void RegisterQString_Native(asIScriptEngine *engine) {
    int r = 0;
    Q_UNUSED(r);

    // Register the string type
#if AS_CAN_USE_CPP11
    // With C++11 it is possible to use asGetTypeTraits to automatically
    // determine the correct flags to use
    r = engine->RegisterObjectType("string", sizeof(QString),
                                   asOBJ_VALUE | asGetTypeTraits<QString>());
    Q_ASSERT(r >= 0);
#else
    r = engine->RegisterObjectType("string", sizeof(QString),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
    Q_ASSERT(r >= 0);
#endif

    r = engine->RegisterStringFactory("string", GetQStringFactorySingleton());
    Q_ASSERT(r >= 0);

    // Register the object operator overloads
    r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,
                                        "void f()", asFUNCTION(ConstructString),
                                        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "string", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(CopyConstructString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(DestructString),
                                        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "string &opAssign(const string &in)",
        asMETHODPR(QString, operator=, (const QString &), QString &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    // Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM,
    // otherwise the linker fails
    r = engine->RegisterObjectMethod(
        "string", "string &opAddAssign(const string &in)",
        asFUNCTION(AddAssignStringToString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    //	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const
    // string &in)", asMETHODPR(string, operator+=, (const string&), string&),
    // asCALL_THISCALL); Q_ASSERT( r >= 0 );

    // Need to use a wrapper for operator== otherwise gcc 4.7 fails to compile
    r = engine->RegisterObjectMethod(
        "string", "bool opEquals(const string &in) const",
        asFUNCTIONPR(StringEquals, (const QString &, const QString &), bool),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "int opCmp(const string &in) const", asFUNCTION(StringCmp),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    r = engine->RegisterObjectMethod(
        "string", "string opAdd(const string &in) const",
        asFUNCTIONPR(operator+, (const QString &, const QString &), QString),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
#else
    r = engine->RegisterObjectMethod(
        "string", "string opAdd(const string &in) const",
        asFUNCTIONPR(operator+, (const QString &, const QString &),
                     const QString),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
#endif

    // The string length can be accessed through methods or through virtual
    // property
    // TODO: Register as size() for consistency with other types
#if AS_USE_ACCESSORS != 1
    r = engine->RegisterObjectMethod("string", "uint length() const",
                                     asFUNCTION(StringLength),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
#endif
    r = engine->RegisterObjectMethod("string", "void resize(uint)",
                                     asFUNCTION(StringResize),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
#if AS_USE_ACCESSORS == 1
    // Don't register these if STL names is used, as they conflict with the
    // method size()
    r = engine->RegisterObjectMethod(
        "string", "uint get_length() const property", asFUNCTION(StringLength),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "void set_length(uint) property",
                                     asFUNCTION(StringResize),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
#endif
    // Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM,
    // otherwise the linker fails
    //	r = engine->RegisterObjectMethod("string", "bool isEmpty() const",
    // asMETHOD(string, empty), asCALL_THISCALL); Q_ASSERT( r >= 0 );
    r = engine->RegisterObjectMethod("string", "bool isEmpty() const",
                                     asFUNCTION(StringIsEmpty),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

    // Register the index operator, both as a mutator and as an inspector
    // Note that we don't register the operator[] directly, as it doesn't do
    // bounds checking
    r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)",
                                     asFUNCTION(StringCharAt),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAt),
        asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

#if AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE == 0
    // Automatic conversion from values
    r = engine->RegisterObjectMethod("string", "string &opAssign(double)",
                                     asFUNCTION(AssignDoubleToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)",
                                     asFUNCTION(AddAssignDoubleToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(double) const",
                                     asFUNCTION(AddStringDouble),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const",
                                     asFUNCTION(AddDoubleString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(float)",
                                     asFUNCTION(AssignFloatToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)",
                                     asFUNCTION(AddAssignFloatToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(float) const",
                                     asFUNCTION(AddStringFloat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const",
                                     asFUNCTION(AddFloatString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(int64)",
                                     asFUNCTION(AssignInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)",
                                     asFUNCTION(AddAssignInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(int64) const",
                                     asFUNCTION(AddStringInt64),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const",
                                     asFUNCTION(AddInt64String),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)",
                                     asFUNCTION(AssignUInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)",
                                     asFUNCTION(AddAssignUInt64ToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const",
                                     asFUNCTION(AddStringUInt64),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const",
                                     asFUNCTION(AddUInt64String),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(bool)",
                                     asFUNCTION(AssignBoolToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)",
                                     asFUNCTION(AddAssignBoolToString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(bool) const",
                                     asFUNCTION(AddStringBool),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const",
                                     asFUNCTION(AddBoolString),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
#endif

    // Utilities
    r = engine->RegisterObjectMethod(
        "string", "string substr(uint start = 0, int count = -1) const",
        asFUNCTION(StringSubString), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "int findFirst(const string &in, uint start = 0) const",
        asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "int findFirstOf(const string &in, uint start = 0) const",
        asFUNCTION(StringFindFirstOf), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "string", "void insert(uint pos, const string &in other)",
        asFUNCTION(StringInsert), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "void erase(uint pos, int count = -1)",
        asFUNCTION(StringErase), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string formatInt(int64 val, const string &in options = \"\", uint "
        "width = 0)",
        asFUNCTION(formatInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "string formatUInt(uint64 val, const string &in options = \"\", uint "
        "width = 0)",
        asFUNCTION(formatUInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "string formatFloat(double val, const string &in options = \"\", uint "
        "width = 0, uint precision = 0)",
        asFUNCTION(formatFloat), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction("int64 parseInt(const string &in, uint "
                                       "base = 10, uint &out byteCount = 0)",
                                       asFUNCTION(parseInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "uint64 parseUInt(const string &in, uint base = 10, uint &out "
        "byteCount = 0)",
        asFUNCTION(parseUInt), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "double parseFloat(const string &in, uint &out byteCount = 0)",
        asFUNCTION(parseFloat), asCALL_CDECL);
    Q_ASSERT(r >= 0);

#if AS_USE_STLNAMES == 1
    // Same as length
    r = engine->RegisterObjectMethod("string", "uint size() const",
                                     asFUNCTION(StringLength),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    // Same as isEmpty
    r = engine->RegisterObjectMethod("string", "bool empty() const",
                                     asFUNCTION(StringIsEmpty),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    // Same as findFirst
    r = engine->RegisterObjectMethod(
        "string", "int find(const string &in, uint start = 0) const",
        asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    // Same as findLast
    r = engine->RegisterObjectMethod(
        "string", "int rfind(const string &in, int start = -1) const",
        asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
#endif

    /* These following things are not avaliable for generic call
     * because it needs a lot of wrapper to do
     * while WingHexExplorer only needs native call
     * (by wingsummer)
     * PULL REQUESTS ARE WELCOMED IF YOU WANT TO ADD GENERIC CALL
     */
    r = engine->RegisterObjectMethod(
        "string",
        "int compare(const string &in val, bool caseSensitive = true) const",
        asFUNCTION(stringCompare), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string",
        "bool contains(const string &in val, bool caseSensitive = true) const",
        asFUNCTION(stringContains), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string",
        "bool endsWith(const string &in val, bool caseSensitive = true) const",
        asFUNCTION(stringEndsWith), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "string repeated(" QSIZETYPE " times)",
        asFUNCTION(stringRepeated), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string",
        "string replace(const string &in before,const string &in after, "
        "bool caseSensitive = true)",
        asFUNCTION(stringReplace), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string simplified()",
                                     asFUNCTION(stringSimplified),
                                     asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string",
        "bool startsWith(const string &in, bool caseSensitive = true)",
        asFUNCTION(stringStartsWith), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string",
        "array<string>@ split(const string &in, bool skipEmpty = false, "
        "bool caseSensitive = true)",
        asFUNCTION(stringSplit), asCALL_CDECL_OBJLAST);
    Q_ASSERT(r >= 0);
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

static void StringInsert_Generic(asIScriptGeneric *gen) {
    QString *self = static_cast<QString *>(gen->GetObject());
    asUINT pos = gen->GetArgDWord(0);
    QString *other = reinterpret_cast<QString *>(gen->GetArgAddress(1));
    StringInsert(pos, *other, *self);
}

static void StringErase_Generic(asIScriptGeneric *gen) {
    QString *self = static_cast<QString *>(gen->GetObject());
    asUINT pos = gen->GetArgDWord(0);
    int count = int(gen->GetArgDWord(1));
    StringErase(pos, count, *self);
}

static void StringFindFirst_Generic(asIScriptGeneric *gen) {
    QString *find = reinterpret_cast<QString *>(gen->GetArgAddress(0));
    asUINT start = gen->GetArgDWord(1);
    QString *self = reinterpret_cast<QString *>(gen->GetObject());
    *reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) =
        StringFindFirst(*find, start, *self);
}

static void StringFindFirstOf_Generic(asIScriptGeneric *gen) {
    QString *find = reinterpret_cast<QString *>(gen->GetArgAddress(0));
    asUINT start = gen->GetArgDWord(1);
    QString *self = reinterpret_cast<QString *>(gen->GetObject());
    *reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) =
        StringFindFirstOf(*find, start, *self);
}

static void StringFindLastOf_Generic(asIScriptGeneric *gen) {
    QString *find = reinterpret_cast<QString *>(gen->GetArgAddress(0));
    asUINT start = gen->GetArgDWord(1);
    QString *self = reinterpret_cast<QString *>(gen->GetObject());
    *reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) =
        StringFindLastOf(*find, start, *self);
}

static void formatInt_Generic(asIScriptGeneric *gen) {
    asINT64 val = gen->GetArgQWord(0);
    QString *options = reinterpret_cast<QString *>(gen->GetArgAddress(1));
    new (gen->GetAddressOfReturnLocation()) QString(formatInt(val, *options));
}

static void formatUInt_Generic(asIScriptGeneric *gen) {
    asQWORD val = gen->GetArgQWord(0);
    QString *options = reinterpret_cast<QString *>(gen->GetArgAddress(1));
    new (gen->GetAddressOfReturnLocation()) QString(formatUInt(val, *options));
}

static void formatFloat_Generic(asIScriptGeneric *gen) {
    double val = gen->GetArgDouble(0);
    QString *options = reinterpret_cast<QString *>(gen->GetArgAddress(1));
    new (gen->GetAddressOfReturnLocation()) QString(formatFloat(val, *options));
}

static void parseInt_Generic(asIScriptGeneric *gen) {
    QString *str = reinterpret_cast<QString *>(gen->GetArgAddress(0));
    asUINT base = gen->GetArgDWord(1);
    asUINT *byteCount = reinterpret_cast<asUINT *>(gen->GetArgAddress(2));
    gen->SetReturnQWord(parseInt(*str, base, byteCount));
}

static void parseUInt_Generic(asIScriptGeneric *gen) {
    QString *str = reinterpret_cast<QString *>(gen->GetArgAddress(0));
    asUINT base = gen->GetArgDWord(1);
    bool *ok = reinterpret_cast<bool *>(gen->GetArgAddress(2));
    gen->SetReturnQWord(parseUInt(*str, base, ok));
}

static void parseFloat_Generic(asIScriptGeneric *gen) {
    QString *str = reinterpret_cast<QString *>(gen->GetArgAddress(0));
    bool *ok = reinterpret_cast<bool *>(gen->GetArgAddress(1));
    gen->SetReturnDouble(parseFloat(*str, ok));
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
    auto ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2FloatGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    float *b = static_cast<float *>(gen->GetAddressOfArg(0));
    auto ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2IntGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    asINT64 *b = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
    auto ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2UIntGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    asQWORD *b = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
    auto ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddString2BoolGeneric(asIScriptGeneric *gen) {
    QString *a = static_cast<QString *>(gen->GetObject());
    bool *b = static_cast<bool *>(gen->GetAddressOfArg(0));
    auto ret_val = *a + QString::number(*b);
    gen->SetReturnObject(&ret_val);
}

static void AddDouble2StringGeneric(asIScriptGeneric *gen) {
    double *a = static_cast<double *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    auto ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddFloat2StringGeneric(asIScriptGeneric *gen) {
    float *a = static_cast<float *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    auto ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddInt2StringGeneric(asIScriptGeneric *gen) {
    asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    auto ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddUInt2StringGeneric(asIScriptGeneric *gen) {
    asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    auto ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}

static void AddBool2StringGeneric(asIScriptGeneric *gen) {
    bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
    QString *b = static_cast<QString *>(gen->GetObject());
    auto ret_val = QString::number(*a) + *b;
    gen->SetReturnObject(&ret_val);
}
#endif

static void StringSubString_Generic(asIScriptGeneric *gen) {
    // Get the arguments
    QString *str = (QString *)gen->GetObject();
    asUINT start = *(int *)gen->GetAddressOfArg(0);
    int count = *(int *)gen->GetAddressOfArg(1);

    // Return the substring
    new (gen->GetAddressOfReturnLocation())
        QString(StringSubString(start, count, *str));
}

void RegisterQString_Generic(asIScriptEngine *engine) {
    int r = 0;
    Q_UNUSED(r);

    // Register the string type
    r = engine->RegisterObjectType("string", sizeof(QString),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
    Q_ASSERT(r >= 0);

    r = engine->RegisterStringFactory("string", GetQStringFactorySingleton());
    Q_ASSERT(r >= 0);

    // Register the object operator overloads
    r = engine->RegisterObjectBehaviour(
        "string", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(ConstructStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "string", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(CopyConstructStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(DestructStringGeneric),
                                        asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "string &opAssign(const string &in)",
        asFUNCTION(AssignStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "string &opAddAssign(const string &in)",
        asFUNCTION(AddAssignStringGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "string", "bool opEquals(const string &in) const",
        asFUNCTION(StringEqualsGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "int opCmp(const string &in) const",
        asFUNCTION(StringCmpGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "string opAdd(const string &in) const",
        asFUNCTION(StringAddGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    // Register the object methods
#if AS_USE_ACCESSORS != 1
    r = engine->RegisterObjectMethod("string", "uint length() const",
                                     asFUNCTION(StringLengthGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
#endif
    r = engine->RegisterObjectMethod("string", "void resize(uint)",
                                     asFUNCTION(StringResizeGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
#if AS_USE_STLNAMES != 1 && AS_USE_ACCESSORS == 1
    r = engine->RegisterObjectMethod(
        "string", "uint get_length() const property",
        asFUNCTION(StringLengthGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "void set_length(uint) property",
                                     asFUNCTION(StringResizeGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
#endif
    r = engine->RegisterObjectMethod("string", "bool isEmpty() const",
                                     asFUNCTION(StringIsEmptyGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    // Register the index operator, both as a mutator and as an inspector
    r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)",
                                     asFUNCTION(StringCharAtGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "const uint8 &opIndex(uint) const",
        asFUNCTION(StringCharAtGeneric), asCALL_GENERIC);
    Q_ASSERT(r >= 0);

#if AS_NO_IMPL_OPS_WITH_STRING_AND_PRIMITIVE == 0
    // Automatic conversion from values
    r = engine->RegisterObjectMethod("string", "string &opAssign(double)",
                                     asFUNCTION(AssignDouble2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)",
                                     asFUNCTION(AddAssignDouble2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(double) const",
                                     asFUNCTION(AddString2DoubleGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const",
                                     asFUNCTION(AddDouble2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(float)",
                                     asFUNCTION(AssignFloat2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)",
                                     asFUNCTION(AddAssignFloat2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(float) const",
                                     asFUNCTION(AddString2FloatGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const",
                                     asFUNCTION(AddFloat2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(int64)",
                                     asFUNCTION(AssignInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)",
                                     asFUNCTION(AddAssignInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(int64) const",
                                     asFUNCTION(AddString2IntGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const",
                                     asFUNCTION(AddInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)",
                                     asFUNCTION(AssignUInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)",
                                     asFUNCTION(AddAssignUInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const",
                                     asFUNCTION(AddString2UIntGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const",
                                     asFUNCTION(AddUInt2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("string", "string &opAssign(bool)",
                                     asFUNCTION(AssignBool2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)",
                                     asFUNCTION(AddAssignBool2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd(bool) const",
                                     asFUNCTION(AddString2BoolGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const",
                                     asFUNCTION(AddBool2StringGeneric),
                                     asCALL_GENERIC);
    Q_ASSERT(r >= 0);
#endif

    r = engine->RegisterObjectMethod(
        "string", "string substr(uint start = 0, int count = -1) const",
        asFUNCTION(StringSubString_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "int findFirst(const string &in, uint start = 0) const",
        asFUNCTION(StringFindFirst_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "int findFirstOf(const string &in, uint start = 0) const",
        asFUNCTION(StringFindFirstOf_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "int findLastOf(const string &in, int start = -1) const",
        asFUNCTION(StringFindLastOf_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "void insert(uint pos, const string &in other)",
        asFUNCTION(StringInsert_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "string", "void erase(uint pos, int count = -1)",
        asFUNCTION(StringErase_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);

    r = engine->RegisterGlobalFunction(
        "string formatInt(int64 val, const string &in options = \"\")",
        asFUNCTION(formatInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "string formatUInt(uint64 val, const string &in options = \"\")",
        asFUNCTION(formatUInt_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "string formatFloat(double val, const string &in options = \"\")",
        asFUNCTION(formatFloat_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction("int64 parseInt(const string &in, uint "
                                       "base = 10, bool &out ok = false)",
                                       asFUNCTION(parseInt_Generic),
                                       asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction("uint64 parseUInt(const string &in, "
                                       "uint base = 10, bool &out ok = false)",
                                       asFUNCTION(parseUInt_Generic),
                                       asCALL_GENERIC);
    Q_ASSERT(r >= 0);
    r = engine->RegisterGlobalFunction(
        "double parseFloat(const string &in, bool &out ok = false)",
        asFUNCTION(parseFloat_Generic), asCALL_GENERIC);
    Q_ASSERT(r >= 0);
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

static int StringCompare(const QString &s1, const QString &s2,
                         bool caseSensitive) {
    return QString::compare(
        s1, s2, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

// This is where the utility functions are registered.
// The string type must have been registered first.
void RegisterQStringUtils(asIScriptEngine *engine) {
    int r;
    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY")) {
        r = engine->RegisterGlobalFunction(
            "string join(const array<string> &in, const string &in)",
            asFUNCTION(StringJoin_Generic), asCALL_GENERIC);
        Q_ASSERT(r >= 0);
    } else {
        r = engine->RegisterGlobalFunction(
            "string join(const array<string> &in, const string &in)",
            asFUNCTION(StringJoin), asCALL_CDECL);
        Q_ASSERT(r >= 0);

        /* These following things are not avaliable for generic call
         * because it needs a lot of wrapper to do
         * while WingHexExplorer only needs native call
         * (by wingsummer)
         * PULL REQUESTS ARE WELCOMED IF YOU WANT TO ADD GENERIC CALL
         */
        r = engine->RegisterGlobalFunction(
            "int compare(const string &in, const string &in, bool = true)",
            asFUNCTION(StringCompare), asCALL_CDECL);
        Q_ASSERT(r >= 0);
    }
}

// This function returns the index of the first position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findFirst(const regex::exp &in exp, uint start = 0) const
static int StringFindFirstReg(const QRegularExpression &exp, asUINT start,
                              const QString &str) {
    // We don't register the method directly because the argument types change
    // between 32bit and 64bit platforms
    return (int)str.indexOf(exp, start);
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
    if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY")) {
        Q_UNIMPLEMENTED();
    } else {
        /* These following things are not avaliable for generic call
         * because it needs a lot of wrapper to do
         * while WingHexExplorer only needs native call
         * (by wingsummer)
         * PULL REQUESTS ARE WELCOMED IF YOU WANT TO ADD GENERIC CALL
         */
        Q_ASSERT(engine->GetTypeInfoByName("regex::exp"));

        int r = engine->RegisterObjectMethod(
            "string",
            "int findFirst(const regex::exp &in, uint start = 0) const",
            asFUNCTION(StringFindFirstReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        r = engine->RegisterObjectMethod(
            "string",
            "int findFirstOf(const regex::exp &in, uint start = 0) const",
            asFUNCTION(StringFindFirstOfReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        r = engine->RegisterObjectMethod(
            "string",
            "int findLastOf(const regex::exp &in, int start = -1) const",
            asFUNCTION(StringFindLastOfReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        r = engine->RegisterObjectMethod(
            "string", "bool contains(const regex::exp &in) const",
            asFUNCTION(stringContainsReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
        r = engine->RegisterObjectMethod(
            "string",
            "array<string>@ split(const regex::exp &in, "
            "bool skipEmpty = false)",
            asFUNCTION(stringSplitReg), asCALL_CDECL_OBJLAST);
        Q_ASSERT(r >= 0);
    }
}

END_AS_NAMESPACE
