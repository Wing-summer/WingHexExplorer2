
// Generated from AngelscriptConsoleParser.g4 by ANTLR 4.13.2


#include "AngelscriptConsoleParserVisitor.h"

#include "AngelscriptConsoleParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct AngelscriptConsoleParserStaticData final {
  AngelscriptConsoleParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  AngelscriptConsoleParserStaticData(const AngelscriptConsoleParserStaticData&) = delete;
  AngelscriptConsoleParserStaticData(AngelscriptConsoleParserStaticData&&) = delete;
  AngelscriptConsoleParserStaticData& operator=(const AngelscriptConsoleParserStaticData&) = delete;
  AngelscriptConsoleParserStaticData& operator=(AngelscriptConsoleParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag angelscriptconsoleparserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<AngelscriptConsoleParserStaticData> angelscriptconsoleparserParserStaticData = nullptr;

void angelscriptconsoleparserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (angelscriptconsoleparserParserStaticData != nullptr) {
    return;
  }
#else
  assert(angelscriptconsoleparserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<AngelscriptConsoleParserStaticData>(
    std::vector<std::string>{
      "script", "emptyDeclaration_", "scriptItem", "declSpecifierSeq", "declSpecifier", 
      "typeSpecifier", "arraySpecifier", "trailingTypeSpecifier", "simpleTypeSpecifier", 
      "asGeneric", "simpleTypeSpecifierList", "constantExpression", "initDeclaratorList", 
      "initDeclarator", "expressionList", "initializer", "primaryExpression", 
      "idExpression", "unqualifiedId", "qualifiedId", "nestedNameSpecifier", 
      "typedefName", "theTypeName", "decltypeSpecifier", "namespaceName", 
      "qualifiednamespacespecifier", "pseudoDestructorName", "typeSpecifierSeq", 
      "theTypeId", "postfixExpression", "unaryExpression", "unaryOperator", 
      "castExpression", "multiplicativeExpression", "additiveExpression", 
      "shiftExpression", "shiftOperator", "relationalExpression", "equalityExpression", 
      "andExpression", "exclusiveOrExpression", "inclusiveOrExpression", 
      "logicalAndExpression", "logicalOrExpression", "conditionalExpression", 
      "expression", "assignmentExpression", "assignmentOperator", "braceOrEqualInitializer", 
      "initializerClause", "initializerList", "bracedInitList", "literal", 
      "booleanLiteral"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "'cast'", "'auto'", "'const'", "'false'", 
      "'null'", "'true'", "'('", "')'", "'['", "']'", "'{'", "'}'", "'+'", 
      "'-'", "'*'", "'/'", "'%'", "'^'", "'&'", "'|'", "'~'", "'!'", "'='", 
      "'<'", "'>'", "'+='", "'-='", "'*='", "'/='", "'%='", "'^='", "'&='", 
      "'|='", "'<<='", "'>>='", "'=='", "'!='", "'<='", "'>='", "'&&'", 
      "'||'", "'++'", "'--'", "','", "'\\u003F'", "':'", "'::'", "';'", 
      "'.'"
    },
    std::vector<std::string>{
      "", "IntegerLiteral", "CharacterLiteral", "FloatingLiteral", "StringLiteral", 
      "UserDefinedLiteral", "Cast", "Auto", "Const", "False_", "Nullptr", 
      "True_", "LeftParen", "RightParen", "LeftBracket", "RightBracket", 
      "LeftBrace", "RightBrace", "Plus", "Minus", "Star", "Div", "Mod", 
      "Xor", "And", "Or", "Tilde", "Not", "Assign", "Less", "Greater", "PlusAssign", 
      "MinusAssign", "StarAssign", "DivAssign", "ModAssign", "XorAssign", 
      "AndAssign", "OrAssign", "LeftShiftAssign", "RightShiftAssign", "Equal", 
      "NotEqual", "LessEqual", "GreaterEqual", "AndAnd", "OrOr", "PlusPlus", 
      "MinusMinus", "Comma", "Question", "Colon", "Doublecolon", "Semi", 
      "Dot", "Identifier", "DecimalLiteral", "OctalLiteral", "HexadecimalLiteral", 
      "BinaryLiteral", "Integersuffix", "UserDefinedIntegerLiteral", "UserDefinedFloatingLiteral", 
      "UserDefinedStringLiteral", "UserDefinedCharacterLiteral", "Whitespace", 
      "Newline", "BlockComment", "LineComment"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,68,511,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,1,0,5,0,110,8,0,10,0,12,0,
  	113,9,0,1,0,1,0,1,1,1,1,1,2,1,2,1,2,1,2,1,2,3,2,124,8,2,1,3,4,3,127,8,
  	3,11,3,12,3,128,1,4,1,4,1,5,1,5,1,6,1,6,3,6,137,8,6,1,6,1,6,1,7,1,7,5,
  	7,143,8,7,10,7,12,7,146,9,7,1,7,3,7,149,8,7,1,8,1,8,3,8,153,8,8,1,9,1,
  	9,1,9,1,9,1,9,1,10,1,10,1,10,5,10,163,8,10,10,10,12,10,166,9,10,1,11,
  	1,11,1,12,1,12,1,12,5,12,173,8,12,10,12,12,12,176,9,12,1,13,1,13,3,13,
  	180,8,13,1,14,1,14,1,15,1,15,1,15,1,15,1,15,3,15,189,8,15,1,16,4,16,192,
  	8,16,11,16,12,16,193,1,16,1,16,1,16,1,16,1,16,3,16,201,8,16,1,17,1,17,
  	3,17,205,8,17,1,18,1,18,1,19,1,19,1,19,1,20,1,20,1,20,1,20,3,20,216,8,
  	20,1,20,1,20,1,20,1,20,1,20,5,20,223,8,20,10,20,12,20,226,9,20,1,21,1,
  	21,1,22,1,22,1,23,1,23,1,23,3,23,235,8,23,1,23,1,23,1,24,1,24,1,25,3,
  	25,242,8,25,1,25,1,25,1,26,3,26,247,8,26,1,26,1,26,1,26,3,26,252,8,26,
  	1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,3,26,263,8,26,1,27,4,27,
  	266,8,27,11,27,12,27,267,1,28,1,28,1,29,1,29,1,29,1,29,5,29,276,8,29,
  	10,29,12,29,279,9,29,1,29,1,29,3,29,283,8,29,1,29,1,29,3,29,287,8,29,
  	1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,29,1,29,3,29,300,8,29,
  	1,29,1,29,3,29,304,8,29,1,29,1,29,1,29,1,29,3,29,310,8,29,1,29,1,29,1,
  	29,1,29,1,29,3,29,317,8,29,1,29,1,29,1,29,1,29,1,29,3,29,324,8,29,1,29,
  	1,29,5,29,328,8,29,10,29,12,29,331,9,29,1,30,1,30,1,30,1,30,3,30,337,
  	8,30,1,30,1,30,1,30,1,30,1,30,3,30,344,8,30,1,31,1,31,1,32,1,32,1,32,
  	1,32,1,32,1,32,1,32,1,32,1,32,3,32,357,8,32,1,33,1,33,1,33,5,33,362,8,
  	33,10,33,12,33,365,9,33,1,34,1,34,1,34,5,34,370,8,34,10,34,12,34,373,
  	9,34,1,35,1,35,1,35,1,35,5,35,379,8,35,10,35,12,35,382,9,35,1,36,1,36,
  	1,36,1,36,3,36,388,8,36,1,37,1,37,1,37,5,37,393,8,37,10,37,12,37,396,
  	9,37,1,38,1,38,1,38,5,38,401,8,38,10,38,12,38,404,9,38,1,39,1,39,1,39,
  	5,39,409,8,39,10,39,12,39,412,9,39,1,40,1,40,1,40,5,40,417,8,40,10,40,
  	12,40,420,9,40,1,41,1,41,1,41,5,41,425,8,41,10,41,12,41,428,9,41,1,42,
  	1,42,1,42,5,42,433,8,42,10,42,12,42,436,9,42,1,43,1,43,1,43,5,43,441,
  	8,43,10,43,12,43,444,9,43,1,44,1,44,1,44,1,44,1,44,1,44,3,44,452,8,44,
  	1,45,1,45,1,45,5,45,457,8,45,10,45,12,45,460,9,45,1,46,1,46,1,46,1,46,
  	1,46,3,46,467,8,46,1,47,1,47,1,48,1,48,1,48,3,48,474,8,48,1,49,1,49,3,
  	49,478,8,49,1,50,1,50,1,50,5,50,483,8,50,10,50,12,50,486,9,50,1,50,3,
  	50,489,8,50,1,51,1,51,1,51,3,51,494,8,51,3,51,496,8,51,1,51,1,51,1,52,
  	1,52,1,52,1,52,1,52,1,52,1,52,3,52,507,8,52,1,53,1,53,1,53,0,2,40,58,
  	54,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,
  	48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,
  	94,96,98,100,102,104,106,0,10,1,0,47,48,2,0,18,20,24,27,1,0,20,22,1,0,
  	18,19,2,0,29,30,43,44,1,0,41,42,2,0,28,28,31,40,2,0,14,14,16,16,2,0,15,
  	15,17,17,2,0,9,9,11,11,527,0,111,1,0,0,0,2,116,1,0,0,0,4,123,1,0,0,0,
  	6,126,1,0,0,0,8,130,1,0,0,0,10,132,1,0,0,0,12,134,1,0,0,0,14,148,1,0,
  	0,0,16,152,1,0,0,0,18,154,1,0,0,0,20,159,1,0,0,0,22,167,1,0,0,0,24,169,
  	1,0,0,0,26,177,1,0,0,0,28,181,1,0,0,0,30,188,1,0,0,0,32,200,1,0,0,0,34,
  	204,1,0,0,0,36,206,1,0,0,0,38,208,1,0,0,0,40,211,1,0,0,0,42,227,1,0,0,
  	0,44,229,1,0,0,0,46,231,1,0,0,0,48,238,1,0,0,0,50,241,1,0,0,0,52,262,
  	1,0,0,0,54,265,1,0,0,0,56,269,1,0,0,0,58,303,1,0,0,0,60,343,1,0,0,0,62,
  	345,1,0,0,0,64,356,1,0,0,0,66,358,1,0,0,0,68,366,1,0,0,0,70,374,1,0,0,
  	0,72,387,1,0,0,0,74,389,1,0,0,0,76,397,1,0,0,0,78,405,1,0,0,0,80,413,
  	1,0,0,0,82,421,1,0,0,0,84,429,1,0,0,0,86,437,1,0,0,0,88,445,1,0,0,0,90,
  	453,1,0,0,0,92,466,1,0,0,0,94,468,1,0,0,0,96,473,1,0,0,0,98,477,1,0,0,
  	0,100,479,1,0,0,0,102,490,1,0,0,0,104,506,1,0,0,0,106,508,1,0,0,0,108,
  	110,3,4,2,0,109,108,1,0,0,0,110,113,1,0,0,0,111,109,1,0,0,0,111,112,1,
  	0,0,0,112,114,1,0,0,0,113,111,1,0,0,0,114,115,5,0,0,1,115,1,1,0,0,0,116,
  	117,5,53,0,0,117,3,1,0,0,0,118,119,3,6,3,0,119,120,3,24,12,0,120,121,
  	5,53,0,0,121,124,1,0,0,0,122,124,3,2,1,0,123,118,1,0,0,0,123,122,1,0,
  	0,0,124,5,1,0,0,0,125,127,3,8,4,0,126,125,1,0,0,0,127,128,1,0,0,0,128,
  	126,1,0,0,0,128,129,1,0,0,0,129,7,1,0,0,0,130,131,3,10,5,0,131,9,1,0,
  	0,0,132,133,3,14,7,0,133,11,1,0,0,0,134,136,5,14,0,0,135,137,3,22,11,
  	0,136,135,1,0,0,0,136,137,1,0,0,0,137,138,1,0,0,0,138,139,5,15,0,0,139,
  	13,1,0,0,0,140,144,3,16,8,0,141,143,3,12,6,0,142,141,1,0,0,0,143,146,
  	1,0,0,0,144,142,1,0,0,0,144,145,1,0,0,0,145,149,1,0,0,0,146,144,1,0,0,
  	0,147,149,5,8,0,0,148,140,1,0,0,0,148,147,1,0,0,0,149,15,1,0,0,0,150,
  	153,5,55,0,0,151,153,3,18,9,0,152,150,1,0,0,0,152,151,1,0,0,0,153,17,
  	1,0,0,0,154,155,5,55,0,0,155,156,5,29,0,0,156,157,3,20,10,0,157,158,5,
  	30,0,0,158,19,1,0,0,0,159,164,3,16,8,0,160,161,5,49,0,0,161,163,3,16,
  	8,0,162,160,1,0,0,0,163,166,1,0,0,0,164,162,1,0,0,0,164,165,1,0,0,0,165,
  	21,1,0,0,0,166,164,1,0,0,0,167,168,5,1,0,0,168,23,1,0,0,0,169,174,3,26,
  	13,0,170,171,5,49,0,0,171,173,3,26,13,0,172,170,1,0,0,0,173,176,1,0,0,
  	0,174,172,1,0,0,0,174,175,1,0,0,0,175,25,1,0,0,0,176,174,1,0,0,0,177,
  	179,5,55,0,0,178,180,3,30,15,0,179,178,1,0,0,0,179,180,1,0,0,0,180,27,
  	1,0,0,0,181,182,3,100,50,0,182,29,1,0,0,0,183,189,3,96,48,0,184,185,5,
  	12,0,0,185,186,3,28,14,0,186,187,5,13,0,0,187,189,1,0,0,0,188,183,1,0,
  	0,0,188,184,1,0,0,0,189,31,1,0,0,0,190,192,3,104,52,0,191,190,1,0,0,0,
  	192,193,1,0,0,0,193,191,1,0,0,0,193,194,1,0,0,0,194,201,1,0,0,0,195,196,
  	5,12,0,0,196,197,3,90,45,0,197,198,5,13,0,0,198,201,1,0,0,0,199,201,3,
  	34,17,0,200,191,1,0,0,0,200,195,1,0,0,0,200,199,1,0,0,0,201,33,1,0,0,
  	0,202,205,3,36,18,0,203,205,3,38,19,0,204,202,1,0,0,0,204,203,1,0,0,0,
  	205,35,1,0,0,0,206,207,5,55,0,0,207,37,1,0,0,0,208,209,3,40,20,0,209,
  	210,3,36,18,0,210,39,1,0,0,0,211,215,6,20,-1,0,212,216,3,44,22,0,213,
  	216,3,48,24,0,214,216,3,46,23,0,215,212,1,0,0,0,215,213,1,0,0,0,215,214,
  	1,0,0,0,215,216,1,0,0,0,216,217,1,0,0,0,217,218,5,52,0,0,218,224,1,0,
  	0,0,219,220,10,1,0,0,220,221,5,55,0,0,221,223,5,52,0,0,222,219,1,0,0,
  	0,223,226,1,0,0,0,224,222,1,0,0,0,224,225,1,0,0,0,225,41,1,0,0,0,226,
  	224,1,0,0,0,227,228,5,55,0,0,228,43,1,0,0,0,229,230,3,42,21,0,230,45,
  	1,0,0,0,231,234,5,12,0,0,232,235,3,90,45,0,233,235,5,7,0,0,234,232,1,
  	0,0,0,234,233,1,0,0,0,235,236,1,0,0,0,236,237,5,13,0,0,237,47,1,0,0,0,
  	238,239,5,55,0,0,239,49,1,0,0,0,240,242,3,40,20,0,241,240,1,0,0,0,241,
  	242,1,0,0,0,242,243,1,0,0,0,243,244,3,48,24,0,244,51,1,0,0,0,245,247,
  	3,40,20,0,246,245,1,0,0,0,246,247,1,0,0,0,247,251,1,0,0,0,248,249,3,44,
  	22,0,249,250,5,52,0,0,250,252,1,0,0,0,251,248,1,0,0,0,251,252,1,0,0,0,
  	252,253,1,0,0,0,253,254,5,26,0,0,254,263,3,44,22,0,255,256,3,40,20,0,
  	256,257,5,52,0,0,257,258,5,26,0,0,258,259,3,44,22,0,259,263,1,0,0,0,260,
  	261,5,26,0,0,261,263,3,46,23,0,262,246,1,0,0,0,262,255,1,0,0,0,262,260,
  	1,0,0,0,263,53,1,0,0,0,264,266,3,10,5,0,265,264,1,0,0,0,266,267,1,0,0,
  	0,267,265,1,0,0,0,267,268,1,0,0,0,268,55,1,0,0,0,269,270,3,54,27,0,270,
  	57,1,0,0,0,271,272,6,29,-1,0,272,304,3,32,16,0,273,277,3,16,8,0,274,276,
  	3,12,6,0,275,274,1,0,0,0,276,279,1,0,0,0,277,275,1,0,0,0,277,278,1,0,
  	0,0,278,286,1,0,0,0,279,277,1,0,0,0,280,282,5,12,0,0,281,283,3,28,14,
  	0,282,281,1,0,0,0,282,283,1,0,0,0,283,284,1,0,0,0,284,287,5,13,0,0,285,
  	287,3,102,51,0,286,280,1,0,0,0,286,285,1,0,0,0,287,304,1,0,0,0,288,289,
  	5,6,0,0,289,290,5,29,0,0,290,291,3,56,28,0,291,292,5,30,0,0,292,293,5,
  	12,0,0,293,294,3,90,45,0,294,295,5,13,0,0,295,304,1,0,0,0,296,299,5,12,
  	0,0,297,300,3,90,45,0,298,300,3,56,28,0,299,297,1,0,0,0,299,298,1,0,0,
  	0,300,301,1,0,0,0,301,302,5,13,0,0,302,304,1,0,0,0,303,271,1,0,0,0,303,
  	273,1,0,0,0,303,288,1,0,0,0,303,296,1,0,0,0,304,329,1,0,0,0,305,306,10,
  	7,0,0,306,309,5,14,0,0,307,310,3,90,45,0,308,310,3,102,51,0,309,307,1,
  	0,0,0,309,308,1,0,0,0,310,311,1,0,0,0,311,312,5,15,0,0,312,328,1,0,0,
  	0,313,314,10,6,0,0,314,316,5,12,0,0,315,317,3,28,14,0,316,315,1,0,0,0,
  	316,317,1,0,0,0,317,318,1,0,0,0,318,328,5,13,0,0,319,320,10,4,0,0,320,
  	323,5,54,0,0,321,324,3,34,17,0,322,324,3,52,26,0,323,321,1,0,0,0,323,
  	322,1,0,0,0,324,328,1,0,0,0,325,326,10,3,0,0,326,328,7,0,0,0,327,305,
  	1,0,0,0,327,313,1,0,0,0,327,319,1,0,0,0,327,325,1,0,0,0,328,331,1,0,0,
  	0,329,327,1,0,0,0,329,330,1,0,0,0,330,59,1,0,0,0,331,329,1,0,0,0,332,
  	344,3,58,29,0,333,337,5,47,0,0,334,337,5,48,0,0,335,337,3,62,31,0,336,
  	333,1,0,0,0,336,334,1,0,0,0,336,335,1,0,0,0,337,338,1,0,0,0,338,344,3,
  	60,30,0,339,340,5,12,0,0,340,341,3,56,28,0,341,342,5,13,0,0,342,344,1,
  	0,0,0,343,332,1,0,0,0,343,336,1,0,0,0,343,339,1,0,0,0,344,61,1,0,0,0,
  	345,346,7,1,0,0,346,63,1,0,0,0,347,357,3,60,30,0,348,349,5,6,0,0,349,
  	350,5,29,0,0,350,351,3,56,28,0,351,352,5,30,0,0,352,353,5,12,0,0,353,
  	354,3,64,32,0,354,355,5,13,0,0,355,357,1,0,0,0,356,347,1,0,0,0,356,348,
  	1,0,0,0,357,65,1,0,0,0,358,363,3,64,32,0,359,360,7,2,0,0,360,362,3,64,
  	32,0,361,359,1,0,0,0,362,365,1,0,0,0,363,361,1,0,0,0,363,364,1,0,0,0,
  	364,67,1,0,0,0,365,363,1,0,0,0,366,371,3,66,33,0,367,368,7,3,0,0,368,
  	370,3,66,33,0,369,367,1,0,0,0,370,373,1,0,0,0,371,369,1,0,0,0,371,372,
  	1,0,0,0,372,69,1,0,0,0,373,371,1,0,0,0,374,380,3,68,34,0,375,376,3,72,
  	36,0,376,377,3,68,34,0,377,379,1,0,0,0,378,375,1,0,0,0,379,382,1,0,0,
  	0,380,378,1,0,0,0,380,381,1,0,0,0,381,71,1,0,0,0,382,380,1,0,0,0,383,
  	384,5,30,0,0,384,388,5,30,0,0,385,386,5,29,0,0,386,388,5,29,0,0,387,383,
  	1,0,0,0,387,385,1,0,0,0,388,73,1,0,0,0,389,394,3,70,35,0,390,391,7,4,
  	0,0,391,393,3,70,35,0,392,390,1,0,0,0,393,396,1,0,0,0,394,392,1,0,0,0,
  	394,395,1,0,0,0,395,75,1,0,0,0,396,394,1,0,0,0,397,402,3,74,37,0,398,
  	399,7,5,0,0,399,401,3,74,37,0,400,398,1,0,0,0,401,404,1,0,0,0,402,400,
  	1,0,0,0,402,403,1,0,0,0,403,77,1,0,0,0,404,402,1,0,0,0,405,410,3,76,38,
  	0,406,407,5,24,0,0,407,409,3,76,38,0,408,406,1,0,0,0,409,412,1,0,0,0,
  	410,408,1,0,0,0,410,411,1,0,0,0,411,79,1,0,0,0,412,410,1,0,0,0,413,418,
  	3,78,39,0,414,415,5,23,0,0,415,417,3,78,39,0,416,414,1,0,0,0,417,420,
  	1,0,0,0,418,416,1,0,0,0,418,419,1,0,0,0,419,81,1,0,0,0,420,418,1,0,0,
  	0,421,426,3,80,40,0,422,423,5,25,0,0,423,425,3,80,40,0,424,422,1,0,0,
  	0,425,428,1,0,0,0,426,424,1,0,0,0,426,427,1,0,0,0,427,83,1,0,0,0,428,
  	426,1,0,0,0,429,434,3,82,41,0,430,431,5,45,0,0,431,433,3,82,41,0,432,
  	430,1,0,0,0,433,436,1,0,0,0,434,432,1,0,0,0,434,435,1,0,0,0,435,85,1,
  	0,0,0,436,434,1,0,0,0,437,442,3,84,42,0,438,439,5,46,0,0,439,441,3,84,
  	42,0,440,438,1,0,0,0,441,444,1,0,0,0,442,440,1,0,0,0,442,443,1,0,0,0,
  	443,87,1,0,0,0,444,442,1,0,0,0,445,451,3,86,43,0,446,447,5,50,0,0,447,
  	448,3,90,45,0,448,449,5,51,0,0,449,450,3,92,46,0,450,452,1,0,0,0,451,
  	446,1,0,0,0,451,452,1,0,0,0,452,89,1,0,0,0,453,458,3,92,46,0,454,455,
  	5,49,0,0,455,457,3,92,46,0,456,454,1,0,0,0,457,460,1,0,0,0,458,456,1,
  	0,0,0,458,459,1,0,0,0,459,91,1,0,0,0,460,458,1,0,0,0,461,467,3,88,44,
  	0,462,463,3,86,43,0,463,464,3,94,47,0,464,465,3,98,49,0,465,467,1,0,0,
  	0,466,461,1,0,0,0,466,462,1,0,0,0,467,93,1,0,0,0,468,469,7,6,0,0,469,
  	95,1,0,0,0,470,471,5,28,0,0,471,474,3,98,49,0,472,474,3,102,51,0,473,
  	470,1,0,0,0,473,472,1,0,0,0,474,97,1,0,0,0,475,478,3,92,46,0,476,478,
  	3,102,51,0,477,475,1,0,0,0,477,476,1,0,0,0,478,99,1,0,0,0,479,484,3,98,
  	49,0,480,481,5,49,0,0,481,483,3,98,49,0,482,480,1,0,0,0,483,486,1,0,0,
  	0,484,482,1,0,0,0,484,485,1,0,0,0,485,488,1,0,0,0,486,484,1,0,0,0,487,
  	489,5,49,0,0,488,487,1,0,0,0,488,489,1,0,0,0,489,101,1,0,0,0,490,495,
  	7,7,0,0,491,493,3,100,50,0,492,494,5,49,0,0,493,492,1,0,0,0,493,494,1,
  	0,0,0,494,496,1,0,0,0,495,491,1,0,0,0,495,496,1,0,0,0,496,497,1,0,0,0,
  	497,498,7,8,0,0,498,103,1,0,0,0,499,507,5,1,0,0,500,507,5,2,0,0,501,507,
  	5,3,0,0,502,507,5,4,0,0,503,507,3,106,53,0,504,507,5,5,0,0,505,507,5,
  	10,0,0,506,499,1,0,0,0,506,500,1,0,0,0,506,501,1,0,0,0,506,502,1,0,0,
  	0,506,503,1,0,0,0,506,504,1,0,0,0,506,505,1,0,0,0,507,105,1,0,0,0,508,
  	509,7,9,0,0,509,107,1,0,0,0,56,111,123,128,136,144,148,152,164,174,179,
  	188,193,200,204,215,224,234,241,246,251,262,267,277,282,286,299,303,309,
  	316,323,327,329,336,343,356,363,371,380,387,394,402,410,418,426,434,442,
  	451,458,466,473,477,484,488,493,495,506
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  angelscriptconsoleparserParserStaticData = std::move(staticData);
}

}

AngelscriptConsoleParser::AngelscriptConsoleParser(TokenStream *input) : AngelscriptConsoleParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

AngelscriptConsoleParser::AngelscriptConsoleParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  AngelscriptConsoleParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *angelscriptconsoleparserParserStaticData->atn, angelscriptconsoleparserParserStaticData->decisionToDFA, angelscriptconsoleparserParserStaticData->sharedContextCache, options);
}

AngelscriptConsoleParser::~AngelscriptConsoleParser() {
  delete _interpreter;
}

const atn::ATN& AngelscriptConsoleParser::getATN() const {
  return *angelscriptconsoleparserParserStaticData->atn;
}

std::string AngelscriptConsoleParser::getGrammarFileName() const {
  return "AngelscriptConsoleParser.g4";
}

const std::vector<std::string>& AngelscriptConsoleParser::getRuleNames() const {
  return angelscriptconsoleparserParserStaticData->ruleNames;
}

const dfa::Vocabulary& AngelscriptConsoleParser::getVocabulary() const {
  return angelscriptconsoleparserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView AngelscriptConsoleParser::getSerializedATN() const {
  return angelscriptconsoleparserParserStaticData->serializedATN;
}


//----------------- ScriptContext ------------------------------------------------------------------

AngelscriptConsoleParser::ScriptContext::ScriptContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::ScriptContext::EOF() {
  return getToken(AngelscriptConsoleParser::EOF, 0);
}

std::vector<AngelscriptConsoleParser::ScriptItemContext *> AngelscriptConsoleParser::ScriptContext::scriptItem() {
  return getRuleContexts<AngelscriptConsoleParser::ScriptItemContext>();
}

AngelscriptConsoleParser::ScriptItemContext* AngelscriptConsoleParser::ScriptContext::scriptItem(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::ScriptItemContext>(i);
}


size_t AngelscriptConsoleParser::ScriptContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleScript;
}


std::any AngelscriptConsoleParser::ScriptContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitScript(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ScriptContext* AngelscriptConsoleParser::script() {
  ScriptContext *_localctx = _tracker.createInstance<ScriptContext>(_ctx, getState());
  enterRule(_localctx, 0, AngelscriptConsoleParser::RuleScript);
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
    setState(111);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 45035996273705216) != 0)) {
      setState(108);
      scriptItem();
      setState(113);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(114);
    match(AngelscriptConsoleParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EmptyDeclaration_Context ------------------------------------------------------------------

AngelscriptConsoleParser::EmptyDeclaration_Context::EmptyDeclaration_Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::EmptyDeclaration_Context::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}


size_t AngelscriptConsoleParser::EmptyDeclaration_Context::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleEmptyDeclaration_;
}


std::any AngelscriptConsoleParser::EmptyDeclaration_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitEmptyDeclaration_(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::EmptyDeclaration_Context* AngelscriptConsoleParser::emptyDeclaration_() {
  EmptyDeclaration_Context *_localctx = _tracker.createInstance<EmptyDeclaration_Context>(_ctx, getState());
  enterRule(_localctx, 2, AngelscriptConsoleParser::RuleEmptyDeclaration_);

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
    match(AngelscriptConsoleParser::Semi);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ScriptItemContext ------------------------------------------------------------------

AngelscriptConsoleParser::ScriptItemContext::ScriptItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::ScriptItemContext::declSpecifierSeq() {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierSeqContext>(0);
}

AngelscriptConsoleParser::InitDeclaratorListContext* AngelscriptConsoleParser::ScriptItemContext::initDeclaratorList() {
  return getRuleContext<AngelscriptConsoleParser::InitDeclaratorListContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::ScriptItemContext::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}

AngelscriptConsoleParser::EmptyDeclaration_Context* AngelscriptConsoleParser::ScriptItemContext::emptyDeclaration_() {
  return getRuleContext<AngelscriptConsoleParser::EmptyDeclaration_Context>(0);
}


size_t AngelscriptConsoleParser::ScriptItemContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleScriptItem;
}


std::any AngelscriptConsoleParser::ScriptItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitScriptItem(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ScriptItemContext* AngelscriptConsoleParser::scriptItem() {
  ScriptItemContext *_localctx = _tracker.createInstance<ScriptItemContext>(_ctx, getState());
  enterRule(_localctx, 4, AngelscriptConsoleParser::RuleScriptItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(123);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Const:
      case AngelscriptConsoleParser::Identifier: {
        enterOuterAlt(_localctx, 1);
        setState(118);
        declSpecifierSeq();
        setState(119);
        initDeclaratorList();
        setState(120);
        match(AngelscriptConsoleParser::Semi);
        break;
      }

      case AngelscriptConsoleParser::Semi: {
        enterOuterAlt(_localctx, 2);
        setState(122);
        emptyDeclaration_();
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

//----------------- DeclSpecifierSeqContext ------------------------------------------------------------------

AngelscriptConsoleParser::DeclSpecifierSeqContext::DeclSpecifierSeqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::DeclSpecifierContext *> AngelscriptConsoleParser::DeclSpecifierSeqContext::declSpecifier() {
  return getRuleContexts<AngelscriptConsoleParser::DeclSpecifierContext>();
}

AngelscriptConsoleParser::DeclSpecifierContext* AngelscriptConsoleParser::DeclSpecifierSeqContext::declSpecifier(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierContext>(i);
}


size_t AngelscriptConsoleParser::DeclSpecifierSeqContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDeclSpecifierSeq;
}


std::any AngelscriptConsoleParser::DeclSpecifierSeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDeclSpecifierSeq(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::declSpecifierSeq() {
  DeclSpecifierSeqContext *_localctx = _tracker.createInstance<DeclSpecifierSeqContext>(_ctx, getState());
  enterRule(_localctx, 6, AngelscriptConsoleParser::RuleDeclSpecifierSeq);

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
    setState(126); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(125);
              declSpecifier();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(128); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclSpecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::DeclSpecifierContext::DeclSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::TypeSpecifierContext* AngelscriptConsoleParser::DeclSpecifierContext::typeSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::TypeSpecifierContext>(0);
}


size_t AngelscriptConsoleParser::DeclSpecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDeclSpecifier;
}


std::any AngelscriptConsoleParser::DeclSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDeclSpecifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::DeclSpecifierContext* AngelscriptConsoleParser::declSpecifier() {
  DeclSpecifierContext *_localctx = _tracker.createInstance<DeclSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 8, AngelscriptConsoleParser::RuleDeclSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(130);
    typeSpecifier();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeSpecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::TypeSpecifierContext::TypeSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::TrailingTypeSpecifierContext* AngelscriptConsoleParser::TypeSpecifierContext::trailingTypeSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::TrailingTypeSpecifierContext>(0);
}


size_t AngelscriptConsoleParser::TypeSpecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTypeSpecifier;
}


std::any AngelscriptConsoleParser::TypeSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTypeSpecifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TypeSpecifierContext* AngelscriptConsoleParser::typeSpecifier() {
  TypeSpecifierContext *_localctx = _tracker.createInstance<TypeSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 10, AngelscriptConsoleParser::RuleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(132);
    trailingTypeSpecifier();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArraySpecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::ArraySpecifierContext::ArraySpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::ArraySpecifierContext::LeftBracket() {
  return getToken(AngelscriptConsoleParser::LeftBracket, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::ArraySpecifierContext::RightBracket() {
  return getToken(AngelscriptConsoleParser::RightBracket, 0);
}

AngelscriptConsoleParser::ConstantExpressionContext* AngelscriptConsoleParser::ArraySpecifierContext::constantExpression() {
  return getRuleContext<AngelscriptConsoleParser::ConstantExpressionContext>(0);
}


size_t AngelscriptConsoleParser::ArraySpecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleArraySpecifier;
}


std::any AngelscriptConsoleParser::ArraySpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitArraySpecifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ArraySpecifierContext* AngelscriptConsoleParser::arraySpecifier() {
  ArraySpecifierContext *_localctx = _tracker.createInstance<ArraySpecifierContext>(_ctx, getState());
  enterRule(_localctx, 12, AngelscriptConsoleParser::RuleArraySpecifier);
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
    setState(134);
    match(AngelscriptConsoleParser::LeftBracket);
    setState(136);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::IntegerLiteral) {
      setState(135);
      constantExpression();
    }
    setState(138);
    match(AngelscriptConsoleParser::RightBracket);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TrailingTypeSpecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::TrailingTypeSpecifierContext::TrailingTypeSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::SimpleTypeSpecifierContext* AngelscriptConsoleParser::TrailingTypeSpecifierContext::simpleTypeSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::SimpleTypeSpecifierContext>(0);
}

std::vector<AngelscriptConsoleParser::ArraySpecifierContext *> AngelscriptConsoleParser::TrailingTypeSpecifierContext::arraySpecifier() {
  return getRuleContexts<AngelscriptConsoleParser::ArraySpecifierContext>();
}

AngelscriptConsoleParser::ArraySpecifierContext* AngelscriptConsoleParser::TrailingTypeSpecifierContext::arraySpecifier(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::ArraySpecifierContext>(i);
}

tree::TerminalNode* AngelscriptConsoleParser::TrailingTypeSpecifierContext::Const() {
  return getToken(AngelscriptConsoleParser::Const, 0);
}


size_t AngelscriptConsoleParser::TrailingTypeSpecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTrailingTypeSpecifier;
}


std::any AngelscriptConsoleParser::TrailingTypeSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTrailingTypeSpecifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TrailingTypeSpecifierContext* AngelscriptConsoleParser::trailingTypeSpecifier() {
  TrailingTypeSpecifierContext *_localctx = _tracker.createInstance<TrailingTypeSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 14, AngelscriptConsoleParser::RuleTrailingTypeSpecifier);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(148);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Identifier: {
        enterOuterAlt(_localctx, 1);
        setState(140);
        simpleTypeSpecifier();
        setState(144);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == AngelscriptConsoleParser::LeftBracket) {
          setState(141);
          arraySpecifier();
          setState(146);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        break;
      }

      case AngelscriptConsoleParser::Const: {
        enterOuterAlt(_localctx, 2);
        setState(147);
        match(AngelscriptConsoleParser::Const);
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

//----------------- SimpleTypeSpecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::SimpleTypeSpecifierContext::SimpleTypeSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

AngelscriptConsoleParser::AsGenericContext* AngelscriptConsoleParser::SimpleTypeSpecifierContext::asGeneric() {
  return getRuleContext<AngelscriptConsoleParser::AsGenericContext>(0);
}


size_t AngelscriptConsoleParser::SimpleTypeSpecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleSimpleTypeSpecifier;
}


std::any AngelscriptConsoleParser::SimpleTypeSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitSimpleTypeSpecifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::SimpleTypeSpecifierContext* AngelscriptConsoleParser::simpleTypeSpecifier() {
  SimpleTypeSpecifierContext *_localctx = _tracker.createInstance<SimpleTypeSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 16, AngelscriptConsoleParser::RuleSimpleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(152);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(150);
      match(AngelscriptConsoleParser::Identifier);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(151);
      asGeneric();
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

//----------------- AsGenericContext ------------------------------------------------------------------

AngelscriptConsoleParser::AsGenericContext::AsGenericContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::AsGenericContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AsGenericContext::Less() {
  return getToken(AngelscriptConsoleParser::Less, 0);
}

AngelscriptConsoleParser::SimpleTypeSpecifierListContext* AngelscriptConsoleParser::AsGenericContext::simpleTypeSpecifierList() {
  return getRuleContext<AngelscriptConsoleParser::SimpleTypeSpecifierListContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::AsGenericContext::Greater() {
  return getToken(AngelscriptConsoleParser::Greater, 0);
}


size_t AngelscriptConsoleParser::AsGenericContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleAsGeneric;
}


std::any AngelscriptConsoleParser::AsGenericContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitAsGeneric(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::AsGenericContext* AngelscriptConsoleParser::asGeneric() {
  AsGenericContext *_localctx = _tracker.createInstance<AsGenericContext>(_ctx, getState());
  enterRule(_localctx, 18, AngelscriptConsoleParser::RuleAsGeneric);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(154);
    match(AngelscriptConsoleParser::Identifier);
    setState(155);
    match(AngelscriptConsoleParser::Less);
    setState(156);
    simpleTypeSpecifierList();
    setState(157);
    match(AngelscriptConsoleParser::Greater);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SimpleTypeSpecifierListContext ------------------------------------------------------------------

AngelscriptConsoleParser::SimpleTypeSpecifierListContext::SimpleTypeSpecifierListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::SimpleTypeSpecifierContext *> AngelscriptConsoleParser::SimpleTypeSpecifierListContext::simpleTypeSpecifier() {
  return getRuleContexts<AngelscriptConsoleParser::SimpleTypeSpecifierContext>();
}

AngelscriptConsoleParser::SimpleTypeSpecifierContext* AngelscriptConsoleParser::SimpleTypeSpecifierListContext::simpleTypeSpecifier(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::SimpleTypeSpecifierContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::SimpleTypeSpecifierListContext::Comma() {
  return getTokens(AngelscriptConsoleParser::Comma);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierListContext::Comma(size_t i) {
  return getToken(AngelscriptConsoleParser::Comma, i);
}


size_t AngelscriptConsoleParser::SimpleTypeSpecifierListContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleSimpleTypeSpecifierList;
}


std::any AngelscriptConsoleParser::SimpleTypeSpecifierListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitSimpleTypeSpecifierList(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::SimpleTypeSpecifierListContext* AngelscriptConsoleParser::simpleTypeSpecifierList() {
  SimpleTypeSpecifierListContext *_localctx = _tracker.createInstance<SimpleTypeSpecifierListContext>(_ctx, getState());
  enterRule(_localctx, 20, AngelscriptConsoleParser::RuleSimpleTypeSpecifierList);
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
    setState(159);
    simpleTypeSpecifier();
    setState(164);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(160);
      match(AngelscriptConsoleParser::Comma);
      setState(161);
      simpleTypeSpecifier();
      setState(166);
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

//----------------- ConstantExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::ConstantExpressionContext::ConstantExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::ConstantExpressionContext::IntegerLiteral() {
  return getToken(AngelscriptConsoleParser::IntegerLiteral, 0);
}


size_t AngelscriptConsoleParser::ConstantExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleConstantExpression;
}


std::any AngelscriptConsoleParser::ConstantExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitConstantExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ConstantExpressionContext* AngelscriptConsoleParser::constantExpression() {
  ConstantExpressionContext *_localctx = _tracker.createInstance<ConstantExpressionContext>(_ctx, getState());
  enterRule(_localctx, 22, AngelscriptConsoleParser::RuleConstantExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(167);
    match(AngelscriptConsoleParser::IntegerLiteral);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitDeclaratorListContext ------------------------------------------------------------------

AngelscriptConsoleParser::InitDeclaratorListContext::InitDeclaratorListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::InitDeclaratorContext *> AngelscriptConsoleParser::InitDeclaratorListContext::initDeclarator() {
  return getRuleContexts<AngelscriptConsoleParser::InitDeclaratorContext>();
}

AngelscriptConsoleParser::InitDeclaratorContext* AngelscriptConsoleParser::InitDeclaratorListContext::initDeclarator(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::InitDeclaratorContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::InitDeclaratorListContext::Comma() {
  return getTokens(AngelscriptConsoleParser::Comma);
}

tree::TerminalNode* AngelscriptConsoleParser::InitDeclaratorListContext::Comma(size_t i) {
  return getToken(AngelscriptConsoleParser::Comma, i);
}


size_t AngelscriptConsoleParser::InitDeclaratorListContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleInitDeclaratorList;
}


std::any AngelscriptConsoleParser::InitDeclaratorListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitInitDeclaratorList(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::InitDeclaratorListContext* AngelscriptConsoleParser::initDeclaratorList() {
  InitDeclaratorListContext *_localctx = _tracker.createInstance<InitDeclaratorListContext>(_ctx, getState());
  enterRule(_localctx, 24, AngelscriptConsoleParser::RuleInitDeclaratorList);
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
    initDeclarator();
    setState(174);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(170);
      match(AngelscriptConsoleParser::Comma);
      setState(171);
      initDeclarator();
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

//----------------- InitDeclaratorContext ------------------------------------------------------------------

AngelscriptConsoleParser::InitDeclaratorContext::InitDeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::InitDeclaratorContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

AngelscriptConsoleParser::InitializerContext* AngelscriptConsoleParser::InitDeclaratorContext::initializer() {
  return getRuleContext<AngelscriptConsoleParser::InitializerContext>(0);
}


size_t AngelscriptConsoleParser::InitDeclaratorContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleInitDeclarator;
}


std::any AngelscriptConsoleParser::InitDeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitInitDeclarator(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::InitDeclaratorContext* AngelscriptConsoleParser::initDeclarator() {
  InitDeclaratorContext *_localctx = _tracker.createInstance<InitDeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 26, AngelscriptConsoleParser::RuleInitDeclarator);
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
    match(AngelscriptConsoleParser::Identifier);
    setState(179);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 268521472) != 0)) {
      setState(178);
      initializer();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionListContext ------------------------------------------------------------------

AngelscriptConsoleParser::ExpressionListContext::ExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::InitializerListContext* AngelscriptConsoleParser::ExpressionListContext::initializerList() {
  return getRuleContext<AngelscriptConsoleParser::InitializerListContext>(0);
}


size_t AngelscriptConsoleParser::ExpressionListContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleExpressionList;
}


std::any AngelscriptConsoleParser::ExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitExpressionList(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ExpressionListContext* AngelscriptConsoleParser::expressionList() {
  ExpressionListContext *_localctx = _tracker.createInstance<ExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 28, AngelscriptConsoleParser::RuleExpressionList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(181);
    initializerList();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitializerContext ------------------------------------------------------------------

AngelscriptConsoleParser::InitializerContext::InitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::BraceOrEqualInitializerContext* AngelscriptConsoleParser::InitializerContext::braceOrEqualInitializer() {
  return getRuleContext<AngelscriptConsoleParser::BraceOrEqualInitializerContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::InitializerContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

AngelscriptConsoleParser::ExpressionListContext* AngelscriptConsoleParser::InitializerContext::expressionList() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionListContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::InitializerContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}


size_t AngelscriptConsoleParser::InitializerContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleInitializer;
}


std::any AngelscriptConsoleParser::InitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitInitializer(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::InitializerContext* AngelscriptConsoleParser::initializer() {
  InitializerContext *_localctx = _tracker.createInstance<InitializerContext>(_ctx, getState());
  enterRule(_localctx, 30, AngelscriptConsoleParser::RuleInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(188);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace:
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(183);
        braceOrEqualInitializer();
        break;
      }

      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 2);
        setState(184);
        match(AngelscriptConsoleParser::LeftParen);
        setState(185);
        expressionList();
        setState(186);
        match(AngelscriptConsoleParser::RightParen);
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

//----------------- PrimaryExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::PrimaryExpressionContext::PrimaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::LiteralContext *> AngelscriptConsoleParser::PrimaryExpressionContext::literal() {
  return getRuleContexts<AngelscriptConsoleParser::LiteralContext>();
}

AngelscriptConsoleParser::LiteralContext* AngelscriptConsoleParser::PrimaryExpressionContext::literal(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::LiteralContext>(i);
}

tree::TerminalNode* AngelscriptConsoleParser::PrimaryExpressionContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::PrimaryExpressionContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::PrimaryExpressionContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

AngelscriptConsoleParser::IdExpressionContext* AngelscriptConsoleParser::PrimaryExpressionContext::idExpression() {
  return getRuleContext<AngelscriptConsoleParser::IdExpressionContext>(0);
}


size_t AngelscriptConsoleParser::PrimaryExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RulePrimaryExpression;
}


std::any AngelscriptConsoleParser::PrimaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::PrimaryExpressionContext* AngelscriptConsoleParser::primaryExpression() {
  PrimaryExpressionContext *_localctx = _tracker.createInstance<PrimaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 32, AngelscriptConsoleParser::RulePrimaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(200);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(191); 
      _errHandler->sync(this);
      alt = 1;
      do {
        switch (alt) {
          case 1: {
                setState(190);
                literal();
                break;
              }

        default:
          throw NoViableAltException(this);
        }
        setState(193); 
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
      } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(195);
      match(AngelscriptConsoleParser::LeftParen);
      setState(196);
      expression();
      setState(197);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(199);
      idExpression();
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

//----------------- IdExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::IdExpressionContext::IdExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::UnqualifiedIdContext* AngelscriptConsoleParser::IdExpressionContext::unqualifiedId() {
  return getRuleContext<AngelscriptConsoleParser::UnqualifiedIdContext>(0);
}

AngelscriptConsoleParser::QualifiedIdContext* AngelscriptConsoleParser::IdExpressionContext::qualifiedId() {
  return getRuleContext<AngelscriptConsoleParser::QualifiedIdContext>(0);
}


size_t AngelscriptConsoleParser::IdExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleIdExpression;
}


std::any AngelscriptConsoleParser::IdExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitIdExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::IdExpressionContext* AngelscriptConsoleParser::idExpression() {
  IdExpressionContext *_localctx = _tracker.createInstance<IdExpressionContext>(_ctx, getState());
  enterRule(_localctx, 34, AngelscriptConsoleParser::RuleIdExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(204);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(202);
      unqualifiedId();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(203);
      qualifiedId();
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

//----------------- UnqualifiedIdContext ------------------------------------------------------------------

AngelscriptConsoleParser::UnqualifiedIdContext::UnqualifiedIdContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::UnqualifiedIdContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}


size_t AngelscriptConsoleParser::UnqualifiedIdContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleUnqualifiedId;
}


std::any AngelscriptConsoleParser::UnqualifiedIdContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitUnqualifiedId(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::UnqualifiedIdContext* AngelscriptConsoleParser::unqualifiedId() {
  UnqualifiedIdContext *_localctx = _tracker.createInstance<UnqualifiedIdContext>(_ctx, getState());
  enterRule(_localctx, 36, AngelscriptConsoleParser::RuleUnqualifiedId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(206);
    match(AngelscriptConsoleParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- QualifiedIdContext ------------------------------------------------------------------

AngelscriptConsoleParser::QualifiedIdContext::QualifiedIdContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::NestedNameSpecifierContext* AngelscriptConsoleParser::QualifiedIdContext::nestedNameSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::NestedNameSpecifierContext>(0);
}

AngelscriptConsoleParser::UnqualifiedIdContext* AngelscriptConsoleParser::QualifiedIdContext::unqualifiedId() {
  return getRuleContext<AngelscriptConsoleParser::UnqualifiedIdContext>(0);
}


size_t AngelscriptConsoleParser::QualifiedIdContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleQualifiedId;
}


std::any AngelscriptConsoleParser::QualifiedIdContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitQualifiedId(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::QualifiedIdContext* AngelscriptConsoleParser::qualifiedId() {
  QualifiedIdContext *_localctx = _tracker.createInstance<QualifiedIdContext>(_ctx, getState());
  enterRule(_localctx, 38, AngelscriptConsoleParser::RuleQualifiedId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(208);
    nestedNameSpecifier(0);
    setState(209);
    unqualifiedId();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NestedNameSpecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::NestedNameSpecifierContext::NestedNameSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::NestedNameSpecifierContext::Doublecolon() {
  return getToken(AngelscriptConsoleParser::Doublecolon, 0);
}

AngelscriptConsoleParser::TheTypeNameContext* AngelscriptConsoleParser::NestedNameSpecifierContext::theTypeName() {
  return getRuleContext<AngelscriptConsoleParser::TheTypeNameContext>(0);
}

AngelscriptConsoleParser::NamespaceNameContext* AngelscriptConsoleParser::NestedNameSpecifierContext::namespaceName() {
  return getRuleContext<AngelscriptConsoleParser::NamespaceNameContext>(0);
}

AngelscriptConsoleParser::DecltypeSpecifierContext* AngelscriptConsoleParser::NestedNameSpecifierContext::decltypeSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::DecltypeSpecifierContext>(0);
}

AngelscriptConsoleParser::NestedNameSpecifierContext* AngelscriptConsoleParser::NestedNameSpecifierContext::nestedNameSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::NestedNameSpecifierContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::NestedNameSpecifierContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}


size_t AngelscriptConsoleParser::NestedNameSpecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleNestedNameSpecifier;
}


std::any AngelscriptConsoleParser::NestedNameSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitNestedNameSpecifier(this);
  else
    return visitor->visitChildren(this);
}


AngelscriptConsoleParser::NestedNameSpecifierContext* AngelscriptConsoleParser::nestedNameSpecifier() {
   return nestedNameSpecifier(0);
}

AngelscriptConsoleParser::NestedNameSpecifierContext* AngelscriptConsoleParser::nestedNameSpecifier(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  AngelscriptConsoleParser::NestedNameSpecifierContext *_localctx = _tracker.createInstance<NestedNameSpecifierContext>(_ctx, parentState);
  AngelscriptConsoleParser::NestedNameSpecifierContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 40;
  enterRecursionRule(_localctx, 40, AngelscriptConsoleParser::RuleNestedNameSpecifier, precedence);

    

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
    setState(215);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      setState(212);
      theTypeName();
      break;
    }

    case 2: {
      setState(213);
      namespaceName();
      break;
    }

    case 3: {
      setState(214);
      decltypeSpecifier();
      break;
    }

    default:
      break;
    }
    setState(217);
    match(AngelscriptConsoleParser::Doublecolon);
    _ctx->stop = _input->LT(-1);
    setState(224);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<NestedNameSpecifierContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleNestedNameSpecifier);
        setState(219);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(220);
        match(AngelscriptConsoleParser::Identifier);
        setState(221);
        match(AngelscriptConsoleParser::Doublecolon); 
      }
      setState(226);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- TypedefNameContext ------------------------------------------------------------------

AngelscriptConsoleParser::TypedefNameContext::TypedefNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::TypedefNameContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}


size_t AngelscriptConsoleParser::TypedefNameContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTypedefName;
}


std::any AngelscriptConsoleParser::TypedefNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTypedefName(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TypedefNameContext* AngelscriptConsoleParser::typedefName() {
  TypedefNameContext *_localctx = _tracker.createInstance<TypedefNameContext>(_ctx, getState());
  enterRule(_localctx, 42, AngelscriptConsoleParser::RuleTypedefName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(227);
    match(AngelscriptConsoleParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TheTypeNameContext ------------------------------------------------------------------

AngelscriptConsoleParser::TheTypeNameContext::TheTypeNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::TypedefNameContext* AngelscriptConsoleParser::TheTypeNameContext::typedefName() {
  return getRuleContext<AngelscriptConsoleParser::TypedefNameContext>(0);
}


size_t AngelscriptConsoleParser::TheTypeNameContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTheTypeName;
}


std::any AngelscriptConsoleParser::TheTypeNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTheTypeName(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TheTypeNameContext* AngelscriptConsoleParser::theTypeName() {
  TheTypeNameContext *_localctx = _tracker.createInstance<TheTypeNameContext>(_ctx, getState());
  enterRule(_localctx, 44, AngelscriptConsoleParser::RuleTheTypeName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(229);
    typedefName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DecltypeSpecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::DecltypeSpecifierContext::DecltypeSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::DecltypeSpecifierContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::DecltypeSpecifierContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::DecltypeSpecifierContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::DecltypeSpecifierContext::Auto() {
  return getToken(AngelscriptConsoleParser::Auto, 0);
}


size_t AngelscriptConsoleParser::DecltypeSpecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDecltypeSpecifier;
}


std::any AngelscriptConsoleParser::DecltypeSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDecltypeSpecifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::DecltypeSpecifierContext* AngelscriptConsoleParser::decltypeSpecifier() {
  DecltypeSpecifierContext *_localctx = _tracker.createInstance<DecltypeSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 46, AngelscriptConsoleParser::RuleDecltypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(231);
    match(AngelscriptConsoleParser::LeftParen);
    setState(234);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::IntegerLiteral:
      case AngelscriptConsoleParser::CharacterLiteral:
      case AngelscriptConsoleParser::FloatingLiteral:
      case AngelscriptConsoleParser::StringLiteral:
      case AngelscriptConsoleParser::UserDefinedLiteral:
      case AngelscriptConsoleParser::Cast:
      case AngelscriptConsoleParser::False_:
      case AngelscriptConsoleParser::Nullptr:
      case AngelscriptConsoleParser::True_:
      case AngelscriptConsoleParser::LeftParen:
      case AngelscriptConsoleParser::Plus:
      case AngelscriptConsoleParser::Minus:
      case AngelscriptConsoleParser::Star:
      case AngelscriptConsoleParser::And:
      case AngelscriptConsoleParser::Or:
      case AngelscriptConsoleParser::Tilde:
      case AngelscriptConsoleParser::Not:
      case AngelscriptConsoleParser::PlusPlus:
      case AngelscriptConsoleParser::MinusMinus:
      case AngelscriptConsoleParser::Doublecolon:
      case AngelscriptConsoleParser::Identifier: {
        setState(232);
        expression();
        break;
      }

      case AngelscriptConsoleParser::Auto: {
        setState(233);
        match(AngelscriptConsoleParser::Auto);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(236);
    match(AngelscriptConsoleParser::RightParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NamespaceNameContext ------------------------------------------------------------------

AngelscriptConsoleParser::NamespaceNameContext::NamespaceNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::NamespaceNameContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}


size_t AngelscriptConsoleParser::NamespaceNameContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleNamespaceName;
}


std::any AngelscriptConsoleParser::NamespaceNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitNamespaceName(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::NamespaceNameContext* AngelscriptConsoleParser::namespaceName() {
  NamespaceNameContext *_localctx = _tracker.createInstance<NamespaceNameContext>(_ctx, getState());
  enterRule(_localctx, 48, AngelscriptConsoleParser::RuleNamespaceName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(238);
    match(AngelscriptConsoleParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- QualifiednamespacespecifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::QualifiednamespacespecifierContext::QualifiednamespacespecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::NamespaceNameContext* AngelscriptConsoleParser::QualifiednamespacespecifierContext::namespaceName() {
  return getRuleContext<AngelscriptConsoleParser::NamespaceNameContext>(0);
}

AngelscriptConsoleParser::NestedNameSpecifierContext* AngelscriptConsoleParser::QualifiednamespacespecifierContext::nestedNameSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::NestedNameSpecifierContext>(0);
}


size_t AngelscriptConsoleParser::QualifiednamespacespecifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleQualifiednamespacespecifier;
}


std::any AngelscriptConsoleParser::QualifiednamespacespecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitQualifiednamespacespecifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::QualifiednamespacespecifierContext* AngelscriptConsoleParser::qualifiednamespacespecifier() {
  QualifiednamespacespecifierContext *_localctx = _tracker.createInstance<QualifiednamespacespecifierContext>(_ctx, getState());
  enterRule(_localctx, 50, AngelscriptConsoleParser::RuleQualifiednamespacespecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(241);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
    case 1: {
      setState(240);
      nestedNameSpecifier(0);
      break;
    }

    default:
      break;
    }
    setState(243);
    namespaceName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PseudoDestructorNameContext ------------------------------------------------------------------

AngelscriptConsoleParser::PseudoDestructorNameContext::PseudoDestructorNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::PseudoDestructorNameContext::Tilde() {
  return getToken(AngelscriptConsoleParser::Tilde, 0);
}

std::vector<AngelscriptConsoleParser::TheTypeNameContext *> AngelscriptConsoleParser::PseudoDestructorNameContext::theTypeName() {
  return getRuleContexts<AngelscriptConsoleParser::TheTypeNameContext>();
}

AngelscriptConsoleParser::TheTypeNameContext* AngelscriptConsoleParser::PseudoDestructorNameContext::theTypeName(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::TheTypeNameContext>(i);
}

AngelscriptConsoleParser::NestedNameSpecifierContext* AngelscriptConsoleParser::PseudoDestructorNameContext::nestedNameSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::NestedNameSpecifierContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::PseudoDestructorNameContext::Doublecolon() {
  return getToken(AngelscriptConsoleParser::Doublecolon, 0);
}

AngelscriptConsoleParser::DecltypeSpecifierContext* AngelscriptConsoleParser::PseudoDestructorNameContext::decltypeSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::DecltypeSpecifierContext>(0);
}


size_t AngelscriptConsoleParser::PseudoDestructorNameContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RulePseudoDestructorName;
}


std::any AngelscriptConsoleParser::PseudoDestructorNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitPseudoDestructorName(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::PseudoDestructorNameContext* AngelscriptConsoleParser::pseudoDestructorName() {
  PseudoDestructorNameContext *_localctx = _tracker.createInstance<PseudoDestructorNameContext>(_ctx, getState());
  enterRule(_localctx, 52, AngelscriptConsoleParser::RulePseudoDestructorName);
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
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(246);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx)) {
      case 1: {
        setState(245);
        nestedNameSpecifier(0);
        break;
      }

      default:
        break;
      }
      setState(251);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Identifier) {
        setState(248);
        theTypeName();
        setState(249);
        match(AngelscriptConsoleParser::Doublecolon);
      }
      setState(253);
      match(AngelscriptConsoleParser::Tilde);
      setState(254);
      theTypeName();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(255);
      nestedNameSpecifier(0);
      setState(256);
      match(AngelscriptConsoleParser::Doublecolon);
      setState(257);
      match(AngelscriptConsoleParser::Tilde);
      setState(258);
      theTypeName();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(260);
      match(AngelscriptConsoleParser::Tilde);
      setState(261);
      decltypeSpecifier();
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

//----------------- TypeSpecifierSeqContext ------------------------------------------------------------------

AngelscriptConsoleParser::TypeSpecifierSeqContext::TypeSpecifierSeqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::TypeSpecifierContext *> AngelscriptConsoleParser::TypeSpecifierSeqContext::typeSpecifier() {
  return getRuleContexts<AngelscriptConsoleParser::TypeSpecifierContext>();
}

AngelscriptConsoleParser::TypeSpecifierContext* AngelscriptConsoleParser::TypeSpecifierSeqContext::typeSpecifier(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::TypeSpecifierContext>(i);
}


size_t AngelscriptConsoleParser::TypeSpecifierSeqContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTypeSpecifierSeq;
}


std::any AngelscriptConsoleParser::TypeSpecifierSeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTypeSpecifierSeq(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TypeSpecifierSeqContext* AngelscriptConsoleParser::typeSpecifierSeq() {
  TypeSpecifierSeqContext *_localctx = _tracker.createInstance<TypeSpecifierSeqContext>(_ctx, getState());
  enterRule(_localctx, 54, AngelscriptConsoleParser::RuleTypeSpecifierSeq);
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
    setState(265); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(264);
      typeSpecifier();
      setState(267); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == AngelscriptConsoleParser::Const

    || _la == AngelscriptConsoleParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TheTypeIdContext ------------------------------------------------------------------

AngelscriptConsoleParser::TheTypeIdContext::TheTypeIdContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::TypeSpecifierSeqContext* AngelscriptConsoleParser::TheTypeIdContext::typeSpecifierSeq() {
  return getRuleContext<AngelscriptConsoleParser::TypeSpecifierSeqContext>(0);
}


size_t AngelscriptConsoleParser::TheTypeIdContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTheTypeId;
}


std::any AngelscriptConsoleParser::TheTypeIdContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTheTypeId(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TheTypeIdContext* AngelscriptConsoleParser::theTypeId() {
  TheTypeIdContext *_localctx = _tracker.createInstance<TheTypeIdContext>(_ctx, getState());
  enterRule(_localctx, 56, AngelscriptConsoleParser::RuleTheTypeId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(269);
    typeSpecifierSeq();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PostfixExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::PostfixExpressionContext::PostfixExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::PrimaryExpressionContext* AngelscriptConsoleParser::PostfixExpressionContext::primaryExpression() {
  return getRuleContext<AngelscriptConsoleParser::PrimaryExpressionContext>(0);
}

AngelscriptConsoleParser::SimpleTypeSpecifierContext* AngelscriptConsoleParser::PostfixExpressionContext::simpleTypeSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::SimpleTypeSpecifierContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::PostfixExpressionContext::bracedInitList() {
  return getRuleContext<AngelscriptConsoleParser::BracedInitListContext>(0);
}

std::vector<AngelscriptConsoleParser::ArraySpecifierContext *> AngelscriptConsoleParser::PostfixExpressionContext::arraySpecifier() {
  return getRuleContexts<AngelscriptConsoleParser::ArraySpecifierContext>();
}

AngelscriptConsoleParser::ArraySpecifierContext* AngelscriptConsoleParser::PostfixExpressionContext::arraySpecifier(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::ArraySpecifierContext>(i);
}

AngelscriptConsoleParser::ExpressionListContext* AngelscriptConsoleParser::PostfixExpressionContext::expressionList() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionListContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::Cast() {
  return getToken(AngelscriptConsoleParser::Cast, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::Less() {
  return getToken(AngelscriptConsoleParser::Less, 0);
}

AngelscriptConsoleParser::TheTypeIdContext* AngelscriptConsoleParser::PostfixExpressionContext::theTypeId() {
  return getRuleContext<AngelscriptConsoleParser::TheTypeIdContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::Greater() {
  return getToken(AngelscriptConsoleParser::Greater, 0);
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::PostfixExpressionContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

AngelscriptConsoleParser::PostfixExpressionContext* AngelscriptConsoleParser::PostfixExpressionContext::postfixExpression() {
  return getRuleContext<AngelscriptConsoleParser::PostfixExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::LeftBracket() {
  return getToken(AngelscriptConsoleParser::LeftBracket, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::RightBracket() {
  return getToken(AngelscriptConsoleParser::RightBracket, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::Dot() {
  return getToken(AngelscriptConsoleParser::Dot, 0);
}

AngelscriptConsoleParser::IdExpressionContext* AngelscriptConsoleParser::PostfixExpressionContext::idExpression() {
  return getRuleContext<AngelscriptConsoleParser::IdExpressionContext>(0);
}

AngelscriptConsoleParser::PseudoDestructorNameContext* AngelscriptConsoleParser::PostfixExpressionContext::pseudoDestructorName() {
  return getRuleContext<AngelscriptConsoleParser::PseudoDestructorNameContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::PlusPlus() {
  return getToken(AngelscriptConsoleParser::PlusPlus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::PostfixExpressionContext::MinusMinus() {
  return getToken(AngelscriptConsoleParser::MinusMinus, 0);
}


size_t AngelscriptConsoleParser::PostfixExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RulePostfixExpression;
}


std::any AngelscriptConsoleParser::PostfixExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitPostfixExpression(this);
  else
    return visitor->visitChildren(this);
}


AngelscriptConsoleParser::PostfixExpressionContext* AngelscriptConsoleParser::postfixExpression() {
   return postfixExpression(0);
}

AngelscriptConsoleParser::PostfixExpressionContext* AngelscriptConsoleParser::postfixExpression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  AngelscriptConsoleParser::PostfixExpressionContext *_localctx = _tracker.createInstance<PostfixExpressionContext>(_ctx, parentState);
  AngelscriptConsoleParser::PostfixExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 58;
  enterRecursionRule(_localctx, 58, AngelscriptConsoleParser::RulePostfixExpression, precedence);

    size_t _la = 0;

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
    setState(303);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx)) {
    case 1: {
      setState(272);
      primaryExpression();
      break;
    }

    case 2: {
      setState(273);
      simpleTypeSpecifier();
      setState(277);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(274);
          arraySpecifier(); 
        }
        setState(279);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx);
      }
      setState(286);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::LeftParen: {
          setState(280);
          match(AngelscriptConsoleParser::LeftParen);
          setState(282);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 40954609364983422) != 0)) {
            setState(281);
            expressionList();
          }
          setState(284);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case AngelscriptConsoleParser::LeftBracket:
        case AngelscriptConsoleParser::LeftBrace: {
          setState(285);
          bracedInitList();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      break;
    }

    case 3: {
      setState(288);
      match(AngelscriptConsoleParser::Cast);
      setState(289);
      match(AngelscriptConsoleParser::Less);
      setState(290);
      theTypeId();
      setState(291);
      match(AngelscriptConsoleParser::Greater);
      setState(292);
      match(AngelscriptConsoleParser::LeftParen);
      setState(293);
      expression();
      setState(294);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 4: {
      setState(296);
      match(AngelscriptConsoleParser::LeftParen);
      setState(299);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
      case 1: {
        setState(297);
        expression();
        break;
      }

      case 2: {
        setState(298);
        theTypeId();
        break;
      }

      default:
        break;
      }
      setState(301);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(329);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(327);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(305);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(306);
          match(AngelscriptConsoleParser::LeftBracket);
          setState(309);
          _errHandler->sync(this);
          switch (_input->LA(1)) {
            case AngelscriptConsoleParser::IntegerLiteral:
            case AngelscriptConsoleParser::CharacterLiteral:
            case AngelscriptConsoleParser::FloatingLiteral:
            case AngelscriptConsoleParser::StringLiteral:
            case AngelscriptConsoleParser::UserDefinedLiteral:
            case AngelscriptConsoleParser::Cast:
            case AngelscriptConsoleParser::False_:
            case AngelscriptConsoleParser::Nullptr:
            case AngelscriptConsoleParser::True_:
            case AngelscriptConsoleParser::LeftParen:
            case AngelscriptConsoleParser::Plus:
            case AngelscriptConsoleParser::Minus:
            case AngelscriptConsoleParser::Star:
            case AngelscriptConsoleParser::And:
            case AngelscriptConsoleParser::Or:
            case AngelscriptConsoleParser::Tilde:
            case AngelscriptConsoleParser::Not:
            case AngelscriptConsoleParser::PlusPlus:
            case AngelscriptConsoleParser::MinusMinus:
            case AngelscriptConsoleParser::Doublecolon:
            case AngelscriptConsoleParser::Identifier: {
              setState(307);
              expression();
              break;
            }

            case AngelscriptConsoleParser::LeftBracket:
            case AngelscriptConsoleParser::LeftBrace: {
              setState(308);
              bracedInitList();
              break;
            }

          default:
            throw NoViableAltException(this);
          }
          setState(311);
          match(AngelscriptConsoleParser::RightBracket);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(313);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(314);
          match(AngelscriptConsoleParser::LeftParen);
          setState(316);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 40954609364983422) != 0)) {
            setState(315);
            expressionList();
          }
          setState(318);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(319);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(320);
          match(AngelscriptConsoleParser::Dot);
          setState(323);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
          case 1: {
            setState(321);
            idExpression();
            break;
          }

          case 2: {
            setState(322);
            pseudoDestructorName();
            break;
          }

          default:
            break;
          }
          break;
        }

        case 4: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(325);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(326);
          _la = _input->LA(1);
          if (!(_la == AngelscriptConsoleParser::PlusPlus

          || _la == AngelscriptConsoleParser::MinusMinus)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          break;
        }

        default:
          break;
        } 
      }
      setState(331);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx);
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

AngelscriptConsoleParser::UnaryExpressionContext::UnaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::PostfixExpressionContext* AngelscriptConsoleParser::UnaryExpressionContext::postfixExpression() {
  return getRuleContext<AngelscriptConsoleParser::PostfixExpressionContext>(0);
}

AngelscriptConsoleParser::UnaryExpressionContext* AngelscriptConsoleParser::UnaryExpressionContext::unaryExpression() {
  return getRuleContext<AngelscriptConsoleParser::UnaryExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryExpressionContext::PlusPlus() {
  return getToken(AngelscriptConsoleParser::PlusPlus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryExpressionContext::MinusMinus() {
  return getToken(AngelscriptConsoleParser::MinusMinus, 0);
}

AngelscriptConsoleParser::UnaryOperatorContext* AngelscriptConsoleParser::UnaryExpressionContext::unaryOperator() {
  return getRuleContext<AngelscriptConsoleParser::UnaryOperatorContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryExpressionContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

AngelscriptConsoleParser::TheTypeIdContext* AngelscriptConsoleParser::UnaryExpressionContext::theTypeId() {
  return getRuleContext<AngelscriptConsoleParser::TheTypeIdContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryExpressionContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}


size_t AngelscriptConsoleParser::UnaryExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleUnaryExpression;
}


std::any AngelscriptConsoleParser::UnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::UnaryExpressionContext* AngelscriptConsoleParser::unaryExpression() {
  UnaryExpressionContext *_localctx = _tracker.createInstance<UnaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 60, AngelscriptConsoleParser::RuleUnaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(343);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(332);
      postfixExpression(0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(336);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::PlusPlus: {
          setState(333);
          match(AngelscriptConsoleParser::PlusPlus);
          break;
        }

        case AngelscriptConsoleParser::MinusMinus: {
          setState(334);
          match(AngelscriptConsoleParser::MinusMinus);
          break;
        }

        case AngelscriptConsoleParser::Plus:
        case AngelscriptConsoleParser::Minus:
        case AngelscriptConsoleParser::Star:
        case AngelscriptConsoleParser::And:
        case AngelscriptConsoleParser::Or:
        case AngelscriptConsoleParser::Tilde:
        case AngelscriptConsoleParser::Not: {
          setState(335);
          unaryOperator();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(338);
      unaryExpression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(339);
      match(AngelscriptConsoleParser::LeftParen);
      setState(340);
      theTypeId();
      setState(341);
      match(AngelscriptConsoleParser::RightParen);
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

//----------------- UnaryOperatorContext ------------------------------------------------------------------

AngelscriptConsoleParser::UnaryOperatorContext::UnaryOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryOperatorContext::Or() {
  return getToken(AngelscriptConsoleParser::Or, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryOperatorContext::Star() {
  return getToken(AngelscriptConsoleParser::Star, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryOperatorContext::And() {
  return getToken(AngelscriptConsoleParser::And, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryOperatorContext::Plus() {
  return getToken(AngelscriptConsoleParser::Plus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryOperatorContext::Tilde() {
  return getToken(AngelscriptConsoleParser::Tilde, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryOperatorContext::Minus() {
  return getToken(AngelscriptConsoleParser::Minus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::UnaryOperatorContext::Not() {
  return getToken(AngelscriptConsoleParser::Not, 0);
}


size_t AngelscriptConsoleParser::UnaryOperatorContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleUnaryOperator;
}


std::any AngelscriptConsoleParser::UnaryOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitUnaryOperator(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::UnaryOperatorContext* AngelscriptConsoleParser::unaryOperator() {
  UnaryOperatorContext *_localctx = _tracker.createInstance<UnaryOperatorContext>(_ctx, getState());
  enterRule(_localctx, 62, AngelscriptConsoleParser::RuleUnaryOperator);
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
    setState(345);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 253493248) != 0))) {
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

AngelscriptConsoleParser::CastExpressionContext::CastExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::UnaryExpressionContext* AngelscriptConsoleParser::CastExpressionContext::unaryExpression() {
  return getRuleContext<AngelscriptConsoleParser::UnaryExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::CastExpressionContext::Cast() {
  return getToken(AngelscriptConsoleParser::Cast, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::CastExpressionContext::Less() {
  return getToken(AngelscriptConsoleParser::Less, 0);
}

AngelscriptConsoleParser::TheTypeIdContext* AngelscriptConsoleParser::CastExpressionContext::theTypeId() {
  return getRuleContext<AngelscriptConsoleParser::TheTypeIdContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::CastExpressionContext::Greater() {
  return getToken(AngelscriptConsoleParser::Greater, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::CastExpressionContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

AngelscriptConsoleParser::CastExpressionContext* AngelscriptConsoleParser::CastExpressionContext::castExpression() {
  return getRuleContext<AngelscriptConsoleParser::CastExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::CastExpressionContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}


size_t AngelscriptConsoleParser::CastExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleCastExpression;
}


std::any AngelscriptConsoleParser::CastExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitCastExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::CastExpressionContext* AngelscriptConsoleParser::castExpression() {
  CastExpressionContext *_localctx = _tracker.createInstance<CastExpressionContext>(_ctx, getState());
  enterRule(_localctx, 64, AngelscriptConsoleParser::RuleCastExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(356);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(347);
      unaryExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(348);
      match(AngelscriptConsoleParser::Cast);
      setState(349);
      match(AngelscriptConsoleParser::Less);
      setState(350);
      theTypeId();
      setState(351);
      match(AngelscriptConsoleParser::Greater);
      setState(352);
      match(AngelscriptConsoleParser::LeftParen);
      setState(353);
      castExpression();
      setState(354);
      match(AngelscriptConsoleParser::RightParen);
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

AngelscriptConsoleParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::CastExpressionContext *> AngelscriptConsoleParser::MultiplicativeExpressionContext::castExpression() {
  return getRuleContexts<AngelscriptConsoleParser::CastExpressionContext>();
}

AngelscriptConsoleParser::CastExpressionContext* AngelscriptConsoleParser::MultiplicativeExpressionContext::castExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::CastExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::MultiplicativeExpressionContext::Star() {
  return getTokens(AngelscriptConsoleParser::Star);
}

tree::TerminalNode* AngelscriptConsoleParser::MultiplicativeExpressionContext::Star(size_t i) {
  return getToken(AngelscriptConsoleParser::Star, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::MultiplicativeExpressionContext::Div() {
  return getTokens(AngelscriptConsoleParser::Div);
}

tree::TerminalNode* AngelscriptConsoleParser::MultiplicativeExpressionContext::Div(size_t i) {
  return getToken(AngelscriptConsoleParser::Div, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::MultiplicativeExpressionContext::Mod() {
  return getTokens(AngelscriptConsoleParser::Mod);
}

tree::TerminalNode* AngelscriptConsoleParser::MultiplicativeExpressionContext::Mod(size_t i) {
  return getToken(AngelscriptConsoleParser::Mod, i);
}


size_t AngelscriptConsoleParser::MultiplicativeExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleMultiplicativeExpression;
}


std::any AngelscriptConsoleParser::MultiplicativeExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::MultiplicativeExpressionContext* AngelscriptConsoleParser::multiplicativeExpression() {
  MultiplicativeExpressionContext *_localctx = _tracker.createInstance<MultiplicativeExpressionContext>(_ctx, getState());
  enterRule(_localctx, 66, AngelscriptConsoleParser::RuleMultiplicativeExpression);
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
    setState(358);
    castExpression();
    setState(363);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 7340032) != 0)) {
      setState(359);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 7340032) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(360);
      castExpression();
      setState(365);
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

AngelscriptConsoleParser::AdditiveExpressionContext::AdditiveExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::MultiplicativeExpressionContext *> AngelscriptConsoleParser::AdditiveExpressionContext::multiplicativeExpression() {
  return getRuleContexts<AngelscriptConsoleParser::MultiplicativeExpressionContext>();
}

AngelscriptConsoleParser::MultiplicativeExpressionContext* AngelscriptConsoleParser::AdditiveExpressionContext::multiplicativeExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::MultiplicativeExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::AdditiveExpressionContext::Plus() {
  return getTokens(AngelscriptConsoleParser::Plus);
}

tree::TerminalNode* AngelscriptConsoleParser::AdditiveExpressionContext::Plus(size_t i) {
  return getToken(AngelscriptConsoleParser::Plus, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::AdditiveExpressionContext::Minus() {
  return getTokens(AngelscriptConsoleParser::Minus);
}

tree::TerminalNode* AngelscriptConsoleParser::AdditiveExpressionContext::Minus(size_t i) {
  return getToken(AngelscriptConsoleParser::Minus, i);
}


size_t AngelscriptConsoleParser::AdditiveExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleAdditiveExpression;
}


std::any AngelscriptConsoleParser::AdditiveExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::AdditiveExpressionContext* AngelscriptConsoleParser::additiveExpression() {
  AdditiveExpressionContext *_localctx = _tracker.createInstance<AdditiveExpressionContext>(_ctx, getState());
  enterRule(_localctx, 68, AngelscriptConsoleParser::RuleAdditiveExpression);
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
    multiplicativeExpression();
    setState(371);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Plus

    || _la == AngelscriptConsoleParser::Minus) {
      setState(367);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Plus

      || _la == AngelscriptConsoleParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(368);
      multiplicativeExpression();
      setState(373);
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

AngelscriptConsoleParser::ShiftExpressionContext::ShiftExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::AdditiveExpressionContext *> AngelscriptConsoleParser::ShiftExpressionContext::additiveExpression() {
  return getRuleContexts<AngelscriptConsoleParser::AdditiveExpressionContext>();
}

AngelscriptConsoleParser::AdditiveExpressionContext* AngelscriptConsoleParser::ShiftExpressionContext::additiveExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::AdditiveExpressionContext>(i);
}

std::vector<AngelscriptConsoleParser::ShiftOperatorContext *> AngelscriptConsoleParser::ShiftExpressionContext::shiftOperator() {
  return getRuleContexts<AngelscriptConsoleParser::ShiftOperatorContext>();
}

AngelscriptConsoleParser::ShiftOperatorContext* AngelscriptConsoleParser::ShiftExpressionContext::shiftOperator(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::ShiftOperatorContext>(i);
}


size_t AngelscriptConsoleParser::ShiftExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleShiftExpression;
}


std::any AngelscriptConsoleParser::ShiftExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitShiftExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ShiftExpressionContext* AngelscriptConsoleParser::shiftExpression() {
  ShiftExpressionContext *_localctx = _tracker.createInstance<ShiftExpressionContext>(_ctx, getState());
  enterRule(_localctx, 70, AngelscriptConsoleParser::RuleShiftExpression);

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
    setState(374);
    additiveExpression();
    setState(380);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(375);
        shiftOperator();
        setState(376);
        additiveExpression(); 
      }
      setState(382);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ShiftOperatorContext ------------------------------------------------------------------

AngelscriptConsoleParser::ShiftOperatorContext::ShiftOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::ShiftOperatorContext::Greater() {
  return getTokens(AngelscriptConsoleParser::Greater);
}

tree::TerminalNode* AngelscriptConsoleParser::ShiftOperatorContext::Greater(size_t i) {
  return getToken(AngelscriptConsoleParser::Greater, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::ShiftOperatorContext::Less() {
  return getTokens(AngelscriptConsoleParser::Less);
}

tree::TerminalNode* AngelscriptConsoleParser::ShiftOperatorContext::Less(size_t i) {
  return getToken(AngelscriptConsoleParser::Less, i);
}


size_t AngelscriptConsoleParser::ShiftOperatorContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleShiftOperator;
}


std::any AngelscriptConsoleParser::ShiftOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitShiftOperator(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ShiftOperatorContext* AngelscriptConsoleParser::shiftOperator() {
  ShiftOperatorContext *_localctx = _tracker.createInstance<ShiftOperatorContext>(_ctx, getState());
  enterRule(_localctx, 72, AngelscriptConsoleParser::RuleShiftOperator);

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
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Greater: {
        enterOuterAlt(_localctx, 1);
        setState(383);
        match(AngelscriptConsoleParser::Greater);
        setState(384);
        match(AngelscriptConsoleParser::Greater);
        break;
      }

      case AngelscriptConsoleParser::Less: {
        enterOuterAlt(_localctx, 2);
        setState(385);
        match(AngelscriptConsoleParser::Less);
        setState(386);
        match(AngelscriptConsoleParser::Less);
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

//----------------- RelationalExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::RelationalExpressionContext::RelationalExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::ShiftExpressionContext *> AngelscriptConsoleParser::RelationalExpressionContext::shiftExpression() {
  return getRuleContexts<AngelscriptConsoleParser::ShiftExpressionContext>();
}

AngelscriptConsoleParser::ShiftExpressionContext* AngelscriptConsoleParser::RelationalExpressionContext::shiftExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::ShiftExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::RelationalExpressionContext::Less() {
  return getTokens(AngelscriptConsoleParser::Less);
}

tree::TerminalNode* AngelscriptConsoleParser::RelationalExpressionContext::Less(size_t i) {
  return getToken(AngelscriptConsoleParser::Less, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::RelationalExpressionContext::Greater() {
  return getTokens(AngelscriptConsoleParser::Greater);
}

tree::TerminalNode* AngelscriptConsoleParser::RelationalExpressionContext::Greater(size_t i) {
  return getToken(AngelscriptConsoleParser::Greater, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::RelationalExpressionContext::LessEqual() {
  return getTokens(AngelscriptConsoleParser::LessEqual);
}

tree::TerminalNode* AngelscriptConsoleParser::RelationalExpressionContext::LessEqual(size_t i) {
  return getToken(AngelscriptConsoleParser::LessEqual, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::RelationalExpressionContext::GreaterEqual() {
  return getTokens(AngelscriptConsoleParser::GreaterEqual);
}

tree::TerminalNode* AngelscriptConsoleParser::RelationalExpressionContext::GreaterEqual(size_t i) {
  return getToken(AngelscriptConsoleParser::GreaterEqual, i);
}


size_t AngelscriptConsoleParser::RelationalExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleRelationalExpression;
}


std::any AngelscriptConsoleParser::RelationalExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitRelationalExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::RelationalExpressionContext* AngelscriptConsoleParser::relationalExpression() {
  RelationalExpressionContext *_localctx = _tracker.createInstance<RelationalExpressionContext>(_ctx, getState());
  enterRule(_localctx, 74, AngelscriptConsoleParser::RuleRelationalExpression);
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
    setState(389);
    shiftExpression();
    setState(394);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 26389889679360) != 0)) {
      setState(390);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 26389889679360) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(391);
      shiftExpression();
      setState(396);
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

//----------------- EqualityExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::EqualityExpressionContext::EqualityExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::RelationalExpressionContext *> AngelscriptConsoleParser::EqualityExpressionContext::relationalExpression() {
  return getRuleContexts<AngelscriptConsoleParser::RelationalExpressionContext>();
}

AngelscriptConsoleParser::RelationalExpressionContext* AngelscriptConsoleParser::EqualityExpressionContext::relationalExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::RelationalExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::EqualityExpressionContext::Equal() {
  return getTokens(AngelscriptConsoleParser::Equal);
}

tree::TerminalNode* AngelscriptConsoleParser::EqualityExpressionContext::Equal(size_t i) {
  return getToken(AngelscriptConsoleParser::Equal, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::EqualityExpressionContext::NotEqual() {
  return getTokens(AngelscriptConsoleParser::NotEqual);
}

tree::TerminalNode* AngelscriptConsoleParser::EqualityExpressionContext::NotEqual(size_t i) {
  return getToken(AngelscriptConsoleParser::NotEqual, i);
}


size_t AngelscriptConsoleParser::EqualityExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleEqualityExpression;
}


std::any AngelscriptConsoleParser::EqualityExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitEqualityExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::EqualityExpressionContext* AngelscriptConsoleParser::equalityExpression() {
  EqualityExpressionContext *_localctx = _tracker.createInstance<EqualityExpressionContext>(_ctx, getState());
  enterRule(_localctx, 76, AngelscriptConsoleParser::RuleEqualityExpression);
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
    setState(397);
    relationalExpression();
    setState(402);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Equal

    || _la == AngelscriptConsoleParser::NotEqual) {
      setState(398);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Equal

      || _la == AngelscriptConsoleParser::NotEqual)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(399);
      relationalExpression();
      setState(404);
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

AngelscriptConsoleParser::AndExpressionContext::AndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::EqualityExpressionContext *> AngelscriptConsoleParser::AndExpressionContext::equalityExpression() {
  return getRuleContexts<AngelscriptConsoleParser::EqualityExpressionContext>();
}

AngelscriptConsoleParser::EqualityExpressionContext* AngelscriptConsoleParser::AndExpressionContext::equalityExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::EqualityExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::AndExpressionContext::And() {
  return getTokens(AngelscriptConsoleParser::And);
}

tree::TerminalNode* AngelscriptConsoleParser::AndExpressionContext::And(size_t i) {
  return getToken(AngelscriptConsoleParser::And, i);
}


size_t AngelscriptConsoleParser::AndExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleAndExpression;
}


std::any AngelscriptConsoleParser::AndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitAndExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::AndExpressionContext* AngelscriptConsoleParser::andExpression() {
  AndExpressionContext *_localctx = _tracker.createInstance<AndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 78, AngelscriptConsoleParser::RuleAndExpression);
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
    setState(405);
    equalityExpression();
    setState(410);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::And) {
      setState(406);
      match(AngelscriptConsoleParser::And);
      setState(407);
      equalityExpression();
      setState(412);
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

AngelscriptConsoleParser::ExclusiveOrExpressionContext::ExclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::AndExpressionContext *> AngelscriptConsoleParser::ExclusiveOrExpressionContext::andExpression() {
  return getRuleContexts<AngelscriptConsoleParser::AndExpressionContext>();
}

AngelscriptConsoleParser::AndExpressionContext* AngelscriptConsoleParser::ExclusiveOrExpressionContext::andExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::AndExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::ExclusiveOrExpressionContext::Xor() {
  return getTokens(AngelscriptConsoleParser::Xor);
}

tree::TerminalNode* AngelscriptConsoleParser::ExclusiveOrExpressionContext::Xor(size_t i) {
  return getToken(AngelscriptConsoleParser::Xor, i);
}


size_t AngelscriptConsoleParser::ExclusiveOrExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleExclusiveOrExpression;
}


std::any AngelscriptConsoleParser::ExclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitExclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ExclusiveOrExpressionContext* AngelscriptConsoleParser::exclusiveOrExpression() {
  ExclusiveOrExpressionContext *_localctx = _tracker.createInstance<ExclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 80, AngelscriptConsoleParser::RuleExclusiveOrExpression);
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
    setState(413);
    andExpression();
    setState(418);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Xor) {
      setState(414);
      match(AngelscriptConsoleParser::Xor);
      setState(415);
      andExpression();
      setState(420);
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

AngelscriptConsoleParser::InclusiveOrExpressionContext::InclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::ExclusiveOrExpressionContext *> AngelscriptConsoleParser::InclusiveOrExpressionContext::exclusiveOrExpression() {
  return getRuleContexts<AngelscriptConsoleParser::ExclusiveOrExpressionContext>();
}

AngelscriptConsoleParser::ExclusiveOrExpressionContext* AngelscriptConsoleParser::InclusiveOrExpressionContext::exclusiveOrExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::ExclusiveOrExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::InclusiveOrExpressionContext::Or() {
  return getTokens(AngelscriptConsoleParser::Or);
}

tree::TerminalNode* AngelscriptConsoleParser::InclusiveOrExpressionContext::Or(size_t i) {
  return getToken(AngelscriptConsoleParser::Or, i);
}


size_t AngelscriptConsoleParser::InclusiveOrExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleInclusiveOrExpression;
}


std::any AngelscriptConsoleParser::InclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitInclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::InclusiveOrExpressionContext* AngelscriptConsoleParser::inclusiveOrExpression() {
  InclusiveOrExpressionContext *_localctx = _tracker.createInstance<InclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 82, AngelscriptConsoleParser::RuleInclusiveOrExpression);
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
    setState(421);
    exclusiveOrExpression();
    setState(426);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Or) {
      setState(422);
      match(AngelscriptConsoleParser::Or);
      setState(423);
      exclusiveOrExpression();
      setState(428);
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

//----------------- LogicalAndExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::LogicalAndExpressionContext::LogicalAndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::InclusiveOrExpressionContext *> AngelscriptConsoleParser::LogicalAndExpressionContext::inclusiveOrExpression() {
  return getRuleContexts<AngelscriptConsoleParser::InclusiveOrExpressionContext>();
}

AngelscriptConsoleParser::InclusiveOrExpressionContext* AngelscriptConsoleParser::LogicalAndExpressionContext::inclusiveOrExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::InclusiveOrExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::LogicalAndExpressionContext::AndAnd() {
  return getTokens(AngelscriptConsoleParser::AndAnd);
}

tree::TerminalNode* AngelscriptConsoleParser::LogicalAndExpressionContext::AndAnd(size_t i) {
  return getToken(AngelscriptConsoleParser::AndAnd, i);
}


size_t AngelscriptConsoleParser::LogicalAndExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLogicalAndExpression;
}


std::any AngelscriptConsoleParser::LogicalAndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLogicalAndExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LogicalAndExpressionContext* AngelscriptConsoleParser::logicalAndExpression() {
  LogicalAndExpressionContext *_localctx = _tracker.createInstance<LogicalAndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 84, AngelscriptConsoleParser::RuleLogicalAndExpression);
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
    setState(429);
    inclusiveOrExpression();
    setState(434);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::AndAnd) {
      setState(430);
      match(AngelscriptConsoleParser::AndAnd);
      setState(431);
      inclusiveOrExpression();
      setState(436);
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

//----------------- LogicalOrExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::LogicalOrExpressionContext::LogicalOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::LogicalAndExpressionContext *> AngelscriptConsoleParser::LogicalOrExpressionContext::logicalAndExpression() {
  return getRuleContexts<AngelscriptConsoleParser::LogicalAndExpressionContext>();
}

AngelscriptConsoleParser::LogicalAndExpressionContext* AngelscriptConsoleParser::LogicalOrExpressionContext::logicalAndExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::LogicalAndExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::LogicalOrExpressionContext::OrOr() {
  return getTokens(AngelscriptConsoleParser::OrOr);
}

tree::TerminalNode* AngelscriptConsoleParser::LogicalOrExpressionContext::OrOr(size_t i) {
  return getToken(AngelscriptConsoleParser::OrOr, i);
}


size_t AngelscriptConsoleParser::LogicalOrExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLogicalOrExpression;
}


std::any AngelscriptConsoleParser::LogicalOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLogicalOrExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LogicalOrExpressionContext* AngelscriptConsoleParser::logicalOrExpression() {
  LogicalOrExpressionContext *_localctx = _tracker.createInstance<LogicalOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 86, AngelscriptConsoleParser::RuleLogicalOrExpression);
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
    setState(437);
    logicalAndExpression();
    setState(442);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::OrOr) {
      setState(438);
      match(AngelscriptConsoleParser::OrOr);
      setState(439);
      logicalAndExpression();
      setState(444);
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

//----------------- ConditionalExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::ConditionalExpressionContext::ConditionalExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::LogicalOrExpressionContext* AngelscriptConsoleParser::ConditionalExpressionContext::logicalOrExpression() {
  return getRuleContext<AngelscriptConsoleParser::LogicalOrExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::ConditionalExpressionContext::Question() {
  return getToken(AngelscriptConsoleParser::Question, 0);
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::ConditionalExpressionContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::ConditionalExpressionContext::Colon() {
  return getToken(AngelscriptConsoleParser::Colon, 0);
}

AngelscriptConsoleParser::AssignmentExpressionContext* AngelscriptConsoleParser::ConditionalExpressionContext::assignmentExpression() {
  return getRuleContext<AngelscriptConsoleParser::AssignmentExpressionContext>(0);
}


size_t AngelscriptConsoleParser::ConditionalExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleConditionalExpression;
}


std::any AngelscriptConsoleParser::ConditionalExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitConditionalExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ConditionalExpressionContext* AngelscriptConsoleParser::conditionalExpression() {
  ConditionalExpressionContext *_localctx = _tracker.createInstance<ConditionalExpressionContext>(_ctx, getState());
  enterRule(_localctx, 88, AngelscriptConsoleParser::RuleConditionalExpression);
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
    setState(445);
    logicalOrExpression();
    setState(451);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Question) {
      setState(446);
      match(AngelscriptConsoleParser::Question);
      setState(447);
      expression();
      setState(448);
      match(AngelscriptConsoleParser::Colon);
      setState(449);
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

//----------------- ExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::AssignmentExpressionContext *> AngelscriptConsoleParser::ExpressionContext::assignmentExpression() {
  return getRuleContexts<AngelscriptConsoleParser::AssignmentExpressionContext>();
}

AngelscriptConsoleParser::AssignmentExpressionContext* AngelscriptConsoleParser::ExpressionContext::assignmentExpression(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::AssignmentExpressionContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::ExpressionContext::Comma() {
  return getTokens(AngelscriptConsoleParser::Comma);
}

tree::TerminalNode* AngelscriptConsoleParser::ExpressionContext::Comma(size_t i) {
  return getToken(AngelscriptConsoleParser::Comma, i);
}


size_t AngelscriptConsoleParser::ExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleExpression;
}


std::any AngelscriptConsoleParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::expression() {
  ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, getState());
  enterRule(_localctx, 90, AngelscriptConsoleParser::RuleExpression);
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
    setState(453);
    assignmentExpression();
    setState(458);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(454);
      match(AngelscriptConsoleParser::Comma);
      setState(455);
      assignmentExpression();
      setState(460);
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

//----------------- AssignmentExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::AssignmentExpressionContext::AssignmentExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::ConditionalExpressionContext* AngelscriptConsoleParser::AssignmentExpressionContext::conditionalExpression() {
  return getRuleContext<AngelscriptConsoleParser::ConditionalExpressionContext>(0);
}

AngelscriptConsoleParser::LogicalOrExpressionContext* AngelscriptConsoleParser::AssignmentExpressionContext::logicalOrExpression() {
  return getRuleContext<AngelscriptConsoleParser::LogicalOrExpressionContext>(0);
}

AngelscriptConsoleParser::AssignmentOperatorContext* AngelscriptConsoleParser::AssignmentExpressionContext::assignmentOperator() {
  return getRuleContext<AngelscriptConsoleParser::AssignmentOperatorContext>(0);
}

AngelscriptConsoleParser::InitializerClauseContext* AngelscriptConsoleParser::AssignmentExpressionContext::initializerClause() {
  return getRuleContext<AngelscriptConsoleParser::InitializerClauseContext>(0);
}


size_t AngelscriptConsoleParser::AssignmentExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleAssignmentExpression;
}


std::any AngelscriptConsoleParser::AssignmentExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitAssignmentExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::AssignmentExpressionContext* AngelscriptConsoleParser::assignmentExpression() {
  AssignmentExpressionContext *_localctx = _tracker.createInstance<AssignmentExpressionContext>(_ctx, getState());
  enterRule(_localctx, 92, AngelscriptConsoleParser::RuleAssignmentExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(466);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(461);
      conditionalExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(462);
      logicalOrExpression();
      setState(463);
      assignmentOperator();
      setState(464);
      initializerClause();
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

//----------------- AssignmentOperatorContext ------------------------------------------------------------------

AngelscriptConsoleParser::AssignmentOperatorContext::AssignmentOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::Assign() {
  return getToken(AngelscriptConsoleParser::Assign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::StarAssign() {
  return getToken(AngelscriptConsoleParser::StarAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::DivAssign() {
  return getToken(AngelscriptConsoleParser::DivAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::ModAssign() {
  return getToken(AngelscriptConsoleParser::ModAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::PlusAssign() {
  return getToken(AngelscriptConsoleParser::PlusAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::MinusAssign() {
  return getToken(AngelscriptConsoleParser::MinusAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::RightShiftAssign() {
  return getToken(AngelscriptConsoleParser::RightShiftAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::LeftShiftAssign() {
  return getToken(AngelscriptConsoleParser::LeftShiftAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::AndAssign() {
  return getToken(AngelscriptConsoleParser::AndAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::XorAssign() {
  return getToken(AngelscriptConsoleParser::XorAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AssignmentOperatorContext::OrAssign() {
  return getToken(AngelscriptConsoleParser::OrAssign, 0);
}


size_t AngelscriptConsoleParser::AssignmentOperatorContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleAssignmentOperator;
}


std::any AngelscriptConsoleParser::AssignmentOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitAssignmentOperator(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::AssignmentOperatorContext* AngelscriptConsoleParser::assignmentOperator() {
  AssignmentOperatorContext *_localctx = _tracker.createInstance<AssignmentOperatorContext>(_ctx, getState());
  enterRule(_localctx, 94, AngelscriptConsoleParser::RuleAssignmentOperator);
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
    setState(468);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2197144207360) != 0))) {
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

//----------------- BraceOrEqualInitializerContext ------------------------------------------------------------------

AngelscriptConsoleParser::BraceOrEqualInitializerContext::BraceOrEqualInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::BraceOrEqualInitializerContext::Assign() {
  return getToken(AngelscriptConsoleParser::Assign, 0);
}

AngelscriptConsoleParser::InitializerClauseContext* AngelscriptConsoleParser::BraceOrEqualInitializerContext::initializerClause() {
  return getRuleContext<AngelscriptConsoleParser::InitializerClauseContext>(0);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::BraceOrEqualInitializerContext::bracedInitList() {
  return getRuleContext<AngelscriptConsoleParser::BracedInitListContext>(0);
}


size_t AngelscriptConsoleParser::BraceOrEqualInitializerContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleBraceOrEqualInitializer;
}


std::any AngelscriptConsoleParser::BraceOrEqualInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitBraceOrEqualInitializer(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::BraceOrEqualInitializerContext* AngelscriptConsoleParser::braceOrEqualInitializer() {
  BraceOrEqualInitializerContext *_localctx = _tracker.createInstance<BraceOrEqualInitializerContext>(_ctx, getState());
  enterRule(_localctx, 96, AngelscriptConsoleParser::RuleBraceOrEqualInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(473);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(470);
        match(AngelscriptConsoleParser::Assign);
        setState(471);
        initializerClause();
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(472);
        bracedInitList();
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

//----------------- InitializerClauseContext ------------------------------------------------------------------

AngelscriptConsoleParser::InitializerClauseContext::InitializerClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::AssignmentExpressionContext* AngelscriptConsoleParser::InitializerClauseContext::assignmentExpression() {
  return getRuleContext<AngelscriptConsoleParser::AssignmentExpressionContext>(0);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::InitializerClauseContext::bracedInitList() {
  return getRuleContext<AngelscriptConsoleParser::BracedInitListContext>(0);
}


size_t AngelscriptConsoleParser::InitializerClauseContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleInitializerClause;
}


std::any AngelscriptConsoleParser::InitializerClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitInitializerClause(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::InitializerClauseContext* AngelscriptConsoleParser::initializerClause() {
  InitializerClauseContext *_localctx = _tracker.createInstance<InitializerClauseContext>(_ctx, getState());
  enterRule(_localctx, 98, AngelscriptConsoleParser::RuleInitializerClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(477);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::IntegerLiteral:
      case AngelscriptConsoleParser::CharacterLiteral:
      case AngelscriptConsoleParser::FloatingLiteral:
      case AngelscriptConsoleParser::StringLiteral:
      case AngelscriptConsoleParser::UserDefinedLiteral:
      case AngelscriptConsoleParser::Cast:
      case AngelscriptConsoleParser::False_:
      case AngelscriptConsoleParser::Nullptr:
      case AngelscriptConsoleParser::True_:
      case AngelscriptConsoleParser::LeftParen:
      case AngelscriptConsoleParser::Plus:
      case AngelscriptConsoleParser::Minus:
      case AngelscriptConsoleParser::Star:
      case AngelscriptConsoleParser::And:
      case AngelscriptConsoleParser::Or:
      case AngelscriptConsoleParser::Tilde:
      case AngelscriptConsoleParser::Not:
      case AngelscriptConsoleParser::PlusPlus:
      case AngelscriptConsoleParser::MinusMinus:
      case AngelscriptConsoleParser::Doublecolon:
      case AngelscriptConsoleParser::Identifier: {
        enterOuterAlt(_localctx, 1);
        setState(475);
        assignmentExpression();
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(476);
        bracedInitList();
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

//----------------- InitializerListContext ------------------------------------------------------------------

AngelscriptConsoleParser::InitializerListContext::InitializerListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::InitializerClauseContext *> AngelscriptConsoleParser::InitializerListContext::initializerClause() {
  return getRuleContexts<AngelscriptConsoleParser::InitializerClauseContext>();
}

AngelscriptConsoleParser::InitializerClauseContext* AngelscriptConsoleParser::InitializerListContext::initializerClause(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::InitializerClauseContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::InitializerListContext::Comma() {
  return getTokens(AngelscriptConsoleParser::Comma);
}

tree::TerminalNode* AngelscriptConsoleParser::InitializerListContext::Comma(size_t i) {
  return getToken(AngelscriptConsoleParser::Comma, i);
}


size_t AngelscriptConsoleParser::InitializerListContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleInitializerList;
}


std::any AngelscriptConsoleParser::InitializerListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitInitializerList(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::InitializerListContext* AngelscriptConsoleParser::initializerList() {
  InitializerListContext *_localctx = _tracker.createInstance<InitializerListContext>(_ctx, getState());
  enterRule(_localctx, 100, AngelscriptConsoleParser::RuleInitializerList);

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
    setState(479);
    initializerClause();
    setState(484);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(480);
        match(AngelscriptConsoleParser::Comma);
        setState(481);
        initializerClause(); 
      }
      setState(486);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx);
    }
    setState(488);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx)) {
    case 1: {
      setState(487);
      match(AngelscriptConsoleParser::Comma);
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

//----------------- BracedInitListContext ------------------------------------------------------------------

AngelscriptConsoleParser::BracedInitListContext::BracedInitListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::BracedInitListContext::LeftBrace() {
  return getToken(AngelscriptConsoleParser::LeftBrace, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::BracedInitListContext::LeftBracket() {
  return getToken(AngelscriptConsoleParser::LeftBracket, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::BracedInitListContext::RightBrace() {
  return getToken(AngelscriptConsoleParser::RightBrace, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::BracedInitListContext::RightBracket() {
  return getToken(AngelscriptConsoleParser::RightBracket, 0);
}

AngelscriptConsoleParser::InitializerListContext* AngelscriptConsoleParser::BracedInitListContext::initializerList() {
  return getRuleContext<AngelscriptConsoleParser::InitializerListContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::BracedInitListContext::Comma() {
  return getToken(AngelscriptConsoleParser::Comma, 0);
}


size_t AngelscriptConsoleParser::BracedInitListContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleBracedInitList;
}


std::any AngelscriptConsoleParser::BracedInitListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitBracedInitList(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::bracedInitList() {
  BracedInitListContext *_localctx = _tracker.createInstance<BracedInitListContext>(_ctx, getState());
  enterRule(_localctx, 102, AngelscriptConsoleParser::RuleBracedInitList);
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
    setState(490);
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::LeftBracket

    || _la == AngelscriptConsoleParser::LeftBrace)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(495);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 40954609364983422) != 0)) {
      setState(491);
      initializerList();
      setState(493);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Comma) {
        setState(492);
        match(AngelscriptConsoleParser::Comma);
      }
    }
    setState(497);
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::RightBracket

    || _la == AngelscriptConsoleParser::RightBrace)) {
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

//----------------- LiteralContext ------------------------------------------------------------------

AngelscriptConsoleParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::LiteralContext::IntegerLiteral() {
  return getToken(AngelscriptConsoleParser::IntegerLiteral, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::LiteralContext::CharacterLiteral() {
  return getToken(AngelscriptConsoleParser::CharacterLiteral, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::LiteralContext::FloatingLiteral() {
  return getToken(AngelscriptConsoleParser::FloatingLiteral, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::LiteralContext::StringLiteral() {
  return getToken(AngelscriptConsoleParser::StringLiteral, 0);
}

AngelscriptConsoleParser::BooleanLiteralContext* AngelscriptConsoleParser::LiteralContext::booleanLiteral() {
  return getRuleContext<AngelscriptConsoleParser::BooleanLiteralContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::LiteralContext::UserDefinedLiteral() {
  return getToken(AngelscriptConsoleParser::UserDefinedLiteral, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::LiteralContext::Nullptr() {
  return getToken(AngelscriptConsoleParser::Nullptr, 0);
}


size_t AngelscriptConsoleParser::LiteralContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLiteral;
}


std::any AngelscriptConsoleParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LiteralContext* AngelscriptConsoleParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 104, AngelscriptConsoleParser::RuleLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(506);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::IntegerLiteral: {
        enterOuterAlt(_localctx, 1);
        setState(499);
        match(AngelscriptConsoleParser::IntegerLiteral);
        break;
      }

      case AngelscriptConsoleParser::CharacterLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(500);
        match(AngelscriptConsoleParser::CharacterLiteral);
        break;
      }

      case AngelscriptConsoleParser::FloatingLiteral: {
        enterOuterAlt(_localctx, 3);
        setState(501);
        match(AngelscriptConsoleParser::FloatingLiteral);
        break;
      }

      case AngelscriptConsoleParser::StringLiteral: {
        enterOuterAlt(_localctx, 4);
        setState(502);
        match(AngelscriptConsoleParser::StringLiteral);
        break;
      }

      case AngelscriptConsoleParser::False_:
      case AngelscriptConsoleParser::True_: {
        enterOuterAlt(_localctx, 5);
        setState(503);
        booleanLiteral();
        break;
      }

      case AngelscriptConsoleParser::UserDefinedLiteral: {
        enterOuterAlt(_localctx, 6);
        setState(504);
        match(AngelscriptConsoleParser::UserDefinedLiteral);
        break;
      }

      case AngelscriptConsoleParser::Nullptr: {
        enterOuterAlt(_localctx, 7);
        setState(505);
        match(AngelscriptConsoleParser::Nullptr);
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

//----------------- BooleanLiteralContext ------------------------------------------------------------------

AngelscriptConsoleParser::BooleanLiteralContext::BooleanLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::BooleanLiteralContext::False_() {
  return getToken(AngelscriptConsoleParser::False_, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::BooleanLiteralContext::True_() {
  return getToken(AngelscriptConsoleParser::True_, 0);
}


size_t AngelscriptConsoleParser::BooleanLiteralContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleBooleanLiteral;
}


std::any AngelscriptConsoleParser::BooleanLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitBooleanLiteral(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::BooleanLiteralContext* AngelscriptConsoleParser::booleanLiteral() {
  BooleanLiteralContext *_localctx = _tracker.createInstance<BooleanLiteralContext>(_ctx, getState());
  enterRule(_localctx, 106, AngelscriptConsoleParser::RuleBooleanLiteral);
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
    setState(508);
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::False_

    || _la == AngelscriptConsoleParser::True_)) {
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

bool AngelscriptConsoleParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 20: return nestedNameSpecifierSempred(antlrcpp::downCast<NestedNameSpecifierContext *>(context), predicateIndex);
    case 29: return postfixExpressionSempred(antlrcpp::downCast<PostfixExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool AngelscriptConsoleParser::nestedNameSpecifierSempred(NestedNameSpecifierContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool AngelscriptConsoleParser::postfixExpressionSempred(PostfixExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 1: return precpred(_ctx, 7);
    case 2: return precpred(_ctx, 6);
    case 3: return precpred(_ctx, 4);
    case 4: return precpred(_ctx, 3);

  default:
    break;
  }
  return true;
}

void AngelscriptConsoleParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  angelscriptconsoleparserParserInitialize();
#else
  ::antlr4::internal::call_once(angelscriptconsoleparserParserOnceFlag, angelscriptconsoleparserParserInitialize);
#endif
}
