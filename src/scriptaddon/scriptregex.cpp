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

#include "scriptregex.h"
#include "class/angelscripthelper.h"

BEGIN_AS_NAMESPACE

#include <QRegularExpression>

// --- helpers for exp (QRegularExpression) ---
static void Exp_DefaultCtor(void *memory) { new (memory) QRegularExpression(); }
static void Exp_Ctor_StringOptions(void *memory, const QString &pattern,
                                   Angel::PatternOptions op) {
    new (memory)
        QRegularExpression(pattern, QRegularExpression::PatternOptions(op));
}
static void Exp_CopyCtor(void *memory, const QRegularExpression &other) {
    new (memory) QRegularExpression(other);
}
static void Exp_Dtor(void *memory) {
    reinterpret_cast<QRegularExpression *>(memory)->~QRegularExpression();
}
static bool Exp_opEquals_wrap(const void *obj,
                              const QRegularExpression &other) {
    const QRegularExpression *self =
        reinterpret_cast<const QRegularExpression *>(obj);
    return (*self) == other;
}

// --- helpers for match (QRegularExpressionMatch) ---
static void Match_DefaultCtor(void *memory) {
    new (memory) QRegularExpressionMatch();
}
static void Match_CopyCtor(void *memory, const QRegularExpressionMatch &other) {
    new (memory) QRegularExpressionMatch(other);
}
static void Match_Dtor(void *memory) {
    reinterpret_cast<QRegularExpressionMatch *>(memory)
        ->~QRegularExpressionMatch();
}

static CScriptArray *
QStringList_to_ScriptArray_string(const QStringList &list) {
    auto ctx = asGetActiveContext();
    if (ctx) {
        auto engine = ctx->GetEngine();
        auto arrType = engine->GetTypeInfoByDecl("array<string>");
        CScriptArray *arr = CScriptArray::Create(arrType, asUINT(list.size()));
        for (asUINT i = 0; i < list.size(); ++i) {
            auto s = list.at(i);
            arr->SetValue(i, &s);
        }
        return arr;
    }
    return nullptr;
}

// namedCaptureGroups wrapper
static CScriptArray *Exp_namedCaptureGroups_wrap(const void *obj) {
    const QRegularExpression *self =
        reinterpret_cast<const QRegularExpression *>(obj);
    QStringList names = self->namedCaptureGroups();
    return QStringList_to_ScriptArray_string(names);
}

static CScriptArray *Match_capturedTexts_wrap(const void *obj) {
    const QRegularExpressionMatch *self =
        reinterpret_cast<const QRegularExpressionMatch *>(obj);
    QStringList names = self->capturedTexts();
    return QStringList_to_ScriptArray_string(names);
}

// anchoredPattern / escape wrappers (use QString variant)
static QString Exp_anchoredPattern_wrap(const QString &expr) {
    return QRegularExpression::anchoredPattern(expr);
}
static QString Exp_escape_wrap(const QString &s) {
    return QRegularExpression::escape(s);
}

// fromWildcard / wildcardToRegularExpression wrappers using general int params
static QRegularExpression Exp_fromWildcard_wrap(const QString &pattern, int cs,
                                                int options) {
    Qt::CaseSensitivity casesens = static_cast<Qt::CaseSensitivity>(cs);
    QRegularExpression::WildcardConversionOptions opts =
        static_cast<QRegularExpression::WildcardConversionOptions>(options);
    return QRegularExpression::fromWildcard(pattern, casesens, opts);
}
static QString Exp_wildcardToRegularExpression_wrap(const QString &pattern,
                                                    int options) {
    QRegularExpression::WildcardConversionOptions opts =
        static_cast<QRegularExpression::WildcardConversionOptions>(options);
    return QRegularExpression::wildcardToRegularExpression(pattern, opts);
}

// ---------- iterator (QRegularExpressionMatchIterator) helpers ----------
static void Iter_DefaultCtor(void *memory) {
    new (memory) QRegularExpressionMatchIterator();
}
static void Iter_CopyCtor(void *memory,
                          const QRegularExpressionMatchIterator &other) {
    new (memory) QRegularExpressionMatchIterator(other);
}
static void Iter_Dtor(void *memory) {
    reinterpret_cast<QRegularExpressionMatchIterator *>(memory)
        ->~QRegularExpressionMatchIterator();
}

void RegisterScriptRegex(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("regex");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAngelType<Angel::PatternOptions>(engine, "PatternOptions");
    registerAngelType<Angel::MatchOptions>(engine, "MatchOptions");
    registerAngelType<Angel::MatchType>(engine, "MatchType");

    // ---- QRegularExpression ("exp") ----
    r = engine->RegisterObjectType("exp", sizeof(QRegularExpression),
                                   asOBJ_VALUE |
                                       asGetTypeTraits<QRegularExpression>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "exp", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR(Exp_DefaultCtor, (void *), void), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "exp", asBEHAVE_CONSTRUCT,
        "void f(const string &in, regex::PatternOptions = "
        "regex::PatternOptions::NoPatternOption)",
        asFUNCTIONPR(Exp_Ctor_StringOptions,
                     (void *, const QString &, Angel::PatternOptions), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "exp", asBEHAVE_CONSTRUCT, "void f(const regex::exp &in)",
        asFUNCTIONPR(Exp_CopyCtor, (void *, const QRegularExpression &), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour("exp", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTIONPR(Exp_Dtor, (void *), void),
                                        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "exp", "regex::exp &opAssign(const regex::exp &in)",
        asMETHODPR(QRegularExpression, operator=, (const QRegularExpression &),
                   QRegularExpression &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // ---- QRegularExpressionMatch ("match") ----
    r = engine->RegisterObjectType(
        "match", sizeof(QRegularExpressionMatch),
        asOBJ_VALUE | asGetTypeTraits<QRegularExpressionMatch>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "match", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR(Match_DefaultCtor, (void *), void), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // copy ctor
    r = engine->RegisterObjectBehaviour(
        "match", asBEHAVE_CONSTRUCT, "void f(const regex::match &in)",
        asFUNCTIONPR(Match_CopyCtor, (void *, const QRegularExpressionMatch &),
                     void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // destructor
    r = engine->RegisterObjectBehaviour(
        "match", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTIONPR(Match_Dtor, (void *), void), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // assignment
    r = engine->RegisterObjectMethod(
        "match", "regex::match &opAssign(const regex::match &in)",
        asMETHODPR(QRegularExpressionMatch, operator=,
                   (const QRegularExpressionMatch &),
                   QRegularExpressionMatch &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "match", "regex::exp regularExpression() const",
        asMETHODPR(QRegularExpressionMatch, regularExpression, () const,
                   QRegularExpression),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "regex::MatchOptions matchOptions() const",
        asMETHODPR(QRegularExpressionMatch, matchOptions, () const,
                   QRegularExpression::MatchOptions),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "regex::MatchType matchType() const",
        asMETHODPR(QRegularExpressionMatch, matchType, () const,
                   QRegularExpression::MatchType),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "bool hasMatch() const",
        asMETHODPR(QRegularExpressionMatch, hasMatch, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "bool hasPartialMatch() const",
        asMETHODPR(QRegularExpressionMatch, hasPartialMatch, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "bool isValid() const",
        asMETHODPR(QRegularExpressionMatch, isValid, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "int lastCapturedIndex() const",
        asMETHODPR(QRegularExpressionMatch, lastCapturedIndex, () const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "string captured(const string &in name) const",
        asMETHODPR(QRegularExpressionMatch, captured, (QAnyStringView) const,
                   QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "string captured(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, captured, (int) const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // capturedEnd / capturedLength / capturedStart (qsizetype)
    r = engine->RegisterObjectMethod(
        "match", QSIZETYPE_WRAP("capturedEnd(const string &in name) const"),
        asMETHODPR(QRegularExpressionMatch, capturedEnd, (QAnyStringView) const,
                   qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", QSIZETYPE_WRAP("capturedEnd(int nth = 0) const"),
        asMETHODPR(QRegularExpressionMatch, capturedEnd, (int) const,
                   qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "match", QSIZETYPE_WRAP("capturedLength(const string &in name) const"),
        asMETHODPR(QRegularExpressionMatch, capturedLength,
                   (QAnyStringView) const, qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", QSIZETYPE_WRAP("capturedLength(int nth = 0) const"),
        asMETHODPR(QRegularExpressionMatch, capturedLength, (int) const,
                   qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "match", QSIZETYPE_WRAP("capturedStart(const string &in name) const"),
        asMETHODPR(QRegularExpressionMatch, capturedStart,
                   (QAnyStringView) const, qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", QSIZETYPE_WRAP("capturedStart(int nth = 0) const"),
        asMETHODPR(QRegularExpressionMatch, capturedStart, (int) const,
                   qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "array<string>@ capturedTexts() const",
        asFunctionPtr(Match_capturedTexts_wrap), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // hasCaptured overloads
    r = engine->RegisterObjectMethod(
        "match", "bool hasCaptured(const string &in name) const",
        asMETHODPR(QRegularExpressionMatch, hasCaptured, (QAnyStringView) const,
                   bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "bool hasCaptured(int nth) const",
        asMETHODPR(QRegularExpressionMatch, hasCaptured, (int) const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Register iterator type and its methods
    r = engine->RegisterObjectType(
        "iterator", sizeof(QRegularExpressionMatchIterator),
        asOBJ_VALUE | asGetTypeTraits<QRegularExpressionMatchIterator>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // ctor / copy / dtor
    r = engine->RegisterObjectBehaviour(
        "iterator", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR(Iter_DefaultCtor, (void *), void), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "iterator", asBEHAVE_CONSTRUCT, "void f(const regex::iterator &in)",
        asFUNCTIONPR(Iter_CopyCtor,
                     (void *, const QRegularExpressionMatchIterator &), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "iterator", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTIONPR(Iter_Dtor, (void *), void), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // methods on iterator
    r = engine->RegisterObjectMethod(
        "iterator", "bool hasNext() const",
        asMETHODPR(QRegularExpressionMatchIterator, hasNext, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "iterator", "bool isValid() const",
        asMETHODPR(QRegularExpressionMatchIterator, isValid, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "iterator", "regex::MatchOptions matchOptions() const",
        asMETHODPR(QRegularExpressionMatchIterator, matchOptions, () const,
                   QRegularExpression::MatchOptions),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "iterator", "regex::MatchType matchType() const",
        asMETHODPR(QRegularExpressionMatchIterator, matchType, () const,
                   QRegularExpression::MatchType),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // next / peekNext
    r = engine->RegisterObjectMethod("iterator", "regex::match next()",
                                     asMETHODPR(QRegularExpressionMatchIterator,
                                                next, (),
                                                QRegularExpressionMatch),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "iterator", "regex::match peekNext() const",
        asMETHODPR(QRegularExpressionMatchIterator, peekNext, () const,
                   QRegularExpressionMatch),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // regularExpression()
    r = engine->RegisterObjectMethod(
        "iterator", "regex::exp regularExpression() const",
        asMETHODPR(QRegularExpressionMatchIterator, regularExpression, () const,
                   QRegularExpression),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // assignment operator
    r = engine->RegisterObjectMethod(
        "iterator", "regex::iterator &opAssign(const regex::iterator &in)",
        asMETHODPR(QRegularExpressionMatchIterator, operator=,
                   (const QRegularExpressionMatchIterator &),
                   QRegularExpressionMatchIterator &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // QRegularExpression...
    r = engine->RegisterObjectMethod(
        "exp", "void setPattern(const string &in)",
        asMETHODPR(QRegularExpression, setPattern, (const QString &), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", "string pattern() const",
        asMETHODPR(QRegularExpression, pattern, () const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", "bool isValid() const",
        asMETHODPR(QRegularExpression, isValid, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", "string errorString() const",
        asMETHODPR(QRegularExpression, errorString, () const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", "int captureCount() const",
        asMETHODPR(QRegularExpression, captureCount, () const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", "array<string>@ namedCaptureGroups() const",
        asFUNCTION(Exp_namedCaptureGroups_wrap), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", QSIZETYPE_WRAP("patternErrorOffset() const"),
        asMETHODPR(QRegularExpression, patternErrorOffset, () const, qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // patternOptions / setPatternOptions
    r = engine->RegisterObjectMethod(
        "exp", "regex::PatternOptions patternOptions() const",
        asMETHODPR(QRegularExpression, patternOptions, () const,
                   QRegularExpression::PatternOptions),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", "void setPatternOptions(regex::PatternOptions options)",
        asMETHODPR(QRegularExpression, setPatternOptions,
                   (QRegularExpression::PatternOptions), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp",
        "regex::match match(const string &in subject, int offset = 0, "
        "regex::MatchType matchType = regex::MatchType::NormalMatch, "
        "regex::MatchOptions matchOptions = "
        "regex::MatchOptions::NoMatchOption) const",
        asMETHODPR(QRegularExpression, match,
                   (const QString &, qsizetype, QRegularExpression::MatchType,
                    QRegularExpression::MatchOptions) const,
                   QRegularExpressionMatch),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "exp", "bool opEquals(const regex::exp &in) const",
        asFUNCTIONPR(Exp_opEquals_wrap,
                     (const void *, const QRegularExpression &), bool),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Global helper functions: anchoredPattern, escape, fromWildcard,
    // wildcardToRegularExpression
    r = engine->RegisterGlobalFunction(
        "string anchoredPattern(const string &in expression)",
        asFUNCTION(Exp_anchoredPattern_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("string escape(const string &in str)",
                                       asFUNCTION(Exp_escape_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromWildcard: use int for case-sensitivity and options to avoid requiring
    // enum bindings
    r = engine->RegisterGlobalFunction("exp fromWildcard(const string &in "
                                       "pattern, int cs = 0, int options = 0)",
                                       asFUNCTION(Exp_fromWildcard_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "string wildcardToRegularExpression(const string &in pattern, int "
        "options = 0)",
        asFUNCTION(Exp_wildcardToRegularExpression_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    engine->SetDefaultNamespace("");
}

END_AS_NAMESPACE
