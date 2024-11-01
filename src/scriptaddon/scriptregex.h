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

#ifndef SCRIPTREGEX_H
#define SCRIPTREGEX_H

#include <QRegularExpression>

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

BEGIN_AS_NAMESPACE

void RegisterScriptRegex(asIScriptEngine *engine);

namespace Angel {

Q_NAMESPACE
enum PatternOptions {
    NoPatternOption = QRegularExpression::NoPatternOption,
    CaseInsensitiveOption = QRegularExpression::CaseInsensitiveOption,
    DotMatchesEverythingOption = QRegularExpression::DotMatchesEverythingOption,
    MultilineOption = QRegularExpression::MultilineOption,
    ExtendedPatternSyntaxOption =
        QRegularExpression::ExtendedPatternSyntaxOption,
    InvertedGreedinessOption = QRegularExpression::InvertedGreedinessOption,
    DontCaptureOption = QRegularExpression::DontCaptureOption,
    UseUnicodePropertiesOption = QRegularExpression::UseUnicodePropertiesOption,
};
Q_ENUM_NS(PatternOptions)

enum MatchOptions {
    NoMatchOption = QRegularExpression::NoMatchOption,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    AnchoredMatchOption = QRegularExpression::AnchorAtOffsetMatchOption,
#else
    AnchoredMatchOption = QRegularExpression::AnchoredMatchOption,
#endif
    DontCheckSubjectStringMatchOption =
        QRegularExpression::DontCheckSubjectStringMatchOption
};
Q_ENUM_NS(MatchOptions)

enum MatchType {
    NormalMatch = QRegularExpression::NormalMatch,
    PartialPreferCompleteMatch = QRegularExpression::PartialPreferFirstMatch,
    PartialPreferFirstMatch = QRegularExpression::PartialPreferFirstMatch,
    NoMatch = QRegularExpression::NoMatch
};
Q_ENUM_NS(MatchType)

} // namespace Angel

END_AS_NAMESPACE

#endif // SCRIPTREGEX_H
