#include "scriptregex.h"
#include "class/angelscripthelper.h"

BEGIN_AS_NAMESPACE

#include <QRegularExpression>

void RegisterScriptRegex(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("regex");
    Q_ASSERT(r >= 0);

    registerAngelType<Angel::PatternOptions>(engine, "PatternOptions");
    registerAngelType<Angel::MatchOptions>(engine, "MatchOptions");
    registerAngelType<Angel::MatchType>(engine, "MatchType");

    r = engine->RegisterObjectType("exp", sizeof(QRegularExpression),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<QRegularExpression>());
    assert(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "exp", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR([](void *memory) { new (memory) QRegularExpression(); },
                     (void *), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "exp", asBEHAVE_CONSTRUCT,
        "void f(string &in, PatternOptions = PatternOptions::NoPatternOption)",
        asFUNCTIONPR(
            [](void *memory, const QString &r, Angel::PatternOptions op) {
                new (memory) QRegularExpression(
                    r, QRegularExpression::PatternOptions(op));
            },
            (void *, const QString &, Angel::PatternOptions), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectType(
        "match", sizeof(QRegularExpressionMatch),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<QRegularExpressionMatch>());
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "match", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR(
            [](void *memory) { new (memory) QRegularExpressionMatch(); },
            (void *), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("match", "exp regularExpression() const",
                                     asMETHODPR(QRegularExpressionMatch,
                                                regularExpression, () const,
                                                QRegularExpression),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "MatchOptions matchOptions() const",
        asMETHODPR(QRegularExpressionMatch, matchOptions, () const,
                   QRegularExpression::MatchOptions),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("match", "MatchType matchType() const",
                                     asMETHODPR(QRegularExpressionMatch,
                                                matchType, () const,
                                                QRegularExpression::MatchType),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "bool hasMatch() const",
        asMETHODPR(QRegularExpressionMatch, hasMatch, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "bool hasPartialMatch() const",
        asMETHODPR(QRegularExpressionMatch, hasPartialMatch, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "bool isValid() const",
        asMETHODPR(QRegularExpressionMatch, isValid, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "int lastCapturedIndex() const",
        asMETHODPR(QRegularExpressionMatch, lastCapturedIndex, () const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "string captured(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, captured, (int) const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "string captured(string &in) const",
        asMETHODPR(QRegularExpressionMatch, captured, (const QString &) const,
                   QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "int capturedStart(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, capturedStart, (int) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "int capturedLength(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, capturedLength, (int) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "int capturedEnd(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, capturedEnd, (int) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "int capturedStart(string &in) const",
        asMETHODPR(QRegularExpressionMatch, capturedStart,
                   (const QString &) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "int capturedLength(string &in) const",
        asMETHODPR(QRegularExpressionMatch, capturedLength,
                   (const QString &) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod(
        "match", "int capturedEnd(string &in) const",
        asMETHODPR(QRegularExpressionMatch, capturedEnd,
                   (const QString &) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    // QRegularExpression...
    r = engine->RegisterObjectMethod(
        "exp", "void setPattern(string &in)",
        asMETHODPR(QRegularExpression, setPattern, (const QString &), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "exp", "string pattern() const",
        asMETHODPR(QRegularExpression, pattern, () const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "exp", "bool isValid() const",
        asMETHODPR(QRegularExpression, isValid, () const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "exp", "string errorString() const",
        asMETHODPR(QRegularExpression, errorString, () const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "exp", "int captureCount() const",
        asMETHODPR(QRegularExpression, captureCount, () const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    engine->SetDefaultNamespace("");
}

END_AS_NAMESPACE
