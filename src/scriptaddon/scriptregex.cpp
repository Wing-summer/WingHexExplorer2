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

void RegisterScriptRegex(asIScriptEngine *engine) {
    int r = engine->SetDefaultNamespace("regex");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    registerAngelType<Angel::PatternOptions>(engine, "PatternOptions");
    registerAngelType<Angel::MatchOptions>(engine, "MatchOptions");
    registerAngelType<Angel::MatchType>(engine, "MatchType");

    r = engine->RegisterObjectType("exp", sizeof(QRegularExpression),
                                   asOBJ_VALUE | asOBJ_POD |
                                       asGetTypeTraits<QRegularExpression>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "exp", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR([](void *memory) { new (memory) QRegularExpression(); },
                     (void *), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

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
    Q_UNUSED(r);

    r = engine->RegisterObjectType(
        "match", sizeof(QRegularExpressionMatch),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<QRegularExpressionMatch>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "match", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR(
            [](void *memory) { new (memory) QRegularExpressionMatch(); },
            (void *), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("match", "exp regularExpression() const",
                                     asMETHODPR(QRegularExpressionMatch,
                                                regularExpression, () const,
                                                QRegularExpression),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "MatchOptions matchOptions() const",
        asMETHODPR(QRegularExpressionMatch, matchOptions, () const,
                   QRegularExpression::MatchOptions),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("match", "MatchType matchType() const",
                                     asMETHODPR(QRegularExpressionMatch,
                                                matchType, () const,
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
        "match", "string captured(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, captured, (int) const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    r = engine->RegisterObjectMethod(
        "match", QSIZETYPE_WRAP("capturedStart(int nth = 0) const"),
        asMETHODPR(QRegularExpressionMatch, capturedStart, (int) const,
                   qsizetype),
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
        "match", QSIZETYPE_WRAP("capturedEnd(int nth = 0) const"),
        asMETHODPR(QRegularExpressionMatch, capturedEnd, (int) const,
                   qsizetype),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

#else
    r = engine->RegisterObjectMethod(
        "match", "int capturedStart(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, capturedStart, (int) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "int capturedLength(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, capturedLength, (int) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "match", "int capturedEnd(int nth = 0) const",
        asMETHODPR(QRegularExpressionMatch, capturedEnd, (int) const, int),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
#endif

    // QRegularExpression...
    r = engine->RegisterObjectMethod(
        "exp", "void setPattern(string &in)",
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

    engine->SetDefaultNamespace("");
}

END_AS_NAMESPACE
