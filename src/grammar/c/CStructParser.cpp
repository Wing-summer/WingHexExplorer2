
// Generated from CStruct.g4 by ANTLR 4.13.2


#include "CStructVisitor.h"

#include "CStructParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct CStructParserStaticData final {
  CStructParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  CStructParserStaticData(const CStructParserStaticData&) = delete;
  CStructParserStaticData(CStructParserStaticData&&) = delete;
  CStructParserStaticData& operator=(const CStructParserStaticData&) = delete;
  CStructParserStaticData& operator=(CStructParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag cstructParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<CStructParserStaticData> cstructParserStaticData = nullptr;

void cstructParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (cstructParserStaticData != nullptr) {
    return;
  }
#else
  assert(cstructParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<CStructParserStaticData>(
    std::vector<std::string>{
      "internalBasicTypes", "signOrUnsigned", "primaryExpression", "postfixExpression", 
      "unaryExpression", "unaryOperator", "castExpression", "multiplicativeExpression", 
      "additiveExpression", "shiftExpression", "andExpression", "exclusiveOrExpression", 
      "inclusiveOrExpression", "assignmentExpressionDef", "assignmentExpression", 
      "declaration", "declarationSpecifier", "typeSpecifier", "structOrUnionSpecifier", 
      "structOrUnion", "alignAsAttr", "structDeclarationList", "structDeclaration", 
      "specifierQualifierList", "structDeclaratorList", "structDeclarator", 
      "enumSpecifier", "enumeratorList", "enumerator", "enumerationConstant", 
      "declarator", "directDeclarator", "pointer", "identifierList", "typeName", 
      "abstractDeclarator", "directAbstractDeclarator", "compilationUnit", 
      "translationUnit", "externalDeclaration", "defineDecl"
    },
    std::vector<std::string>{
      "", "'='", "'char'", "'const'", "'double'", "'enum'", "'float'", "'int'", 
      "'long'", "'typedef'", "'alignas'", "'sizeof'", "'short'", "'signed'", 
      "'struct'", "'union'", "'unsigned'", "'void'", "'\\u003F'", "'('", 
      "')'", "'['", "']'", "'{'", "'}'", "'<<'", "'>>'", "'+'", "'++'", 
      "'-'", "'--'", "'*'", "'/'", "'%'", "'&'", "'|'", "'^'", "'~'", "':'", 
      "';'", "','"
    },
    std::vector<std::string>{
      "", "", "Char", "Const", "Double", "Enum", "Float", "Int", "Long", 
      "TypeDef", "AlignAs", "SizeOf", "Short", "Signed", "Struct", "Union", 
      "Unsigned", "Void", "QuestionMark", "LeftParen", "RightParen", "LeftBracket", 
      "RightBracket", "LeftBrace", "RightBrace", "LeftShift", "RightShift", 
      "Plus", "PlusPlus", "Minus", "MinusMinus", "Star", "Div", "Mod", "And", 
      "Or", "Caret", "Tilde", "Colon", "Semi", "Comma", "Identifier", "IntegerConstant", 
      "StringLiteral", "MultiLineMacroDefine", "MultiLineMacro", "DirectiveInclude", 
      "DirectiveDefine", "Directive", "Whitespace", "Newline", "LINE_CONTINUATION", 
      "BlockComment", "LineComment"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,53,428,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,1,0,3,0,84,8,0,1,
  	0,3,0,87,8,0,1,0,1,0,1,0,3,0,92,8,0,1,0,3,0,95,8,0,1,0,3,0,98,8,0,1,0,
  	3,0,101,8,0,1,0,1,0,1,0,1,0,1,0,3,0,108,8,0,1,0,3,0,111,8,0,1,0,1,0,3,
  	0,115,8,0,1,1,1,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,3,2,130,
  	8,2,1,3,1,3,5,3,134,8,3,10,3,12,3,137,9,3,1,4,5,4,140,8,4,10,4,12,4,143,
  	9,4,1,4,1,4,1,4,1,4,3,4,149,8,4,1,5,1,5,1,6,1,6,1,6,1,6,1,6,1,6,1,6,3,
  	6,160,8,6,1,7,1,7,1,7,5,7,165,8,7,10,7,12,7,168,9,7,1,8,1,8,1,8,5,8,173,
  	8,8,10,8,12,8,176,9,8,1,9,1,9,1,9,5,9,181,8,9,10,9,12,9,184,9,9,1,10,
  	1,10,1,10,5,10,189,8,10,10,10,12,10,192,9,10,1,11,1,11,1,11,5,11,197,
  	8,11,10,11,12,11,200,9,11,1,12,1,12,1,12,5,12,205,8,12,10,12,12,12,208,
  	9,12,1,13,1,13,5,13,212,8,13,10,13,12,13,215,9,13,1,13,1,13,1,14,1,14,
  	3,14,221,8,14,1,15,1,15,1,15,1,15,1,15,1,15,3,15,229,8,15,1,15,1,15,1,
  	15,3,15,234,8,15,1,16,1,16,3,16,238,8,16,1,17,1,17,1,17,1,17,1,17,3,17,
  	245,8,17,1,18,1,18,3,18,249,8,18,1,18,3,18,252,8,18,1,18,1,18,3,18,256,
  	8,18,1,18,1,18,1,18,1,18,1,18,3,18,263,8,18,1,19,1,19,1,20,1,20,1,20,
  	1,20,1,20,1,21,4,21,273,8,21,11,21,12,21,274,1,22,1,22,1,22,1,22,1,22,
  	1,22,1,22,3,22,284,8,22,1,23,3,23,287,8,23,1,23,1,23,3,23,291,8,23,1,
  	24,1,24,1,24,5,24,296,8,24,10,24,12,24,299,9,24,1,25,1,25,3,25,303,8,
  	25,1,25,1,25,3,25,307,8,25,1,26,1,26,3,26,311,8,26,1,26,1,26,1,26,3,26,
  	316,8,26,1,26,1,26,1,26,1,26,3,26,322,8,26,1,27,1,27,1,27,5,27,327,8,
  	27,10,27,12,27,330,9,27,1,28,1,28,1,28,3,28,335,8,28,1,29,1,29,1,30,3,
  	30,340,8,30,1,30,1,30,1,31,1,31,1,31,1,31,1,31,1,31,1,31,3,31,351,8,31,
  	1,31,1,31,1,31,1,31,1,31,5,31,358,8,31,10,31,12,31,361,9,31,1,32,1,32,
  	3,32,365,8,32,4,32,367,8,32,11,32,12,32,368,1,33,1,33,1,33,5,33,374,8,
  	33,10,33,12,33,377,9,33,1,34,1,34,3,34,381,8,34,1,35,1,35,3,35,385,8,
  	35,1,35,3,35,388,8,35,1,36,1,36,1,36,1,36,1,36,1,36,1,36,1,36,1,36,3,
  	36,399,8,36,1,36,1,36,1,36,1,36,1,36,5,36,406,8,36,10,36,12,36,409,9,
  	36,1,37,3,37,412,8,37,1,37,1,37,1,38,4,38,417,8,38,11,38,12,38,418,1,
  	39,1,39,1,39,3,39,424,8,39,1,40,1,40,1,40,0,2,62,72,41,0,2,4,6,8,10,12,
  	14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,
  	60,62,64,66,68,70,72,74,76,78,80,0,9,3,0,2,2,7,8,12,12,2,0,13,13,16,16,
  	2,0,28,28,30,30,3,0,27,27,29,29,37,37,1,0,31,33,2,0,27,27,29,29,1,0,25,
  	26,1,0,14,15,2,0,44,44,46,47,454,0,114,1,0,0,0,2,116,1,0,0,0,4,129,1,
  	0,0,0,6,131,1,0,0,0,8,141,1,0,0,0,10,150,1,0,0,0,12,159,1,0,0,0,14,161,
  	1,0,0,0,16,169,1,0,0,0,18,177,1,0,0,0,20,185,1,0,0,0,22,193,1,0,0,0,24,
  	201,1,0,0,0,26,209,1,0,0,0,28,220,1,0,0,0,30,233,1,0,0,0,32,237,1,0,0,
  	0,34,244,1,0,0,0,36,262,1,0,0,0,38,264,1,0,0,0,40,266,1,0,0,0,42,272,
  	1,0,0,0,44,283,1,0,0,0,46,286,1,0,0,0,48,292,1,0,0,0,50,306,1,0,0,0,52,
  	321,1,0,0,0,54,323,1,0,0,0,56,331,1,0,0,0,58,336,1,0,0,0,60,339,1,0,0,
  	0,62,350,1,0,0,0,64,366,1,0,0,0,66,370,1,0,0,0,68,378,1,0,0,0,70,387,
  	1,0,0,0,72,398,1,0,0,0,74,411,1,0,0,0,76,416,1,0,0,0,78,423,1,0,0,0,80,
  	425,1,0,0,0,82,84,3,2,1,0,83,82,1,0,0,0,83,84,1,0,0,0,84,86,1,0,0,0,85,
  	87,5,3,0,0,86,85,1,0,0,0,86,87,1,0,0,0,87,88,1,0,0,0,88,115,7,0,0,0,89,
  	91,7,0,0,0,90,92,5,3,0,0,91,90,1,0,0,0,91,92,1,0,0,0,92,94,1,0,0,0,93,
  	95,3,2,1,0,94,93,1,0,0,0,94,95,1,0,0,0,95,115,1,0,0,0,96,98,3,2,1,0,97,
  	96,1,0,0,0,97,98,1,0,0,0,98,100,1,0,0,0,99,101,5,3,0,0,100,99,1,0,0,0,
  	100,101,1,0,0,0,101,102,1,0,0,0,102,103,5,8,0,0,103,115,5,8,0,0,104,105,
  	5,8,0,0,105,107,5,8,0,0,106,108,5,3,0,0,107,106,1,0,0,0,107,108,1,0,0,
  	0,108,110,1,0,0,0,109,111,3,2,1,0,110,109,1,0,0,0,110,111,1,0,0,0,111,
  	115,1,0,0,0,112,115,5,6,0,0,113,115,5,4,0,0,114,83,1,0,0,0,114,89,1,0,
  	0,0,114,97,1,0,0,0,114,104,1,0,0,0,114,112,1,0,0,0,114,113,1,0,0,0,115,
  	1,1,0,0,0,116,117,7,1,0,0,117,3,1,0,0,0,118,130,5,41,0,0,119,130,5,42,
  	0,0,120,121,5,19,0,0,121,122,3,28,14,0,122,123,5,20,0,0,123,130,1,0,0,
  	0,124,125,5,11,0,0,125,126,5,19,0,0,126,127,3,46,23,0,127,128,5,20,0,
  	0,128,130,1,0,0,0,129,118,1,0,0,0,129,119,1,0,0,0,129,120,1,0,0,0,129,
  	124,1,0,0,0,130,5,1,0,0,0,131,135,3,4,2,0,132,134,7,2,0,0,133,132,1,0,
  	0,0,134,137,1,0,0,0,135,133,1,0,0,0,135,136,1,0,0,0,136,7,1,0,0,0,137,
  	135,1,0,0,0,138,140,7,2,0,0,139,138,1,0,0,0,140,143,1,0,0,0,141,139,1,
  	0,0,0,141,142,1,0,0,0,142,148,1,0,0,0,143,141,1,0,0,0,144,149,3,6,3,0,
  	145,146,3,10,5,0,146,147,3,12,6,0,147,149,1,0,0,0,148,144,1,0,0,0,148,
  	145,1,0,0,0,149,9,1,0,0,0,150,151,7,3,0,0,151,11,1,0,0,0,152,153,5,19,
  	0,0,153,154,3,68,34,0,154,155,5,20,0,0,155,156,3,12,6,0,156,160,1,0,0,
  	0,157,160,3,8,4,0,158,160,5,42,0,0,159,152,1,0,0,0,159,157,1,0,0,0,159,
  	158,1,0,0,0,160,13,1,0,0,0,161,166,3,12,6,0,162,163,7,4,0,0,163,165,3,
  	12,6,0,164,162,1,0,0,0,165,168,1,0,0,0,166,164,1,0,0,0,166,167,1,0,0,
  	0,167,15,1,0,0,0,168,166,1,0,0,0,169,174,3,14,7,0,170,171,7,5,0,0,171,
  	173,3,14,7,0,172,170,1,0,0,0,173,176,1,0,0,0,174,172,1,0,0,0,174,175,
  	1,0,0,0,175,17,1,0,0,0,176,174,1,0,0,0,177,182,3,16,8,0,178,179,7,6,0,
  	0,179,181,3,16,8,0,180,178,1,0,0,0,181,184,1,0,0,0,182,180,1,0,0,0,182,
  	183,1,0,0,0,183,19,1,0,0,0,184,182,1,0,0,0,185,190,3,18,9,0,186,187,5,
  	34,0,0,187,189,3,18,9,0,188,186,1,0,0,0,189,192,1,0,0,0,190,188,1,0,0,
  	0,190,191,1,0,0,0,191,21,1,0,0,0,192,190,1,0,0,0,193,198,3,20,10,0,194,
  	195,5,36,0,0,195,197,3,20,10,0,196,194,1,0,0,0,197,200,1,0,0,0,198,196,
  	1,0,0,0,198,199,1,0,0,0,199,23,1,0,0,0,200,198,1,0,0,0,201,206,3,22,11,
  	0,202,203,5,35,0,0,203,205,3,22,11,0,204,202,1,0,0,0,205,208,1,0,0,0,
  	206,204,1,0,0,0,206,207,1,0,0,0,207,25,1,0,0,0,208,206,1,0,0,0,209,213,
  	3,28,14,0,210,212,5,39,0,0,211,210,1,0,0,0,212,215,1,0,0,0,213,211,1,
  	0,0,0,213,214,1,0,0,0,214,216,1,0,0,0,215,213,1,0,0,0,216,217,5,0,0,1,
  	217,27,1,0,0,0,218,221,5,42,0,0,219,221,3,24,12,0,220,218,1,0,0,0,220,
  	219,1,0,0,0,221,29,1,0,0,0,222,223,3,32,16,0,223,224,5,39,0,0,224,234,
  	1,0,0,0,225,226,5,9,0,0,226,228,3,34,17,0,227,229,3,64,32,0,228,227,1,
  	0,0,0,228,229,1,0,0,0,229,230,1,0,0,0,230,231,5,41,0,0,231,232,5,39,0,
  	0,232,234,1,0,0,0,233,222,1,0,0,0,233,225,1,0,0,0,234,31,1,0,0,0,235,
  	238,3,36,18,0,236,238,3,52,26,0,237,235,1,0,0,0,237,236,1,0,0,0,238,33,
  	1,0,0,0,239,245,5,17,0,0,240,245,3,0,0,0,241,245,3,36,18,0,242,245,3,
  	52,26,0,243,245,5,41,0,0,244,239,1,0,0,0,244,240,1,0,0,0,244,241,1,0,
  	0,0,244,242,1,0,0,0,244,243,1,0,0,0,245,35,1,0,0,0,246,248,3,38,19,0,
  	247,249,3,40,20,0,248,247,1,0,0,0,248,249,1,0,0,0,249,251,1,0,0,0,250,
  	252,5,41,0,0,251,250,1,0,0,0,251,252,1,0,0,0,252,253,1,0,0,0,253,255,
  	5,23,0,0,254,256,3,42,21,0,255,254,1,0,0,0,255,256,1,0,0,0,256,257,1,
  	0,0,0,257,258,5,24,0,0,258,263,1,0,0,0,259,260,3,38,19,0,260,261,5,41,
  	0,0,261,263,1,0,0,0,262,246,1,0,0,0,262,259,1,0,0,0,263,37,1,0,0,0,264,
  	265,7,7,0,0,265,39,1,0,0,0,266,267,5,10,0,0,267,268,5,19,0,0,268,269,
  	5,42,0,0,269,270,5,20,0,0,270,41,1,0,0,0,271,273,3,44,22,0,272,271,1,
  	0,0,0,273,274,1,0,0,0,274,272,1,0,0,0,274,275,1,0,0,0,275,43,1,0,0,0,
  	276,277,3,46,23,0,277,278,3,48,24,0,278,279,5,39,0,0,279,284,1,0,0,0,
  	280,281,3,46,23,0,281,282,5,39,0,0,282,284,1,0,0,0,283,276,1,0,0,0,283,
  	280,1,0,0,0,284,45,1,0,0,0,285,287,5,3,0,0,286,285,1,0,0,0,286,287,1,
  	0,0,0,287,288,1,0,0,0,288,290,3,34,17,0,289,291,5,3,0,0,290,289,1,0,0,
  	0,290,291,1,0,0,0,291,47,1,0,0,0,292,297,3,50,25,0,293,294,5,40,0,0,294,
  	296,3,50,25,0,295,293,1,0,0,0,296,299,1,0,0,0,297,295,1,0,0,0,297,298,
  	1,0,0,0,298,49,1,0,0,0,299,297,1,0,0,0,300,307,3,60,30,0,301,303,3,60,
  	30,0,302,301,1,0,0,0,302,303,1,0,0,0,303,304,1,0,0,0,304,305,5,38,0,0,
  	305,307,3,28,14,0,306,300,1,0,0,0,306,302,1,0,0,0,307,51,1,0,0,0,308,
  	310,5,5,0,0,309,311,5,41,0,0,310,309,1,0,0,0,310,311,1,0,0,0,311,312,
  	1,0,0,0,312,313,5,23,0,0,313,315,3,54,27,0,314,316,5,40,0,0,315,314,1,
  	0,0,0,315,316,1,0,0,0,316,317,1,0,0,0,317,318,5,24,0,0,318,322,1,0,0,
  	0,319,320,5,5,0,0,320,322,5,41,0,0,321,308,1,0,0,0,321,319,1,0,0,0,322,
  	53,1,0,0,0,323,328,3,56,28,0,324,325,5,40,0,0,325,327,3,56,28,0,326,324,
  	1,0,0,0,327,330,1,0,0,0,328,326,1,0,0,0,328,329,1,0,0,0,329,55,1,0,0,
  	0,330,328,1,0,0,0,331,334,3,58,29,0,332,333,5,1,0,0,333,335,3,28,14,0,
  	334,332,1,0,0,0,334,335,1,0,0,0,335,57,1,0,0,0,336,337,5,41,0,0,337,59,
  	1,0,0,0,338,340,3,64,32,0,339,338,1,0,0,0,339,340,1,0,0,0,340,341,1,0,
  	0,0,341,342,3,62,31,0,342,61,1,0,0,0,343,344,6,31,-1,0,344,351,5,41,0,
  	0,345,351,5,18,0,0,346,347,5,19,0,0,347,348,3,60,30,0,348,349,5,20,0,
  	0,349,351,1,0,0,0,350,343,1,0,0,0,350,345,1,0,0,0,350,346,1,0,0,0,351,
  	359,1,0,0,0,352,353,10,1,0,0,353,354,5,21,0,0,354,355,3,28,14,0,355,356,
  	5,22,0,0,356,358,1,0,0,0,357,352,1,0,0,0,358,361,1,0,0,0,359,357,1,0,
  	0,0,359,360,1,0,0,0,360,63,1,0,0,0,361,359,1,0,0,0,362,364,5,31,0,0,363,
  	365,5,3,0,0,364,363,1,0,0,0,364,365,1,0,0,0,365,367,1,0,0,0,366,362,1,
  	0,0,0,367,368,1,0,0,0,368,366,1,0,0,0,368,369,1,0,0,0,369,65,1,0,0,0,
  	370,375,5,41,0,0,371,372,5,40,0,0,372,374,5,41,0,0,373,371,1,0,0,0,374,
  	377,1,0,0,0,375,373,1,0,0,0,375,376,1,0,0,0,376,67,1,0,0,0,377,375,1,
  	0,0,0,378,380,3,46,23,0,379,381,3,70,35,0,380,379,1,0,0,0,380,381,1,0,
  	0,0,381,69,1,0,0,0,382,388,3,64,32,0,383,385,3,64,32,0,384,383,1,0,0,
  	0,384,385,1,0,0,0,385,386,1,0,0,0,386,388,3,72,36,0,387,382,1,0,0,0,387,
  	384,1,0,0,0,388,71,1,0,0,0,389,390,6,36,-1,0,390,391,5,19,0,0,391,392,
  	3,70,35,0,392,393,5,20,0,0,393,399,1,0,0,0,394,395,5,21,0,0,395,396,3,
  	28,14,0,396,397,5,22,0,0,397,399,1,0,0,0,398,389,1,0,0,0,398,394,1,0,
  	0,0,399,407,1,0,0,0,400,401,10,1,0,0,401,402,5,21,0,0,402,403,3,28,14,
  	0,403,404,5,22,0,0,404,406,1,0,0,0,405,400,1,0,0,0,406,409,1,0,0,0,407,
  	405,1,0,0,0,407,408,1,0,0,0,408,73,1,0,0,0,409,407,1,0,0,0,410,412,3,
  	76,38,0,411,410,1,0,0,0,411,412,1,0,0,0,412,413,1,0,0,0,413,414,5,0,0,
  	1,414,75,1,0,0,0,415,417,3,78,39,0,416,415,1,0,0,0,417,418,1,0,0,0,418,
  	416,1,0,0,0,418,419,1,0,0,0,419,77,1,0,0,0,420,424,3,30,15,0,421,424,
  	3,80,40,0,422,424,5,39,0,0,423,420,1,0,0,0,423,421,1,0,0,0,423,422,1,
  	0,0,0,424,79,1,0,0,0,425,426,7,8,0,0,426,81,1,0,0,0,56,83,86,91,94,97,
  	100,107,110,114,129,135,141,148,159,166,174,182,190,198,206,213,220,228,
  	233,237,244,248,251,255,262,274,283,286,290,297,302,306,310,315,321,328,
  	334,339,350,359,364,368,375,380,384,387,398,407,411,418,423
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  cstructParserStaticData = std::move(staticData);
}

}

CStructParser::CStructParser(TokenStream *input) : CStructParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

CStructParser::CStructParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  CStructParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *cstructParserStaticData->atn, cstructParserStaticData->decisionToDFA, cstructParserStaticData->sharedContextCache, options);
}

CStructParser::~CStructParser() {
  delete _interpreter;
}

const atn::ATN& CStructParser::getATN() const {
  return *cstructParserStaticData->atn;
}

std::string CStructParser::getGrammarFileName() const {
  return "CStruct.g4";
}

const std::vector<std::string>& CStructParser::getRuleNames() const {
  return cstructParserStaticData->ruleNames;
}

const dfa::Vocabulary& CStructParser::getVocabulary() const {
  return cstructParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView CStructParser::getSerializedATN() const {
  return cstructParserStaticData->serializedATN;
}


//----------------- InternalBasicTypesContext ------------------------------------------------------------------

CStructParser::InternalBasicTypesContext::InternalBasicTypesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::InternalBasicTypesContext::Char() {
  return getToken(CStructParser::Char, 0);
}

tree::TerminalNode* CStructParser::InternalBasicTypesContext::Short() {
  return getToken(CStructParser::Short, 0);
}

tree::TerminalNode* CStructParser::InternalBasicTypesContext::Int() {
  return getToken(CStructParser::Int, 0);
}

std::vector<tree::TerminalNode *> CStructParser::InternalBasicTypesContext::Long() {
  return getTokens(CStructParser::Long);
}

tree::TerminalNode* CStructParser::InternalBasicTypesContext::Long(size_t i) {
  return getToken(CStructParser::Long, i);
}

CStructParser::SignOrUnsignedContext* CStructParser::InternalBasicTypesContext::signOrUnsigned() {
  return getRuleContext<CStructParser::SignOrUnsignedContext>(0);
}

tree::TerminalNode* CStructParser::InternalBasicTypesContext::Const() {
  return getToken(CStructParser::Const, 0);
}

tree::TerminalNode* CStructParser::InternalBasicTypesContext::Float() {
  return getToken(CStructParser::Float, 0);
}

tree::TerminalNode* CStructParser::InternalBasicTypesContext::Double() {
  return getToken(CStructParser::Double, 0);
}


size_t CStructParser::InternalBasicTypesContext::getRuleIndex() const {
  return CStructParser::RuleInternalBasicTypes;
}


std::any CStructParser::InternalBasicTypesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitInternalBasicTypes(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::InternalBasicTypesContext* CStructParser::internalBasicTypes() {
  InternalBasicTypesContext *_localctx = _tracker.createInstance<InternalBasicTypesContext>(_ctx, getState());
  enterRule(_localctx, 0, CStructParser::RuleInternalBasicTypes);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(114);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(83);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Signed

      || _la == CStructParser::Unsigned) {
        setState(82);
        signOrUnsigned();
      }
      setState(86);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Const) {
        setState(85);
        match(CStructParser::Const);
      }
      setState(88);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 4484) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(89);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 4484) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(91);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
      case 1: {
        setState(90);
        match(CStructParser::Const);
        break;
      }

      default:
        break;
      }
      setState(94);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Signed

      || _la == CStructParser::Unsigned) {
        setState(93);
        signOrUnsigned();
      }
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(97);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Signed

      || _la == CStructParser::Unsigned) {
        setState(96);
        signOrUnsigned();
      }
      setState(100);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Const) {
        setState(99);
        match(CStructParser::Const);
      }
      setState(102);
      match(CStructParser::Long);
      setState(103);
      match(CStructParser::Long);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(104);
      match(CStructParser::Long);
      setState(105);
      match(CStructParser::Long);
      setState(107);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
      case 1: {
        setState(106);
        match(CStructParser::Const);
        break;
      }

      default:
        break;
      }
      setState(110);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Signed

      || _la == CStructParser::Unsigned) {
        setState(109);
        signOrUnsigned();
      }
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(112);
      match(CStructParser::Float);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(113);
      match(CStructParser::Double);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SignOrUnsignedContext ------------------------------------------------------------------

CStructParser::SignOrUnsignedContext::SignOrUnsignedContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::SignOrUnsignedContext::Signed() {
  return getToken(CStructParser::Signed, 0);
}

tree::TerminalNode* CStructParser::SignOrUnsignedContext::Unsigned() {
  return getToken(CStructParser::Unsigned, 0);
}


size_t CStructParser::SignOrUnsignedContext::getRuleIndex() const {
  return CStructParser::RuleSignOrUnsigned;
}


std::any CStructParser::SignOrUnsignedContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitSignOrUnsigned(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::SignOrUnsignedContext* CStructParser::signOrUnsigned() {
  SignOrUnsignedContext *_localctx = _tracker.createInstance<SignOrUnsignedContext>(_ctx, getState());
  enterRule(_localctx, 2, CStructParser::RuleSignOrUnsigned);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(116);
    _la = _input->LA(1);
    if (!(_la == CStructParser::Signed

    || _la == CStructParser::Unsigned)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpressionContext ------------------------------------------------------------------

CStructParser::PrimaryExpressionContext::PrimaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::PrimaryExpressionContext::Identifier() {
  return getToken(CStructParser::Identifier, 0);
}

tree::TerminalNode* CStructParser::PrimaryExpressionContext::IntegerConstant() {
  return getToken(CStructParser::IntegerConstant, 0);
}

tree::TerminalNode* CStructParser::PrimaryExpressionContext::LeftParen() {
  return getToken(CStructParser::LeftParen, 0);
}

CStructParser::AssignmentExpressionContext* CStructParser::PrimaryExpressionContext::assignmentExpression() {
  return getRuleContext<CStructParser::AssignmentExpressionContext>(0);
}

tree::TerminalNode* CStructParser::PrimaryExpressionContext::RightParen() {
  return getToken(CStructParser::RightParen, 0);
}

tree::TerminalNode* CStructParser::PrimaryExpressionContext::SizeOf() {
  return getToken(CStructParser::SizeOf, 0);
}

CStructParser::SpecifierQualifierListContext* CStructParser::PrimaryExpressionContext::specifierQualifierList() {
  return getRuleContext<CStructParser::SpecifierQualifierListContext>(0);
}


size_t CStructParser::PrimaryExpressionContext::getRuleIndex() const {
  return CStructParser::RulePrimaryExpression;
}


std::any CStructParser::PrimaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::PrimaryExpressionContext* CStructParser::primaryExpression() {
  PrimaryExpressionContext *_localctx = _tracker.createInstance<PrimaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 4, CStructParser::RulePrimaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(129);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::Identifier: {
        enterOuterAlt(_localctx, 1);
        setState(118);
        match(CStructParser::Identifier);
        break;
      }

      case CStructParser::IntegerConstant: {
        enterOuterAlt(_localctx, 2);
        setState(119);
        match(CStructParser::IntegerConstant);
        break;
      }

      case CStructParser::LeftParen: {
        enterOuterAlt(_localctx, 3);
        setState(120);
        match(CStructParser::LeftParen);
        setState(121);
        assignmentExpression();
        setState(122);
        match(CStructParser::RightParen);
        break;
      }

      case CStructParser::SizeOf: {
        enterOuterAlt(_localctx, 4);
        setState(124);
        match(CStructParser::SizeOf);
        setState(125);
        match(CStructParser::LeftParen);
        setState(126);
        specifierQualifierList();
        setState(127);
        match(CStructParser::RightParen);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PostfixExpressionContext ------------------------------------------------------------------

CStructParser::PostfixExpressionContext::PostfixExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::PrimaryExpressionContext* CStructParser::PostfixExpressionContext::primaryExpression() {
  return getRuleContext<CStructParser::PrimaryExpressionContext>(0);
}

std::vector<tree::TerminalNode *> CStructParser::PostfixExpressionContext::PlusPlus() {
  return getTokens(CStructParser::PlusPlus);
}

tree::TerminalNode* CStructParser::PostfixExpressionContext::PlusPlus(size_t i) {
  return getToken(CStructParser::PlusPlus, i);
}

std::vector<tree::TerminalNode *> CStructParser::PostfixExpressionContext::MinusMinus() {
  return getTokens(CStructParser::MinusMinus);
}

tree::TerminalNode* CStructParser::PostfixExpressionContext::MinusMinus(size_t i) {
  return getToken(CStructParser::MinusMinus, i);
}


size_t CStructParser::PostfixExpressionContext::getRuleIndex() const {
  return CStructParser::RulePostfixExpression;
}


std::any CStructParser::PostfixExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitPostfixExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::PostfixExpressionContext* CStructParser::postfixExpression() {
  PostfixExpressionContext *_localctx = _tracker.createInstance<PostfixExpressionContext>(_ctx, getState());
  enterRule(_localctx, 6, CStructParser::RulePostfixExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(131);
    primaryExpression();
    setState(135);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::PlusPlus

    || _la == CStructParser::MinusMinus) {
      setState(132);
      _la = _input->LA(1);
      if (!(_la == CStructParser::PlusPlus

      || _la == CStructParser::MinusMinus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(137);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpressionContext ------------------------------------------------------------------

CStructParser::UnaryExpressionContext::UnaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::PostfixExpressionContext* CStructParser::UnaryExpressionContext::postfixExpression() {
  return getRuleContext<CStructParser::PostfixExpressionContext>(0);
}

CStructParser::UnaryOperatorContext* CStructParser::UnaryExpressionContext::unaryOperator() {
  return getRuleContext<CStructParser::UnaryOperatorContext>(0);
}

CStructParser::CastExpressionContext* CStructParser::UnaryExpressionContext::castExpression() {
  return getRuleContext<CStructParser::CastExpressionContext>(0);
}

std::vector<tree::TerminalNode *> CStructParser::UnaryExpressionContext::PlusPlus() {
  return getTokens(CStructParser::PlusPlus);
}

tree::TerminalNode* CStructParser::UnaryExpressionContext::PlusPlus(size_t i) {
  return getToken(CStructParser::PlusPlus, i);
}

std::vector<tree::TerminalNode *> CStructParser::UnaryExpressionContext::MinusMinus() {
  return getTokens(CStructParser::MinusMinus);
}

tree::TerminalNode* CStructParser::UnaryExpressionContext::MinusMinus(size_t i) {
  return getToken(CStructParser::MinusMinus, i);
}


size_t CStructParser::UnaryExpressionContext::getRuleIndex() const {
  return CStructParser::RuleUnaryExpression;
}


std::any CStructParser::UnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::UnaryExpressionContext* CStructParser::unaryExpression() {
  UnaryExpressionContext *_localctx = _tracker.createInstance<UnaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 8, CStructParser::RuleUnaryExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(141);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::PlusPlus

    || _la == CStructParser::MinusMinus) {
      setState(138);
      _la = _input->LA(1);
      if (!(_la == CStructParser::PlusPlus

      || _la == CStructParser::MinusMinus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(143);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(148);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::SizeOf:
      case CStructParser::LeftParen:
      case CStructParser::Identifier:
      case CStructParser::IntegerConstant: {
        setState(144);
        postfixExpression();
        break;
      }

      case CStructParser::Plus:
      case CStructParser::Minus:
      case CStructParser::Tilde: {
        setState(145);
        unaryOperator();
        setState(146);
        castExpression();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryOperatorContext ------------------------------------------------------------------

CStructParser::UnaryOperatorContext::UnaryOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::UnaryOperatorContext::Plus() {
  return getToken(CStructParser::Plus, 0);
}

tree::TerminalNode* CStructParser::UnaryOperatorContext::Minus() {
  return getToken(CStructParser::Minus, 0);
}

tree::TerminalNode* CStructParser::UnaryOperatorContext::Tilde() {
  return getToken(CStructParser::Tilde, 0);
}


size_t CStructParser::UnaryOperatorContext::getRuleIndex() const {
  return CStructParser::RuleUnaryOperator;
}


std::any CStructParser::UnaryOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitUnaryOperator(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::UnaryOperatorContext* CStructParser::unaryOperator() {
  UnaryOperatorContext *_localctx = _tracker.createInstance<UnaryOperatorContext>(_ctx, getState());
  enterRule(_localctx, 10, CStructParser::RuleUnaryOperator);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(150);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 138110042112) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CastExpressionContext ------------------------------------------------------------------

CStructParser::CastExpressionContext::CastExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::CastExpressionContext::LeftParen() {
  return getToken(CStructParser::LeftParen, 0);
}

CStructParser::TypeNameContext* CStructParser::CastExpressionContext::typeName() {
  return getRuleContext<CStructParser::TypeNameContext>(0);
}

tree::TerminalNode* CStructParser::CastExpressionContext::RightParen() {
  return getToken(CStructParser::RightParen, 0);
}

CStructParser::CastExpressionContext* CStructParser::CastExpressionContext::castExpression() {
  return getRuleContext<CStructParser::CastExpressionContext>(0);
}

CStructParser::UnaryExpressionContext* CStructParser::CastExpressionContext::unaryExpression() {
  return getRuleContext<CStructParser::UnaryExpressionContext>(0);
}

tree::TerminalNode* CStructParser::CastExpressionContext::IntegerConstant() {
  return getToken(CStructParser::IntegerConstant, 0);
}


size_t CStructParser::CastExpressionContext::getRuleIndex() const {
  return CStructParser::RuleCastExpression;
}


std::any CStructParser::CastExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitCastExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::CastExpressionContext* CStructParser::castExpression() {
  CastExpressionContext *_localctx = _tracker.createInstance<CastExpressionContext>(_ctx, getState());
  enterRule(_localctx, 12, CStructParser::RuleCastExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(159);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(152);
      match(CStructParser::LeftParen);
      setState(153);
      typeName();
      setState(154);
      match(CStructParser::RightParen);
      setState(155);
      castExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(157);
      unaryExpression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(158);
      match(CStructParser::IntegerConstant);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MultiplicativeExpressionContext ------------------------------------------------------------------

CStructParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::CastExpressionContext *> CStructParser::MultiplicativeExpressionContext::castExpression() {
  return getRuleContexts<CStructParser::CastExpressionContext>();
}

CStructParser::CastExpressionContext* CStructParser::MultiplicativeExpressionContext::castExpression(size_t i) {
  return getRuleContext<CStructParser::CastExpressionContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::MultiplicativeExpressionContext::Star() {
  return getTokens(CStructParser::Star);
}

tree::TerminalNode* CStructParser::MultiplicativeExpressionContext::Star(size_t i) {
  return getToken(CStructParser::Star, i);
}

std::vector<tree::TerminalNode *> CStructParser::MultiplicativeExpressionContext::Div() {
  return getTokens(CStructParser::Div);
}

tree::TerminalNode* CStructParser::MultiplicativeExpressionContext::Div(size_t i) {
  return getToken(CStructParser::Div, i);
}

std::vector<tree::TerminalNode *> CStructParser::MultiplicativeExpressionContext::Mod() {
  return getTokens(CStructParser::Mod);
}

tree::TerminalNode* CStructParser::MultiplicativeExpressionContext::Mod(size_t i) {
  return getToken(CStructParser::Mod, i);
}


size_t CStructParser::MultiplicativeExpressionContext::getRuleIndex() const {
  return CStructParser::RuleMultiplicativeExpression;
}


std::any CStructParser::MultiplicativeExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::MultiplicativeExpressionContext* CStructParser::multiplicativeExpression() {
  MultiplicativeExpressionContext *_localctx = _tracker.createInstance<MultiplicativeExpressionContext>(_ctx, getState());
  enterRule(_localctx, 14, CStructParser::RuleMultiplicativeExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(161);
    castExpression();
    setState(166);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15032385536) != 0)) {
      setState(162);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 15032385536) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(163);
      castExpression();
      setState(168);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AdditiveExpressionContext ------------------------------------------------------------------

CStructParser::AdditiveExpressionContext::AdditiveExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::MultiplicativeExpressionContext *> CStructParser::AdditiveExpressionContext::multiplicativeExpression() {
  return getRuleContexts<CStructParser::MultiplicativeExpressionContext>();
}

CStructParser::MultiplicativeExpressionContext* CStructParser::AdditiveExpressionContext::multiplicativeExpression(size_t i) {
  return getRuleContext<CStructParser::MultiplicativeExpressionContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::AdditiveExpressionContext::Plus() {
  return getTokens(CStructParser::Plus);
}

tree::TerminalNode* CStructParser::AdditiveExpressionContext::Plus(size_t i) {
  return getToken(CStructParser::Plus, i);
}

std::vector<tree::TerminalNode *> CStructParser::AdditiveExpressionContext::Minus() {
  return getTokens(CStructParser::Minus);
}

tree::TerminalNode* CStructParser::AdditiveExpressionContext::Minus(size_t i) {
  return getToken(CStructParser::Minus, i);
}


size_t CStructParser::AdditiveExpressionContext::getRuleIndex() const {
  return CStructParser::RuleAdditiveExpression;
}


std::any CStructParser::AdditiveExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::AdditiveExpressionContext* CStructParser::additiveExpression() {
  AdditiveExpressionContext *_localctx = _tracker.createInstance<AdditiveExpressionContext>(_ctx, getState());
  enterRule(_localctx, 16, CStructParser::RuleAdditiveExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(169);
    multiplicativeExpression();
    setState(174);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::Plus

    || _la == CStructParser::Minus) {
      setState(170);
      _la = _input->LA(1);
      if (!(_la == CStructParser::Plus

      || _la == CStructParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(171);
      multiplicativeExpression();
      setState(176);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShiftExpressionContext ------------------------------------------------------------------

CStructParser::ShiftExpressionContext::ShiftExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::AdditiveExpressionContext *> CStructParser::ShiftExpressionContext::additiveExpression() {
  return getRuleContexts<CStructParser::AdditiveExpressionContext>();
}

CStructParser::AdditiveExpressionContext* CStructParser::ShiftExpressionContext::additiveExpression(size_t i) {
  return getRuleContext<CStructParser::AdditiveExpressionContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::ShiftExpressionContext::LeftShift() {
  return getTokens(CStructParser::LeftShift);
}

tree::TerminalNode* CStructParser::ShiftExpressionContext::LeftShift(size_t i) {
  return getToken(CStructParser::LeftShift, i);
}

std::vector<tree::TerminalNode *> CStructParser::ShiftExpressionContext::RightShift() {
  return getTokens(CStructParser::RightShift);
}

tree::TerminalNode* CStructParser::ShiftExpressionContext::RightShift(size_t i) {
  return getToken(CStructParser::RightShift, i);
}


size_t CStructParser::ShiftExpressionContext::getRuleIndex() const {
  return CStructParser::RuleShiftExpression;
}


std::any CStructParser::ShiftExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitShiftExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::ShiftExpressionContext* CStructParser::shiftExpression() {
  ShiftExpressionContext *_localctx = _tracker.createInstance<ShiftExpressionContext>(_ctx, getState());
  enterRule(_localctx, 18, CStructParser::RuleShiftExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(177);
    additiveExpression();
    setState(182);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::LeftShift

    || _la == CStructParser::RightShift) {
      setState(178);
      _la = _input->LA(1);
      if (!(_la == CStructParser::LeftShift

      || _la == CStructParser::RightShift)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(179);
      additiveExpression();
      setState(184);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AndExpressionContext ------------------------------------------------------------------

CStructParser::AndExpressionContext::AndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::ShiftExpressionContext *> CStructParser::AndExpressionContext::shiftExpression() {
  return getRuleContexts<CStructParser::ShiftExpressionContext>();
}

CStructParser::ShiftExpressionContext* CStructParser::AndExpressionContext::shiftExpression(size_t i) {
  return getRuleContext<CStructParser::ShiftExpressionContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::AndExpressionContext::And() {
  return getTokens(CStructParser::And);
}

tree::TerminalNode* CStructParser::AndExpressionContext::And(size_t i) {
  return getToken(CStructParser::And, i);
}


size_t CStructParser::AndExpressionContext::getRuleIndex() const {
  return CStructParser::RuleAndExpression;
}


std::any CStructParser::AndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitAndExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::AndExpressionContext* CStructParser::andExpression() {
  AndExpressionContext *_localctx = _tracker.createInstance<AndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 20, CStructParser::RuleAndExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(185);
    shiftExpression();
    setState(190);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::And) {
      setState(186);
      match(CStructParser::And);
      setState(187);
      shiftExpression();
      setState(192);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExclusiveOrExpressionContext ------------------------------------------------------------------

CStructParser::ExclusiveOrExpressionContext::ExclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::AndExpressionContext *> CStructParser::ExclusiveOrExpressionContext::andExpression() {
  return getRuleContexts<CStructParser::AndExpressionContext>();
}

CStructParser::AndExpressionContext* CStructParser::ExclusiveOrExpressionContext::andExpression(size_t i) {
  return getRuleContext<CStructParser::AndExpressionContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::ExclusiveOrExpressionContext::Caret() {
  return getTokens(CStructParser::Caret);
}

tree::TerminalNode* CStructParser::ExclusiveOrExpressionContext::Caret(size_t i) {
  return getToken(CStructParser::Caret, i);
}


size_t CStructParser::ExclusiveOrExpressionContext::getRuleIndex() const {
  return CStructParser::RuleExclusiveOrExpression;
}


std::any CStructParser::ExclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitExclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::ExclusiveOrExpressionContext* CStructParser::exclusiveOrExpression() {
  ExclusiveOrExpressionContext *_localctx = _tracker.createInstance<ExclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 22, CStructParser::RuleExclusiveOrExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(193);
    andExpression();
    setState(198);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::Caret) {
      setState(194);
      match(CStructParser::Caret);
      setState(195);
      andExpression();
      setState(200);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InclusiveOrExpressionContext ------------------------------------------------------------------

CStructParser::InclusiveOrExpressionContext::InclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::ExclusiveOrExpressionContext *> CStructParser::InclusiveOrExpressionContext::exclusiveOrExpression() {
  return getRuleContexts<CStructParser::ExclusiveOrExpressionContext>();
}

CStructParser::ExclusiveOrExpressionContext* CStructParser::InclusiveOrExpressionContext::exclusiveOrExpression(size_t i) {
  return getRuleContext<CStructParser::ExclusiveOrExpressionContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::InclusiveOrExpressionContext::Or() {
  return getTokens(CStructParser::Or);
}

tree::TerminalNode* CStructParser::InclusiveOrExpressionContext::Or(size_t i) {
  return getToken(CStructParser::Or, i);
}


size_t CStructParser::InclusiveOrExpressionContext::getRuleIndex() const {
  return CStructParser::RuleInclusiveOrExpression;
}


std::any CStructParser::InclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitInclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::InclusiveOrExpressionContext* CStructParser::inclusiveOrExpression() {
  InclusiveOrExpressionContext *_localctx = _tracker.createInstance<InclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 24, CStructParser::RuleInclusiveOrExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(201);
    exclusiveOrExpression();
    setState(206);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::Or) {
      setState(202);
      match(CStructParser::Or);
      setState(203);
      exclusiveOrExpression();
      setState(208);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignmentExpressionDefContext ------------------------------------------------------------------

CStructParser::AssignmentExpressionDefContext::AssignmentExpressionDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::AssignmentExpressionContext* CStructParser::AssignmentExpressionDefContext::assignmentExpression() {
  return getRuleContext<CStructParser::AssignmentExpressionContext>(0);
}

tree::TerminalNode* CStructParser::AssignmentExpressionDefContext::EOF() {
  return getToken(CStructParser::EOF, 0);
}

std::vector<tree::TerminalNode *> CStructParser::AssignmentExpressionDefContext::Semi() {
  return getTokens(CStructParser::Semi);
}

tree::TerminalNode* CStructParser::AssignmentExpressionDefContext::Semi(size_t i) {
  return getToken(CStructParser::Semi, i);
}


size_t CStructParser::AssignmentExpressionDefContext::getRuleIndex() const {
  return CStructParser::RuleAssignmentExpressionDef;
}


std::any CStructParser::AssignmentExpressionDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitAssignmentExpressionDef(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::AssignmentExpressionDefContext* CStructParser::assignmentExpressionDef() {
  AssignmentExpressionDefContext *_localctx = _tracker.createInstance<AssignmentExpressionDefContext>(_ctx, getState());
  enterRule(_localctx, 26, CStructParser::RuleAssignmentExpressionDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(209);
    assignmentExpression();
    setState(213);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::Semi) {
      setState(210);
      match(CStructParser::Semi);
      setState(215);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(216);
    match(CStructParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignmentExpressionContext ------------------------------------------------------------------

CStructParser::AssignmentExpressionContext::AssignmentExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::AssignmentExpressionContext::IntegerConstant() {
  return getToken(CStructParser::IntegerConstant, 0);
}

CStructParser::InclusiveOrExpressionContext* CStructParser::AssignmentExpressionContext::inclusiveOrExpression() {
  return getRuleContext<CStructParser::InclusiveOrExpressionContext>(0);
}


size_t CStructParser::AssignmentExpressionContext::getRuleIndex() const {
  return CStructParser::RuleAssignmentExpression;
}


std::any CStructParser::AssignmentExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitAssignmentExpression(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::AssignmentExpressionContext* CStructParser::assignmentExpression() {
  AssignmentExpressionContext *_localctx = _tracker.createInstance<AssignmentExpressionContext>(_ctx, getState());
  enterRule(_localctx, 28, CStructParser::RuleAssignmentExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(220);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(218);
      match(CStructParser::IntegerConstant);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(219);
      inclusiveOrExpression();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclarationContext ------------------------------------------------------------------

CStructParser::DeclarationContext::DeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::DeclarationSpecifierContext* CStructParser::DeclarationContext::declarationSpecifier() {
  return getRuleContext<CStructParser::DeclarationSpecifierContext>(0);
}

tree::TerminalNode* CStructParser::DeclarationContext::Semi() {
  return getToken(CStructParser::Semi, 0);
}

tree::TerminalNode* CStructParser::DeclarationContext::TypeDef() {
  return getToken(CStructParser::TypeDef, 0);
}

CStructParser::TypeSpecifierContext* CStructParser::DeclarationContext::typeSpecifier() {
  return getRuleContext<CStructParser::TypeSpecifierContext>(0);
}

tree::TerminalNode* CStructParser::DeclarationContext::Identifier() {
  return getToken(CStructParser::Identifier, 0);
}

CStructParser::PointerContext* CStructParser::DeclarationContext::pointer() {
  return getRuleContext<CStructParser::PointerContext>(0);
}


size_t CStructParser::DeclarationContext::getRuleIndex() const {
  return CStructParser::RuleDeclaration;
}


std::any CStructParser::DeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitDeclaration(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::DeclarationContext* CStructParser::declaration() {
  DeclarationContext *_localctx = _tracker.createInstance<DeclarationContext>(_ctx, getState());
  enterRule(_localctx, 30, CStructParser::RuleDeclaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(233);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::Enum:
      case CStructParser::Struct:
      case CStructParser::Union: {
        enterOuterAlt(_localctx, 1);
        setState(222);
        declarationSpecifier();
        setState(223);
        match(CStructParser::Semi);
        break;
      }

      case CStructParser::TypeDef: {
        enterOuterAlt(_localctx, 2);
        setState(225);
        match(CStructParser::TypeDef);
        setState(226);
        typeSpecifier();
        setState(228);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == CStructParser::Star) {
          setState(227);
          pointer();
        }
        setState(230);
        match(CStructParser::Identifier);
        setState(231);
        match(CStructParser::Semi);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclarationSpecifierContext ------------------------------------------------------------------

CStructParser::DeclarationSpecifierContext::DeclarationSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::StructOrUnionSpecifierContext* CStructParser::DeclarationSpecifierContext::structOrUnionSpecifier() {
  return getRuleContext<CStructParser::StructOrUnionSpecifierContext>(0);
}

CStructParser::EnumSpecifierContext* CStructParser::DeclarationSpecifierContext::enumSpecifier() {
  return getRuleContext<CStructParser::EnumSpecifierContext>(0);
}


size_t CStructParser::DeclarationSpecifierContext::getRuleIndex() const {
  return CStructParser::RuleDeclarationSpecifier;
}


std::any CStructParser::DeclarationSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitDeclarationSpecifier(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::DeclarationSpecifierContext* CStructParser::declarationSpecifier() {
  DeclarationSpecifierContext *_localctx = _tracker.createInstance<DeclarationSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 32, CStructParser::RuleDeclarationSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(237);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::Struct:
      case CStructParser::Union: {
        enterOuterAlt(_localctx, 1);
        setState(235);
        structOrUnionSpecifier();
        break;
      }

      case CStructParser::Enum: {
        enterOuterAlt(_localctx, 2);
        setState(236);
        enumSpecifier();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeSpecifierContext ------------------------------------------------------------------

CStructParser::TypeSpecifierContext::TypeSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::TypeSpecifierContext::Void() {
  return getToken(CStructParser::Void, 0);
}

CStructParser::InternalBasicTypesContext* CStructParser::TypeSpecifierContext::internalBasicTypes() {
  return getRuleContext<CStructParser::InternalBasicTypesContext>(0);
}

CStructParser::StructOrUnionSpecifierContext* CStructParser::TypeSpecifierContext::structOrUnionSpecifier() {
  return getRuleContext<CStructParser::StructOrUnionSpecifierContext>(0);
}

CStructParser::EnumSpecifierContext* CStructParser::TypeSpecifierContext::enumSpecifier() {
  return getRuleContext<CStructParser::EnumSpecifierContext>(0);
}

tree::TerminalNode* CStructParser::TypeSpecifierContext::Identifier() {
  return getToken(CStructParser::Identifier, 0);
}


size_t CStructParser::TypeSpecifierContext::getRuleIndex() const {
  return CStructParser::RuleTypeSpecifier;
}


std::any CStructParser::TypeSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitTypeSpecifier(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::TypeSpecifierContext* CStructParser::typeSpecifier() {
  TypeSpecifierContext *_localctx = _tracker.createInstance<TypeSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 34, CStructParser::RuleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(244);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::Void: {
        enterOuterAlt(_localctx, 1);
        setState(239);
        match(CStructParser::Void);
        break;
      }

      case CStructParser::Char:
      case CStructParser::Const:
      case CStructParser::Double:
      case CStructParser::Float:
      case CStructParser::Int:
      case CStructParser::Long:
      case CStructParser::Short:
      case CStructParser::Signed:
      case CStructParser::Unsigned: {
        enterOuterAlt(_localctx, 2);
        setState(240);
        internalBasicTypes();
        break;
      }

      case CStructParser::Struct:
      case CStructParser::Union: {
        enterOuterAlt(_localctx, 3);
        setState(241);
        structOrUnionSpecifier();
        break;
      }

      case CStructParser::Enum: {
        enterOuterAlt(_localctx, 4);
        setState(242);
        enumSpecifier();
        break;
      }

      case CStructParser::Identifier: {
        enterOuterAlt(_localctx, 5);
        setState(243);
        match(CStructParser::Identifier);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructOrUnionSpecifierContext ------------------------------------------------------------------

CStructParser::StructOrUnionSpecifierContext::StructOrUnionSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::StructOrUnionContext* CStructParser::StructOrUnionSpecifierContext::structOrUnion() {
  return getRuleContext<CStructParser::StructOrUnionContext>(0);
}

tree::TerminalNode* CStructParser::StructOrUnionSpecifierContext::LeftBrace() {
  return getToken(CStructParser::LeftBrace, 0);
}

tree::TerminalNode* CStructParser::StructOrUnionSpecifierContext::RightBrace() {
  return getToken(CStructParser::RightBrace, 0);
}

CStructParser::AlignAsAttrContext* CStructParser::StructOrUnionSpecifierContext::alignAsAttr() {
  return getRuleContext<CStructParser::AlignAsAttrContext>(0);
}

tree::TerminalNode* CStructParser::StructOrUnionSpecifierContext::Identifier() {
  return getToken(CStructParser::Identifier, 0);
}

CStructParser::StructDeclarationListContext* CStructParser::StructOrUnionSpecifierContext::structDeclarationList() {
  return getRuleContext<CStructParser::StructDeclarationListContext>(0);
}


size_t CStructParser::StructOrUnionSpecifierContext::getRuleIndex() const {
  return CStructParser::RuleStructOrUnionSpecifier;
}


std::any CStructParser::StructOrUnionSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitStructOrUnionSpecifier(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::StructOrUnionSpecifierContext* CStructParser::structOrUnionSpecifier() {
  StructOrUnionSpecifierContext *_localctx = _tracker.createInstance<StructOrUnionSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 36, CStructParser::RuleStructOrUnionSpecifier);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(262);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(246);
      structOrUnion();
      setState(248);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::AlignAs) {
        setState(247);
        alignAsAttr();
      }
      setState(251);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Identifier) {
        setState(250);
        match(CStructParser::Identifier);
      }
      setState(253);
      match(CStructParser::LeftBrace);
      setState(255);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 2199023514108) != 0)) {
        setState(254);
        structDeclarationList();
      }
      setState(257);
      match(CStructParser::RightBrace);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(259);
      structOrUnion();
      setState(260);
      match(CStructParser::Identifier);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructOrUnionContext ------------------------------------------------------------------

CStructParser::StructOrUnionContext::StructOrUnionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::StructOrUnionContext::Struct() {
  return getToken(CStructParser::Struct, 0);
}

tree::TerminalNode* CStructParser::StructOrUnionContext::Union() {
  return getToken(CStructParser::Union, 0);
}


size_t CStructParser::StructOrUnionContext::getRuleIndex() const {
  return CStructParser::RuleStructOrUnion;
}


std::any CStructParser::StructOrUnionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitStructOrUnion(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::StructOrUnionContext* CStructParser::structOrUnion() {
  StructOrUnionContext *_localctx = _tracker.createInstance<StructOrUnionContext>(_ctx, getState());
  enterRule(_localctx, 38, CStructParser::RuleStructOrUnion);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(264);
    _la = _input->LA(1);
    if (!(_la == CStructParser::Struct

    || _la == CStructParser::Union)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AlignAsAttrContext ------------------------------------------------------------------

CStructParser::AlignAsAttrContext::AlignAsAttrContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::AlignAsAttrContext::AlignAs() {
  return getToken(CStructParser::AlignAs, 0);
}

tree::TerminalNode* CStructParser::AlignAsAttrContext::LeftParen() {
  return getToken(CStructParser::LeftParen, 0);
}

tree::TerminalNode* CStructParser::AlignAsAttrContext::IntegerConstant() {
  return getToken(CStructParser::IntegerConstant, 0);
}

tree::TerminalNode* CStructParser::AlignAsAttrContext::RightParen() {
  return getToken(CStructParser::RightParen, 0);
}


size_t CStructParser::AlignAsAttrContext::getRuleIndex() const {
  return CStructParser::RuleAlignAsAttr;
}


std::any CStructParser::AlignAsAttrContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitAlignAsAttr(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::AlignAsAttrContext* CStructParser::alignAsAttr() {
  AlignAsAttrContext *_localctx = _tracker.createInstance<AlignAsAttrContext>(_ctx, getState());
  enterRule(_localctx, 40, CStructParser::RuleAlignAsAttr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(266);
    match(CStructParser::AlignAs);
    setState(267);
    match(CStructParser::LeftParen);
    setState(268);
    match(CStructParser::IntegerConstant);
    setState(269);
    match(CStructParser::RightParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructDeclarationListContext ------------------------------------------------------------------

CStructParser::StructDeclarationListContext::StructDeclarationListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::StructDeclarationContext *> CStructParser::StructDeclarationListContext::structDeclaration() {
  return getRuleContexts<CStructParser::StructDeclarationContext>();
}

CStructParser::StructDeclarationContext* CStructParser::StructDeclarationListContext::structDeclaration(size_t i) {
  return getRuleContext<CStructParser::StructDeclarationContext>(i);
}


size_t CStructParser::StructDeclarationListContext::getRuleIndex() const {
  return CStructParser::RuleStructDeclarationList;
}


std::any CStructParser::StructDeclarationListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitStructDeclarationList(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::StructDeclarationListContext* CStructParser::structDeclarationList() {
  StructDeclarationListContext *_localctx = _tracker.createInstance<StructDeclarationListContext>(_ctx, getState());
  enterRule(_localctx, 42, CStructParser::RuleStructDeclarationList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(272); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(271);
      structDeclaration();
      setState(274); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2199023514108) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructDeclarationContext ------------------------------------------------------------------

CStructParser::StructDeclarationContext::StructDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::SpecifierQualifierListContext* CStructParser::StructDeclarationContext::specifierQualifierList() {
  return getRuleContext<CStructParser::SpecifierQualifierListContext>(0);
}

CStructParser::StructDeclaratorListContext* CStructParser::StructDeclarationContext::structDeclaratorList() {
  return getRuleContext<CStructParser::StructDeclaratorListContext>(0);
}

tree::TerminalNode* CStructParser::StructDeclarationContext::Semi() {
  return getToken(CStructParser::Semi, 0);
}


size_t CStructParser::StructDeclarationContext::getRuleIndex() const {
  return CStructParser::RuleStructDeclaration;
}


std::any CStructParser::StructDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitStructDeclaration(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::StructDeclarationContext* CStructParser::structDeclaration() {
  StructDeclarationContext *_localctx = _tracker.createInstance<StructDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 44, CStructParser::RuleStructDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(283);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(276);
      specifierQualifierList();
      setState(277);
      structDeclaratorList();
      setState(278);
      match(CStructParser::Semi);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(280);
      specifierQualifierList();
      setState(281);
      match(CStructParser::Semi);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SpecifierQualifierListContext ------------------------------------------------------------------

CStructParser::SpecifierQualifierListContext::SpecifierQualifierListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::TypeSpecifierContext* CStructParser::SpecifierQualifierListContext::typeSpecifier() {
  return getRuleContext<CStructParser::TypeSpecifierContext>(0);
}

std::vector<tree::TerminalNode *> CStructParser::SpecifierQualifierListContext::Const() {
  return getTokens(CStructParser::Const);
}

tree::TerminalNode* CStructParser::SpecifierQualifierListContext::Const(size_t i) {
  return getToken(CStructParser::Const, i);
}


size_t CStructParser::SpecifierQualifierListContext::getRuleIndex() const {
  return CStructParser::RuleSpecifierQualifierList;
}


std::any CStructParser::SpecifierQualifierListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitSpecifierQualifierList(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::SpecifierQualifierListContext* CStructParser::specifierQualifierList() {
  SpecifierQualifierListContext *_localctx = _tracker.createInstance<SpecifierQualifierListContext>(_ctx, getState());
  enterRule(_localctx, 46, CStructParser::RuleSpecifierQualifierList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(286);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
    case 1: {
      setState(285);
      match(CStructParser::Const);
      break;
    }

    default:
      break;
    }
    setState(288);
    typeSpecifier();
    setState(290);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CStructParser::Const) {
      setState(289);
      match(CStructParser::Const);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructDeclaratorListContext ------------------------------------------------------------------

CStructParser::StructDeclaratorListContext::StructDeclaratorListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::StructDeclaratorContext *> CStructParser::StructDeclaratorListContext::structDeclarator() {
  return getRuleContexts<CStructParser::StructDeclaratorContext>();
}

CStructParser::StructDeclaratorContext* CStructParser::StructDeclaratorListContext::structDeclarator(size_t i) {
  return getRuleContext<CStructParser::StructDeclaratorContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::StructDeclaratorListContext::Comma() {
  return getTokens(CStructParser::Comma);
}

tree::TerminalNode* CStructParser::StructDeclaratorListContext::Comma(size_t i) {
  return getToken(CStructParser::Comma, i);
}


size_t CStructParser::StructDeclaratorListContext::getRuleIndex() const {
  return CStructParser::RuleStructDeclaratorList;
}


std::any CStructParser::StructDeclaratorListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitStructDeclaratorList(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::StructDeclaratorListContext* CStructParser::structDeclaratorList() {
  StructDeclaratorListContext *_localctx = _tracker.createInstance<StructDeclaratorListContext>(_ctx, getState());
  enterRule(_localctx, 48, CStructParser::RuleStructDeclaratorList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(292);
    structDeclarator();
    setState(297);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::Comma) {
      setState(293);
      match(CStructParser::Comma);
      setState(294);
      structDeclarator();
      setState(299);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructDeclaratorContext ------------------------------------------------------------------

CStructParser::StructDeclaratorContext::StructDeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::DeclaratorContext* CStructParser::StructDeclaratorContext::declarator() {
  return getRuleContext<CStructParser::DeclaratorContext>(0);
}

tree::TerminalNode* CStructParser::StructDeclaratorContext::Colon() {
  return getToken(CStructParser::Colon, 0);
}

CStructParser::AssignmentExpressionContext* CStructParser::StructDeclaratorContext::assignmentExpression() {
  return getRuleContext<CStructParser::AssignmentExpressionContext>(0);
}


size_t CStructParser::StructDeclaratorContext::getRuleIndex() const {
  return CStructParser::RuleStructDeclarator;
}


std::any CStructParser::StructDeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitStructDeclarator(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::StructDeclaratorContext* CStructParser::structDeclarator() {
  StructDeclaratorContext *_localctx = _tracker.createInstance<StructDeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 50, CStructParser::RuleStructDeclarator);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(306);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(300);
      declarator();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(302);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 2201171525632) != 0)) {
        setState(301);
        declarator();
      }
      setState(304);
      match(CStructParser::Colon);
      setState(305);
      assignmentExpression();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumSpecifierContext ------------------------------------------------------------------

CStructParser::EnumSpecifierContext::EnumSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::EnumSpecifierContext::Enum() {
  return getToken(CStructParser::Enum, 0);
}

tree::TerminalNode* CStructParser::EnumSpecifierContext::LeftBrace() {
  return getToken(CStructParser::LeftBrace, 0);
}

CStructParser::EnumeratorListContext* CStructParser::EnumSpecifierContext::enumeratorList() {
  return getRuleContext<CStructParser::EnumeratorListContext>(0);
}

tree::TerminalNode* CStructParser::EnumSpecifierContext::RightBrace() {
  return getToken(CStructParser::RightBrace, 0);
}

tree::TerminalNode* CStructParser::EnumSpecifierContext::Identifier() {
  return getToken(CStructParser::Identifier, 0);
}

tree::TerminalNode* CStructParser::EnumSpecifierContext::Comma() {
  return getToken(CStructParser::Comma, 0);
}


size_t CStructParser::EnumSpecifierContext::getRuleIndex() const {
  return CStructParser::RuleEnumSpecifier;
}


std::any CStructParser::EnumSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitEnumSpecifier(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::EnumSpecifierContext* CStructParser::enumSpecifier() {
  EnumSpecifierContext *_localctx = _tracker.createInstance<EnumSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 52, CStructParser::RuleEnumSpecifier);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(321);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(308);
      match(CStructParser::Enum);
      setState(310);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Identifier) {
        setState(309);
        match(CStructParser::Identifier);
      }
      setState(312);
      match(CStructParser::LeftBrace);
      setState(313);
      enumeratorList();
      setState(315);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Comma) {
        setState(314);
        match(CStructParser::Comma);
      }
      setState(317);
      match(CStructParser::RightBrace);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(319);
      match(CStructParser::Enum);
      setState(320);
      match(CStructParser::Identifier);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumeratorListContext ------------------------------------------------------------------

CStructParser::EnumeratorListContext::EnumeratorListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::EnumeratorContext *> CStructParser::EnumeratorListContext::enumerator() {
  return getRuleContexts<CStructParser::EnumeratorContext>();
}

CStructParser::EnumeratorContext* CStructParser::EnumeratorListContext::enumerator(size_t i) {
  return getRuleContext<CStructParser::EnumeratorContext>(i);
}

std::vector<tree::TerminalNode *> CStructParser::EnumeratorListContext::Comma() {
  return getTokens(CStructParser::Comma);
}

tree::TerminalNode* CStructParser::EnumeratorListContext::Comma(size_t i) {
  return getToken(CStructParser::Comma, i);
}


size_t CStructParser::EnumeratorListContext::getRuleIndex() const {
  return CStructParser::RuleEnumeratorList;
}


std::any CStructParser::EnumeratorListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitEnumeratorList(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::EnumeratorListContext* CStructParser::enumeratorList() {
  EnumeratorListContext *_localctx = _tracker.createInstance<EnumeratorListContext>(_ctx, getState());
  enterRule(_localctx, 54, CStructParser::RuleEnumeratorList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(323);
    enumerator();
    setState(328);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(324);
        match(CStructParser::Comma);
        setState(325);
        enumerator(); 
      }
      setState(330);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumeratorContext ------------------------------------------------------------------

CStructParser::EnumeratorContext::EnumeratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::EnumerationConstantContext* CStructParser::EnumeratorContext::enumerationConstant() {
  return getRuleContext<CStructParser::EnumerationConstantContext>(0);
}

CStructParser::AssignmentExpressionContext* CStructParser::EnumeratorContext::assignmentExpression() {
  return getRuleContext<CStructParser::AssignmentExpressionContext>(0);
}


size_t CStructParser::EnumeratorContext::getRuleIndex() const {
  return CStructParser::RuleEnumerator;
}


std::any CStructParser::EnumeratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitEnumerator(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::EnumeratorContext* CStructParser::enumerator() {
  EnumeratorContext *_localctx = _tracker.createInstance<EnumeratorContext>(_ctx, getState());
  enterRule(_localctx, 56, CStructParser::RuleEnumerator);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(331);
    enumerationConstant();
    setState(334);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CStructParser::T__0) {
      setState(332);
      match(CStructParser::T__0);
      setState(333);
      assignmentExpression();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumerationConstantContext ------------------------------------------------------------------

CStructParser::EnumerationConstantContext::EnumerationConstantContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::EnumerationConstantContext::Identifier() {
  return getToken(CStructParser::Identifier, 0);
}


size_t CStructParser::EnumerationConstantContext::getRuleIndex() const {
  return CStructParser::RuleEnumerationConstant;
}


std::any CStructParser::EnumerationConstantContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitEnumerationConstant(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::EnumerationConstantContext* CStructParser::enumerationConstant() {
  EnumerationConstantContext *_localctx = _tracker.createInstance<EnumerationConstantContext>(_ctx, getState());
  enterRule(_localctx, 58, CStructParser::RuleEnumerationConstant);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(336);
    match(CStructParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclaratorContext ------------------------------------------------------------------

CStructParser::DeclaratorContext::DeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::DirectDeclaratorContext* CStructParser::DeclaratorContext::directDeclarator() {
  return getRuleContext<CStructParser::DirectDeclaratorContext>(0);
}

CStructParser::PointerContext* CStructParser::DeclaratorContext::pointer() {
  return getRuleContext<CStructParser::PointerContext>(0);
}


size_t CStructParser::DeclaratorContext::getRuleIndex() const {
  return CStructParser::RuleDeclarator;
}


std::any CStructParser::DeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitDeclarator(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::DeclaratorContext* CStructParser::declarator() {
  DeclaratorContext *_localctx = _tracker.createInstance<DeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 60, CStructParser::RuleDeclarator);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(339);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CStructParser::Star) {
      setState(338);
      pointer();
    }
    setState(341);
    directDeclarator(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DirectDeclaratorContext ------------------------------------------------------------------

CStructParser::DirectDeclaratorContext::DirectDeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::DirectDeclaratorContext::Identifier() {
  return getToken(CStructParser::Identifier, 0);
}

tree::TerminalNode* CStructParser::DirectDeclaratorContext::QuestionMark() {
  return getToken(CStructParser::QuestionMark, 0);
}

tree::TerminalNode* CStructParser::DirectDeclaratorContext::LeftParen() {
  return getToken(CStructParser::LeftParen, 0);
}

CStructParser::DeclaratorContext* CStructParser::DirectDeclaratorContext::declarator() {
  return getRuleContext<CStructParser::DeclaratorContext>(0);
}

tree::TerminalNode* CStructParser::DirectDeclaratorContext::RightParen() {
  return getToken(CStructParser::RightParen, 0);
}

CStructParser::DirectDeclaratorContext* CStructParser::DirectDeclaratorContext::directDeclarator() {
  return getRuleContext<CStructParser::DirectDeclaratorContext>(0);
}

tree::TerminalNode* CStructParser::DirectDeclaratorContext::LeftBracket() {
  return getToken(CStructParser::LeftBracket, 0);
}

CStructParser::AssignmentExpressionContext* CStructParser::DirectDeclaratorContext::assignmentExpression() {
  return getRuleContext<CStructParser::AssignmentExpressionContext>(0);
}

tree::TerminalNode* CStructParser::DirectDeclaratorContext::RightBracket() {
  return getToken(CStructParser::RightBracket, 0);
}


size_t CStructParser::DirectDeclaratorContext::getRuleIndex() const {
  return CStructParser::RuleDirectDeclarator;
}


std::any CStructParser::DirectDeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitDirectDeclarator(this);
  else
    return visitor->visitChildren(this);
}


CStructParser::DirectDeclaratorContext* CStructParser::directDeclarator() {
   return directDeclarator(0);
}

CStructParser::DirectDeclaratorContext* CStructParser::directDeclarator(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  CStructParser::DirectDeclaratorContext *_localctx = _tracker.createInstance<DirectDeclaratorContext>(_ctx, parentState);
  CStructParser::DirectDeclaratorContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 62;
  enterRecursionRule(_localctx, 62, CStructParser::RuleDirectDeclarator, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(350);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::Identifier: {
        setState(344);
        match(CStructParser::Identifier);
        break;
      }

      case CStructParser::QuestionMark: {
        setState(345);
        match(CStructParser::QuestionMark);
        break;
      }

      case CStructParser::LeftParen: {
        setState(346);
        match(CStructParser::LeftParen);
        setState(347);
        declarator();
        setState(348);
        match(CStructParser::RightParen);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(359);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<DirectDeclaratorContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleDirectDeclarator);
        setState(352);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(353);
        match(CStructParser::LeftBracket);
        setState(354);
        assignmentExpression();
        setState(355);
        match(CStructParser::RightBracket); 
      }
      setState(361);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 44, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- PointerContext ------------------------------------------------------------------

CStructParser::PointerContext::PointerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> CStructParser::PointerContext::Star() {
  return getTokens(CStructParser::Star);
}

tree::TerminalNode* CStructParser::PointerContext::Star(size_t i) {
  return getToken(CStructParser::Star, i);
}

std::vector<tree::TerminalNode *> CStructParser::PointerContext::Const() {
  return getTokens(CStructParser::Const);
}

tree::TerminalNode* CStructParser::PointerContext::Const(size_t i) {
  return getToken(CStructParser::Const, i);
}


size_t CStructParser::PointerContext::getRuleIndex() const {
  return CStructParser::RulePointer;
}


std::any CStructParser::PointerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitPointer(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::PointerContext* CStructParser::pointer() {
  PointerContext *_localctx = _tracker.createInstance<PointerContext>(_ctx, getState());
  enterRule(_localctx, 64, CStructParser::RulePointer);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(366); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(362);
      match(CStructParser::Star);
      setState(364);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Const) {
        setState(363);
        match(CStructParser::Const);
      }
      setState(368); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == CStructParser::Star);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IdentifierListContext ------------------------------------------------------------------

CStructParser::IdentifierListContext::IdentifierListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> CStructParser::IdentifierListContext::Identifier() {
  return getTokens(CStructParser::Identifier);
}

tree::TerminalNode* CStructParser::IdentifierListContext::Identifier(size_t i) {
  return getToken(CStructParser::Identifier, i);
}

std::vector<tree::TerminalNode *> CStructParser::IdentifierListContext::Comma() {
  return getTokens(CStructParser::Comma);
}

tree::TerminalNode* CStructParser::IdentifierListContext::Comma(size_t i) {
  return getToken(CStructParser::Comma, i);
}


size_t CStructParser::IdentifierListContext::getRuleIndex() const {
  return CStructParser::RuleIdentifierList;
}


std::any CStructParser::IdentifierListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitIdentifierList(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::IdentifierListContext* CStructParser::identifierList() {
  IdentifierListContext *_localctx = _tracker.createInstance<IdentifierListContext>(_ctx, getState());
  enterRule(_localctx, 66, CStructParser::RuleIdentifierList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(370);
    match(CStructParser::Identifier);
    setState(375);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CStructParser::Comma) {
      setState(371);
      match(CStructParser::Comma);
      setState(372);
      match(CStructParser::Identifier);
      setState(377);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeNameContext ------------------------------------------------------------------

CStructParser::TypeNameContext::TypeNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::SpecifierQualifierListContext* CStructParser::TypeNameContext::specifierQualifierList() {
  return getRuleContext<CStructParser::SpecifierQualifierListContext>(0);
}

CStructParser::AbstractDeclaratorContext* CStructParser::TypeNameContext::abstractDeclarator() {
  return getRuleContext<CStructParser::AbstractDeclaratorContext>(0);
}


size_t CStructParser::TypeNameContext::getRuleIndex() const {
  return CStructParser::RuleTypeName;
}


std::any CStructParser::TypeNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitTypeName(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::TypeNameContext* CStructParser::typeName() {
  TypeNameContext *_localctx = _tracker.createInstance<TypeNameContext>(_ctx, getState());
  enterRule(_localctx, 68, CStructParser::RuleTypeName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(378);
    specifierQualifierList();
    setState(380);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2150105088) != 0)) {
      setState(379);
      abstractDeclarator();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AbstractDeclaratorContext ------------------------------------------------------------------

CStructParser::AbstractDeclaratorContext::AbstractDeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::PointerContext* CStructParser::AbstractDeclaratorContext::pointer() {
  return getRuleContext<CStructParser::PointerContext>(0);
}

CStructParser::DirectAbstractDeclaratorContext* CStructParser::AbstractDeclaratorContext::directAbstractDeclarator() {
  return getRuleContext<CStructParser::DirectAbstractDeclaratorContext>(0);
}


size_t CStructParser::AbstractDeclaratorContext::getRuleIndex() const {
  return CStructParser::RuleAbstractDeclarator;
}


std::any CStructParser::AbstractDeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitAbstractDeclarator(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::AbstractDeclaratorContext* CStructParser::abstractDeclarator() {
  AbstractDeclaratorContext *_localctx = _tracker.createInstance<AbstractDeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 70, CStructParser::RuleAbstractDeclarator);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(387);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(382);
      pointer();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(384);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == CStructParser::Star) {
        setState(383);
        pointer();
      }
      setState(386);
      directAbstractDeclarator(0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DirectAbstractDeclaratorContext ------------------------------------------------------------------

CStructParser::DirectAbstractDeclaratorContext::DirectAbstractDeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::DirectAbstractDeclaratorContext::LeftParen() {
  return getToken(CStructParser::LeftParen, 0);
}

CStructParser::AbstractDeclaratorContext* CStructParser::DirectAbstractDeclaratorContext::abstractDeclarator() {
  return getRuleContext<CStructParser::AbstractDeclaratorContext>(0);
}

tree::TerminalNode* CStructParser::DirectAbstractDeclaratorContext::RightParen() {
  return getToken(CStructParser::RightParen, 0);
}

tree::TerminalNode* CStructParser::DirectAbstractDeclaratorContext::LeftBracket() {
  return getToken(CStructParser::LeftBracket, 0);
}

CStructParser::AssignmentExpressionContext* CStructParser::DirectAbstractDeclaratorContext::assignmentExpression() {
  return getRuleContext<CStructParser::AssignmentExpressionContext>(0);
}

tree::TerminalNode* CStructParser::DirectAbstractDeclaratorContext::RightBracket() {
  return getToken(CStructParser::RightBracket, 0);
}

CStructParser::DirectAbstractDeclaratorContext* CStructParser::DirectAbstractDeclaratorContext::directAbstractDeclarator() {
  return getRuleContext<CStructParser::DirectAbstractDeclaratorContext>(0);
}


size_t CStructParser::DirectAbstractDeclaratorContext::getRuleIndex() const {
  return CStructParser::RuleDirectAbstractDeclarator;
}


std::any CStructParser::DirectAbstractDeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitDirectAbstractDeclarator(this);
  else
    return visitor->visitChildren(this);
}


CStructParser::DirectAbstractDeclaratorContext* CStructParser::directAbstractDeclarator() {
   return directAbstractDeclarator(0);
}

CStructParser::DirectAbstractDeclaratorContext* CStructParser::directAbstractDeclarator(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  CStructParser::DirectAbstractDeclaratorContext *_localctx = _tracker.createInstance<DirectAbstractDeclaratorContext>(_ctx, parentState);
  CStructParser::DirectAbstractDeclaratorContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 72;
  enterRecursionRule(_localctx, 72, CStructParser::RuleDirectAbstractDeclarator, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(398);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::LeftParen: {
        setState(390);
        match(CStructParser::LeftParen);
        setState(391);
        abstractDeclarator();
        setState(392);
        match(CStructParser::RightParen);
        break;
      }

      case CStructParser::LeftBracket: {
        setState(394);
        match(CStructParser::LeftBracket);
        setState(395);
        assignmentExpression();
        setState(396);
        match(CStructParser::RightBracket);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(407);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<DirectAbstractDeclaratorContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleDirectAbstractDeclarator);
        setState(400);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(401);
        match(CStructParser::LeftBracket);
        setState(402);
        assignmentExpression();
        setState(403);
        match(CStructParser::RightBracket); 
      }
      setState(409);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- CompilationUnitContext ------------------------------------------------------------------

CStructParser::CompilationUnitContext::CompilationUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::CompilationUnitContext::EOF() {
  return getToken(CStructParser::EOF, 0);
}

CStructParser::TranslationUnitContext* CStructParser::CompilationUnitContext::translationUnit() {
  return getRuleContext<CStructParser::TranslationUnitContext>(0);
}


size_t CStructParser::CompilationUnitContext::getRuleIndex() const {
  return CStructParser::RuleCompilationUnit;
}


std::any CStructParser::CompilationUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitCompilationUnit(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::CompilationUnitContext* CStructParser::compilationUnit() {
  CompilationUnitContext *_localctx = _tracker.createInstance<CompilationUnitContext>(_ctx, getState());
  enterRule(_localctx, 74, CStructParser::RuleCompilationUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(411);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 229248174440992) != 0)) {
      setState(410);
      translationUnit();
    }
    setState(413);
    match(CStructParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TranslationUnitContext ------------------------------------------------------------------

CStructParser::TranslationUnitContext::TranslationUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CStructParser::ExternalDeclarationContext *> CStructParser::TranslationUnitContext::externalDeclaration() {
  return getRuleContexts<CStructParser::ExternalDeclarationContext>();
}

CStructParser::ExternalDeclarationContext* CStructParser::TranslationUnitContext::externalDeclaration(size_t i) {
  return getRuleContext<CStructParser::ExternalDeclarationContext>(i);
}


size_t CStructParser::TranslationUnitContext::getRuleIndex() const {
  return CStructParser::RuleTranslationUnit;
}


std::any CStructParser::TranslationUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitTranslationUnit(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::TranslationUnitContext* CStructParser::translationUnit() {
  TranslationUnitContext *_localctx = _tracker.createInstance<TranslationUnitContext>(_ctx, getState());
  enterRule(_localctx, 76, CStructParser::RuleTranslationUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(416); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(415);
      externalDeclaration();
      setState(418); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 229248174440992) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExternalDeclarationContext ------------------------------------------------------------------

CStructParser::ExternalDeclarationContext::ExternalDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CStructParser::DeclarationContext* CStructParser::ExternalDeclarationContext::declaration() {
  return getRuleContext<CStructParser::DeclarationContext>(0);
}

CStructParser::DefineDeclContext* CStructParser::ExternalDeclarationContext::defineDecl() {
  return getRuleContext<CStructParser::DefineDeclContext>(0);
}

tree::TerminalNode* CStructParser::ExternalDeclarationContext::Semi() {
  return getToken(CStructParser::Semi, 0);
}


size_t CStructParser::ExternalDeclarationContext::getRuleIndex() const {
  return CStructParser::RuleExternalDeclaration;
}


std::any CStructParser::ExternalDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitExternalDeclaration(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::ExternalDeclarationContext* CStructParser::externalDeclaration() {
  ExternalDeclarationContext *_localctx = _tracker.createInstance<ExternalDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 78, CStructParser::RuleExternalDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(423);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CStructParser::Enum:
      case CStructParser::TypeDef:
      case CStructParser::Struct:
      case CStructParser::Union: {
        enterOuterAlt(_localctx, 1);
        setState(420);
        declaration();
        break;
      }

      case CStructParser::MultiLineMacroDefine:
      case CStructParser::DirectiveInclude:
      case CStructParser::DirectiveDefine: {
        enterOuterAlt(_localctx, 2);
        setState(421);
        defineDecl();
        break;
      }

      case CStructParser::Semi: {
        enterOuterAlt(_localctx, 3);
        setState(422);
        match(CStructParser::Semi);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DefineDeclContext ------------------------------------------------------------------

CStructParser::DefineDeclContext::DefineDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CStructParser::DefineDeclContext::DirectiveDefine() {
  return getToken(CStructParser::DirectiveDefine, 0);
}

tree::TerminalNode* CStructParser::DefineDeclContext::MultiLineMacroDefine() {
  return getToken(CStructParser::MultiLineMacroDefine, 0);
}

tree::TerminalNode* CStructParser::DefineDeclContext::DirectiveInclude() {
  return getToken(CStructParser::DirectiveInclude, 0);
}


size_t CStructParser::DefineDeclContext::getRuleIndex() const {
  return CStructParser::RuleDefineDecl;
}


std::any CStructParser::DefineDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CStructVisitor*>(visitor))
    return parserVisitor->visitDefineDecl(this);
  else
    return visitor->visitChildren(this);
}

CStructParser::DefineDeclContext* CStructParser::defineDecl() {
  DefineDeclContext *_localctx = _tracker.createInstance<DefineDeclContext>(_ctx, getState());
  enterRule(_localctx, 80, CStructParser::RuleDefineDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(425);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 228698418577408) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool CStructParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 31: return directDeclaratorSempred(antlrcpp::downCast<DirectDeclaratorContext *>(context), predicateIndex);
    case 36: return directAbstractDeclaratorSempred(antlrcpp::downCast<DirectAbstractDeclaratorContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool CStructParser::directDeclaratorSempred(DirectDeclaratorContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool CStructParser::directAbstractDeclaratorSempred(DirectAbstractDeclaratorContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 1: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void CStructParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  cstructParserInitialize();
#else
  ::antlr4::internal::call_once(cstructParserOnceFlag, cstructParserInitialize);
#endif
}
