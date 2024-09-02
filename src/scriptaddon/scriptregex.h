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
