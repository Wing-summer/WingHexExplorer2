
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
      "script", "scriptItem", "asGeneric", "simpleTypeSpecifierList", "booleanLiteral", 
      "primaryExpression", "idExpression", "unqualifiedId", "qualifiedId", 
      "nestedNameSpecifier", "lambdaExpression", "lambdaIntroducer", "lambdaCapture", 
      "captureDefault", "captureList", "capture", "simpleCapture", "initcapture", 
      "lambdaDeclarator", "postfixExpression", "expressionList", "pseudoDestructorName", 
      "unaryExpression", "unaryOperator", "newPlacement", "newInitializer_", 
      "castExpression", "multiplicativeExpression", "additiveExpression", 
      "shiftExpression", "shiftOperator", "relationalExpression", "equalityExpression", 
      "andExpression", "exclusiveOrExpression", "inclusiveOrExpression", 
      "logicalAndExpression", "logicalOrExpression", "conditionalExpression", 
      "assignmentExpression", "assignmentOperator", "expression", "constantExpression", 
      "statement", "labeledStatement", "expressionStatement", "compoundStatement", 
      "statementSeq", "selectionStatement", "condition", "iterationStatement", 
      "forInitStatement", "forRangeDeclaration", "forRangeInitializer", 
      "jumpStatement", "declarationseq", "declaration", "aliasDeclaration", 
      "simpleDeclaration", "emptyDeclaration_", "declSpecifier", "declSpecifierSeq", 
      "typedefName", "typeSpecifier", "trailingTypeSpecifier", "typeSpecifierSeq", 
      "trailingTypeSpecifierSeq", "simpleTypeSpecifier", "theTypeName", 
      "decltypeSpecifier", "namespaceName", "qualifiednamespacespecifier", 
      "balancedTokenSeq", "balancedtoken", "initDeclaratorList", "initDeclarator", 
      "declarator", "declaratorDef", "parametersAndQualifiers", "refqualifier", 
      "declaratorid", "theTypeId", "parameterDeclarationClause", "parameterDeclarationList", 
      "parameterDeclaration", "initializer", "braceOrEqualInitializer", 
      "initializerClause", "initializerList", "bracedInitList", "theOperator", 
      "literal"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "'cast'", "'int'", "'int8'", "'int16'", "'int32'", 
      "'int64'", "'uint'", "'uint8'", "'uint16'", "'uint32'", "'uint64'", 
      "'float'", "'double'", "'bool'", "'auto'", "'break'", "'case'", "'catch'", 
      "'char'", "'const'", "'continue'", "'default'", "'do'", "'else'", 
      "'false'", "'for'", "'if'", "'null'", "'return'", "'short'", "'switch'", 
      "'true'", "'void'", "'while'", "'('", "')'", "'['", "']'", "'{'", 
      "'}'", "'+'", "'-'", "'*'", "'/'", "'%'", "'^'", "'&'", "'|'", "'~'", 
      "'!'", "'='", "'<'", "'>'", "'+='", "'-='", "'*='", "'/='", "'%='", 
      "'^='", "'&='", "'|='", "'<<='", "'>>='", "'=='", "'!='", "'<='", 
      "'>='", "'&&'", "'||'", "'++'", "'--'", "','", "'\\u003F'", "':'", 
      "'::'", "';'", "'.'"
    },
    std::vector<std::string>{
      "", "IntegerLiteral", "CharacterLiteral", "FloatingLiteral", "StringLiteral", 
      "UserDefinedLiteral", "Cast", "Int", "Int8", "Int16", "Int32", "Int64", 
      "UInt", "UInt8", "UInt16", "UInt32", "UInt64", "Float", "Double", 
      "Bool", "Auto", "Break", "Case", "Catch", "Char", "Const", "Continue", 
      "Default", "Do", "Else", "False_", "For", "If", "Nullptr", "Return", 
      "Short", "Switch", "True_", "Void", "While", "LeftParen", "RightParen", 
      "LeftBracket", "RightBracket", "LeftBrace", "RightBrace", "Plus", 
      "Minus", "Star", "Div", "Mod", "Xor", "And", "Or", "Tilde", "Not", 
      "Assign", "Less", "Greater", "PlusAssign", "MinusAssign", "StarAssign", 
      "DivAssign", "ModAssign", "XorAssign", "AndAssign", "OrAssign", "LeftShiftAssign", 
      "RightShiftAssign", "Equal", "NotEqual", "LessEqual", "GreaterEqual", 
      "AndAnd", "OrOr", "PlusPlus", "MinusMinus", "Comma", "Question", "Colon", 
      "Doublecolon", "Semi", "Dot", "Identifier", "DecimalLiteral", "OctalLiteral", 
      "HexadecimalLiteral", "BinaryLiteral", "Integersuffix", "UserDefinedIntegerLiteral", 
      "UserDefinedFloatingLiteral", "UserDefinedStringLiteral", "UserDefinedCharacterLiteral", 
      "Whitespace", "Newline", "BlockComment", "LineComment"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,96,907,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,7,
  	56,2,57,7,57,2,58,7,58,2,59,7,59,2,60,7,60,2,61,7,61,2,62,7,62,2,63,7,
  	63,2,64,7,64,2,65,7,65,2,66,7,66,2,67,7,67,2,68,7,68,2,69,7,69,2,70,7,
  	70,2,71,7,71,2,72,7,72,2,73,7,73,2,74,7,74,2,75,7,75,2,76,7,76,2,77,7,
  	77,2,78,7,78,2,79,7,79,2,80,7,80,2,81,7,81,2,82,7,82,2,83,7,83,2,84,7,
  	84,2,85,7,85,2,86,7,86,2,87,7,87,2,88,7,88,2,89,7,89,2,90,7,90,2,91,7,
  	91,1,0,5,0,186,8,0,10,0,12,0,189,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,3,1,
  	198,8,1,1,2,1,2,1,2,1,2,1,2,1,3,1,3,1,3,5,3,208,8,3,10,3,12,3,211,9,3,
  	1,4,1,4,1,5,4,5,216,8,5,11,5,12,5,217,1,5,1,5,1,5,1,5,1,5,1,5,3,5,226,
  	8,5,1,6,1,6,3,6,230,8,6,1,7,1,7,1,8,1,8,1,8,1,9,1,9,1,9,1,9,3,9,241,8,
  	9,1,9,1,9,1,9,1,9,1,9,5,9,248,8,9,10,9,12,9,251,9,9,1,10,1,10,3,10,255,
  	8,10,1,10,1,10,1,11,1,11,3,11,261,8,11,1,11,1,11,1,12,1,12,1,12,1,12,
  	3,12,269,8,12,3,12,271,8,12,1,13,1,13,1,14,1,14,1,14,5,14,278,8,14,10,
  	14,12,14,281,9,14,1,15,1,15,3,15,285,8,15,1,16,3,16,288,8,16,1,16,1,16,
  	1,17,3,17,293,8,17,1,17,1,17,1,17,1,18,1,18,3,18,300,8,18,1,18,1,18,1,
  	19,1,19,1,19,1,19,1,19,3,19,309,8,19,1,19,1,19,3,19,313,8,19,1,19,1,19,
  	1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,3,19,326,8,19,1,19,1,19,
  	3,19,330,8,19,1,19,1,19,1,19,1,19,3,19,336,8,19,1,19,1,19,1,19,1,19,1,
  	19,3,19,343,8,19,1,19,1,19,1,19,1,19,1,19,3,19,350,8,19,1,19,1,19,5,19,
  	354,8,19,10,19,12,19,357,9,19,1,20,1,20,1,21,3,21,362,8,21,1,21,1,21,
  	1,21,3,21,367,8,21,1,21,1,21,1,21,1,21,1,21,1,21,1,21,1,21,1,21,3,21,
  	378,8,21,1,22,1,22,1,22,1,22,3,22,384,8,22,1,22,1,22,1,22,1,22,1,22,3,
  	22,391,8,22,1,23,1,23,1,24,1,24,1,24,1,24,1,25,1,25,3,25,401,8,25,1,25,
  	1,25,3,25,405,8,25,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,3,26,
  	416,8,26,1,27,1,27,1,27,5,27,421,8,27,10,27,12,27,424,9,27,1,28,1,28,
  	1,28,5,28,429,8,28,10,28,12,28,432,9,28,1,29,1,29,1,29,1,29,5,29,438,
  	8,29,10,29,12,29,441,9,29,1,30,1,30,1,30,1,30,3,30,447,8,30,1,31,1,31,
  	1,31,5,31,452,8,31,10,31,12,31,455,9,31,1,32,1,32,1,32,5,32,460,8,32,
  	10,32,12,32,463,9,32,1,33,1,33,1,33,5,33,468,8,33,10,33,12,33,471,9,33,
  	1,34,1,34,1,34,5,34,476,8,34,10,34,12,34,479,9,34,1,35,1,35,1,35,5,35,
  	484,8,35,10,35,12,35,487,9,35,1,36,1,36,1,36,5,36,492,8,36,10,36,12,36,
  	495,9,36,1,37,1,37,1,37,5,37,500,8,37,10,37,12,37,503,9,37,1,38,1,38,
  	1,38,1,38,1,38,1,38,3,38,511,8,38,1,39,1,39,1,39,1,39,1,39,3,39,518,8,
  	39,1,40,1,40,1,41,1,41,1,41,5,41,525,8,41,10,41,12,41,528,9,41,1,42,1,
  	42,1,43,1,43,1,43,1,43,1,43,1,43,1,43,3,43,539,8,43,1,44,1,44,1,44,1,
  	44,3,44,545,8,44,1,44,1,44,1,44,1,45,3,45,551,8,45,1,45,1,45,1,46,1,46,
  	3,46,557,8,46,1,46,1,46,1,47,4,47,562,8,47,11,47,12,47,563,1,48,1,48,
  	1,48,1,48,1,48,1,48,1,48,3,48,573,8,48,1,48,1,48,1,48,1,48,1,48,1,48,
  	3,48,581,8,48,1,49,1,49,1,49,1,49,1,49,1,49,3,49,589,8,49,3,49,591,8,
  	49,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,
  	50,1,50,1,50,1,50,1,50,3,50,611,8,50,1,50,1,50,3,50,615,8,50,1,50,1,50,
  	1,50,1,50,3,50,621,8,50,1,50,1,50,1,50,3,50,626,8,50,1,51,1,51,3,51,630,
  	8,51,1,52,1,52,1,52,1,53,1,53,3,53,637,8,53,1,54,1,54,1,54,1,55,4,55,
  	643,8,55,11,55,12,55,644,1,56,1,56,3,56,649,8,56,1,57,1,57,1,57,1,57,
  	1,57,1,58,1,58,1,58,3,58,659,8,58,1,58,1,58,1,59,1,59,1,60,1,60,1,61,
  	4,61,668,8,61,11,61,12,61,669,1,62,1,62,1,63,1,63,1,64,1,64,3,64,678,
  	8,64,1,65,4,65,681,8,65,11,65,12,65,682,1,66,4,66,686,8,66,11,66,12,66,
  	687,1,67,3,67,691,8,67,1,67,1,67,1,67,1,67,1,67,1,67,1,67,1,67,1,67,1,
  	67,1,67,1,67,1,67,1,67,1,67,1,67,1,67,1,67,3,67,711,8,67,1,68,1,68,1,
  	69,1,69,1,69,3,69,718,8,69,1,69,1,69,1,70,1,70,1,71,3,71,725,8,71,1,71,
  	1,71,1,72,4,72,730,8,72,11,72,12,72,731,1,73,1,73,1,73,1,73,1,73,1,73,
  	1,73,1,73,1,73,1,73,1,73,1,73,1,73,4,73,747,8,73,11,73,12,73,748,3,73,
  	751,8,73,1,74,1,74,1,74,5,74,756,8,74,10,74,12,74,759,9,74,1,75,1,75,
  	3,75,763,8,75,1,76,1,76,1,76,1,77,1,77,1,77,1,77,1,77,1,77,1,77,3,77,
  	775,8,77,1,77,3,77,778,8,77,5,77,780,8,77,10,77,12,77,783,9,77,1,78,1,
  	78,3,78,787,8,78,1,78,1,78,3,78,791,8,78,1,78,3,78,794,8,78,1,79,1,79,
  	1,80,1,80,1,81,1,81,1,82,1,82,1,83,1,83,1,83,5,83,807,8,83,10,83,12,83,
  	810,9,83,1,84,1,84,3,84,814,8,84,1,84,1,84,3,84,818,8,84,1,85,1,85,1,
  	85,1,85,1,85,3,85,825,8,85,1,86,1,86,1,86,3,86,830,8,86,1,87,1,87,3,87,
  	834,8,87,1,88,1,88,1,88,5,88,839,8,88,10,88,12,88,842,9,88,1,88,3,88,
  	845,8,88,1,89,1,89,1,89,3,89,850,8,89,3,89,852,8,89,1,89,1,89,1,90,1,
  	90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,
  	90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,
  	90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,1,90,3,90,896,8,90,1,
  	91,1,91,1,91,1,91,1,91,1,91,1,91,3,91,905,8,91,1,91,0,3,18,38,154,92,
  	0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,
  	50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,
  	96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,
  	132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,
  	168,170,172,174,176,178,180,182,0,14,2,0,30,30,37,37,2,0,52,52,56,56,
  	1,0,75,76,2,0,46,48,52,55,1,0,48,50,1,0,46,47,2,0,57,58,71,72,1,0,69,
  	70,2,0,56,56,59,68,3,0,21,21,26,26,34,34,1,0,40,45,2,0,52,52,73,73,2,
  	0,42,42,44,44,2,0,43,43,45,45,987,0,187,1,0,0,0,2,197,1,0,0,0,4,199,1,
  	0,0,0,6,204,1,0,0,0,8,212,1,0,0,0,10,225,1,0,0,0,12,229,1,0,0,0,14,231,
  	1,0,0,0,16,233,1,0,0,0,18,236,1,0,0,0,20,252,1,0,0,0,22,258,1,0,0,0,24,
  	270,1,0,0,0,26,272,1,0,0,0,28,274,1,0,0,0,30,284,1,0,0,0,32,287,1,0,0,
  	0,34,292,1,0,0,0,36,297,1,0,0,0,38,329,1,0,0,0,40,358,1,0,0,0,42,377,
  	1,0,0,0,44,390,1,0,0,0,46,392,1,0,0,0,48,394,1,0,0,0,50,404,1,0,0,0,52,
  	415,1,0,0,0,54,417,1,0,0,0,56,425,1,0,0,0,58,433,1,0,0,0,60,446,1,0,0,
  	0,62,448,1,0,0,0,64,456,1,0,0,0,66,464,1,0,0,0,68,472,1,0,0,0,70,480,
  	1,0,0,0,72,488,1,0,0,0,74,496,1,0,0,0,76,504,1,0,0,0,78,517,1,0,0,0,80,
  	519,1,0,0,0,82,521,1,0,0,0,84,529,1,0,0,0,86,538,1,0,0,0,88,544,1,0,0,
  	0,90,550,1,0,0,0,92,554,1,0,0,0,94,561,1,0,0,0,96,580,1,0,0,0,98,590,
  	1,0,0,0,100,625,1,0,0,0,102,629,1,0,0,0,104,631,1,0,0,0,106,636,1,0,0,
  	0,108,638,1,0,0,0,110,642,1,0,0,0,112,648,1,0,0,0,114,650,1,0,0,0,116,
  	655,1,0,0,0,118,662,1,0,0,0,120,664,1,0,0,0,122,667,1,0,0,0,124,671,1,
  	0,0,0,126,673,1,0,0,0,128,677,1,0,0,0,130,680,1,0,0,0,132,685,1,0,0,0,
  	134,710,1,0,0,0,136,712,1,0,0,0,138,714,1,0,0,0,140,721,1,0,0,0,142,724,
  	1,0,0,0,144,729,1,0,0,0,146,750,1,0,0,0,148,752,1,0,0,0,150,760,1,0,0,
  	0,152,764,1,0,0,0,154,767,1,0,0,0,156,784,1,0,0,0,158,795,1,0,0,0,160,
  	797,1,0,0,0,162,799,1,0,0,0,164,801,1,0,0,0,166,803,1,0,0,0,168,811,1,
  	0,0,0,170,824,1,0,0,0,172,829,1,0,0,0,174,833,1,0,0,0,176,835,1,0,0,0,
  	178,846,1,0,0,0,180,895,1,0,0,0,182,904,1,0,0,0,184,186,3,2,1,0,185,184,
  	1,0,0,0,186,189,1,0,0,0,187,185,1,0,0,0,187,188,1,0,0,0,188,190,1,0,0,
  	0,189,187,1,0,0,0,190,191,5,0,0,1,191,1,1,0,0,0,192,193,3,122,61,0,193,
  	194,3,148,74,0,194,195,5,81,0,0,195,198,1,0,0,0,196,198,3,86,43,0,197,
  	192,1,0,0,0,197,196,1,0,0,0,198,3,1,0,0,0,199,200,5,83,0,0,200,201,5,
  	57,0,0,201,202,3,6,3,0,202,203,5,58,0,0,203,5,1,0,0,0,204,209,3,122,61,
  	0,205,206,5,77,0,0,206,208,3,122,61,0,207,205,1,0,0,0,208,211,1,0,0,0,
  	209,207,1,0,0,0,209,210,1,0,0,0,210,7,1,0,0,0,211,209,1,0,0,0,212,213,
  	7,0,0,0,213,9,1,0,0,0,214,216,3,182,91,0,215,214,1,0,0,0,216,217,1,0,
  	0,0,217,215,1,0,0,0,217,218,1,0,0,0,218,226,1,0,0,0,219,220,5,40,0,0,
  	220,221,3,82,41,0,221,222,5,41,0,0,222,226,1,0,0,0,223,226,3,12,6,0,224,
  	226,3,20,10,0,225,215,1,0,0,0,225,219,1,0,0,0,225,223,1,0,0,0,225,224,
  	1,0,0,0,226,11,1,0,0,0,227,230,3,14,7,0,228,230,3,16,8,0,229,227,1,0,
  	0,0,229,228,1,0,0,0,230,13,1,0,0,0,231,232,5,83,0,0,232,15,1,0,0,0,233,
  	234,3,18,9,0,234,235,3,14,7,0,235,17,1,0,0,0,236,240,6,9,-1,0,237,241,
  	3,136,68,0,238,241,3,140,70,0,239,241,3,138,69,0,240,237,1,0,0,0,240,
  	238,1,0,0,0,240,239,1,0,0,0,240,241,1,0,0,0,241,242,1,0,0,0,242,243,5,
  	80,0,0,243,249,1,0,0,0,244,245,10,1,0,0,245,246,5,83,0,0,246,248,5,80,
  	0,0,247,244,1,0,0,0,248,251,1,0,0,0,249,247,1,0,0,0,249,250,1,0,0,0,250,
  	19,1,0,0,0,251,249,1,0,0,0,252,254,3,22,11,0,253,255,3,36,18,0,254,253,
  	1,0,0,0,254,255,1,0,0,0,255,256,1,0,0,0,256,257,3,92,46,0,257,21,1,0,
  	0,0,258,260,5,42,0,0,259,261,3,24,12,0,260,259,1,0,0,0,260,261,1,0,0,
  	0,261,262,1,0,0,0,262,263,5,43,0,0,263,23,1,0,0,0,264,271,3,28,14,0,265,
  	268,3,26,13,0,266,267,5,77,0,0,267,269,3,28,14,0,268,266,1,0,0,0,268,
  	269,1,0,0,0,269,271,1,0,0,0,270,264,1,0,0,0,270,265,1,0,0,0,271,25,1,
  	0,0,0,272,273,7,1,0,0,273,27,1,0,0,0,274,279,3,30,15,0,275,276,5,77,0,
  	0,276,278,3,30,15,0,277,275,1,0,0,0,278,281,1,0,0,0,279,277,1,0,0,0,279,
  	280,1,0,0,0,280,29,1,0,0,0,281,279,1,0,0,0,282,285,3,32,16,0,283,285,
  	3,34,17,0,284,282,1,0,0,0,284,283,1,0,0,0,285,31,1,0,0,0,286,288,5,52,
  	0,0,287,286,1,0,0,0,287,288,1,0,0,0,288,289,1,0,0,0,289,290,5,83,0,0,
  	290,33,1,0,0,0,291,293,5,52,0,0,292,291,1,0,0,0,292,293,1,0,0,0,293,294,
  	1,0,0,0,294,295,5,83,0,0,295,296,3,170,85,0,296,35,1,0,0,0,297,299,5,
  	40,0,0,298,300,3,164,82,0,299,298,1,0,0,0,299,300,1,0,0,0,300,301,1,0,
  	0,0,301,302,5,41,0,0,302,37,1,0,0,0,303,304,6,19,-1,0,304,330,3,10,5,
  	0,305,312,3,134,67,0,306,308,5,40,0,0,307,309,3,40,20,0,308,307,1,0,0,
  	0,308,309,1,0,0,0,309,310,1,0,0,0,310,313,5,41,0,0,311,313,3,178,89,0,
  	312,306,1,0,0,0,312,311,1,0,0,0,313,330,1,0,0,0,314,315,5,6,0,0,315,316,
  	5,57,0,0,316,317,3,162,81,0,317,318,5,58,0,0,318,319,5,40,0,0,319,320,
  	3,82,41,0,320,321,5,41,0,0,321,330,1,0,0,0,322,325,5,40,0,0,323,326,3,
  	82,41,0,324,326,3,162,81,0,325,323,1,0,0,0,325,324,1,0,0,0,326,327,1,
  	0,0,0,327,328,5,41,0,0,328,330,1,0,0,0,329,303,1,0,0,0,329,305,1,0,0,
  	0,329,314,1,0,0,0,329,322,1,0,0,0,330,355,1,0,0,0,331,332,10,7,0,0,332,
  	335,5,42,0,0,333,336,3,82,41,0,334,336,3,178,89,0,335,333,1,0,0,0,335,
  	334,1,0,0,0,336,337,1,0,0,0,337,338,5,43,0,0,338,354,1,0,0,0,339,340,
  	10,6,0,0,340,342,5,40,0,0,341,343,3,40,20,0,342,341,1,0,0,0,342,343,1,
  	0,0,0,343,344,1,0,0,0,344,354,5,41,0,0,345,346,10,4,0,0,346,349,5,82,
  	0,0,347,350,3,12,6,0,348,350,3,42,21,0,349,347,1,0,0,0,349,348,1,0,0,
  	0,350,354,1,0,0,0,351,352,10,3,0,0,352,354,7,2,0,0,353,331,1,0,0,0,353,
  	339,1,0,0,0,353,345,1,0,0,0,353,351,1,0,0,0,354,357,1,0,0,0,355,353,1,
  	0,0,0,355,356,1,0,0,0,356,39,1,0,0,0,357,355,1,0,0,0,358,359,3,176,88,
  	0,359,41,1,0,0,0,360,362,3,18,9,0,361,360,1,0,0,0,361,362,1,0,0,0,362,
  	366,1,0,0,0,363,364,3,136,68,0,364,365,5,80,0,0,365,367,1,0,0,0,366,363,
  	1,0,0,0,366,367,1,0,0,0,367,368,1,0,0,0,368,369,5,54,0,0,369,378,3,136,
  	68,0,370,371,3,18,9,0,371,372,5,80,0,0,372,373,5,54,0,0,373,374,3,136,
  	68,0,374,378,1,0,0,0,375,376,5,54,0,0,376,378,3,138,69,0,377,361,1,0,
  	0,0,377,370,1,0,0,0,377,375,1,0,0,0,378,43,1,0,0,0,379,391,3,38,19,0,
  	380,384,5,75,0,0,381,384,5,76,0,0,382,384,3,46,23,0,383,380,1,0,0,0,383,
  	381,1,0,0,0,383,382,1,0,0,0,384,385,1,0,0,0,385,391,3,44,22,0,386,387,
  	5,40,0,0,387,388,3,162,81,0,388,389,5,41,0,0,389,391,1,0,0,0,390,379,
  	1,0,0,0,390,383,1,0,0,0,390,386,1,0,0,0,391,45,1,0,0,0,392,393,7,3,0,
  	0,393,47,1,0,0,0,394,395,5,40,0,0,395,396,3,40,20,0,396,397,5,41,0,0,
  	397,49,1,0,0,0,398,400,5,40,0,0,399,401,3,40,20,0,400,399,1,0,0,0,400,
  	401,1,0,0,0,401,402,1,0,0,0,402,405,5,41,0,0,403,405,3,178,89,0,404,398,
  	1,0,0,0,404,403,1,0,0,0,405,51,1,0,0,0,406,416,3,44,22,0,407,408,5,6,
  	0,0,408,409,5,57,0,0,409,410,3,162,81,0,410,411,5,58,0,0,411,412,5,40,
  	0,0,412,413,3,52,26,0,413,414,5,41,0,0,414,416,1,0,0,0,415,406,1,0,0,
  	0,415,407,1,0,0,0,416,53,1,0,0,0,417,422,3,52,26,0,418,419,7,4,0,0,419,
  	421,3,52,26,0,420,418,1,0,0,0,421,424,1,0,0,0,422,420,1,0,0,0,422,423,
  	1,0,0,0,423,55,1,0,0,0,424,422,1,0,0,0,425,430,3,54,27,0,426,427,7,5,
  	0,0,427,429,3,54,27,0,428,426,1,0,0,0,429,432,1,0,0,0,430,428,1,0,0,0,
  	430,431,1,0,0,0,431,57,1,0,0,0,432,430,1,0,0,0,433,439,3,56,28,0,434,
  	435,3,60,30,0,435,436,3,56,28,0,436,438,1,0,0,0,437,434,1,0,0,0,438,441,
  	1,0,0,0,439,437,1,0,0,0,439,440,1,0,0,0,440,59,1,0,0,0,441,439,1,0,0,
  	0,442,443,5,58,0,0,443,447,5,58,0,0,444,445,5,57,0,0,445,447,5,57,0,0,
  	446,442,1,0,0,0,446,444,1,0,0,0,447,61,1,0,0,0,448,453,3,58,29,0,449,
  	450,7,6,0,0,450,452,3,58,29,0,451,449,1,0,0,0,452,455,1,0,0,0,453,451,
  	1,0,0,0,453,454,1,0,0,0,454,63,1,0,0,0,455,453,1,0,0,0,456,461,3,62,31,
  	0,457,458,7,7,0,0,458,460,3,62,31,0,459,457,1,0,0,0,460,463,1,0,0,0,461,
  	459,1,0,0,0,461,462,1,0,0,0,462,65,1,0,0,0,463,461,1,0,0,0,464,469,3,
  	64,32,0,465,466,5,52,0,0,466,468,3,64,32,0,467,465,1,0,0,0,468,471,1,
  	0,0,0,469,467,1,0,0,0,469,470,1,0,0,0,470,67,1,0,0,0,471,469,1,0,0,0,
  	472,477,3,66,33,0,473,474,5,51,0,0,474,476,3,66,33,0,475,473,1,0,0,0,
  	476,479,1,0,0,0,477,475,1,0,0,0,477,478,1,0,0,0,478,69,1,0,0,0,479,477,
  	1,0,0,0,480,485,3,68,34,0,481,482,5,53,0,0,482,484,3,68,34,0,483,481,
  	1,0,0,0,484,487,1,0,0,0,485,483,1,0,0,0,485,486,1,0,0,0,486,71,1,0,0,
  	0,487,485,1,0,0,0,488,493,3,70,35,0,489,490,5,73,0,0,490,492,3,70,35,
  	0,491,489,1,0,0,0,492,495,1,0,0,0,493,491,1,0,0,0,493,494,1,0,0,0,494,
  	73,1,0,0,0,495,493,1,0,0,0,496,501,3,72,36,0,497,498,5,74,0,0,498,500,
  	3,72,36,0,499,497,1,0,0,0,500,503,1,0,0,0,501,499,1,0,0,0,501,502,1,0,
  	0,0,502,75,1,0,0,0,503,501,1,0,0,0,504,510,3,74,37,0,505,506,5,78,0,0,
  	506,507,3,82,41,0,507,508,5,79,0,0,508,509,3,78,39,0,509,511,1,0,0,0,
  	510,505,1,0,0,0,510,511,1,0,0,0,511,77,1,0,0,0,512,518,3,76,38,0,513,
  	514,3,74,37,0,514,515,3,80,40,0,515,516,3,174,87,0,516,518,1,0,0,0,517,
  	512,1,0,0,0,517,513,1,0,0,0,518,79,1,0,0,0,519,520,7,8,0,0,520,81,1,0,
  	0,0,521,526,3,78,39,0,522,523,5,77,0,0,523,525,3,78,39,0,524,522,1,0,
  	0,0,525,528,1,0,0,0,526,524,1,0,0,0,526,527,1,0,0,0,527,83,1,0,0,0,528,
  	526,1,0,0,0,529,530,3,76,38,0,530,85,1,0,0,0,531,539,3,88,44,0,532,539,
  	3,112,56,0,533,539,3,90,45,0,534,539,3,92,46,0,535,539,3,96,48,0,536,
  	539,3,100,50,0,537,539,3,108,54,0,538,531,1,0,0,0,538,532,1,0,0,0,538,
  	533,1,0,0,0,538,534,1,0,0,0,538,535,1,0,0,0,538,536,1,0,0,0,538,537,1,
  	0,0,0,539,87,1,0,0,0,540,545,5,83,0,0,541,542,5,22,0,0,542,545,3,84,42,
  	0,543,545,5,27,0,0,544,540,1,0,0,0,544,541,1,0,0,0,544,543,1,0,0,0,545,
  	546,1,0,0,0,546,547,5,79,0,0,547,548,3,86,43,0,548,89,1,0,0,0,549,551,
  	3,82,41,0,550,549,1,0,0,0,550,551,1,0,0,0,551,552,1,0,0,0,552,553,5,81,
  	0,0,553,91,1,0,0,0,554,556,5,44,0,0,555,557,3,94,47,0,556,555,1,0,0,0,
  	556,557,1,0,0,0,557,558,1,0,0,0,558,559,5,45,0,0,559,93,1,0,0,0,560,562,
  	3,86,43,0,561,560,1,0,0,0,562,563,1,0,0,0,563,561,1,0,0,0,563,564,1,0,
  	0,0,564,95,1,0,0,0,565,566,5,32,0,0,566,567,5,40,0,0,567,568,3,98,49,
  	0,568,569,5,41,0,0,569,572,3,86,43,0,570,571,5,29,0,0,571,573,3,86,43,
  	0,572,570,1,0,0,0,572,573,1,0,0,0,573,581,1,0,0,0,574,575,5,36,0,0,575,
  	576,5,40,0,0,576,577,3,98,49,0,577,578,5,41,0,0,578,579,3,86,43,0,579,
  	581,1,0,0,0,580,565,1,0,0,0,580,574,1,0,0,0,581,97,1,0,0,0,582,591,3,
  	82,41,0,583,584,3,122,61,0,584,588,3,152,76,0,585,586,5,56,0,0,586,589,
  	3,174,87,0,587,589,3,178,89,0,588,585,1,0,0,0,588,587,1,0,0,0,589,591,
  	1,0,0,0,590,582,1,0,0,0,590,583,1,0,0,0,591,99,1,0,0,0,592,593,5,39,0,
  	0,593,594,5,40,0,0,594,595,3,98,49,0,595,596,5,41,0,0,596,597,3,86,43,
  	0,597,626,1,0,0,0,598,599,5,28,0,0,599,600,3,86,43,0,600,601,5,39,0,0,
  	601,602,5,40,0,0,602,603,3,82,41,0,603,604,5,41,0,0,604,605,5,81,0,0,
  	605,626,1,0,0,0,606,607,5,31,0,0,607,620,5,40,0,0,608,610,3,102,51,0,
  	609,611,3,98,49,0,610,609,1,0,0,0,610,611,1,0,0,0,611,612,1,0,0,0,612,
  	614,5,81,0,0,613,615,3,82,41,0,614,613,1,0,0,0,614,615,1,0,0,0,615,621,
  	1,0,0,0,616,617,3,104,52,0,617,618,5,79,0,0,618,619,3,106,53,0,619,621,
  	1,0,0,0,620,608,1,0,0,0,620,616,1,0,0,0,621,622,1,0,0,0,622,623,5,41,
  	0,0,623,624,3,86,43,0,624,626,1,0,0,0,625,592,1,0,0,0,625,598,1,0,0,0,
  	625,606,1,0,0,0,626,101,1,0,0,0,627,630,3,90,45,0,628,630,3,116,58,0,
  	629,627,1,0,0,0,629,628,1,0,0,0,630,103,1,0,0,0,631,632,3,122,61,0,632,
  	633,5,83,0,0,633,105,1,0,0,0,634,637,3,82,41,0,635,637,3,178,89,0,636,
  	634,1,0,0,0,636,635,1,0,0,0,637,107,1,0,0,0,638,639,7,9,0,0,639,640,5,
  	81,0,0,640,109,1,0,0,0,641,643,3,112,56,0,642,641,1,0,0,0,643,644,1,0,
  	0,0,644,642,1,0,0,0,644,645,1,0,0,0,645,111,1,0,0,0,646,649,3,116,58,
  	0,647,649,3,118,59,0,648,646,1,0,0,0,648,647,1,0,0,0,649,113,1,0,0,0,
  	650,651,5,83,0,0,651,652,5,56,0,0,652,653,3,162,81,0,653,654,5,81,0,0,
  	654,115,1,0,0,0,655,658,3,122,61,0,656,659,3,148,74,0,657,659,3,78,39,
  	0,658,656,1,0,0,0,658,657,1,0,0,0,658,659,1,0,0,0,659,660,1,0,0,0,660,
  	661,5,81,0,0,661,117,1,0,0,0,662,663,5,81,0,0,663,119,1,0,0,0,664,665,
  	3,126,63,0,665,121,1,0,0,0,666,668,3,120,60,0,667,666,1,0,0,0,668,669,
  	1,0,0,0,669,667,1,0,0,0,669,670,1,0,0,0,670,123,1,0,0,0,671,672,5,83,
  	0,0,672,125,1,0,0,0,673,674,3,128,64,0,674,127,1,0,0,0,675,678,3,134,
  	67,0,676,678,5,25,0,0,677,675,1,0,0,0,677,676,1,0,0,0,678,129,1,0,0,0,
  	679,681,3,126,63,0,680,679,1,0,0,0,681,682,1,0,0,0,682,680,1,0,0,0,682,
  	683,1,0,0,0,683,131,1,0,0,0,684,686,3,128,64,0,685,684,1,0,0,0,686,687,
  	1,0,0,0,687,685,1,0,0,0,687,688,1,0,0,0,688,133,1,0,0,0,689,691,3,18,
  	9,0,690,689,1,0,0,0,690,691,1,0,0,0,691,692,1,0,0,0,692,711,3,136,68,
  	0,693,711,3,4,2,0,694,711,5,7,0,0,695,711,5,8,0,0,696,711,5,9,0,0,697,
  	711,5,10,0,0,698,711,5,11,0,0,699,711,5,12,0,0,700,711,5,13,0,0,701,711,
  	5,14,0,0,702,711,5,15,0,0,703,711,5,16,0,0,704,711,5,17,0,0,705,711,5,
  	18,0,0,706,711,5,19,0,0,707,711,5,38,0,0,708,711,5,20,0,0,709,711,3,138,
  	69,0,710,690,1,0,0,0,710,693,1,0,0,0,710,694,1,0,0,0,710,695,1,0,0,0,
  	710,696,1,0,0,0,710,697,1,0,0,0,710,698,1,0,0,0,710,699,1,0,0,0,710,700,
  	1,0,0,0,710,701,1,0,0,0,710,702,1,0,0,0,710,703,1,0,0,0,710,704,1,0,0,
  	0,710,705,1,0,0,0,710,706,1,0,0,0,710,707,1,0,0,0,710,708,1,0,0,0,710,
  	709,1,0,0,0,711,135,1,0,0,0,712,713,3,124,62,0,713,137,1,0,0,0,714,717,
  	5,40,0,0,715,718,3,82,41,0,716,718,5,20,0,0,717,715,1,0,0,0,717,716,1,
  	0,0,0,718,719,1,0,0,0,719,720,5,41,0,0,720,139,1,0,0,0,721,722,5,83,0,
  	0,722,141,1,0,0,0,723,725,3,18,9,0,724,723,1,0,0,0,724,725,1,0,0,0,725,
  	726,1,0,0,0,726,727,3,140,70,0,727,143,1,0,0,0,728,730,3,146,73,0,729,
  	728,1,0,0,0,730,731,1,0,0,0,731,729,1,0,0,0,731,732,1,0,0,0,732,145,1,
  	0,0,0,733,734,5,40,0,0,734,735,3,144,72,0,735,736,5,41,0,0,736,751,1,
  	0,0,0,737,738,5,42,0,0,738,739,3,144,72,0,739,740,5,43,0,0,740,751,1,
  	0,0,0,741,742,5,44,0,0,742,743,3,144,72,0,743,744,5,45,0,0,744,751,1,
  	0,0,0,745,747,8,10,0,0,746,745,1,0,0,0,747,748,1,0,0,0,748,746,1,0,0,
  	0,748,749,1,0,0,0,749,751,1,0,0,0,750,733,1,0,0,0,750,737,1,0,0,0,750,
  	741,1,0,0,0,750,746,1,0,0,0,751,147,1,0,0,0,752,757,3,150,75,0,753,754,
  	5,77,0,0,754,756,3,150,75,0,755,753,1,0,0,0,756,759,1,0,0,0,757,755,1,
  	0,0,0,757,758,1,0,0,0,758,149,1,0,0,0,759,757,1,0,0,0,760,762,5,83,0,
  	0,761,763,3,170,85,0,762,761,1,0,0,0,762,763,1,0,0,0,763,151,1,0,0,0,
  	764,765,3,154,77,0,765,766,3,156,78,0,766,153,1,0,0,0,767,768,6,77,-1,
  	0,768,769,3,160,80,0,769,781,1,0,0,0,770,777,10,1,0,0,771,778,3,156,78,
  	0,772,774,5,42,0,0,773,775,3,84,42,0,774,773,1,0,0,0,774,775,1,0,0,0,
  	775,776,1,0,0,0,776,778,5,43,0,0,777,771,1,0,0,0,777,772,1,0,0,0,778,
  	780,1,0,0,0,779,770,1,0,0,0,780,783,1,0,0,0,781,779,1,0,0,0,781,782,1,
  	0,0,0,782,155,1,0,0,0,783,781,1,0,0,0,784,786,5,40,0,0,785,787,3,164,
  	82,0,786,785,1,0,0,0,786,787,1,0,0,0,787,788,1,0,0,0,788,790,5,41,0,0,
  	789,791,5,25,0,0,790,789,1,0,0,0,790,791,1,0,0,0,791,793,1,0,0,0,792,
  	794,3,158,79,0,793,792,1,0,0,0,793,794,1,0,0,0,794,157,1,0,0,0,795,796,
  	7,11,0,0,796,159,1,0,0,0,797,798,3,12,6,0,798,161,1,0,0,0,799,800,3,130,
  	65,0,800,163,1,0,0,0,801,802,3,166,83,0,802,165,1,0,0,0,803,808,3,168,
  	84,0,804,805,5,77,0,0,805,807,3,168,84,0,806,804,1,0,0,0,807,810,1,0,
  	0,0,808,806,1,0,0,0,808,809,1,0,0,0,809,167,1,0,0,0,810,808,1,0,0,0,811,
  	813,3,122,61,0,812,814,5,83,0,0,813,812,1,0,0,0,813,814,1,0,0,0,814,817,
  	1,0,0,0,815,816,5,56,0,0,816,818,3,174,87,0,817,815,1,0,0,0,817,818,1,
  	0,0,0,818,169,1,0,0,0,819,825,3,172,86,0,820,821,5,40,0,0,821,822,3,40,
  	20,0,822,823,5,41,0,0,823,825,1,0,0,0,824,819,1,0,0,0,824,820,1,0,0,0,
  	825,171,1,0,0,0,826,827,5,56,0,0,827,830,3,174,87,0,828,830,3,178,89,
  	0,829,826,1,0,0,0,829,828,1,0,0,0,830,173,1,0,0,0,831,834,3,78,39,0,832,
  	834,3,178,89,0,833,831,1,0,0,0,833,832,1,0,0,0,834,175,1,0,0,0,835,840,
  	3,174,87,0,836,837,5,77,0,0,837,839,3,174,87,0,838,836,1,0,0,0,839,842,
  	1,0,0,0,840,838,1,0,0,0,840,841,1,0,0,0,841,844,1,0,0,0,842,840,1,0,0,
  	0,843,845,5,77,0,0,844,843,1,0,0,0,844,845,1,0,0,0,845,177,1,0,0,0,846,
  	851,7,12,0,0,847,849,3,176,88,0,848,850,5,77,0,0,849,848,1,0,0,0,849,
  	850,1,0,0,0,850,852,1,0,0,0,851,847,1,0,0,0,851,852,1,0,0,0,852,853,1,
  	0,0,0,853,854,7,13,0,0,854,179,1,0,0,0,855,896,1,0,0,0,856,896,5,46,0,
  	0,857,896,5,47,0,0,858,896,5,48,0,0,859,896,5,49,0,0,860,896,5,50,0,0,
  	861,896,5,51,0,0,862,896,5,52,0,0,863,896,5,53,0,0,864,896,5,54,0,0,865,
  	896,5,55,0,0,866,896,5,56,0,0,867,896,5,58,0,0,868,896,5,57,0,0,869,896,
  	5,72,0,0,870,896,5,59,0,0,871,896,5,60,0,0,872,896,5,61,0,0,873,896,5,
  	63,0,0,874,896,5,64,0,0,875,896,5,65,0,0,876,896,5,66,0,0,877,878,5,57,
  	0,0,878,896,5,57,0,0,879,880,5,58,0,0,880,896,5,58,0,0,881,896,5,68,0,
  	0,882,896,5,67,0,0,883,896,5,69,0,0,884,896,5,70,0,0,885,896,5,71,0,0,
  	886,896,5,73,0,0,887,896,5,74,0,0,888,896,5,75,0,0,889,896,5,76,0,0,890,
  	896,5,77,0,0,891,892,5,40,0,0,892,896,5,41,0,0,893,894,5,42,0,0,894,896,
  	5,43,0,0,895,855,1,0,0,0,895,856,1,0,0,0,895,857,1,0,0,0,895,858,1,0,
  	0,0,895,859,1,0,0,0,895,860,1,0,0,0,895,861,1,0,0,0,895,862,1,0,0,0,895,
  	863,1,0,0,0,895,864,1,0,0,0,895,865,1,0,0,0,895,866,1,0,0,0,895,867,1,
  	0,0,0,895,868,1,0,0,0,895,869,1,0,0,0,895,870,1,0,0,0,895,871,1,0,0,0,
  	895,872,1,0,0,0,895,873,1,0,0,0,895,874,1,0,0,0,895,875,1,0,0,0,895,876,
  	1,0,0,0,895,877,1,0,0,0,895,879,1,0,0,0,895,881,1,0,0,0,895,882,1,0,0,
  	0,895,883,1,0,0,0,895,884,1,0,0,0,895,885,1,0,0,0,895,886,1,0,0,0,895,
  	887,1,0,0,0,895,888,1,0,0,0,895,889,1,0,0,0,895,890,1,0,0,0,895,891,1,
  	0,0,0,895,893,1,0,0,0,896,181,1,0,0,0,897,905,5,1,0,0,898,905,5,2,0,0,
  	899,905,5,3,0,0,900,905,5,4,0,0,901,905,3,8,4,0,902,905,5,5,0,0,903,905,
  	5,33,0,0,904,897,1,0,0,0,904,898,1,0,0,0,904,899,1,0,0,0,904,900,1,0,
  	0,0,904,901,1,0,0,0,904,902,1,0,0,0,904,903,1,0,0,0,905,183,1,0,0,0,97,
  	187,197,209,217,225,229,240,249,254,260,268,270,279,284,287,292,299,308,
  	312,325,329,335,342,349,353,355,361,366,377,383,390,400,404,415,422,430,
  	439,446,453,461,469,477,485,493,501,510,517,526,538,544,550,556,563,572,
  	580,588,590,610,614,620,625,629,636,644,648,658,669,677,682,687,690,710,
  	717,724,731,748,750,757,762,774,777,781,786,790,793,808,813,817,824,829,
  	833,840,844,849,851,895,904
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
    setState(187);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070729953837054) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 355) != 0)) {
      setState(184);
      scriptItem();
      setState(189);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(190);
    match(AngelscriptConsoleParser::EOF);
   
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

AngelscriptConsoleParser::StatementContext* AngelscriptConsoleParser::ScriptItemContext::statement() {
  return getRuleContext<AngelscriptConsoleParser::StatementContext>(0);
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
  enterRule(_localctx, 2, AngelscriptConsoleParser::RuleScriptItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(197);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(192);
      declSpecifierSeq();
      setState(193);
      initDeclaratorList();
      setState(194);
      match(AngelscriptConsoleParser::Semi);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(196);
      statement();
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
  enterRule(_localctx, 4, AngelscriptConsoleParser::RuleAsGeneric);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(199);
    match(AngelscriptConsoleParser::Identifier);
    setState(200);
    match(AngelscriptConsoleParser::Less);
    setState(201);
    simpleTypeSpecifierList();
    setState(202);
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

std::vector<AngelscriptConsoleParser::DeclSpecifierSeqContext *> AngelscriptConsoleParser::SimpleTypeSpecifierListContext::declSpecifierSeq() {
  return getRuleContexts<AngelscriptConsoleParser::DeclSpecifierSeqContext>();
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::SimpleTypeSpecifierListContext::declSpecifierSeq(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierSeqContext>(i);
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
  enterRule(_localctx, 6, AngelscriptConsoleParser::RuleSimpleTypeSpecifierList);
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
    setState(204);
    declSpecifierSeq();
    setState(209);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(205);
      match(AngelscriptConsoleParser::Comma);
      setState(206);
      declSpecifierSeq();
      setState(211);
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
  enterRule(_localctx, 8, AngelscriptConsoleParser::RuleBooleanLiteral);
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
    setState(212);
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

AngelscriptConsoleParser::LambdaExpressionContext* AngelscriptConsoleParser::PrimaryExpressionContext::lambdaExpression() {
  return getRuleContext<AngelscriptConsoleParser::LambdaExpressionContext>(0);
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
  enterRule(_localctx, 10, AngelscriptConsoleParser::RulePrimaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(225);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(215); 
      _errHandler->sync(this);
      alt = 1;
      do {
        switch (alt) {
          case 1: {
                setState(214);
                literal();
                break;
              }

        default:
          throw NoViableAltException(this);
        }
        setState(217); 
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
      } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(219);
      match(AngelscriptConsoleParser::LeftParen);
      setState(220);
      expression();
      setState(221);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(223);
      idExpression();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(224);
      lambdaExpression();
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
  enterRule(_localctx, 12, AngelscriptConsoleParser::RuleIdExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(229);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(227);
      unqualifiedId();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(228);
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
  enterRule(_localctx, 14, AngelscriptConsoleParser::RuleUnqualifiedId);

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
  enterRule(_localctx, 16, AngelscriptConsoleParser::RuleQualifiedId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(233);
    nestedNameSpecifier(0);
    setState(234);
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
  size_t startState = 18;
  enterRecursionRule(_localctx, 18, AngelscriptConsoleParser::RuleNestedNameSpecifier, precedence);

    

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
    setState(240);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      setState(237);
      theTypeName();
      break;
    }

    case 2: {
      setState(238);
      namespaceName();
      break;
    }

    case 3: {
      setState(239);
      decltypeSpecifier();
      break;
    }

    default:
      break;
    }
    setState(242);
    match(AngelscriptConsoleParser::Doublecolon);
    _ctx->stop = _input->LT(-1);
    setState(249);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<NestedNameSpecifierContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleNestedNameSpecifier);
        setState(244);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(245);
        match(AngelscriptConsoleParser::Identifier);
        setState(246);
        match(AngelscriptConsoleParser::Doublecolon); 
      }
      setState(251);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- LambdaExpressionContext ------------------------------------------------------------------

AngelscriptConsoleParser::LambdaExpressionContext::LambdaExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::LambdaIntroducerContext* AngelscriptConsoleParser::LambdaExpressionContext::lambdaIntroducer() {
  return getRuleContext<AngelscriptConsoleParser::LambdaIntroducerContext>(0);
}

AngelscriptConsoleParser::CompoundStatementContext* AngelscriptConsoleParser::LambdaExpressionContext::compoundStatement() {
  return getRuleContext<AngelscriptConsoleParser::CompoundStatementContext>(0);
}

AngelscriptConsoleParser::LambdaDeclaratorContext* AngelscriptConsoleParser::LambdaExpressionContext::lambdaDeclarator() {
  return getRuleContext<AngelscriptConsoleParser::LambdaDeclaratorContext>(0);
}


size_t AngelscriptConsoleParser::LambdaExpressionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLambdaExpression;
}


std::any AngelscriptConsoleParser::LambdaExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLambdaExpression(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LambdaExpressionContext* AngelscriptConsoleParser::lambdaExpression() {
  LambdaExpressionContext *_localctx = _tracker.createInstance<LambdaExpressionContext>(_ctx, getState());
  enterRule(_localctx, 20, AngelscriptConsoleParser::RuleLambdaExpression);
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
    setState(252);
    lambdaIntroducer();
    setState(254);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::LeftParen) {
      setState(253);
      lambdaDeclarator();
    }
    setState(256);
    compoundStatement();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LambdaIntroducerContext ------------------------------------------------------------------

AngelscriptConsoleParser::LambdaIntroducerContext::LambdaIntroducerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::LambdaIntroducerContext::LeftBracket() {
  return getToken(AngelscriptConsoleParser::LeftBracket, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::LambdaIntroducerContext::RightBracket() {
  return getToken(AngelscriptConsoleParser::RightBracket, 0);
}

AngelscriptConsoleParser::LambdaCaptureContext* AngelscriptConsoleParser::LambdaIntroducerContext::lambdaCapture() {
  return getRuleContext<AngelscriptConsoleParser::LambdaCaptureContext>(0);
}


size_t AngelscriptConsoleParser::LambdaIntroducerContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLambdaIntroducer;
}


std::any AngelscriptConsoleParser::LambdaIntroducerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLambdaIntroducer(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LambdaIntroducerContext* AngelscriptConsoleParser::lambdaIntroducer() {
  LambdaIntroducerContext *_localctx = _tracker.createInstance<LambdaIntroducerContext>(_ctx, getState());
  enterRule(_localctx, 22, AngelscriptConsoleParser::RuleLambdaIntroducer);
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
    setState(258);
    match(AngelscriptConsoleParser::LeftBracket);
    setState(260);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 52) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 52)) & 2147483665) != 0)) {
      setState(259);
      lambdaCapture();
    }
    setState(262);
    match(AngelscriptConsoleParser::RightBracket);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LambdaCaptureContext ------------------------------------------------------------------

AngelscriptConsoleParser::LambdaCaptureContext::LambdaCaptureContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::CaptureListContext* AngelscriptConsoleParser::LambdaCaptureContext::captureList() {
  return getRuleContext<AngelscriptConsoleParser::CaptureListContext>(0);
}

AngelscriptConsoleParser::CaptureDefaultContext* AngelscriptConsoleParser::LambdaCaptureContext::captureDefault() {
  return getRuleContext<AngelscriptConsoleParser::CaptureDefaultContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::LambdaCaptureContext::Comma() {
  return getToken(AngelscriptConsoleParser::Comma, 0);
}


size_t AngelscriptConsoleParser::LambdaCaptureContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLambdaCapture;
}


std::any AngelscriptConsoleParser::LambdaCaptureContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLambdaCapture(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LambdaCaptureContext* AngelscriptConsoleParser::lambdaCapture() {
  LambdaCaptureContext *_localctx = _tracker.createInstance<LambdaCaptureContext>(_ctx, getState());
  enterRule(_localctx, 24, AngelscriptConsoleParser::RuleLambdaCapture);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(270);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(264);
      captureList();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(265);
      captureDefault();
      setState(268);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Comma) {
        setState(266);
        match(AngelscriptConsoleParser::Comma);
        setState(267);
        captureList();
      }
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

//----------------- CaptureDefaultContext ------------------------------------------------------------------

AngelscriptConsoleParser::CaptureDefaultContext::CaptureDefaultContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::CaptureDefaultContext::And() {
  return getToken(AngelscriptConsoleParser::And, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::CaptureDefaultContext::Assign() {
  return getToken(AngelscriptConsoleParser::Assign, 0);
}


size_t AngelscriptConsoleParser::CaptureDefaultContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleCaptureDefault;
}


std::any AngelscriptConsoleParser::CaptureDefaultContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitCaptureDefault(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::CaptureDefaultContext* AngelscriptConsoleParser::captureDefault() {
  CaptureDefaultContext *_localctx = _tracker.createInstance<CaptureDefaultContext>(_ctx, getState());
  enterRule(_localctx, 26, AngelscriptConsoleParser::RuleCaptureDefault);
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
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::And

    || _la == AngelscriptConsoleParser::Assign)) {
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

//----------------- CaptureListContext ------------------------------------------------------------------

AngelscriptConsoleParser::CaptureListContext::CaptureListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::CaptureContext *> AngelscriptConsoleParser::CaptureListContext::capture() {
  return getRuleContexts<AngelscriptConsoleParser::CaptureContext>();
}

AngelscriptConsoleParser::CaptureContext* AngelscriptConsoleParser::CaptureListContext::capture(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::CaptureContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::CaptureListContext::Comma() {
  return getTokens(AngelscriptConsoleParser::Comma);
}

tree::TerminalNode* AngelscriptConsoleParser::CaptureListContext::Comma(size_t i) {
  return getToken(AngelscriptConsoleParser::Comma, i);
}


size_t AngelscriptConsoleParser::CaptureListContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleCaptureList;
}


std::any AngelscriptConsoleParser::CaptureListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitCaptureList(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::CaptureListContext* AngelscriptConsoleParser::captureList() {
  CaptureListContext *_localctx = _tracker.createInstance<CaptureListContext>(_ctx, getState());
  enterRule(_localctx, 28, AngelscriptConsoleParser::RuleCaptureList);
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
    setState(274);
    capture();
    setState(279);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(275);
      match(AngelscriptConsoleParser::Comma);
      setState(276);
      capture();
      setState(281);
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

//----------------- CaptureContext ------------------------------------------------------------------

AngelscriptConsoleParser::CaptureContext::CaptureContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::SimpleCaptureContext* AngelscriptConsoleParser::CaptureContext::simpleCapture() {
  return getRuleContext<AngelscriptConsoleParser::SimpleCaptureContext>(0);
}

AngelscriptConsoleParser::InitcaptureContext* AngelscriptConsoleParser::CaptureContext::initcapture() {
  return getRuleContext<AngelscriptConsoleParser::InitcaptureContext>(0);
}


size_t AngelscriptConsoleParser::CaptureContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleCapture;
}


std::any AngelscriptConsoleParser::CaptureContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitCapture(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::CaptureContext* AngelscriptConsoleParser::capture() {
  CaptureContext *_localctx = _tracker.createInstance<CaptureContext>(_ctx, getState());
  enterRule(_localctx, 30, AngelscriptConsoleParser::RuleCapture);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(284);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(282);
      simpleCapture();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(283);
      initcapture();
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

//----------------- SimpleCaptureContext ------------------------------------------------------------------

AngelscriptConsoleParser::SimpleCaptureContext::SimpleCaptureContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleCaptureContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleCaptureContext::And() {
  return getToken(AngelscriptConsoleParser::And, 0);
}


size_t AngelscriptConsoleParser::SimpleCaptureContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleSimpleCapture;
}


std::any AngelscriptConsoleParser::SimpleCaptureContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitSimpleCapture(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::SimpleCaptureContext* AngelscriptConsoleParser::simpleCapture() {
  SimpleCaptureContext *_localctx = _tracker.createInstance<SimpleCaptureContext>(_ctx, getState());
  enterRule(_localctx, 32, AngelscriptConsoleParser::RuleSimpleCapture);
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
    setState(287);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::And) {
      setState(286);
      match(AngelscriptConsoleParser::And);
    }
    setState(289);
    match(AngelscriptConsoleParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitcaptureContext ------------------------------------------------------------------

AngelscriptConsoleParser::InitcaptureContext::InitcaptureContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::InitcaptureContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

AngelscriptConsoleParser::InitializerContext* AngelscriptConsoleParser::InitcaptureContext::initializer() {
  return getRuleContext<AngelscriptConsoleParser::InitializerContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::InitcaptureContext::And() {
  return getToken(AngelscriptConsoleParser::And, 0);
}


size_t AngelscriptConsoleParser::InitcaptureContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleInitcapture;
}


std::any AngelscriptConsoleParser::InitcaptureContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitInitcapture(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::InitcaptureContext* AngelscriptConsoleParser::initcapture() {
  InitcaptureContext *_localctx = _tracker.createInstance<InitcaptureContext>(_ctx, getState());
  enterRule(_localctx, 34, AngelscriptConsoleParser::RuleInitcapture);
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
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::And) {
      setState(291);
      match(AngelscriptConsoleParser::And);
    }
    setState(294);
    match(AngelscriptConsoleParser::Identifier);
    setState(295);
    initializer();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LambdaDeclaratorContext ------------------------------------------------------------------

AngelscriptConsoleParser::LambdaDeclaratorContext::LambdaDeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::LambdaDeclaratorContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::LambdaDeclaratorContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

AngelscriptConsoleParser::ParameterDeclarationClauseContext* AngelscriptConsoleParser::LambdaDeclaratorContext::parameterDeclarationClause() {
  return getRuleContext<AngelscriptConsoleParser::ParameterDeclarationClauseContext>(0);
}


size_t AngelscriptConsoleParser::LambdaDeclaratorContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLambdaDeclarator;
}


std::any AngelscriptConsoleParser::LambdaDeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLambdaDeclarator(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LambdaDeclaratorContext* AngelscriptConsoleParser::lambdaDeclarator() {
  LambdaDeclaratorContext *_localctx = _tracker.createInstance<LambdaDeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 36, AngelscriptConsoleParser::RuleLambdaDeclarator);
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
    setState(297);
    match(AngelscriptConsoleParser::LeftParen);
    setState(299);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier) {
      setState(298);
      parameterDeclarationClause();
    }
    setState(301);
    match(AngelscriptConsoleParser::RightParen);
   
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
  size_t startState = 38;
  enterRecursionRule(_localctx, 38, AngelscriptConsoleParser::RulePostfixExpression, precedence);

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
    setState(329);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
    case 1: {
      setState(304);
      primaryExpression();
      break;
    }

    case 2: {
      setState(305);
      simpleTypeSpecifier();
      setState(312);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::LeftParen: {
          setState(306);
          match(AngelscriptConsoleParser::LeftParen);
          setState(308);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(307);
            expressionList();
          }
          setState(310);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case AngelscriptConsoleParser::LeftBracket:
        case AngelscriptConsoleParser::LeftBrace: {
          setState(311);
          bracedInitList();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      break;
    }

    case 3: {
      setState(314);
      match(AngelscriptConsoleParser::Cast);
      setState(315);
      match(AngelscriptConsoleParser::Less);
      setState(316);
      theTypeId();
      setState(317);
      match(AngelscriptConsoleParser::Greater);
      setState(318);
      match(AngelscriptConsoleParser::LeftParen);
      setState(319);
      expression();
      setState(320);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 4: {
      setState(322);
      match(AngelscriptConsoleParser::LeftParen);
      setState(325);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 19, _ctx)) {
      case 1: {
        setState(323);
        expression();
        break;
      }

      case 2: {
        setState(324);
        theTypeId();
        break;
      }

      default:
        break;
      }
      setState(327);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(355);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(353);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(331);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(332);
          match(AngelscriptConsoleParser::LeftBracket);
          setState(335);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
          case 1: {
            setState(333);
            expression();
            break;
          }

          case 2: {
            setState(334);
            bracedInitList();
            break;
          }

          default:
            break;
          }
          setState(337);
          match(AngelscriptConsoleParser::RightBracket);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(339);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(340);
          match(AngelscriptConsoleParser::LeftParen);
          setState(342);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(341);
            expressionList();
          }
          setState(344);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(345);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(346);
          match(AngelscriptConsoleParser::Dot);
          setState(349);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
          case 1: {
            setState(347);
            idExpression();
            break;
          }

          case 2: {
            setState(348);
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
          setState(351);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(352);
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
      setState(357);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
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
  enterRule(_localctx, 40, AngelscriptConsoleParser::RuleExpressionList);

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
    initializerList();
   
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
  enterRule(_localctx, 42, AngelscriptConsoleParser::RulePseudoDestructorName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(377);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(361);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx)) {
      case 1: {
        setState(360);
        nestedNameSpecifier(0);
        break;
      }

      default:
        break;
      }
      setState(366);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Identifier) {
        setState(363);
        theTypeName();
        setState(364);
        match(AngelscriptConsoleParser::Doublecolon);
      }
      setState(368);
      match(AngelscriptConsoleParser::Tilde);
      setState(369);
      theTypeName();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(370);
      nestedNameSpecifier(0);
      setState(371);
      match(AngelscriptConsoleParser::Doublecolon);
      setState(372);
      match(AngelscriptConsoleParser::Tilde);
      setState(373);
      theTypeName();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(375);
      match(AngelscriptConsoleParser::Tilde);
      setState(376);
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
  enterRule(_localctx, 44, AngelscriptConsoleParser::RuleUnaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(390);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(379);
      postfixExpression(0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(383);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::PlusPlus: {
          setState(380);
          match(AngelscriptConsoleParser::PlusPlus);
          break;
        }

        case AngelscriptConsoleParser::MinusMinus: {
          setState(381);
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
          setState(382);
          unaryOperator();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(385);
      unaryExpression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(386);
      match(AngelscriptConsoleParser::LeftParen);
      setState(387);
      theTypeId();
      setState(388);
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
  enterRule(_localctx, 46, AngelscriptConsoleParser::RuleUnaryOperator);
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
    setState(392);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68046575619801088) != 0))) {
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

//----------------- NewPlacementContext ------------------------------------------------------------------

AngelscriptConsoleParser::NewPlacementContext::NewPlacementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::NewPlacementContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

AngelscriptConsoleParser::ExpressionListContext* AngelscriptConsoleParser::NewPlacementContext::expressionList() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionListContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::NewPlacementContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}


size_t AngelscriptConsoleParser::NewPlacementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleNewPlacement;
}


std::any AngelscriptConsoleParser::NewPlacementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitNewPlacement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::NewPlacementContext* AngelscriptConsoleParser::newPlacement() {
  NewPlacementContext *_localctx = _tracker.createInstance<NewPlacementContext>(_ctx, getState());
  enterRule(_localctx, 48, AngelscriptConsoleParser::RuleNewPlacement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(394);
    match(AngelscriptConsoleParser::LeftParen);
    setState(395);
    expressionList();
    setState(396);
    match(AngelscriptConsoleParser::RightParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NewInitializer_Context ------------------------------------------------------------------

AngelscriptConsoleParser::NewInitializer_Context::NewInitializer_Context(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::NewInitializer_Context::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::NewInitializer_Context::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

AngelscriptConsoleParser::ExpressionListContext* AngelscriptConsoleParser::NewInitializer_Context::expressionList() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionListContext>(0);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::NewInitializer_Context::bracedInitList() {
  return getRuleContext<AngelscriptConsoleParser::BracedInitListContext>(0);
}


size_t AngelscriptConsoleParser::NewInitializer_Context::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleNewInitializer_;
}


std::any AngelscriptConsoleParser::NewInitializer_Context::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitNewInitializer_(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::NewInitializer_Context* AngelscriptConsoleParser::newInitializer_() {
  NewInitializer_Context *_localctx = _tracker.createInstance<NewInitializer_Context>(_ctx, getState());
  enterRule(_localctx, 50, AngelscriptConsoleParser::RuleNewInitializer_);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(404);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 1);
        setState(398);
        match(AngelscriptConsoleParser::LeftParen);
        setState(400);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 75)) & 291) != 0)) {
          setState(399);
          expressionList();
        }
        setState(402);
        match(AngelscriptConsoleParser::RightParen);
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(403);
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
  enterRule(_localctx, 52, AngelscriptConsoleParser::RuleCastExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(415);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(406);
      unaryExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(407);
      match(AngelscriptConsoleParser::Cast);
      setState(408);
      match(AngelscriptConsoleParser::Less);
      setState(409);
      theTypeId();
      setState(410);
      match(AngelscriptConsoleParser::Greater);
      setState(411);
      match(AngelscriptConsoleParser::LeftParen);
      setState(412);
      castExpression();
      setState(413);
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
  enterRule(_localctx, 54, AngelscriptConsoleParser::RuleMultiplicativeExpression);
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
    setState(417);
    castExpression();
    setState(422);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1970324836974592) != 0)) {
      setState(418);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 1970324836974592) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(419);
      castExpression();
      setState(424);
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
  enterRule(_localctx, 56, AngelscriptConsoleParser::RuleAdditiveExpression);
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
    multiplicativeExpression();
    setState(430);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Plus

    || _la == AngelscriptConsoleParser::Minus) {
      setState(426);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Plus

      || _la == AngelscriptConsoleParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(427);
      multiplicativeExpression();
      setState(432);
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
  enterRule(_localctx, 58, AngelscriptConsoleParser::RuleShiftExpression);

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
    setState(433);
    additiveExpression();
    setState(439);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(434);
        shiftOperator();
        setState(435);
        additiveExpression(); 
      }
      setState(441);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx);
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
  enterRule(_localctx, 60, AngelscriptConsoleParser::RuleShiftOperator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(446);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Greater: {
        enterOuterAlt(_localctx, 1);
        setState(442);
        match(AngelscriptConsoleParser::Greater);
        setState(443);
        match(AngelscriptConsoleParser::Greater);
        break;
      }

      case AngelscriptConsoleParser::Less: {
        enterOuterAlt(_localctx, 2);
        setState(444);
        match(AngelscriptConsoleParser::Less);
        setState(445);
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
  enterRule(_localctx, 62, AngelscriptConsoleParser::RuleRelationalExpression);
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
    setState(448);
    shiftExpression();
    setState(453);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 57) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 57)) & 49155) != 0)) {
      setState(449);
      _la = _input->LA(1);
      if (!(((((_la - 57) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 57)) & 49155) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(450);
      shiftExpression();
      setState(455);
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
  enterRule(_localctx, 64, AngelscriptConsoleParser::RuleEqualityExpression);
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
    setState(456);
    relationalExpression();
    setState(461);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Equal

    || _la == AngelscriptConsoleParser::NotEqual) {
      setState(457);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Equal

      || _la == AngelscriptConsoleParser::NotEqual)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(458);
      relationalExpression();
      setState(463);
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
  enterRule(_localctx, 66, AngelscriptConsoleParser::RuleAndExpression);
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
    setState(464);
    equalityExpression();
    setState(469);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::And) {
      setState(465);
      match(AngelscriptConsoleParser::And);
      setState(466);
      equalityExpression();
      setState(471);
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
  enterRule(_localctx, 68, AngelscriptConsoleParser::RuleExclusiveOrExpression);
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
    setState(472);
    andExpression();
    setState(477);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Xor) {
      setState(473);
      match(AngelscriptConsoleParser::Xor);
      setState(474);
      andExpression();
      setState(479);
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
  enterRule(_localctx, 70, AngelscriptConsoleParser::RuleInclusiveOrExpression);
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
    setState(480);
    exclusiveOrExpression();
    setState(485);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Or) {
      setState(481);
      match(AngelscriptConsoleParser::Or);
      setState(482);
      exclusiveOrExpression();
      setState(487);
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
  enterRule(_localctx, 72, AngelscriptConsoleParser::RuleLogicalAndExpression);
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
    setState(488);
    inclusiveOrExpression();
    setState(493);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::AndAnd) {
      setState(489);
      match(AngelscriptConsoleParser::AndAnd);
      setState(490);
      inclusiveOrExpression();
      setState(495);
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
  enterRule(_localctx, 74, AngelscriptConsoleParser::RuleLogicalOrExpression);
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
    setState(496);
    logicalAndExpression();
    setState(501);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::OrOr) {
      setState(497);
      match(AngelscriptConsoleParser::OrOr);
      setState(498);
      logicalAndExpression();
      setState(503);
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
  enterRule(_localctx, 76, AngelscriptConsoleParser::RuleConditionalExpression);
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
    setState(504);
    logicalOrExpression();
    setState(510);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Question) {
      setState(505);
      match(AngelscriptConsoleParser::Question);
      setState(506);
      expression();
      setState(507);
      match(AngelscriptConsoleParser::Colon);
      setState(508);
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
  enterRule(_localctx, 78, AngelscriptConsoleParser::RuleAssignmentExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(517);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 46, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(512);
      conditionalExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(513);
      logicalOrExpression();
      setState(514);
      assignmentOperator();
      setState(515);
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
  enterRule(_localctx, 80, AngelscriptConsoleParser::RuleAssignmentOperator);
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
    setState(519);
    _la = _input->LA(1);
    if (!(((((_la - 56) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 56)) & 8185) != 0))) {
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
  enterRule(_localctx, 82, AngelscriptConsoleParser::RuleExpression);
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
    setState(521);
    assignmentExpression();
    setState(526);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(522);
      match(AngelscriptConsoleParser::Comma);
      setState(523);
      assignmentExpression();
      setState(528);
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

AngelscriptConsoleParser::ConditionalExpressionContext* AngelscriptConsoleParser::ConstantExpressionContext::conditionalExpression() {
  return getRuleContext<AngelscriptConsoleParser::ConditionalExpressionContext>(0);
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
  enterRule(_localctx, 84, AngelscriptConsoleParser::RuleConstantExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(529);
    conditionalExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::LabeledStatementContext* AngelscriptConsoleParser::StatementContext::labeledStatement() {
  return getRuleContext<AngelscriptConsoleParser::LabeledStatementContext>(0);
}

AngelscriptConsoleParser::DeclarationContext* AngelscriptConsoleParser::StatementContext::declaration() {
  return getRuleContext<AngelscriptConsoleParser::DeclarationContext>(0);
}

AngelscriptConsoleParser::ExpressionStatementContext* AngelscriptConsoleParser::StatementContext::expressionStatement() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionStatementContext>(0);
}

AngelscriptConsoleParser::CompoundStatementContext* AngelscriptConsoleParser::StatementContext::compoundStatement() {
  return getRuleContext<AngelscriptConsoleParser::CompoundStatementContext>(0);
}

AngelscriptConsoleParser::SelectionStatementContext* AngelscriptConsoleParser::StatementContext::selectionStatement() {
  return getRuleContext<AngelscriptConsoleParser::SelectionStatementContext>(0);
}

AngelscriptConsoleParser::IterationStatementContext* AngelscriptConsoleParser::StatementContext::iterationStatement() {
  return getRuleContext<AngelscriptConsoleParser::IterationStatementContext>(0);
}

AngelscriptConsoleParser::JumpStatementContext* AngelscriptConsoleParser::StatementContext::jumpStatement() {
  return getRuleContext<AngelscriptConsoleParser::JumpStatementContext>(0);
}


size_t AngelscriptConsoleParser::StatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleStatement;
}


std::any AngelscriptConsoleParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::StatementContext* AngelscriptConsoleParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 86, AngelscriptConsoleParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(538);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(531);
      labeledStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(532);
      declaration();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(533);
      expressionStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(534);
      compoundStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(535);
      selectionStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(536);
      iterationStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(537);
      jumpStatement();
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

//----------------- LabeledStatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::LabeledStatementContext::LabeledStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::LabeledStatementContext::Colon() {
  return getToken(AngelscriptConsoleParser::Colon, 0);
}

AngelscriptConsoleParser::StatementContext* AngelscriptConsoleParser::LabeledStatementContext::statement() {
  return getRuleContext<AngelscriptConsoleParser::StatementContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::LabeledStatementContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::LabeledStatementContext::Case() {
  return getToken(AngelscriptConsoleParser::Case, 0);
}

AngelscriptConsoleParser::ConstantExpressionContext* AngelscriptConsoleParser::LabeledStatementContext::constantExpression() {
  return getRuleContext<AngelscriptConsoleParser::ConstantExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::LabeledStatementContext::Default() {
  return getToken(AngelscriptConsoleParser::Default, 0);
}


size_t AngelscriptConsoleParser::LabeledStatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleLabeledStatement;
}


std::any AngelscriptConsoleParser::LabeledStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitLabeledStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::LabeledStatementContext* AngelscriptConsoleParser::labeledStatement() {
  LabeledStatementContext *_localctx = _tracker.createInstance<LabeledStatementContext>(_ctx, getState());
  enterRule(_localctx, 88, AngelscriptConsoleParser::RuleLabeledStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(544);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Identifier: {
        setState(540);
        match(AngelscriptConsoleParser::Identifier);
        break;
      }

      case AngelscriptConsoleParser::Case: {
        setState(541);
        match(AngelscriptConsoleParser::Case);
        setState(542);
        constantExpression();
        break;
      }

      case AngelscriptConsoleParser::Default: {
        setState(543);
        match(AngelscriptConsoleParser::Default);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(546);
    match(AngelscriptConsoleParser::Colon);
    setState(547);
    statement();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionStatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::ExpressionStatementContext::ExpressionStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::ExpressionStatementContext::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::ExpressionStatementContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}


size_t AngelscriptConsoleParser::ExpressionStatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleExpressionStatement;
}


std::any AngelscriptConsoleParser::ExpressionStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitExpressionStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ExpressionStatementContext* AngelscriptConsoleParser::expressionStatement() {
  ExpressionStatementContext *_localctx = _tracker.createInstance<ExpressionStatementContext>(_ctx, getState());
  enterRule(_localctx, 90, AngelscriptConsoleParser::RuleExpressionStatement);
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
    setState(550);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 291) != 0)) {
      setState(549);
      expression();
    }
    setState(552);
    match(AngelscriptConsoleParser::Semi);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CompoundStatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::CompoundStatementContext::CompoundStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::CompoundStatementContext::LeftBrace() {
  return getToken(AngelscriptConsoleParser::LeftBrace, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::CompoundStatementContext::RightBrace() {
  return getToken(AngelscriptConsoleParser::RightBrace, 0);
}

AngelscriptConsoleParser::StatementSeqContext* AngelscriptConsoleParser::CompoundStatementContext::statementSeq() {
  return getRuleContext<AngelscriptConsoleParser::StatementSeqContext>(0);
}


size_t AngelscriptConsoleParser::CompoundStatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleCompoundStatement;
}


std::any AngelscriptConsoleParser::CompoundStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitCompoundStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::CompoundStatementContext* AngelscriptConsoleParser::compoundStatement() {
  CompoundStatementContext *_localctx = _tracker.createInstance<CompoundStatementContext>(_ctx, getState());
  enterRule(_localctx, 92, AngelscriptConsoleParser::RuleCompoundStatement);
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
    setState(554);
    match(AngelscriptConsoleParser::LeftBrace);
    setState(556);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070729953837054) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 355) != 0)) {
      setState(555);
      statementSeq();
    }
    setState(558);
    match(AngelscriptConsoleParser::RightBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementSeqContext ------------------------------------------------------------------

AngelscriptConsoleParser::StatementSeqContext::StatementSeqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::StatementContext *> AngelscriptConsoleParser::StatementSeqContext::statement() {
  return getRuleContexts<AngelscriptConsoleParser::StatementContext>();
}

AngelscriptConsoleParser::StatementContext* AngelscriptConsoleParser::StatementSeqContext::statement(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::StatementContext>(i);
}


size_t AngelscriptConsoleParser::StatementSeqContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleStatementSeq;
}


std::any AngelscriptConsoleParser::StatementSeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitStatementSeq(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::StatementSeqContext* AngelscriptConsoleParser::statementSeq() {
  StatementSeqContext *_localctx = _tracker.createInstance<StatementSeqContext>(_ctx, getState());
  enterRule(_localctx, 94, AngelscriptConsoleParser::RuleStatementSeq);
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
    setState(561); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(560);
      statement();
      setState(563); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070729953837054) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 355) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SelectionStatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::SelectionStatementContext::SelectionStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::SelectionStatementContext::If() {
  return getToken(AngelscriptConsoleParser::If, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SelectionStatementContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

AngelscriptConsoleParser::ConditionContext* AngelscriptConsoleParser::SelectionStatementContext::condition() {
  return getRuleContext<AngelscriptConsoleParser::ConditionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::SelectionStatementContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

std::vector<AngelscriptConsoleParser::StatementContext *> AngelscriptConsoleParser::SelectionStatementContext::statement() {
  return getRuleContexts<AngelscriptConsoleParser::StatementContext>();
}

AngelscriptConsoleParser::StatementContext* AngelscriptConsoleParser::SelectionStatementContext::statement(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::StatementContext>(i);
}

tree::TerminalNode* AngelscriptConsoleParser::SelectionStatementContext::Else() {
  return getToken(AngelscriptConsoleParser::Else, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SelectionStatementContext::Switch() {
  return getToken(AngelscriptConsoleParser::Switch, 0);
}


size_t AngelscriptConsoleParser::SelectionStatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleSelectionStatement;
}


std::any AngelscriptConsoleParser::SelectionStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitSelectionStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::SelectionStatementContext* AngelscriptConsoleParser::selectionStatement() {
  SelectionStatementContext *_localctx = _tracker.createInstance<SelectionStatementContext>(_ctx, getState());
  enterRule(_localctx, 96, AngelscriptConsoleParser::RuleSelectionStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(580);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::If: {
        enterOuterAlt(_localctx, 1);
        setState(565);
        match(AngelscriptConsoleParser::If);
        setState(566);
        match(AngelscriptConsoleParser::LeftParen);
        setState(567);
        condition();
        setState(568);
        match(AngelscriptConsoleParser::RightParen);
        setState(569);
        statement();
        setState(572);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx)) {
        case 1: {
          setState(570);
          match(AngelscriptConsoleParser::Else);
          setState(571);
          statement();
          break;
        }

        default:
          break;
        }
        break;
      }

      case AngelscriptConsoleParser::Switch: {
        enterOuterAlt(_localctx, 2);
        setState(574);
        match(AngelscriptConsoleParser::Switch);
        setState(575);
        match(AngelscriptConsoleParser::LeftParen);
        setState(576);
        condition();
        setState(577);
        match(AngelscriptConsoleParser::RightParen);
        setState(578);
        statement();
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

//----------------- ConditionContext ------------------------------------------------------------------

AngelscriptConsoleParser::ConditionContext::ConditionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::ConditionContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::ConditionContext::declSpecifierSeq() {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierSeqContext>(0);
}

AngelscriptConsoleParser::DeclaratorContext* AngelscriptConsoleParser::ConditionContext::declarator() {
  return getRuleContext<AngelscriptConsoleParser::DeclaratorContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::ConditionContext::Assign() {
  return getToken(AngelscriptConsoleParser::Assign, 0);
}

AngelscriptConsoleParser::InitializerClauseContext* AngelscriptConsoleParser::ConditionContext::initializerClause() {
  return getRuleContext<AngelscriptConsoleParser::InitializerClauseContext>(0);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::ConditionContext::bracedInitList() {
  return getRuleContext<AngelscriptConsoleParser::BracedInitListContext>(0);
}


size_t AngelscriptConsoleParser::ConditionContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleCondition;
}


std::any AngelscriptConsoleParser::ConditionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitCondition(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ConditionContext* AngelscriptConsoleParser::condition() {
  ConditionContext *_localctx = _tracker.createInstance<ConditionContext>(_ctx, getState());
  enterRule(_localctx, 98, AngelscriptConsoleParser::RuleCondition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(590);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(582);
      expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(583);
      declSpecifierSeq();
      setState(584);
      declarator();
      setState(588);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::Assign: {
          setState(585);
          match(AngelscriptConsoleParser::Assign);
          setState(586);
          initializerClause();
          break;
        }

        case AngelscriptConsoleParser::LeftBracket:
        case AngelscriptConsoleParser::LeftBrace: {
          setState(587);
          bracedInitList();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
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

//----------------- IterationStatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::IterationStatementContext::IterationStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::IterationStatementContext::While() {
  return getToken(AngelscriptConsoleParser::While, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::IterationStatementContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

AngelscriptConsoleParser::ConditionContext* AngelscriptConsoleParser::IterationStatementContext::condition() {
  return getRuleContext<AngelscriptConsoleParser::ConditionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::IterationStatementContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

AngelscriptConsoleParser::StatementContext* AngelscriptConsoleParser::IterationStatementContext::statement() {
  return getRuleContext<AngelscriptConsoleParser::StatementContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::IterationStatementContext::Do() {
  return getToken(AngelscriptConsoleParser::Do, 0);
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::IterationStatementContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::IterationStatementContext::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::IterationStatementContext::For() {
  return getToken(AngelscriptConsoleParser::For, 0);
}

AngelscriptConsoleParser::ForInitStatementContext* AngelscriptConsoleParser::IterationStatementContext::forInitStatement() {
  return getRuleContext<AngelscriptConsoleParser::ForInitStatementContext>(0);
}

AngelscriptConsoleParser::ForRangeDeclarationContext* AngelscriptConsoleParser::IterationStatementContext::forRangeDeclaration() {
  return getRuleContext<AngelscriptConsoleParser::ForRangeDeclarationContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::IterationStatementContext::Colon() {
  return getToken(AngelscriptConsoleParser::Colon, 0);
}

AngelscriptConsoleParser::ForRangeInitializerContext* AngelscriptConsoleParser::IterationStatementContext::forRangeInitializer() {
  return getRuleContext<AngelscriptConsoleParser::ForRangeInitializerContext>(0);
}


size_t AngelscriptConsoleParser::IterationStatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleIterationStatement;
}


std::any AngelscriptConsoleParser::IterationStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitIterationStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::IterationStatementContext* AngelscriptConsoleParser::iterationStatement() {
  IterationStatementContext *_localctx = _tracker.createInstance<IterationStatementContext>(_ctx, getState());
  enterRule(_localctx, 100, AngelscriptConsoleParser::RuleIterationStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(625);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::While: {
        enterOuterAlt(_localctx, 1);
        setState(592);
        match(AngelscriptConsoleParser::While);
        setState(593);
        match(AngelscriptConsoleParser::LeftParen);
        setState(594);
        condition();
        setState(595);
        match(AngelscriptConsoleParser::RightParen);
        setState(596);
        statement();
        break;
      }

      case AngelscriptConsoleParser::Do: {
        enterOuterAlt(_localctx, 2);
        setState(598);
        match(AngelscriptConsoleParser::Do);
        setState(599);
        statement();
        setState(600);
        match(AngelscriptConsoleParser::While);
        setState(601);
        match(AngelscriptConsoleParser::LeftParen);
        setState(602);
        expression();
        setState(603);
        match(AngelscriptConsoleParser::RightParen);
        setState(604);
        match(AngelscriptConsoleParser::Semi);
        break;
      }

      case AngelscriptConsoleParser::For: {
        enterOuterAlt(_localctx, 3);
        setState(606);
        match(AngelscriptConsoleParser::For);
        setState(607);
        match(AngelscriptConsoleParser::LeftParen);
        setState(620);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx)) {
        case 1: {
          setState(608);
          forInitStatement();
          setState(610);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68052495194128382) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(609);
            condition();
          }
          setState(612);
          match(AngelscriptConsoleParser::Semi);
          setState(614);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(613);
            expression();
          }
          break;
        }

        case 2: {
          setState(616);
          forRangeDeclaration();
          setState(617);
          match(AngelscriptConsoleParser::Colon);
          setState(618);
          forRangeInitializer();
          break;
        }

        default:
          break;
        }
        setState(622);
        match(AngelscriptConsoleParser::RightParen);
        setState(623);
        statement();
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

//----------------- ForInitStatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::ForInitStatementContext::ForInitStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::ExpressionStatementContext* AngelscriptConsoleParser::ForInitStatementContext::expressionStatement() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionStatementContext>(0);
}

AngelscriptConsoleParser::SimpleDeclarationContext* AngelscriptConsoleParser::ForInitStatementContext::simpleDeclaration() {
  return getRuleContext<AngelscriptConsoleParser::SimpleDeclarationContext>(0);
}


size_t AngelscriptConsoleParser::ForInitStatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleForInitStatement;
}


std::any AngelscriptConsoleParser::ForInitStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitForInitStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ForInitStatementContext* AngelscriptConsoleParser::forInitStatement() {
  ForInitStatementContext *_localctx = _tracker.createInstance<ForInitStatementContext>(_ctx, getState());
  enterRule(_localctx, 102, AngelscriptConsoleParser::RuleForInitStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(629);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(627);
      expressionStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(628);
      simpleDeclaration();
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

//----------------- ForRangeDeclarationContext ------------------------------------------------------------------

AngelscriptConsoleParser::ForRangeDeclarationContext::ForRangeDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::ForRangeDeclarationContext::declSpecifierSeq() {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierSeqContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::ForRangeDeclarationContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}


size_t AngelscriptConsoleParser::ForRangeDeclarationContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleForRangeDeclaration;
}


std::any AngelscriptConsoleParser::ForRangeDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitForRangeDeclaration(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ForRangeDeclarationContext* AngelscriptConsoleParser::forRangeDeclaration() {
  ForRangeDeclarationContext *_localctx = _tracker.createInstance<ForRangeDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 104, AngelscriptConsoleParser::RuleForRangeDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(631);
    declSpecifierSeq();
    setState(632);
    match(AngelscriptConsoleParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForRangeInitializerContext ------------------------------------------------------------------

AngelscriptConsoleParser::ForRangeInitializerContext::ForRangeInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::ForRangeInitializerContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::ForRangeInitializerContext::bracedInitList() {
  return getRuleContext<AngelscriptConsoleParser::BracedInitListContext>(0);
}


size_t AngelscriptConsoleParser::ForRangeInitializerContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleForRangeInitializer;
}


std::any AngelscriptConsoleParser::ForRangeInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitForRangeInitializer(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ForRangeInitializerContext* AngelscriptConsoleParser::forRangeInitializer() {
  ForRangeInitializerContext *_localctx = _tracker.createInstance<ForRangeInitializerContext>(_ctx, getState());
  enterRule(_localctx, 106, AngelscriptConsoleParser::RuleForRangeInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(636);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(634);
      expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(635);
      bracedInitList();
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

//----------------- JumpStatementContext ------------------------------------------------------------------

AngelscriptConsoleParser::JumpStatementContext::JumpStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::JumpStatementContext::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::JumpStatementContext::Break() {
  return getToken(AngelscriptConsoleParser::Break, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::JumpStatementContext::Continue() {
  return getToken(AngelscriptConsoleParser::Continue, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::JumpStatementContext::Return() {
  return getToken(AngelscriptConsoleParser::Return, 0);
}


size_t AngelscriptConsoleParser::JumpStatementContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleJumpStatement;
}


std::any AngelscriptConsoleParser::JumpStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitJumpStatement(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::JumpStatementContext* AngelscriptConsoleParser::jumpStatement() {
  JumpStatementContext *_localctx = _tracker.createInstance<JumpStatementContext>(_ctx, getState());
  enterRule(_localctx, 108, AngelscriptConsoleParser::RuleJumpStatement);
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
    setState(638);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 17249075200) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(639);
    match(AngelscriptConsoleParser::Semi);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclarationseqContext ------------------------------------------------------------------

AngelscriptConsoleParser::DeclarationseqContext::DeclarationseqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::DeclarationContext *> AngelscriptConsoleParser::DeclarationseqContext::declaration() {
  return getRuleContexts<AngelscriptConsoleParser::DeclarationContext>();
}

AngelscriptConsoleParser::DeclarationContext* AngelscriptConsoleParser::DeclarationseqContext::declaration(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::DeclarationContext>(i);
}


size_t AngelscriptConsoleParser::DeclarationseqContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDeclarationseq;
}


std::any AngelscriptConsoleParser::DeclarationseqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDeclarationseq(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::DeclarationseqContext* AngelscriptConsoleParser::declarationseq() {
  DeclarationseqContext *_localctx = _tracker.createInstance<DeclarationseqContext>(_ctx, getState());
  enterRule(_localctx, 110, AngelscriptConsoleParser::RuleDeclarationseq);
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
    setState(642); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(641);
      declaration();
      setState(644); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || ((((_la - 80) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 80)) & 11) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclarationContext ------------------------------------------------------------------

AngelscriptConsoleParser::DeclarationContext::DeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::SimpleDeclarationContext* AngelscriptConsoleParser::DeclarationContext::simpleDeclaration() {
  return getRuleContext<AngelscriptConsoleParser::SimpleDeclarationContext>(0);
}

AngelscriptConsoleParser::EmptyDeclaration_Context* AngelscriptConsoleParser::DeclarationContext::emptyDeclaration_() {
  return getRuleContext<AngelscriptConsoleParser::EmptyDeclaration_Context>(0);
}


size_t AngelscriptConsoleParser::DeclarationContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDeclaration;
}


std::any AngelscriptConsoleParser::DeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDeclaration(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::DeclarationContext* AngelscriptConsoleParser::declaration() {
  DeclarationContext *_localctx = _tracker.createInstance<DeclarationContext>(_ctx, getState());
  enterRule(_localctx, 112, AngelscriptConsoleParser::RuleDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(648);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Int:
      case AngelscriptConsoleParser::Int8:
      case AngelscriptConsoleParser::Int16:
      case AngelscriptConsoleParser::Int32:
      case AngelscriptConsoleParser::Int64:
      case AngelscriptConsoleParser::UInt:
      case AngelscriptConsoleParser::UInt8:
      case AngelscriptConsoleParser::UInt16:
      case AngelscriptConsoleParser::UInt32:
      case AngelscriptConsoleParser::UInt64:
      case AngelscriptConsoleParser::Float:
      case AngelscriptConsoleParser::Double:
      case AngelscriptConsoleParser::Bool:
      case AngelscriptConsoleParser::Auto:
      case AngelscriptConsoleParser::Const:
      case AngelscriptConsoleParser::Void:
      case AngelscriptConsoleParser::LeftParen:
      case AngelscriptConsoleParser::Doublecolon:
      case AngelscriptConsoleParser::Identifier: {
        enterOuterAlt(_localctx, 1);
        setState(646);
        simpleDeclaration();
        break;
      }

      case AngelscriptConsoleParser::Semi: {
        enterOuterAlt(_localctx, 2);
        setState(647);
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

//----------------- AliasDeclarationContext ------------------------------------------------------------------

AngelscriptConsoleParser::AliasDeclarationContext::AliasDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::AliasDeclarationContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::AliasDeclarationContext::Assign() {
  return getToken(AngelscriptConsoleParser::Assign, 0);
}

AngelscriptConsoleParser::TheTypeIdContext* AngelscriptConsoleParser::AliasDeclarationContext::theTypeId() {
  return getRuleContext<AngelscriptConsoleParser::TheTypeIdContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::AliasDeclarationContext::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}


size_t AngelscriptConsoleParser::AliasDeclarationContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleAliasDeclaration;
}


std::any AngelscriptConsoleParser::AliasDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitAliasDeclaration(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::AliasDeclarationContext* AngelscriptConsoleParser::aliasDeclaration() {
  AliasDeclarationContext *_localctx = _tracker.createInstance<AliasDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 114, AngelscriptConsoleParser::RuleAliasDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(650);
    match(AngelscriptConsoleParser::Identifier);
    setState(651);
    match(AngelscriptConsoleParser::Assign);
    setState(652);
    theTypeId();
    setState(653);
    match(AngelscriptConsoleParser::Semi);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SimpleDeclarationContext ------------------------------------------------------------------

AngelscriptConsoleParser::SimpleDeclarationContext::SimpleDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::SimpleDeclarationContext::declSpecifierSeq() {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierSeqContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleDeclarationContext::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}

AngelscriptConsoleParser::InitDeclaratorListContext* AngelscriptConsoleParser::SimpleDeclarationContext::initDeclaratorList() {
  return getRuleContext<AngelscriptConsoleParser::InitDeclaratorListContext>(0);
}

AngelscriptConsoleParser::AssignmentExpressionContext* AngelscriptConsoleParser::SimpleDeclarationContext::assignmentExpression() {
  return getRuleContext<AngelscriptConsoleParser::AssignmentExpressionContext>(0);
}


size_t AngelscriptConsoleParser::SimpleDeclarationContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleSimpleDeclaration;
}


std::any AngelscriptConsoleParser::SimpleDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitSimpleDeclaration(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::SimpleDeclarationContext* AngelscriptConsoleParser::simpleDeclaration() {
  SimpleDeclarationContext *_localctx = _tracker.createInstance<SimpleDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 116, AngelscriptConsoleParser::RuleSimpleDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(655);
    declSpecifierSeq();
    setState(658);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx)) {
    case 1: {
      setState(656);
      initDeclaratorList();
      break;
    }

    case 2: {
      setState(657);
      assignmentExpression();
      break;
    }

    default:
      break;
    }
    setState(660);
    match(AngelscriptConsoleParser::Semi);
   
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
  enterRule(_localctx, 118, AngelscriptConsoleParser::RuleEmptyDeclaration_);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(662);
    match(AngelscriptConsoleParser::Semi);
   
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
  enterRule(_localctx, 120, AngelscriptConsoleParser::RuleDeclSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(664);
    typeSpecifier();
   
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
  enterRule(_localctx, 122, AngelscriptConsoleParser::RuleDeclSpecifierSeq);

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
    setState(667); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(666);
              declSpecifier();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(669); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 66, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
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
  enterRule(_localctx, 124, AngelscriptConsoleParser::RuleTypedefName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(671);
    match(AngelscriptConsoleParser::Identifier);
   
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
  enterRule(_localctx, 126, AngelscriptConsoleParser::RuleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(673);
    trailingTypeSpecifier();
   
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
  enterRule(_localctx, 128, AngelscriptConsoleParser::RuleTrailingTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(677);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Int:
      case AngelscriptConsoleParser::Int8:
      case AngelscriptConsoleParser::Int16:
      case AngelscriptConsoleParser::Int32:
      case AngelscriptConsoleParser::Int64:
      case AngelscriptConsoleParser::UInt:
      case AngelscriptConsoleParser::UInt8:
      case AngelscriptConsoleParser::UInt16:
      case AngelscriptConsoleParser::UInt32:
      case AngelscriptConsoleParser::UInt64:
      case AngelscriptConsoleParser::Float:
      case AngelscriptConsoleParser::Double:
      case AngelscriptConsoleParser::Bool:
      case AngelscriptConsoleParser::Auto:
      case AngelscriptConsoleParser::Void:
      case AngelscriptConsoleParser::LeftParen:
      case AngelscriptConsoleParser::Doublecolon:
      case AngelscriptConsoleParser::Identifier: {
        enterOuterAlt(_localctx, 1);
        setState(675);
        simpleTypeSpecifier();
        break;
      }

      case AngelscriptConsoleParser::Const: {
        enterOuterAlt(_localctx, 2);
        setState(676);
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
  enterRule(_localctx, 130, AngelscriptConsoleParser::RuleTypeSpecifierSeq);
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
    setState(680); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(679);
      typeSpecifier();
      setState(682); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TrailingTypeSpecifierSeqContext ------------------------------------------------------------------

AngelscriptConsoleParser::TrailingTypeSpecifierSeqContext::TrailingTypeSpecifierSeqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::TrailingTypeSpecifierContext *> AngelscriptConsoleParser::TrailingTypeSpecifierSeqContext::trailingTypeSpecifier() {
  return getRuleContexts<AngelscriptConsoleParser::TrailingTypeSpecifierContext>();
}

AngelscriptConsoleParser::TrailingTypeSpecifierContext* AngelscriptConsoleParser::TrailingTypeSpecifierSeqContext::trailingTypeSpecifier(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::TrailingTypeSpecifierContext>(i);
}


size_t AngelscriptConsoleParser::TrailingTypeSpecifierSeqContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTrailingTypeSpecifierSeq;
}


std::any AngelscriptConsoleParser::TrailingTypeSpecifierSeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTrailingTypeSpecifierSeq(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TrailingTypeSpecifierSeqContext* AngelscriptConsoleParser::trailingTypeSpecifierSeq() {
  TrailingTypeSpecifierSeqContext *_localctx = _tracker.createInstance<TrailingTypeSpecifierSeqContext>(_ctx, getState());
  enterRule(_localctx, 132, AngelscriptConsoleParser::RuleTrailingTypeSpecifierSeq);
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
    setState(685); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(684);
      trailingTypeSpecifier();
      setState(687); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier);
   
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

AngelscriptConsoleParser::TheTypeNameContext* AngelscriptConsoleParser::SimpleTypeSpecifierContext::theTypeName() {
  return getRuleContext<AngelscriptConsoleParser::TheTypeNameContext>(0);
}

AngelscriptConsoleParser::NestedNameSpecifierContext* AngelscriptConsoleParser::SimpleTypeSpecifierContext::nestedNameSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::NestedNameSpecifierContext>(0);
}

AngelscriptConsoleParser::AsGenericContext* AngelscriptConsoleParser::SimpleTypeSpecifierContext::asGeneric() {
  return getRuleContext<AngelscriptConsoleParser::AsGenericContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Int() {
  return getToken(AngelscriptConsoleParser::Int, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Int8() {
  return getToken(AngelscriptConsoleParser::Int8, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Int16() {
  return getToken(AngelscriptConsoleParser::Int16, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Int32() {
  return getToken(AngelscriptConsoleParser::Int32, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Int64() {
  return getToken(AngelscriptConsoleParser::Int64, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::UInt() {
  return getToken(AngelscriptConsoleParser::UInt, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::UInt8() {
  return getToken(AngelscriptConsoleParser::UInt8, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::UInt16() {
  return getToken(AngelscriptConsoleParser::UInt16, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::UInt32() {
  return getToken(AngelscriptConsoleParser::UInt32, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::UInt64() {
  return getToken(AngelscriptConsoleParser::UInt64, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Float() {
  return getToken(AngelscriptConsoleParser::Float, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Double() {
  return getToken(AngelscriptConsoleParser::Double, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Bool() {
  return getToken(AngelscriptConsoleParser::Bool, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Void() {
  return getToken(AngelscriptConsoleParser::Void, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::SimpleTypeSpecifierContext::Auto() {
  return getToken(AngelscriptConsoleParser::Auto, 0);
}

AngelscriptConsoleParser::DecltypeSpecifierContext* AngelscriptConsoleParser::SimpleTypeSpecifierContext::decltypeSpecifier() {
  return getRuleContext<AngelscriptConsoleParser::DecltypeSpecifierContext>(0);
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
  enterRule(_localctx, 134, AngelscriptConsoleParser::RuleSimpleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(710);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 71, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(690);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 70, _ctx)) {
      case 1: {
        setState(689);
        nestedNameSpecifier(0);
        break;
      }

      default:
        break;
      }
      setState(692);
      theTypeName();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(693);
      asGeneric();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(694);
      match(AngelscriptConsoleParser::Int);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(695);
      match(AngelscriptConsoleParser::Int8);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(696);
      match(AngelscriptConsoleParser::Int16);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(697);
      match(AngelscriptConsoleParser::Int32);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(698);
      match(AngelscriptConsoleParser::Int64);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(699);
      match(AngelscriptConsoleParser::UInt);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(700);
      match(AngelscriptConsoleParser::UInt8);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(701);
      match(AngelscriptConsoleParser::UInt16);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(702);
      match(AngelscriptConsoleParser::UInt32);
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(703);
      match(AngelscriptConsoleParser::UInt64);
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(704);
      match(AngelscriptConsoleParser::Float);
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(705);
      match(AngelscriptConsoleParser::Double);
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(706);
      match(AngelscriptConsoleParser::Bool);
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(707);
      match(AngelscriptConsoleParser::Void);
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(708);
      match(AngelscriptConsoleParser::Auto);
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(709);
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
  enterRule(_localctx, 136, AngelscriptConsoleParser::RuleTheTypeName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(712);
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
  enterRule(_localctx, 138, AngelscriptConsoleParser::RuleDecltypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(714);
    match(AngelscriptConsoleParser::LeftParen);
    setState(717);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 72, _ctx)) {
    case 1: {
      setState(715);
      expression();
      break;
    }

    case 2: {
      setState(716);
      match(AngelscriptConsoleParser::Auto);
      break;
    }

    default:
      break;
    }
    setState(719);
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
  enterRule(_localctx, 140, AngelscriptConsoleParser::RuleNamespaceName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(721);
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
  enterRule(_localctx, 142, AngelscriptConsoleParser::RuleQualifiednamespacespecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(724);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 73, _ctx)) {
    case 1: {
      setState(723);
      nestedNameSpecifier(0);
      break;
    }

    default:
      break;
    }
    setState(726);
    namespaceName();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BalancedTokenSeqContext ------------------------------------------------------------------

AngelscriptConsoleParser::BalancedTokenSeqContext::BalancedTokenSeqContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::BalancedtokenContext *> AngelscriptConsoleParser::BalancedTokenSeqContext::balancedtoken() {
  return getRuleContexts<AngelscriptConsoleParser::BalancedtokenContext>();
}

AngelscriptConsoleParser::BalancedtokenContext* AngelscriptConsoleParser::BalancedTokenSeqContext::balancedtoken(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::BalancedtokenContext>(i);
}


size_t AngelscriptConsoleParser::BalancedTokenSeqContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleBalancedTokenSeq;
}


std::any AngelscriptConsoleParser::BalancedTokenSeqContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitBalancedTokenSeq(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::BalancedTokenSeqContext* AngelscriptConsoleParser::balancedTokenSeq() {
  BalancedTokenSeqContext *_localctx = _tracker.createInstance<BalancedTokenSeqContext>(_ctx, getState());
  enterRule(_localctx, 144, AngelscriptConsoleParser::RuleBalancedTokenSeq);
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
    setState(729); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(728);
      balancedtoken();
      setState(731); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & -46179488366594) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & 8589934591) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BalancedtokenContext ------------------------------------------------------------------

AngelscriptConsoleParser::BalancedtokenContext::BalancedtokenContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::BalancedtokenContext::LeftParen() {
  return getTokens(AngelscriptConsoleParser::LeftParen);
}

tree::TerminalNode* AngelscriptConsoleParser::BalancedtokenContext::LeftParen(size_t i) {
  return getToken(AngelscriptConsoleParser::LeftParen, i);
}

AngelscriptConsoleParser::BalancedTokenSeqContext* AngelscriptConsoleParser::BalancedtokenContext::balancedTokenSeq() {
  return getRuleContext<AngelscriptConsoleParser::BalancedTokenSeqContext>(0);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::BalancedtokenContext::RightParen() {
  return getTokens(AngelscriptConsoleParser::RightParen);
}

tree::TerminalNode* AngelscriptConsoleParser::BalancedtokenContext::RightParen(size_t i) {
  return getToken(AngelscriptConsoleParser::RightParen, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::BalancedtokenContext::LeftBracket() {
  return getTokens(AngelscriptConsoleParser::LeftBracket);
}

tree::TerminalNode* AngelscriptConsoleParser::BalancedtokenContext::LeftBracket(size_t i) {
  return getToken(AngelscriptConsoleParser::LeftBracket, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::BalancedtokenContext::RightBracket() {
  return getTokens(AngelscriptConsoleParser::RightBracket);
}

tree::TerminalNode* AngelscriptConsoleParser::BalancedtokenContext::RightBracket(size_t i) {
  return getToken(AngelscriptConsoleParser::RightBracket, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::BalancedtokenContext::LeftBrace() {
  return getTokens(AngelscriptConsoleParser::LeftBrace);
}

tree::TerminalNode* AngelscriptConsoleParser::BalancedtokenContext::LeftBrace(size_t i) {
  return getToken(AngelscriptConsoleParser::LeftBrace, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::BalancedtokenContext::RightBrace() {
  return getTokens(AngelscriptConsoleParser::RightBrace);
}

tree::TerminalNode* AngelscriptConsoleParser::BalancedtokenContext::RightBrace(size_t i) {
  return getToken(AngelscriptConsoleParser::RightBrace, i);
}


size_t AngelscriptConsoleParser::BalancedtokenContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleBalancedtoken;
}


std::any AngelscriptConsoleParser::BalancedtokenContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitBalancedtoken(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::BalancedtokenContext* AngelscriptConsoleParser::balancedtoken() {
  BalancedtokenContext *_localctx = _tracker.createInstance<BalancedtokenContext>(_ctx, getState());
  enterRule(_localctx, 146, AngelscriptConsoleParser::RuleBalancedtoken);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(750);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 1);
        setState(733);
        match(AngelscriptConsoleParser::LeftParen);
        setState(734);
        balancedTokenSeq();
        setState(735);
        match(AngelscriptConsoleParser::RightParen);
        break;
      }

      case AngelscriptConsoleParser::LeftBracket: {
        enterOuterAlt(_localctx, 2);
        setState(737);
        match(AngelscriptConsoleParser::LeftBracket);
        setState(738);
        balancedTokenSeq();
        setState(739);
        match(AngelscriptConsoleParser::RightBracket);
        break;
      }

      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 3);
        setState(741);
        match(AngelscriptConsoleParser::LeftBrace);
        setState(742);
        balancedTokenSeq();
        setState(743);
        match(AngelscriptConsoleParser::RightBrace);
        break;
      }

      case AngelscriptConsoleParser::IntegerLiteral:
      case AngelscriptConsoleParser::CharacterLiteral:
      case AngelscriptConsoleParser::FloatingLiteral:
      case AngelscriptConsoleParser::StringLiteral:
      case AngelscriptConsoleParser::UserDefinedLiteral:
      case AngelscriptConsoleParser::Cast:
      case AngelscriptConsoleParser::Int:
      case AngelscriptConsoleParser::Int8:
      case AngelscriptConsoleParser::Int16:
      case AngelscriptConsoleParser::Int32:
      case AngelscriptConsoleParser::Int64:
      case AngelscriptConsoleParser::UInt:
      case AngelscriptConsoleParser::UInt8:
      case AngelscriptConsoleParser::UInt16:
      case AngelscriptConsoleParser::UInt32:
      case AngelscriptConsoleParser::UInt64:
      case AngelscriptConsoleParser::Float:
      case AngelscriptConsoleParser::Double:
      case AngelscriptConsoleParser::Bool:
      case AngelscriptConsoleParser::Auto:
      case AngelscriptConsoleParser::Break:
      case AngelscriptConsoleParser::Case:
      case AngelscriptConsoleParser::Catch:
      case AngelscriptConsoleParser::Char:
      case AngelscriptConsoleParser::Const:
      case AngelscriptConsoleParser::Continue:
      case AngelscriptConsoleParser::Default:
      case AngelscriptConsoleParser::Do:
      case AngelscriptConsoleParser::Else:
      case AngelscriptConsoleParser::False_:
      case AngelscriptConsoleParser::For:
      case AngelscriptConsoleParser::If:
      case AngelscriptConsoleParser::Nullptr:
      case AngelscriptConsoleParser::Return:
      case AngelscriptConsoleParser::Short:
      case AngelscriptConsoleParser::Switch:
      case AngelscriptConsoleParser::True_:
      case AngelscriptConsoleParser::Void:
      case AngelscriptConsoleParser::While:
      case AngelscriptConsoleParser::Plus:
      case AngelscriptConsoleParser::Minus:
      case AngelscriptConsoleParser::Star:
      case AngelscriptConsoleParser::Div:
      case AngelscriptConsoleParser::Mod:
      case AngelscriptConsoleParser::Xor:
      case AngelscriptConsoleParser::And:
      case AngelscriptConsoleParser::Or:
      case AngelscriptConsoleParser::Tilde:
      case AngelscriptConsoleParser::Not:
      case AngelscriptConsoleParser::Assign:
      case AngelscriptConsoleParser::Less:
      case AngelscriptConsoleParser::Greater:
      case AngelscriptConsoleParser::PlusAssign:
      case AngelscriptConsoleParser::MinusAssign:
      case AngelscriptConsoleParser::StarAssign:
      case AngelscriptConsoleParser::DivAssign:
      case AngelscriptConsoleParser::ModAssign:
      case AngelscriptConsoleParser::XorAssign:
      case AngelscriptConsoleParser::AndAssign:
      case AngelscriptConsoleParser::OrAssign:
      case AngelscriptConsoleParser::LeftShiftAssign:
      case AngelscriptConsoleParser::RightShiftAssign:
      case AngelscriptConsoleParser::Equal:
      case AngelscriptConsoleParser::NotEqual:
      case AngelscriptConsoleParser::LessEqual:
      case AngelscriptConsoleParser::GreaterEqual:
      case AngelscriptConsoleParser::AndAnd:
      case AngelscriptConsoleParser::OrOr:
      case AngelscriptConsoleParser::PlusPlus:
      case AngelscriptConsoleParser::MinusMinus:
      case AngelscriptConsoleParser::Comma:
      case AngelscriptConsoleParser::Question:
      case AngelscriptConsoleParser::Colon:
      case AngelscriptConsoleParser::Doublecolon:
      case AngelscriptConsoleParser::Semi:
      case AngelscriptConsoleParser::Dot:
      case AngelscriptConsoleParser::Identifier:
      case AngelscriptConsoleParser::DecimalLiteral:
      case AngelscriptConsoleParser::OctalLiteral:
      case AngelscriptConsoleParser::HexadecimalLiteral:
      case AngelscriptConsoleParser::BinaryLiteral:
      case AngelscriptConsoleParser::Integersuffix:
      case AngelscriptConsoleParser::UserDefinedIntegerLiteral:
      case AngelscriptConsoleParser::UserDefinedFloatingLiteral:
      case AngelscriptConsoleParser::UserDefinedStringLiteral:
      case AngelscriptConsoleParser::UserDefinedCharacterLiteral:
      case AngelscriptConsoleParser::Whitespace:
      case AngelscriptConsoleParser::Newline:
      case AngelscriptConsoleParser::BlockComment:
      case AngelscriptConsoleParser::LineComment: {
        enterOuterAlt(_localctx, 4);
        setState(746); 
        _errHandler->sync(this);
        alt = 1;
        do {
          switch (alt) {
            case 1: {
                  setState(745);
                  _la = _input->LA(1);
                  if (_la == 0 || _la == Token::EOF || ((((_la & ~ 0x3fULL) == 0) &&
                    ((1ULL << _la) & 69269232549888) != 0))) {
                  _errHandler->recoverInline(this);
                  }
                  else {
                    _errHandler->reportMatch(this);
                    consume();
                  }
                  break;
                }

          default:
            throw NoViableAltException(this);
          }
          setState(748); 
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 75, _ctx);
        } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
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
  enterRule(_localctx, 148, AngelscriptConsoleParser::RuleInitDeclaratorList);
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
    setState(752);
    initDeclarator();
    setState(757);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(753);
      match(AngelscriptConsoleParser::Comma);
      setState(754);
      initDeclarator();
      setState(759);
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
  enterRule(_localctx, 150, AngelscriptConsoleParser::RuleInitDeclarator);
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
    setState(760);
    match(AngelscriptConsoleParser::Identifier);
    setState(762);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72080683782111232) != 0)) {
      setState(761);
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

//----------------- DeclaratorContext ------------------------------------------------------------------

AngelscriptConsoleParser::DeclaratorContext::DeclaratorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::DeclaratorDefContext* AngelscriptConsoleParser::DeclaratorContext::declaratorDef() {
  return getRuleContext<AngelscriptConsoleParser::DeclaratorDefContext>(0);
}

AngelscriptConsoleParser::ParametersAndQualifiersContext* AngelscriptConsoleParser::DeclaratorContext::parametersAndQualifiers() {
  return getRuleContext<AngelscriptConsoleParser::ParametersAndQualifiersContext>(0);
}


size_t AngelscriptConsoleParser::DeclaratorContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDeclarator;
}


std::any AngelscriptConsoleParser::DeclaratorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDeclarator(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::DeclaratorContext* AngelscriptConsoleParser::declarator() {
  DeclaratorContext *_localctx = _tracker.createInstance<DeclaratorContext>(_ctx, getState());
  enterRule(_localctx, 152, AngelscriptConsoleParser::RuleDeclarator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(764);
    declaratorDef(0);
    setState(765);
    parametersAndQualifiers();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclaratorDefContext ------------------------------------------------------------------

AngelscriptConsoleParser::DeclaratorDefContext::DeclaratorDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::DeclaratoridContext* AngelscriptConsoleParser::DeclaratorDefContext::declaratorid() {
  return getRuleContext<AngelscriptConsoleParser::DeclaratoridContext>(0);
}

AngelscriptConsoleParser::DeclaratorDefContext* AngelscriptConsoleParser::DeclaratorDefContext::declaratorDef() {
  return getRuleContext<AngelscriptConsoleParser::DeclaratorDefContext>(0);
}

AngelscriptConsoleParser::ParametersAndQualifiersContext* AngelscriptConsoleParser::DeclaratorDefContext::parametersAndQualifiers() {
  return getRuleContext<AngelscriptConsoleParser::ParametersAndQualifiersContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::DeclaratorDefContext::LeftBracket() {
  return getToken(AngelscriptConsoleParser::LeftBracket, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::DeclaratorDefContext::RightBracket() {
  return getToken(AngelscriptConsoleParser::RightBracket, 0);
}

AngelscriptConsoleParser::ConstantExpressionContext* AngelscriptConsoleParser::DeclaratorDefContext::constantExpression() {
  return getRuleContext<AngelscriptConsoleParser::ConstantExpressionContext>(0);
}


size_t AngelscriptConsoleParser::DeclaratorDefContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDeclaratorDef;
}


std::any AngelscriptConsoleParser::DeclaratorDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDeclaratorDef(this);
  else
    return visitor->visitChildren(this);
}


AngelscriptConsoleParser::DeclaratorDefContext* AngelscriptConsoleParser::declaratorDef() {
   return declaratorDef(0);
}

AngelscriptConsoleParser::DeclaratorDefContext* AngelscriptConsoleParser::declaratorDef(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  AngelscriptConsoleParser::DeclaratorDefContext *_localctx = _tracker.createInstance<DeclaratorDefContext>(_ctx, parentState);
  AngelscriptConsoleParser::DeclaratorDefContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 154;
  enterRecursionRule(_localctx, 154, AngelscriptConsoleParser::RuleDeclaratorDef, precedence);

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
    setState(768);
    declaratorid();
    _ctx->stop = _input->LT(-1);
    setState(781);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 81, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<DeclaratorDefContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleDeclaratorDef);
        setState(770);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(777);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case AngelscriptConsoleParser::LeftParen: {
            setState(771);
            parametersAndQualifiers();
            break;
          }

          case AngelscriptConsoleParser::LeftBracket: {
            setState(772);
            match(AngelscriptConsoleParser::LeftBracket);
            setState(774);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if ((((_la & ~ 0x3fULL) == 0) &&
              ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
              ((1ULL << (_la - 75)) & 291) != 0)) {
              setState(773);
              constantExpression();
            }
            setState(776);
            match(AngelscriptConsoleParser::RightBracket);
            break;
          }

        default:
          throw NoViableAltException(this);
        } 
      }
      setState(783);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 81, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- ParametersAndQualifiersContext ------------------------------------------------------------------

AngelscriptConsoleParser::ParametersAndQualifiersContext::ParametersAndQualifiersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::ParametersAndQualifiersContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::ParametersAndQualifiersContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

AngelscriptConsoleParser::ParameterDeclarationClauseContext* AngelscriptConsoleParser::ParametersAndQualifiersContext::parameterDeclarationClause() {
  return getRuleContext<AngelscriptConsoleParser::ParameterDeclarationClauseContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::ParametersAndQualifiersContext::Const() {
  return getToken(AngelscriptConsoleParser::Const, 0);
}

AngelscriptConsoleParser::RefqualifierContext* AngelscriptConsoleParser::ParametersAndQualifiersContext::refqualifier() {
  return getRuleContext<AngelscriptConsoleParser::RefqualifierContext>(0);
}


size_t AngelscriptConsoleParser::ParametersAndQualifiersContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleParametersAndQualifiers;
}


std::any AngelscriptConsoleParser::ParametersAndQualifiersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitParametersAndQualifiers(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ParametersAndQualifiersContext* AngelscriptConsoleParser::parametersAndQualifiers() {
  ParametersAndQualifiersContext *_localctx = _tracker.createInstance<ParametersAndQualifiersContext>(_ctx, getState());
  enterRule(_localctx, 156, AngelscriptConsoleParser::RuleParametersAndQualifiers);
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
    setState(784);
    match(AngelscriptConsoleParser::LeftParen);
    setState(786);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier) {
      setState(785);
      parameterDeclarationClause();
    }
    setState(788);
    match(AngelscriptConsoleParser::RightParen);
    setState(790);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 83, _ctx)) {
    case 1: {
      setState(789);
      match(AngelscriptConsoleParser::Const);
      break;
    }

    default:
      break;
    }
    setState(793);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 84, _ctx)) {
    case 1: {
      setState(792);
      refqualifier();
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

//----------------- RefqualifierContext ------------------------------------------------------------------

AngelscriptConsoleParser::RefqualifierContext::RefqualifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::RefqualifierContext::And() {
  return getToken(AngelscriptConsoleParser::And, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::RefqualifierContext::AndAnd() {
  return getToken(AngelscriptConsoleParser::AndAnd, 0);
}


size_t AngelscriptConsoleParser::RefqualifierContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleRefqualifier;
}


std::any AngelscriptConsoleParser::RefqualifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitRefqualifier(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::RefqualifierContext* AngelscriptConsoleParser::refqualifier() {
  RefqualifierContext *_localctx = _tracker.createInstance<RefqualifierContext>(_ctx, getState());
  enterRule(_localctx, 158, AngelscriptConsoleParser::RuleRefqualifier);
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
    setState(795);
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::And

    || _la == AngelscriptConsoleParser::AndAnd)) {
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

//----------------- DeclaratoridContext ------------------------------------------------------------------

AngelscriptConsoleParser::DeclaratoridContext::DeclaratoridContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::IdExpressionContext* AngelscriptConsoleParser::DeclaratoridContext::idExpression() {
  return getRuleContext<AngelscriptConsoleParser::IdExpressionContext>(0);
}


size_t AngelscriptConsoleParser::DeclaratoridContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleDeclaratorid;
}


std::any AngelscriptConsoleParser::DeclaratoridContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitDeclaratorid(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::DeclaratoridContext* AngelscriptConsoleParser::declaratorid() {
  DeclaratoridContext *_localctx = _tracker.createInstance<DeclaratoridContext>(_ctx, getState());
  enterRule(_localctx, 160, AngelscriptConsoleParser::RuleDeclaratorid);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(797);
    idExpression();
   
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
  enterRule(_localctx, 162, AngelscriptConsoleParser::RuleTheTypeId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(799);
    typeSpecifierSeq();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterDeclarationClauseContext ------------------------------------------------------------------

AngelscriptConsoleParser::ParameterDeclarationClauseContext::ParameterDeclarationClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::ParameterDeclarationListContext* AngelscriptConsoleParser::ParameterDeclarationClauseContext::parameterDeclarationList() {
  return getRuleContext<AngelscriptConsoleParser::ParameterDeclarationListContext>(0);
}


size_t AngelscriptConsoleParser::ParameterDeclarationClauseContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleParameterDeclarationClause;
}


std::any AngelscriptConsoleParser::ParameterDeclarationClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitParameterDeclarationClause(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ParameterDeclarationClauseContext* AngelscriptConsoleParser::parameterDeclarationClause() {
  ParameterDeclarationClauseContext *_localctx = _tracker.createInstance<ParameterDeclarationClauseContext>(_ctx, getState());
  enterRule(_localctx, 164, AngelscriptConsoleParser::RuleParameterDeclarationClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(801);
    parameterDeclarationList();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterDeclarationListContext ------------------------------------------------------------------

AngelscriptConsoleParser::ParameterDeclarationListContext::ParameterDeclarationListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptConsoleParser::ParameterDeclarationContext *> AngelscriptConsoleParser::ParameterDeclarationListContext::parameterDeclaration() {
  return getRuleContexts<AngelscriptConsoleParser::ParameterDeclarationContext>();
}

AngelscriptConsoleParser::ParameterDeclarationContext* AngelscriptConsoleParser::ParameterDeclarationListContext::parameterDeclaration(size_t i) {
  return getRuleContext<AngelscriptConsoleParser::ParameterDeclarationContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::ParameterDeclarationListContext::Comma() {
  return getTokens(AngelscriptConsoleParser::Comma);
}

tree::TerminalNode* AngelscriptConsoleParser::ParameterDeclarationListContext::Comma(size_t i) {
  return getToken(AngelscriptConsoleParser::Comma, i);
}


size_t AngelscriptConsoleParser::ParameterDeclarationListContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleParameterDeclarationList;
}


std::any AngelscriptConsoleParser::ParameterDeclarationListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitParameterDeclarationList(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ParameterDeclarationListContext* AngelscriptConsoleParser::parameterDeclarationList() {
  ParameterDeclarationListContext *_localctx = _tracker.createInstance<ParameterDeclarationListContext>(_ctx, getState());
  enterRule(_localctx, 166, AngelscriptConsoleParser::RuleParameterDeclarationList);
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
    setState(803);
    parameterDeclaration();
    setState(808);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(804);
      match(AngelscriptConsoleParser::Comma);
      setState(805);
      parameterDeclaration();
      setState(810);
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

//----------------- ParameterDeclarationContext ------------------------------------------------------------------

AngelscriptConsoleParser::ParameterDeclarationContext::ParameterDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::ParameterDeclarationContext::declSpecifierSeq() {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierSeqContext>(0);
}

tree::TerminalNode* AngelscriptConsoleParser::ParameterDeclarationContext::Identifier() {
  return getToken(AngelscriptConsoleParser::Identifier, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::ParameterDeclarationContext::Assign() {
  return getToken(AngelscriptConsoleParser::Assign, 0);
}

AngelscriptConsoleParser::InitializerClauseContext* AngelscriptConsoleParser::ParameterDeclarationContext::initializerClause() {
  return getRuleContext<AngelscriptConsoleParser::InitializerClauseContext>(0);
}


size_t AngelscriptConsoleParser::ParameterDeclarationContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleParameterDeclaration;
}


std::any AngelscriptConsoleParser::ParameterDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitParameterDeclaration(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::ParameterDeclarationContext* AngelscriptConsoleParser::parameterDeclaration() {
  ParameterDeclarationContext *_localctx = _tracker.createInstance<ParameterDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 168, AngelscriptConsoleParser::RuleParameterDeclaration);
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
    setState(811);
    declSpecifierSeq();
    setState(813);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Identifier) {
      setState(812);
      match(AngelscriptConsoleParser::Identifier);
    }
    setState(817);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Assign) {
      setState(815);
      match(AngelscriptConsoleParser::Assign);
      setState(816);
      initializerClause();
    }
   
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
  enterRule(_localctx, 170, AngelscriptConsoleParser::RuleInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(824);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace:
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(819);
        braceOrEqualInitializer();
        break;
      }

      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 2);
        setState(820);
        match(AngelscriptConsoleParser::LeftParen);
        setState(821);
        expressionList();
        setState(822);
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
  enterRule(_localctx, 172, AngelscriptConsoleParser::RuleBraceOrEqualInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(829);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(826);
        match(AngelscriptConsoleParser::Assign);
        setState(827);
        initializerClause();
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(828);
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
  enterRule(_localctx, 174, AngelscriptConsoleParser::RuleInitializerClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(833);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 90, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(831);
      assignmentExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(832);
      bracedInitList();
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
  enterRule(_localctx, 176, AngelscriptConsoleParser::RuleInitializerList);

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
    setState(835);
    initializerClause();
    setState(840);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 91, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(836);
        match(AngelscriptConsoleParser::Comma);
        setState(837);
        initializerClause(); 
      }
      setState(842);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 91, _ctx);
    }
    setState(844);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 92, _ctx)) {
    case 1: {
      setState(843);
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
  enterRule(_localctx, 178, AngelscriptConsoleParser::RuleBracedInitList);
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
    setState(846);
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::LeftBracket

    || _la == AngelscriptConsoleParser::LeftBrace)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(851);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 291) != 0)) {
      setState(847);
      initializerList();
      setState(849);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Comma) {
        setState(848);
        match(AngelscriptConsoleParser::Comma);
      }
    }
    setState(853);
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

//----------------- TheOperatorContext ------------------------------------------------------------------

AngelscriptConsoleParser::TheOperatorContext::TheOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Plus() {
  return getToken(AngelscriptConsoleParser::Plus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Minus() {
  return getToken(AngelscriptConsoleParser::Minus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Star() {
  return getToken(AngelscriptConsoleParser::Star, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Div() {
  return getToken(AngelscriptConsoleParser::Div, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Mod() {
  return getToken(AngelscriptConsoleParser::Mod, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Xor() {
  return getToken(AngelscriptConsoleParser::Xor, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::And() {
  return getToken(AngelscriptConsoleParser::And, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Or() {
  return getToken(AngelscriptConsoleParser::Or, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Tilde() {
  return getToken(AngelscriptConsoleParser::Tilde, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Not() {
  return getToken(AngelscriptConsoleParser::Not, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Assign() {
  return getToken(AngelscriptConsoleParser::Assign, 0);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::TheOperatorContext::Greater() {
  return getTokens(AngelscriptConsoleParser::Greater);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Greater(size_t i) {
  return getToken(AngelscriptConsoleParser::Greater, i);
}

std::vector<tree::TerminalNode *> AngelscriptConsoleParser::TheOperatorContext::Less() {
  return getTokens(AngelscriptConsoleParser::Less);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Less(size_t i) {
  return getToken(AngelscriptConsoleParser::Less, i);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::GreaterEqual() {
  return getToken(AngelscriptConsoleParser::GreaterEqual, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::PlusAssign() {
  return getToken(AngelscriptConsoleParser::PlusAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::MinusAssign() {
  return getToken(AngelscriptConsoleParser::MinusAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::StarAssign() {
  return getToken(AngelscriptConsoleParser::StarAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::ModAssign() {
  return getToken(AngelscriptConsoleParser::ModAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::XorAssign() {
  return getToken(AngelscriptConsoleParser::XorAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::AndAssign() {
  return getToken(AngelscriptConsoleParser::AndAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::OrAssign() {
  return getToken(AngelscriptConsoleParser::OrAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::RightShiftAssign() {
  return getToken(AngelscriptConsoleParser::RightShiftAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::LeftShiftAssign() {
  return getToken(AngelscriptConsoleParser::LeftShiftAssign, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Equal() {
  return getToken(AngelscriptConsoleParser::Equal, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::NotEqual() {
  return getToken(AngelscriptConsoleParser::NotEqual, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::LessEqual() {
  return getToken(AngelscriptConsoleParser::LessEqual, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::AndAnd() {
  return getToken(AngelscriptConsoleParser::AndAnd, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::OrOr() {
  return getToken(AngelscriptConsoleParser::OrOr, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::PlusPlus() {
  return getToken(AngelscriptConsoleParser::PlusPlus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::MinusMinus() {
  return getToken(AngelscriptConsoleParser::MinusMinus, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::Comma() {
  return getToken(AngelscriptConsoleParser::Comma, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::LeftParen() {
  return getToken(AngelscriptConsoleParser::LeftParen, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::RightParen() {
  return getToken(AngelscriptConsoleParser::RightParen, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::LeftBracket() {
  return getToken(AngelscriptConsoleParser::LeftBracket, 0);
}

tree::TerminalNode* AngelscriptConsoleParser::TheOperatorContext::RightBracket() {
  return getToken(AngelscriptConsoleParser::RightBracket, 0);
}


size_t AngelscriptConsoleParser::TheOperatorContext::getRuleIndex() const {
  return AngelscriptConsoleParser::RuleTheOperator;
}


std::any AngelscriptConsoleParser::TheOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptConsoleParserVisitor*>(visitor))
    return parserVisitor->visitTheOperator(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptConsoleParser::TheOperatorContext* AngelscriptConsoleParser::theOperator() {
  TheOperatorContext *_localctx = _tracker.createInstance<TheOperatorContext>(_ctx, getState());
  enterRule(_localctx, 180, AngelscriptConsoleParser::RuleTheOperator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(895);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 95, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);

      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(856);
      match(AngelscriptConsoleParser::Plus);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(857);
      match(AngelscriptConsoleParser::Minus);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(858);
      match(AngelscriptConsoleParser::Star);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(859);
      match(AngelscriptConsoleParser::Div);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(860);
      match(AngelscriptConsoleParser::Mod);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(861);
      match(AngelscriptConsoleParser::Xor);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(862);
      match(AngelscriptConsoleParser::And);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(863);
      match(AngelscriptConsoleParser::Or);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(864);
      match(AngelscriptConsoleParser::Tilde);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(865);
      match(AngelscriptConsoleParser::Not);
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(866);
      match(AngelscriptConsoleParser::Assign);
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(867);
      match(AngelscriptConsoleParser::Greater);
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(868);
      match(AngelscriptConsoleParser::Less);
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(869);
      match(AngelscriptConsoleParser::GreaterEqual);
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(870);
      match(AngelscriptConsoleParser::PlusAssign);
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(871);
      match(AngelscriptConsoleParser::MinusAssign);
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(872);
      match(AngelscriptConsoleParser::StarAssign);
      break;
    }

    case 19: {
      enterOuterAlt(_localctx, 19);
      setState(873);
      match(AngelscriptConsoleParser::ModAssign);
      break;
    }

    case 20: {
      enterOuterAlt(_localctx, 20);
      setState(874);
      match(AngelscriptConsoleParser::XorAssign);
      break;
    }

    case 21: {
      enterOuterAlt(_localctx, 21);
      setState(875);
      match(AngelscriptConsoleParser::AndAssign);
      break;
    }

    case 22: {
      enterOuterAlt(_localctx, 22);
      setState(876);
      match(AngelscriptConsoleParser::OrAssign);
      break;
    }

    case 23: {
      enterOuterAlt(_localctx, 23);
      setState(877);
      match(AngelscriptConsoleParser::Less);
      setState(878);
      match(AngelscriptConsoleParser::Less);
      break;
    }

    case 24: {
      enterOuterAlt(_localctx, 24);
      setState(879);
      match(AngelscriptConsoleParser::Greater);
      setState(880);
      match(AngelscriptConsoleParser::Greater);
      break;
    }

    case 25: {
      enterOuterAlt(_localctx, 25);
      setState(881);
      match(AngelscriptConsoleParser::RightShiftAssign);
      break;
    }

    case 26: {
      enterOuterAlt(_localctx, 26);
      setState(882);
      match(AngelscriptConsoleParser::LeftShiftAssign);
      break;
    }

    case 27: {
      enterOuterAlt(_localctx, 27);
      setState(883);
      match(AngelscriptConsoleParser::Equal);
      break;
    }

    case 28: {
      enterOuterAlt(_localctx, 28);
      setState(884);
      match(AngelscriptConsoleParser::NotEqual);
      break;
    }

    case 29: {
      enterOuterAlt(_localctx, 29);
      setState(885);
      match(AngelscriptConsoleParser::LessEqual);
      break;
    }

    case 30: {
      enterOuterAlt(_localctx, 30);
      setState(886);
      match(AngelscriptConsoleParser::AndAnd);
      break;
    }

    case 31: {
      enterOuterAlt(_localctx, 31);
      setState(887);
      match(AngelscriptConsoleParser::OrOr);
      break;
    }

    case 32: {
      enterOuterAlt(_localctx, 32);
      setState(888);
      match(AngelscriptConsoleParser::PlusPlus);
      break;
    }

    case 33: {
      enterOuterAlt(_localctx, 33);
      setState(889);
      match(AngelscriptConsoleParser::MinusMinus);
      break;
    }

    case 34: {
      enterOuterAlt(_localctx, 34);
      setState(890);
      match(AngelscriptConsoleParser::Comma);
      break;
    }

    case 35: {
      enterOuterAlt(_localctx, 35);
      setState(891);
      match(AngelscriptConsoleParser::LeftParen);
      setState(892);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 36: {
      enterOuterAlt(_localctx, 36);
      setState(893);
      match(AngelscriptConsoleParser::LeftBracket);
      setState(894);
      match(AngelscriptConsoleParser::RightBracket);
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
  enterRule(_localctx, 182, AngelscriptConsoleParser::RuleLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(904);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::IntegerLiteral: {
        enterOuterAlt(_localctx, 1);
        setState(897);
        match(AngelscriptConsoleParser::IntegerLiteral);
        break;
      }

      case AngelscriptConsoleParser::CharacterLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(898);
        match(AngelscriptConsoleParser::CharacterLiteral);
        break;
      }

      case AngelscriptConsoleParser::FloatingLiteral: {
        enterOuterAlt(_localctx, 3);
        setState(899);
        match(AngelscriptConsoleParser::FloatingLiteral);
        break;
      }

      case AngelscriptConsoleParser::StringLiteral: {
        enterOuterAlt(_localctx, 4);
        setState(900);
        match(AngelscriptConsoleParser::StringLiteral);
        break;
      }

      case AngelscriptConsoleParser::False_:
      case AngelscriptConsoleParser::True_: {
        enterOuterAlt(_localctx, 5);
        setState(901);
        booleanLiteral();
        break;
      }

      case AngelscriptConsoleParser::UserDefinedLiteral: {
        enterOuterAlt(_localctx, 6);
        setState(902);
        match(AngelscriptConsoleParser::UserDefinedLiteral);
        break;
      }

      case AngelscriptConsoleParser::Nullptr: {
        enterOuterAlt(_localctx, 7);
        setState(903);
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

bool AngelscriptConsoleParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 9: return nestedNameSpecifierSempred(antlrcpp::downCast<NestedNameSpecifierContext *>(context), predicateIndex);
    case 19: return postfixExpressionSempred(antlrcpp::downCast<PostfixExpressionContext *>(context), predicateIndex);
    case 77: return declaratorDefSempred(antlrcpp::downCast<DeclaratorDefContext *>(context), predicateIndex);

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

bool AngelscriptConsoleParser::declaratorDefSempred(DeclaratorDefContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 5: return precpred(_ctx, 1);

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
