#include "qscilexeras.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>

QsciLexerAs::QsciLexerAs(QObject *parent, bool caseInsensitiveKeywords)
    : QsciLexer(parent), fold_atelse(false), fold_comments(false),
      fold_compact(true), fold_preproc(true), style_preproc(false),
      dollars(true), highlight_triple(false), highlight_hash(false),
      highlight_back(false), highlight_escape(false), vs_escape(false),
      nocase(caseInsensitiveKeywords) {}

// The dtor.
QsciLexerAs::~QsciLexerAs() {}

// Returns the language name.
const char *QsciLexerAs::language() const { return "AngelScript"; }

// Returns the lexer name.
const char *QsciLexerAs::lexer() const { return (nocase ? "asnocase" : "as"); }

// Return the set of character sequences that can separate auto-completion
// words.
QStringList QsciLexerAs::autoCompletionWordSeparators() const {
    QStringList wl;

    wl << "::"
       << ".";

    return wl;
}

// Return the list of keywords that can start a block.
const char *QsciLexerAs::blockStartKeyword(int *style) const {
    if (style)
        *style = Keyword;

    return "case catch class default do else for if private "
           "protected public try while mixin";
}

// Return the list of characters that can start a block.
const char *QsciLexerAs::blockStart(int *style) const {
    if (style)
        *style = Operator;

    return "{";
}

// Return the list of characters that can end a block.
const char *QsciLexerAs::blockEnd(int *style) const {
    if (style)
        *style = Operator;

    return "}";
}

// Return the style used for braces.
int QsciLexerAs::braceStyle() const { return Operator; }

// Return the string of characters that comprise a word.
const char *QsciLexerAs::wordCharacters() const {
    return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#";
}

// Returns the foreground colour of the text for a style.
QColor QsciLexerAs::defaultColor(int style) const {
    switch (style) {
    case Default:
        return QColor(0x80, 0x80, 0x80);

    case Comment:
    case CommentLine:
        return QColor(0x00, 0x7f, 0x00);

    case CommentDoc:
    case CommentLineDoc:
    case PreProcessorCommentLineDoc:
        return QColor(0x3f, 0x70, 0x3f);

    case Number:
        return QColor(0x00, 0x7f, 0x7f);

    case Keyword:
        return QColor(0x00, 0x00, 0x7f);

    case DoubleQuotedString:
    case SingleQuotedString:
    case RawString:
        return QColor(0x7f, 0x00, 0x7f);

    case PreProcessor:
        return QColor(0x7f, 0x7f, 0x00);

    case Operator:
    case UnclosedString:
        return QColor(0x00, 0x00, 0x00);

    case VerbatimString:
    case TripleQuotedVerbatimString:
    case HashQuotedString:
        return QColor(0x00, 0x7f, 0x00);

    case Regex:
        return QColor(0x3f, 0x7f, 0x3f);

    case CommentDocKeyword:
        return QColor(0x30, 0x60, 0xa0);

    case CommentDocKeywordError:
        return QColor(0x80, 0x40, 0x20);

    case PreProcessorComment:
        return QColor(0x65, 0x99, 0x00);

    case InactiveDefault:
    case InactiveUUID:
    case InactiveCommentLineDoc:
    case InactiveKeywordSet2:
    case InactiveCommentDocKeyword:
    case InactiveCommentDocKeywordError:
    case InactivePreProcessorCommentLineDoc:
        return QColor(0xc0, 0xc0, 0xc0);

    case InactiveComment:
    case InactiveCommentLine:
    case InactiveNumber:
    case InactiveVerbatimString:
    case InactiveTripleQuotedVerbatimString:
    case InactiveHashQuotedString:
        return QColor(0x90, 0xb0, 0x90);

    case InactiveCommentDoc:
        return QColor(0xd0, 0xd0, 0xd0);

    case InactiveKeyword:
        return QColor(0x90, 0x90, 0xb0);

    case InactiveDoubleQuotedString:
    case InactiveSingleQuotedString:
    case InactiveRawString:
        return QColor(0xb0, 0x90, 0xb0);

    case InactivePreProcessor:
        return QColor(0xb0, 0xb0, 0x90);

    case InactiveOperator:
    case InactiveIdentifier:
    case InactiveGlobalClass:
        return QColor(0xb0, 0xb0, 0xb0);

    case InactiveUnclosedString:
        return QColor(0x00, 0x00, 0x00);

    case InactiveRegex:
        return QColor(0x7f, 0xaf, 0x7f);

    case InactivePreProcessorComment:
        return QColor(0xa0, 0xc0, 0x90);

    case UserLiteral:
        return QColor(0xc0, 0x60, 0x00);

    case InactiveUserLiteral:
        return QColor(0xd7, 0xa0, 0x90);

    case TaskMarker:
        return QColor(0xbe, 0x07, 0xff);

    case InactiveTaskMarker:
        return QColor(0xc3, 0xa1, 0xcf);
    }

    return QsciLexer::defaultColor(style);
}

// Returns the end-of-line fill for a style.
bool QsciLexerAs::defaultEolFill(int style) const {
    switch (style) {
    case UnclosedString:
    case InactiveUnclosedString:
    case VerbatimString:
    case InactiveVerbatimString:
    case Regex:
    case InactiveRegex:
    case TripleQuotedVerbatimString:
    case InactiveTripleQuotedVerbatimString:
    case HashQuotedString:
    case InactiveHashQuotedString:
        return true;
    }

    return QsciLexer::defaultEolFill(style);
}

// Returns the font of the text for a style.
QFont QsciLexerAs::defaultFont(int style) const {
    QFont f;

    switch (style) {
    case Comment:
    case InactiveComment:
    case CommentLine:
    case InactiveCommentLine:
    case CommentDoc:
    case InactiveCommentDoc:
    case CommentLineDoc:
    case InactiveCommentLineDoc:
    case CommentDocKeyword:
    case InactiveCommentDocKeyword:
    case CommentDocKeywordError:
    case InactiveCommentDocKeywordError:
    case TaskMarker:
    case InactiveTaskMarker:
#if defined(Q_OS_WIN)
        f = QFont("Comic Sans MS", 9);
#elif defined(Q_OS_MAC)
        f = QFont("Comic Sans MS", 12);
#else
        f = QFont("Bitstream Vera Serif", 9);
#endif
        break;

    case Keyword:
    case InactiveKeyword:
    case Operator:
    case InactiveOperator:
        f = QsciLexer::defaultFont(style);
        f.setBold(true);
        break;

    case DoubleQuotedString:
    case InactiveDoubleQuotedString:
    case SingleQuotedString:
    case InactiveSingleQuotedString:
    case UnclosedString:
    case InactiveUnclosedString:
    case VerbatimString:
    case InactiveVerbatimString:
    case Regex:
    case InactiveRegex:
    case TripleQuotedVerbatimString:
    case InactiveTripleQuotedVerbatimString:
    case HashQuotedString:
    case InactiveHashQuotedString:
#if defined(Q_OS_WIN)
        f = QFont("Courier New", 10);
#elif defined(Q_OS_MAC)
        f = QFont("Courier", 12);
#else
        f = QFont("Bitstream Vera Sans Mono", 9);
#endif
        break;

    default:
        f = QsciLexer::defaultFont(style);
    }

    return f;
}

// Returns the set of keywords.
const char *QsciLexerAs::keywords(int set) const {
    if (set == 1)
        return "and abstract auto bool break case cast catch class const "
               "continue default delete do double else enum explicit external "
               "false final float for from funcdef function get if import in "
               "inout int interface int8 int16 int32 int64 is mixin namespace "
               "not null or out override private property protected return set "
               "shared super switch this true try typedef uint uint8 uint16 "
               "uint32 uint64 void while xor";

    if (set == 3)
        return "a addindex addtogroup anchor arg attention author b "
               "brief bug c class code date def defgroup deprecated "
               "dontinclude e em endcode endhtmlonly endif "
               "endlatexonly endlink endverbatim enum example "
               "exception f$ f[ f] file fn hideinitializer "
               "htmlinclude htmlonly if image include ingroup "
               "internal invariant interface latexonly li line link "
               "mainpage name namespace nosubgrouping note overload "
               "p page par param post pre ref relates remarks return "
               "retval sa section see showinitializer since skip "
               "skipline struct subsection test throw todo typedef "
               "union until var verbatim verbinclude version warning "
               "weakgroup $ @ \\ & < > # { }";

    return 0;
}

// Returns the user name of a style.
QString QsciLexerAs::description(int style) const {
    switch (style) {
    case Default:
        return tr("Default");

    case InactiveDefault:
        return tr("Inactive default");

    case Comment:
        return tr("AngelScript comment");

    case InactiveComment:
        return tr("Inactive AngelScript comment");

    case CommentLine:
        return tr("AngelScript comment");

    case InactiveCommentLine:
        return tr("Inactive AngelScript comment");

    case CommentDoc:
        return tr("JavaDoc style AngelScript comment");

    case InactiveCommentDoc:
        return tr("Inactive JavaDoc style AngelScript comment");

    case Number:
        return tr("Number");

    case InactiveNumber:
        return tr("Inactive number");

    case Keyword:
        return tr("Keyword");

    case InactiveKeyword:
        return tr("Inactive keyword");

    case DoubleQuotedString:
        return tr("Double-quoted string");

    case InactiveDoubleQuotedString:
        return tr("Inactive double-quoted string");

    case SingleQuotedString:
        return tr("Single-quoted string");

    case InactiveSingleQuotedString:
        return tr("Inactive single-quoted string");

    case UUID:
        return tr("IDL UUID");

    case InactiveUUID:
        return tr("Inactive IDL UUID");

    case PreProcessor:
        return tr("Pre-processor block");

    case InactivePreProcessor:
        return tr("Inactive pre-processor block");

    case Operator:
        return tr("Operator");

    case InactiveOperator:
        return tr("Inactive operator");

    case Identifier:
        return tr("Identifier");

    case InactiveIdentifier:
        return tr("Inactive identifier");

    case UnclosedString:
        return tr("Unclosed string");

    case InactiveUnclosedString:
        return tr("Inactive unclosed string");

    case VerbatimString:
        return tr("C# verbatim string");

    case InactiveVerbatimString:
        return tr("Inactive C# verbatim string");

    case Regex:
        return tr("JavaScript regular expression");

    case InactiveRegex:
        return tr("Inactive JavaScript regular expression");

    case CommentLineDoc:
        return tr("JavaDoc style AngelScript comment");

    case InactiveCommentLineDoc:
        return tr("Inactive JavaDoc style AngelScript comment");

    case KeywordSet2:
        return tr("Secondary keywords and identifiers");

    case InactiveKeywordSet2:
        return tr("Inactive secondary keywords and identifiers");

    case CommentDocKeyword:
        return tr("JavaDoc keyword");

    case InactiveCommentDocKeyword:
        return tr("Inactive JavaDoc keyword");

    case CommentDocKeywordError:
        return tr("JavaDoc keyword error");

    case InactiveCommentDocKeywordError:
        return tr("Inactive JavaDoc keyword error");

    case GlobalClass:
        return tr("Global classes and typedefs");

    case InactiveGlobalClass:
        return tr("Inactive global classes and typedefs");

    case RawString:
        return tr("AngelScript raw string");

    case InactiveRawString:
        return tr("Inactive AngelScript raw string");

    case TripleQuotedVerbatimString:
        return tr("Vala triple-quoted verbatim string");

    case InactiveTripleQuotedVerbatimString:
        return tr("Inactive Vala triple-quoted verbatim string");

    case HashQuotedString:
        return tr("Pike hash-quoted string");

    case InactiveHashQuotedString:
        return tr("Inactive Pike hash-quoted string");

    case PreProcessorComment:
        return tr("Pre-processor AngelScript comment");

    case InactivePreProcessorComment:
        return tr("Inactive pre-processor AngelScript comment");

    case PreProcessorCommentLineDoc:
        return tr("JavaDoc style pre-processor comment");

    case InactivePreProcessorCommentLineDoc:
        return tr("Inactive JavaDoc style pre-processor comment");

    case UserLiteral:
        return tr("User-defined literal");

    case InactiveUserLiteral:
        return tr("Inactive user-defined literal");

    case TaskMarker:
        return tr("Task marker");

    case InactiveTaskMarker:
        return tr("Inactive task marker");

    case EscapeSequence:
        return tr("Escape sequence");

    case InactiveEscapeSequence:
        return tr("Inactive escape sequence");
    }

    return QString();
}

// Returns the background colour of the text for a style.
QColor QsciLexerAs::defaultPaper(int style) const {
    switch (style) {
    case UnclosedString:
    case InactiveUnclosedString:
        return QColor(0xe0, 0xc0, 0xe0);

    case VerbatimString:
    case InactiveVerbatimString:
    case TripleQuotedVerbatimString:
    case InactiveTripleQuotedVerbatimString:
        return QColor(0xe0, 0xff, 0xe0);

    case Regex:
    case InactiveRegex:
        return QColor(0xe0, 0xf0, 0xe0);

    case RawString:
    case InactiveRawString:
        return QColor(0xff, 0xf3, 0xff);

    case HashQuotedString:
    case InactiveHashQuotedString:
        return QColor(0xe7, 0xff, 0xd7);
    }

    return QsciLexer::defaultPaper(style);
}

// Refresh all properties.
void QsciLexerAs::refreshProperties() {
    setAtElseProp();
    setCommentProp();
    setCompactProp();
    setPreprocProp();
    setStylePreprocProp();
    setDollarsProp();
    setHighlightTripleProp();
    setHighlightHashProp();
    setHighlightBackProp();
    setHighlightEscapeProp();
    setVerbatimStringEscapeProp();
}

// Read properties from the settings.
bool QsciLexerAs::readProperties(QSettings &qs, const QString &prefix) {
    fold_atelse = qs.value(prefix + "foldatelse", false).toBool();
    fold_comments = qs.value(prefix + "foldcomments", false).toBool();
    fold_compact = qs.value(prefix + "foldcompact", true).toBool();
    fold_preproc = qs.value(prefix + "foldpreprocessor", true).toBool();
    style_preproc = qs.value(prefix + "stylepreprocessor", false).toBool();
    dollars = qs.value(prefix + "dollars", true).toBool();
    highlight_triple = qs.value(prefix + "highlighttriple", false).toBool();
    highlight_hash = qs.value(prefix + "highlighthash", false).toBool();
    highlight_back = qs.value(prefix + "highlightback", false).toBool();
    highlight_escape = qs.value(prefix + "highlightescape", false).toBool();
    vs_escape = qs.value(prefix + "verbatimstringescape", false).toBool();

    return true;
}

// Write properties to the settings.
bool QsciLexerAs::writeProperties(QSettings &qs, const QString &prefix) const {
    qs.setValue(prefix + "foldatelse", fold_atelse);
    qs.setValue(prefix + "foldcomments", fold_comments);
    qs.setValue(prefix + "foldcompact", fold_compact);
    qs.setValue(prefix + "foldpreprocessor", fold_preproc);
    qs.setValue(prefix + "stylepreprocessor", style_preproc);
    qs.setValue(prefix + "dollars", dollars);
    qs.setValue(prefix + "highlighttriple", highlight_triple);
    qs.setValue(prefix + "highlighthash", highlight_hash);
    qs.setValue(prefix + "highlightback", highlight_back);
    qs.setValue(prefix + "highlightescape", highlight_escape);
    qs.setValue(prefix + "verbatimstringescape", vs_escape);

    return true;
}

// Set if else can be folded.
void QsciLexerAs::setFoldAtElse(bool fold) {
    fold_atelse = fold;

    setAtElseProp();
}

// Set the "fold.at.else" property.
void QsciLexerAs::setAtElseProp() {
    emit propertyChanged("fold.at.else", (fold_atelse ? "1" : "0"));
}

// Set if comments can be folded.
void QsciLexerAs::setFoldComments(bool fold) {
    fold_comments = fold;

    setCommentProp();
}

// Set the "fold.comment" property.
void QsciLexerAs::setCommentProp() {
    emit propertyChanged("fold.comment", (fold_comments ? "1" : "0"));
}

// Set if folds are compact
void QsciLexerAs::setFoldCompact(bool fold) {
    fold_compact = fold;

    setCompactProp();
}

// Set the "fold.compact" property.
void QsciLexerAs::setCompactProp() {
    emit propertyChanged("fold.compact", (fold_compact ? "1" : "0"));
}

// Set if preprocessor blocks can be folded.
void QsciLexerAs::setFoldPreprocessor(bool fold) {
    fold_preproc = fold;

    setPreprocProp();
}

// Set the "fold.preprocessor" property.
void QsciLexerAs::setPreprocProp() {
    emit propertyChanged("fold.preprocessor", (fold_preproc ? "1" : "0"));
}

// Set if preprocessor lines are styled.
void QsciLexerAs::setStylePreprocessor(bool style) {
    style_preproc = style;

    setStylePreprocProp();
}

// Set the "styling.within.preprocessor" property.
void QsciLexerAs::setStylePreprocProp() {
    emit propertyChanged("styling.within.preprocessor",
                         (style_preproc ? "1" : "0"));
}

// Set if '$' characters are allowed.
void QsciLexerAs::setDollarsAllowed(bool allowed) {
    dollars = allowed;

    setDollarsProp();
}

// Set the "lexer.as.allow.dollars" property.
void QsciLexerAs::setDollarsProp() {
    emit propertyChanged("lexer.as.allow.dollars", (dollars ? "1" : "0"));
}

// Set if triple quoted strings are highlighted.
void QsciLexerAs::setHighlightTripleQuotedStrings(bool enabled) {
    highlight_triple = enabled;

    setHighlightTripleProp();
}

// Set the "lexer.as.triplequoted.strings" property.
void QsciLexerAs::setHighlightTripleProp() {
    emit propertyChanged("lexer.as.triplequoted.strings",
                         (highlight_triple ? "1" : "0"));
}

// Set if hash quoted strings are highlighted.
void QsciLexerAs::setHighlightHashQuotedStrings(bool enabled) {
    highlight_hash = enabled;

    setHighlightHashProp();
}

// Set the "lexer.as.hashquoted.strings" property.
void QsciLexerAs::setHighlightHashProp() {
    emit propertyChanged("lexer.as.hashquoted.strings",
                         (highlight_hash ? "1" : "0"));
}

// Set if back-quoted strings are highlighted.
void QsciLexerAs::setHighlightBackQuotedStrings(bool enabled) {
    highlight_back = enabled;

    setHighlightBackProp();
}

// Set the "lexer.as.backquoted.strings" property.
void QsciLexerAs::setHighlightBackProp() {
    emit propertyChanged("lexer.as.backquoted.strings",
                         (highlight_back ? "1" : "0"));
}

// Set if escape sequences in strings are highlighted.
void QsciLexerAs::setHighlightEscapeSequences(bool enabled) {
    highlight_escape = enabled;

    setHighlightEscapeProp();
}

// Set the "lexer.as.escape.sequence" property.
void QsciLexerAs::setHighlightEscapeProp() {
    emit propertyChanged("lexer.as.escape.sequence",
                         (highlight_escape ? "1" : "0"));
}

// Set if escape sequences in verbatim strings are allowed.
void QsciLexerAs::setVerbatimStringEscapeSequencesAllowed(bool allowed) {
    vs_escape = allowed;

    setVerbatimStringEscapeProp();
}

// Set the "lexer.as.verbatim.strings.allow.escapes" property.
void QsciLexerAs::setVerbatimStringEscapeProp() {
    emit propertyChanged("lexer.as.verbatim.strings.allow.escapes",
                         (vs_escape ? "1" : "0"));
}
