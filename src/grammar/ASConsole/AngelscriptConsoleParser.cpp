
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
      "script", "asGeneric", "simpleTypeSpecifierList", "booleanLiteral", 
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
  	4,1,96,911,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
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
  	84,2,85,7,85,2,86,7,86,2,87,7,87,2,88,7,88,2,89,7,89,2,90,7,90,1,0,3,
  	0,184,8,0,1,0,1,0,3,0,188,8,0,1,0,3,0,191,8,0,1,1,1,1,1,1,1,1,1,1,1,2,
  	1,2,1,2,5,2,201,8,2,10,2,12,2,204,9,2,1,3,1,3,1,4,4,4,209,8,4,11,4,12,
  	4,210,1,4,1,4,1,4,1,4,1,4,1,4,3,4,219,8,4,1,5,1,5,3,5,223,8,5,1,6,1,6,
  	1,7,1,7,1,7,1,8,1,8,1,8,1,8,3,8,234,8,8,1,8,1,8,1,8,1,8,1,8,5,8,241,8,
  	8,10,8,12,8,244,9,8,1,9,1,9,3,9,248,8,9,1,9,1,9,1,10,1,10,3,10,254,8,
  	10,1,10,1,10,1,11,1,11,1,11,1,11,3,11,262,8,11,3,11,264,8,11,1,12,1,12,
  	1,13,1,13,1,13,5,13,271,8,13,10,13,12,13,274,9,13,1,14,1,14,3,14,278,
  	8,14,1,15,3,15,281,8,15,1,15,1,15,1,16,3,16,286,8,16,1,16,1,16,1,16,1,
  	17,1,17,3,17,293,8,17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,3,18,302,8,18,
  	1,18,1,18,3,18,306,8,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,
  	1,18,1,18,3,18,319,8,18,1,18,1,18,3,18,323,8,18,1,18,1,18,1,18,1,18,3,
  	18,329,8,18,1,18,1,18,1,18,1,18,1,18,3,18,336,8,18,1,18,1,18,1,18,1,18,
  	1,18,3,18,343,8,18,1,18,1,18,5,18,347,8,18,10,18,12,18,350,9,18,1,19,
  	1,19,1,20,3,20,355,8,20,1,20,1,20,1,20,3,20,360,8,20,1,20,1,20,1,20,1,
  	20,1,20,1,20,1,20,1,20,1,20,3,20,371,8,20,1,21,1,21,1,21,1,21,3,21,377,
  	8,21,1,21,1,21,1,21,1,21,1,21,3,21,384,8,21,1,22,1,22,1,23,1,23,1,23,
  	1,23,1,24,1,24,3,24,394,8,24,1,24,1,24,3,24,398,8,24,1,25,1,25,1,25,1,
  	25,1,25,1,25,1,25,1,25,1,25,3,25,409,8,25,1,26,1,26,1,26,5,26,414,8,26,
  	10,26,12,26,417,9,26,1,27,1,27,1,27,5,27,422,8,27,10,27,12,27,425,9,27,
  	1,28,1,28,1,28,1,28,5,28,431,8,28,10,28,12,28,434,9,28,1,29,1,29,1,29,
  	1,29,3,29,440,8,29,1,30,1,30,1,30,5,30,445,8,30,10,30,12,30,448,9,30,
  	1,31,1,31,1,31,5,31,453,8,31,10,31,12,31,456,9,31,1,32,1,32,1,32,5,32,
  	461,8,32,10,32,12,32,464,9,32,1,33,1,33,1,33,5,33,469,8,33,10,33,12,33,
  	472,9,33,1,34,1,34,1,34,5,34,477,8,34,10,34,12,34,480,9,34,1,35,1,35,
  	1,35,5,35,485,8,35,10,35,12,35,488,9,35,1,36,1,36,1,36,5,36,493,8,36,
  	10,36,12,36,496,9,36,1,37,1,37,1,37,1,37,1,37,1,37,3,37,504,8,37,1,38,
  	1,38,1,38,1,38,1,38,3,38,511,8,38,1,39,1,39,1,40,1,40,1,40,5,40,518,8,
  	40,10,40,12,40,521,9,40,1,41,1,41,1,42,1,42,1,42,1,42,1,42,1,42,3,42,
  	531,8,42,3,42,533,8,42,1,43,1,43,1,43,1,43,3,43,539,8,43,1,43,1,43,1,
  	43,1,44,3,44,545,8,44,1,44,1,44,1,45,1,45,3,45,551,8,45,1,45,1,45,1,46,
  	4,46,556,8,46,11,46,12,46,557,1,47,1,47,1,47,1,47,1,47,1,47,1,47,3,47,
  	567,8,47,1,47,1,47,1,47,1,47,1,47,1,47,3,47,575,8,47,1,48,1,48,1,48,1,
  	48,1,48,1,48,3,48,583,8,48,3,48,585,8,48,1,49,1,49,1,49,1,49,1,49,1,49,
  	1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,49,1,49,3,49,605,
  	8,49,1,49,1,49,3,49,609,8,49,1,49,1,49,1,49,1,49,3,49,615,8,49,1,49,1,
  	49,1,49,3,49,620,8,49,1,50,1,50,3,50,624,8,50,1,51,1,51,1,51,1,52,1,52,
  	3,52,631,8,52,1,53,1,53,1,53,1,53,1,53,3,53,638,8,53,3,53,640,8,53,1,
  	53,1,53,1,54,4,54,645,8,54,11,54,12,54,646,1,55,1,55,3,55,651,8,55,1,
  	56,1,56,1,56,1,56,1,56,1,57,3,57,659,8,57,1,57,1,57,3,57,663,8,57,1,57,
  	1,57,1,58,1,58,1,59,1,59,1,60,4,60,672,8,60,11,60,12,60,673,1,61,1,61,
  	1,62,1,62,1,63,1,63,3,63,682,8,63,1,64,4,64,685,8,64,11,64,12,64,686,
  	1,65,4,65,690,8,65,11,65,12,65,691,1,66,3,66,695,8,66,1,66,1,66,1,66,
  	1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,66,1,66,
  	1,66,3,66,715,8,66,1,67,1,67,1,68,1,68,1,68,3,68,722,8,68,1,68,1,68,1,
  	69,1,69,1,70,3,70,729,8,70,1,70,1,70,1,71,4,71,734,8,71,11,71,12,71,735,
  	1,72,1,72,1,72,1,72,1,72,1,72,1,72,1,72,1,72,1,72,1,72,1,72,1,72,4,72,
  	751,8,72,11,72,12,72,752,3,72,755,8,72,1,73,1,73,1,73,5,73,760,8,73,10,
  	73,12,73,763,9,73,1,74,1,74,3,74,767,8,74,1,75,1,75,1,75,1,76,1,76,1,
  	76,1,76,1,76,1,76,1,76,3,76,779,8,76,1,76,3,76,782,8,76,5,76,784,8,76,
  	10,76,12,76,787,9,76,1,77,1,77,3,77,791,8,77,1,77,1,77,3,77,795,8,77,
  	1,77,3,77,798,8,77,1,78,1,78,1,79,1,79,1,80,1,80,1,81,1,81,1,82,1,82,
  	1,82,5,82,811,8,82,10,82,12,82,814,9,82,1,83,1,83,3,83,818,8,83,1,83,
  	1,83,3,83,822,8,83,1,84,1,84,1,84,1,84,1,84,3,84,829,8,84,1,85,1,85,1,
  	85,3,85,834,8,85,1,86,1,86,3,86,838,8,86,1,87,1,87,1,87,5,87,843,8,87,
  	10,87,12,87,846,9,87,1,87,3,87,849,8,87,1,88,1,88,1,88,3,88,854,8,88,
  	3,88,856,8,88,1,88,1,88,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,
  	1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,
  	1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,1,89,
  	1,89,1,89,1,89,3,89,900,8,89,1,90,1,90,1,90,1,90,1,90,1,90,1,90,3,90,
  	909,8,90,1,90,1,673,3,16,36,152,91,0,2,4,6,8,10,12,14,16,18,20,22,24,
  	26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,
  	72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,
  	114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,148,
  	150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,0,13,
  	2,0,30,30,37,37,2,0,52,52,56,56,1,0,75,76,2,0,46,48,52,55,1,0,48,50,1,
  	0,46,47,2,0,57,58,71,72,1,0,69,70,2,0,56,56,59,68,1,0,40,45,2,0,52,52,
  	73,73,2,0,42,42,44,44,2,0,43,43,45,45,997,0,190,1,0,0,0,2,192,1,0,0,0,
  	4,197,1,0,0,0,6,205,1,0,0,0,8,218,1,0,0,0,10,222,1,0,0,0,12,224,1,0,0,
  	0,14,226,1,0,0,0,16,229,1,0,0,0,18,245,1,0,0,0,20,251,1,0,0,0,22,263,
  	1,0,0,0,24,265,1,0,0,0,26,267,1,0,0,0,28,277,1,0,0,0,30,280,1,0,0,0,32,
  	285,1,0,0,0,34,290,1,0,0,0,36,322,1,0,0,0,38,351,1,0,0,0,40,370,1,0,0,
  	0,42,383,1,0,0,0,44,385,1,0,0,0,46,387,1,0,0,0,48,397,1,0,0,0,50,408,
  	1,0,0,0,52,410,1,0,0,0,54,418,1,0,0,0,56,426,1,0,0,0,58,439,1,0,0,0,60,
  	441,1,0,0,0,62,449,1,0,0,0,64,457,1,0,0,0,66,465,1,0,0,0,68,473,1,0,0,
  	0,70,481,1,0,0,0,72,489,1,0,0,0,74,497,1,0,0,0,76,510,1,0,0,0,78,512,
  	1,0,0,0,80,514,1,0,0,0,82,522,1,0,0,0,84,532,1,0,0,0,86,538,1,0,0,0,88,
  	544,1,0,0,0,90,548,1,0,0,0,92,555,1,0,0,0,94,574,1,0,0,0,96,584,1,0,0,
  	0,98,619,1,0,0,0,100,623,1,0,0,0,102,625,1,0,0,0,104,630,1,0,0,0,106,
  	639,1,0,0,0,108,644,1,0,0,0,110,650,1,0,0,0,112,652,1,0,0,0,114,658,1,
  	0,0,0,116,666,1,0,0,0,118,668,1,0,0,0,120,671,1,0,0,0,122,675,1,0,0,0,
  	124,677,1,0,0,0,126,681,1,0,0,0,128,684,1,0,0,0,130,689,1,0,0,0,132,714,
  	1,0,0,0,134,716,1,0,0,0,136,718,1,0,0,0,138,725,1,0,0,0,140,728,1,0,0,
  	0,142,733,1,0,0,0,144,754,1,0,0,0,146,756,1,0,0,0,148,764,1,0,0,0,150,
  	768,1,0,0,0,152,771,1,0,0,0,154,788,1,0,0,0,156,799,1,0,0,0,158,801,1,
  	0,0,0,160,803,1,0,0,0,162,805,1,0,0,0,164,807,1,0,0,0,166,815,1,0,0,0,
  	168,828,1,0,0,0,170,833,1,0,0,0,172,837,1,0,0,0,174,839,1,0,0,0,176,850,
  	1,0,0,0,178,899,1,0,0,0,180,908,1,0,0,0,182,184,3,88,44,0,183,182,1,0,
  	0,0,183,184,1,0,0,0,184,185,1,0,0,0,185,191,5,0,0,1,186,188,3,92,46,0,
  	187,186,1,0,0,0,187,188,1,0,0,0,188,189,1,0,0,0,189,191,5,0,0,1,190,183,
  	1,0,0,0,190,187,1,0,0,0,191,1,1,0,0,0,192,193,5,83,0,0,193,194,5,57,0,
  	0,194,195,3,4,2,0,195,196,5,58,0,0,196,3,1,0,0,0,197,202,3,120,60,0,198,
  	199,5,77,0,0,199,201,3,120,60,0,200,198,1,0,0,0,201,204,1,0,0,0,202,200,
  	1,0,0,0,202,203,1,0,0,0,203,5,1,0,0,0,204,202,1,0,0,0,205,206,7,0,0,0,
  	206,7,1,0,0,0,207,209,3,180,90,0,208,207,1,0,0,0,209,210,1,0,0,0,210,
  	208,1,0,0,0,210,211,1,0,0,0,211,219,1,0,0,0,212,213,5,40,0,0,213,214,
  	3,80,40,0,214,215,5,41,0,0,215,219,1,0,0,0,216,219,3,10,5,0,217,219,3,
  	18,9,0,218,208,1,0,0,0,218,212,1,0,0,0,218,216,1,0,0,0,218,217,1,0,0,
  	0,219,9,1,0,0,0,220,223,3,12,6,0,221,223,3,14,7,0,222,220,1,0,0,0,222,
  	221,1,0,0,0,223,11,1,0,0,0,224,225,5,83,0,0,225,13,1,0,0,0,226,227,3,
  	16,8,0,227,228,3,12,6,0,228,15,1,0,0,0,229,233,6,8,-1,0,230,234,3,134,
  	67,0,231,234,3,138,69,0,232,234,3,136,68,0,233,230,1,0,0,0,233,231,1,
  	0,0,0,233,232,1,0,0,0,233,234,1,0,0,0,234,235,1,0,0,0,235,236,5,80,0,
  	0,236,242,1,0,0,0,237,238,10,1,0,0,238,239,5,83,0,0,239,241,5,80,0,0,
  	240,237,1,0,0,0,241,244,1,0,0,0,242,240,1,0,0,0,242,243,1,0,0,0,243,17,
  	1,0,0,0,244,242,1,0,0,0,245,247,3,20,10,0,246,248,3,34,17,0,247,246,1,
  	0,0,0,247,248,1,0,0,0,248,249,1,0,0,0,249,250,3,90,45,0,250,19,1,0,0,
  	0,251,253,5,42,0,0,252,254,3,22,11,0,253,252,1,0,0,0,253,254,1,0,0,0,
  	254,255,1,0,0,0,255,256,5,43,0,0,256,21,1,0,0,0,257,264,3,26,13,0,258,
  	261,3,24,12,0,259,260,5,77,0,0,260,262,3,26,13,0,261,259,1,0,0,0,261,
  	262,1,0,0,0,262,264,1,0,0,0,263,257,1,0,0,0,263,258,1,0,0,0,264,23,1,
  	0,0,0,265,266,7,1,0,0,266,25,1,0,0,0,267,272,3,28,14,0,268,269,5,77,0,
  	0,269,271,3,28,14,0,270,268,1,0,0,0,271,274,1,0,0,0,272,270,1,0,0,0,272,
  	273,1,0,0,0,273,27,1,0,0,0,274,272,1,0,0,0,275,278,3,30,15,0,276,278,
  	3,32,16,0,277,275,1,0,0,0,277,276,1,0,0,0,278,29,1,0,0,0,279,281,5,52,
  	0,0,280,279,1,0,0,0,280,281,1,0,0,0,281,282,1,0,0,0,282,283,5,83,0,0,
  	283,31,1,0,0,0,284,286,5,52,0,0,285,284,1,0,0,0,285,286,1,0,0,0,286,287,
  	1,0,0,0,287,288,5,83,0,0,288,289,3,168,84,0,289,33,1,0,0,0,290,292,5,
  	40,0,0,291,293,3,162,81,0,292,291,1,0,0,0,292,293,1,0,0,0,293,294,1,0,
  	0,0,294,295,5,41,0,0,295,35,1,0,0,0,296,297,6,18,-1,0,297,323,3,8,4,0,
  	298,305,3,132,66,0,299,301,5,40,0,0,300,302,3,38,19,0,301,300,1,0,0,0,
  	301,302,1,0,0,0,302,303,1,0,0,0,303,306,5,41,0,0,304,306,3,176,88,0,305,
  	299,1,0,0,0,305,304,1,0,0,0,306,323,1,0,0,0,307,308,5,6,0,0,308,309,5,
  	57,0,0,309,310,3,160,80,0,310,311,5,58,0,0,311,312,5,40,0,0,312,313,3,
  	80,40,0,313,314,5,41,0,0,314,323,1,0,0,0,315,318,5,40,0,0,316,319,3,80,
  	40,0,317,319,3,160,80,0,318,316,1,0,0,0,318,317,1,0,0,0,319,320,1,0,0,
  	0,320,321,5,41,0,0,321,323,1,0,0,0,322,296,1,0,0,0,322,298,1,0,0,0,322,
  	307,1,0,0,0,322,315,1,0,0,0,323,348,1,0,0,0,324,325,10,7,0,0,325,328,
  	5,42,0,0,326,329,3,80,40,0,327,329,3,176,88,0,328,326,1,0,0,0,328,327,
  	1,0,0,0,329,330,1,0,0,0,330,331,5,43,0,0,331,347,1,0,0,0,332,333,10,6,
  	0,0,333,335,5,40,0,0,334,336,3,38,19,0,335,334,1,0,0,0,335,336,1,0,0,
  	0,336,337,1,0,0,0,337,347,5,41,0,0,338,339,10,4,0,0,339,342,5,82,0,0,
  	340,343,3,10,5,0,341,343,3,40,20,0,342,340,1,0,0,0,342,341,1,0,0,0,343,
  	347,1,0,0,0,344,345,10,3,0,0,345,347,7,2,0,0,346,324,1,0,0,0,346,332,
  	1,0,0,0,346,338,1,0,0,0,346,344,1,0,0,0,347,350,1,0,0,0,348,346,1,0,0,
  	0,348,349,1,0,0,0,349,37,1,0,0,0,350,348,1,0,0,0,351,352,3,174,87,0,352,
  	39,1,0,0,0,353,355,3,16,8,0,354,353,1,0,0,0,354,355,1,0,0,0,355,359,1,
  	0,0,0,356,357,3,134,67,0,357,358,5,80,0,0,358,360,1,0,0,0,359,356,1,0,
  	0,0,359,360,1,0,0,0,360,361,1,0,0,0,361,362,5,54,0,0,362,371,3,134,67,
  	0,363,364,3,16,8,0,364,365,5,80,0,0,365,366,5,54,0,0,366,367,3,134,67,
  	0,367,371,1,0,0,0,368,369,5,54,0,0,369,371,3,136,68,0,370,354,1,0,0,0,
  	370,363,1,0,0,0,370,368,1,0,0,0,371,41,1,0,0,0,372,384,3,36,18,0,373,
  	377,5,75,0,0,374,377,5,76,0,0,375,377,3,44,22,0,376,373,1,0,0,0,376,374,
  	1,0,0,0,376,375,1,0,0,0,377,378,1,0,0,0,378,384,3,42,21,0,379,380,5,40,
  	0,0,380,381,3,160,80,0,381,382,5,41,0,0,382,384,1,0,0,0,383,372,1,0,0,
  	0,383,376,1,0,0,0,383,379,1,0,0,0,384,43,1,0,0,0,385,386,7,3,0,0,386,
  	45,1,0,0,0,387,388,5,40,0,0,388,389,3,38,19,0,389,390,5,41,0,0,390,47,
  	1,0,0,0,391,393,5,40,0,0,392,394,3,38,19,0,393,392,1,0,0,0,393,394,1,
  	0,0,0,394,395,1,0,0,0,395,398,5,41,0,0,396,398,3,176,88,0,397,391,1,0,
  	0,0,397,396,1,0,0,0,398,49,1,0,0,0,399,409,3,42,21,0,400,401,5,6,0,0,
  	401,402,5,57,0,0,402,403,3,160,80,0,403,404,5,58,0,0,404,405,5,40,0,0,
  	405,406,3,50,25,0,406,407,5,41,0,0,407,409,1,0,0,0,408,399,1,0,0,0,408,
  	400,1,0,0,0,409,51,1,0,0,0,410,415,3,50,25,0,411,412,7,4,0,0,412,414,
  	3,50,25,0,413,411,1,0,0,0,414,417,1,0,0,0,415,413,1,0,0,0,415,416,1,0,
  	0,0,416,53,1,0,0,0,417,415,1,0,0,0,418,423,3,52,26,0,419,420,7,5,0,0,
  	420,422,3,52,26,0,421,419,1,0,0,0,422,425,1,0,0,0,423,421,1,0,0,0,423,
  	424,1,0,0,0,424,55,1,0,0,0,425,423,1,0,0,0,426,432,3,54,27,0,427,428,
  	3,58,29,0,428,429,3,54,27,0,429,431,1,0,0,0,430,427,1,0,0,0,431,434,1,
  	0,0,0,432,430,1,0,0,0,432,433,1,0,0,0,433,57,1,0,0,0,434,432,1,0,0,0,
  	435,436,5,58,0,0,436,440,5,58,0,0,437,438,5,57,0,0,438,440,5,57,0,0,439,
  	435,1,0,0,0,439,437,1,0,0,0,440,59,1,0,0,0,441,446,3,56,28,0,442,443,
  	7,6,0,0,443,445,3,56,28,0,444,442,1,0,0,0,445,448,1,0,0,0,446,444,1,0,
  	0,0,446,447,1,0,0,0,447,61,1,0,0,0,448,446,1,0,0,0,449,454,3,60,30,0,
  	450,451,7,7,0,0,451,453,3,60,30,0,452,450,1,0,0,0,453,456,1,0,0,0,454,
  	452,1,0,0,0,454,455,1,0,0,0,455,63,1,0,0,0,456,454,1,0,0,0,457,462,3,
  	62,31,0,458,459,5,52,0,0,459,461,3,62,31,0,460,458,1,0,0,0,461,464,1,
  	0,0,0,462,460,1,0,0,0,462,463,1,0,0,0,463,65,1,0,0,0,464,462,1,0,0,0,
  	465,470,3,64,32,0,466,467,5,51,0,0,467,469,3,64,32,0,468,466,1,0,0,0,
  	469,472,1,0,0,0,470,468,1,0,0,0,470,471,1,0,0,0,471,67,1,0,0,0,472,470,
  	1,0,0,0,473,478,3,66,33,0,474,475,5,53,0,0,475,477,3,66,33,0,476,474,
  	1,0,0,0,477,480,1,0,0,0,478,476,1,0,0,0,478,479,1,0,0,0,479,69,1,0,0,
  	0,480,478,1,0,0,0,481,486,3,68,34,0,482,483,5,73,0,0,483,485,3,68,34,
  	0,484,482,1,0,0,0,485,488,1,0,0,0,486,484,1,0,0,0,486,487,1,0,0,0,487,
  	71,1,0,0,0,488,486,1,0,0,0,489,494,3,70,35,0,490,491,5,74,0,0,491,493,
  	3,70,35,0,492,490,1,0,0,0,493,496,1,0,0,0,494,492,1,0,0,0,494,495,1,0,
  	0,0,495,73,1,0,0,0,496,494,1,0,0,0,497,503,3,72,36,0,498,499,5,78,0,0,
  	499,500,3,80,40,0,500,501,5,79,0,0,501,502,3,76,38,0,502,504,1,0,0,0,
  	503,498,1,0,0,0,503,504,1,0,0,0,504,75,1,0,0,0,505,511,3,74,37,0,506,
  	507,3,72,36,0,507,508,3,78,39,0,508,509,3,172,86,0,509,511,1,0,0,0,510,
  	505,1,0,0,0,510,506,1,0,0,0,511,77,1,0,0,0,512,513,7,8,0,0,513,79,1,0,
  	0,0,514,519,3,76,38,0,515,516,5,77,0,0,516,518,3,76,38,0,517,515,1,0,
  	0,0,518,521,1,0,0,0,519,517,1,0,0,0,519,520,1,0,0,0,520,81,1,0,0,0,521,
  	519,1,0,0,0,522,523,3,74,37,0,523,83,1,0,0,0,524,533,3,86,43,0,525,531,
  	3,88,44,0,526,531,3,90,45,0,527,531,3,94,47,0,528,531,3,98,49,0,529,531,
  	3,106,53,0,530,525,1,0,0,0,530,526,1,0,0,0,530,527,1,0,0,0,530,528,1,
  	0,0,0,530,529,1,0,0,0,531,533,1,0,0,0,532,524,1,0,0,0,532,530,1,0,0,0,
  	533,85,1,0,0,0,534,539,5,83,0,0,535,536,5,22,0,0,536,539,3,82,41,0,537,
  	539,5,27,0,0,538,534,1,0,0,0,538,535,1,0,0,0,538,537,1,0,0,0,539,540,
  	1,0,0,0,540,541,5,79,0,0,541,542,3,84,42,0,542,87,1,0,0,0,543,545,3,80,
  	40,0,544,543,1,0,0,0,544,545,1,0,0,0,545,546,1,0,0,0,546,547,5,81,0,0,
  	547,89,1,0,0,0,548,550,5,44,0,0,549,551,3,92,46,0,550,549,1,0,0,0,550,
  	551,1,0,0,0,551,552,1,0,0,0,552,553,5,45,0,0,553,91,1,0,0,0,554,556,3,
  	84,42,0,555,554,1,0,0,0,556,557,1,0,0,0,557,555,1,0,0,0,557,558,1,0,0,
  	0,558,93,1,0,0,0,559,560,5,32,0,0,560,561,5,40,0,0,561,562,3,96,48,0,
  	562,563,5,41,0,0,563,566,3,84,42,0,564,565,5,29,0,0,565,567,3,84,42,0,
  	566,564,1,0,0,0,566,567,1,0,0,0,567,575,1,0,0,0,568,569,5,36,0,0,569,
  	570,5,40,0,0,570,571,3,96,48,0,571,572,5,41,0,0,572,573,3,84,42,0,573,
  	575,1,0,0,0,574,559,1,0,0,0,574,568,1,0,0,0,575,95,1,0,0,0,576,585,3,
  	80,40,0,577,578,3,120,60,0,578,582,3,150,75,0,579,580,5,56,0,0,580,583,
  	3,172,86,0,581,583,3,176,88,0,582,579,1,0,0,0,582,581,1,0,0,0,583,585,
  	1,0,0,0,584,576,1,0,0,0,584,577,1,0,0,0,585,97,1,0,0,0,586,587,5,39,0,
  	0,587,588,5,40,0,0,588,589,3,96,48,0,589,590,5,41,0,0,590,591,3,84,42,
  	0,591,620,1,0,0,0,592,593,5,28,0,0,593,594,3,84,42,0,594,595,5,39,0,0,
  	595,596,5,40,0,0,596,597,3,80,40,0,597,598,5,41,0,0,598,599,5,81,0,0,
  	599,620,1,0,0,0,600,601,5,31,0,0,601,614,5,40,0,0,602,604,3,100,50,0,
  	603,605,3,96,48,0,604,603,1,0,0,0,604,605,1,0,0,0,605,606,1,0,0,0,606,
  	608,5,81,0,0,607,609,3,80,40,0,608,607,1,0,0,0,608,609,1,0,0,0,609,615,
  	1,0,0,0,610,611,3,102,51,0,611,612,5,79,0,0,612,613,3,104,52,0,613,615,
  	1,0,0,0,614,602,1,0,0,0,614,610,1,0,0,0,615,616,1,0,0,0,616,617,5,41,
  	0,0,617,618,3,84,42,0,618,620,1,0,0,0,619,586,1,0,0,0,619,592,1,0,0,0,
  	619,600,1,0,0,0,620,99,1,0,0,0,621,624,3,88,44,0,622,624,3,114,57,0,623,
  	621,1,0,0,0,623,622,1,0,0,0,624,101,1,0,0,0,625,626,3,120,60,0,626,627,
  	5,83,0,0,627,103,1,0,0,0,628,631,3,80,40,0,629,631,3,176,88,0,630,628,
  	1,0,0,0,630,629,1,0,0,0,631,105,1,0,0,0,632,640,5,21,0,0,633,640,5,26,
  	0,0,634,637,5,34,0,0,635,638,3,80,40,0,636,638,3,176,88,0,637,635,1,0,
  	0,0,637,636,1,0,0,0,637,638,1,0,0,0,638,640,1,0,0,0,639,632,1,0,0,0,639,
  	633,1,0,0,0,639,634,1,0,0,0,640,641,1,0,0,0,641,642,5,81,0,0,642,107,
  	1,0,0,0,643,645,3,110,55,0,644,643,1,0,0,0,645,646,1,0,0,0,646,644,1,
  	0,0,0,646,647,1,0,0,0,647,109,1,0,0,0,648,651,3,114,57,0,649,651,3,116,
  	58,0,650,648,1,0,0,0,650,649,1,0,0,0,651,111,1,0,0,0,652,653,5,83,0,0,
  	653,654,5,56,0,0,654,655,3,160,80,0,655,656,5,81,0,0,656,113,1,0,0,0,
  	657,659,3,120,60,0,658,657,1,0,0,0,658,659,1,0,0,0,659,662,1,0,0,0,660,
  	663,3,146,73,0,661,663,3,76,38,0,662,660,1,0,0,0,662,661,1,0,0,0,662,
  	663,1,0,0,0,663,664,1,0,0,0,664,665,5,81,0,0,665,115,1,0,0,0,666,667,
  	5,81,0,0,667,117,1,0,0,0,668,669,3,124,62,0,669,119,1,0,0,0,670,672,3,
  	118,59,0,671,670,1,0,0,0,672,673,1,0,0,0,673,674,1,0,0,0,673,671,1,0,
  	0,0,674,121,1,0,0,0,675,676,5,83,0,0,676,123,1,0,0,0,677,678,3,126,63,
  	0,678,125,1,0,0,0,679,682,3,132,66,0,680,682,5,25,0,0,681,679,1,0,0,0,
  	681,680,1,0,0,0,682,127,1,0,0,0,683,685,3,124,62,0,684,683,1,0,0,0,685,
  	686,1,0,0,0,686,684,1,0,0,0,686,687,1,0,0,0,687,129,1,0,0,0,688,690,3,
  	126,63,0,689,688,1,0,0,0,690,691,1,0,0,0,691,689,1,0,0,0,691,692,1,0,
  	0,0,692,131,1,0,0,0,693,695,3,16,8,0,694,693,1,0,0,0,694,695,1,0,0,0,
  	695,696,1,0,0,0,696,715,3,134,67,0,697,715,3,2,1,0,698,715,5,7,0,0,699,
  	715,5,8,0,0,700,715,5,9,0,0,701,715,5,10,0,0,702,715,5,11,0,0,703,715,
  	5,12,0,0,704,715,5,13,0,0,705,715,5,14,0,0,706,715,5,15,0,0,707,715,5,
  	16,0,0,708,715,5,17,0,0,709,715,5,18,0,0,710,715,5,19,0,0,711,715,5,38,
  	0,0,712,715,5,20,0,0,713,715,3,136,68,0,714,694,1,0,0,0,714,697,1,0,0,
  	0,714,698,1,0,0,0,714,699,1,0,0,0,714,700,1,0,0,0,714,701,1,0,0,0,714,
  	702,1,0,0,0,714,703,1,0,0,0,714,704,1,0,0,0,714,705,1,0,0,0,714,706,1,
  	0,0,0,714,707,1,0,0,0,714,708,1,0,0,0,714,709,1,0,0,0,714,710,1,0,0,0,
  	714,711,1,0,0,0,714,712,1,0,0,0,714,713,1,0,0,0,715,133,1,0,0,0,716,717,
  	3,122,61,0,717,135,1,0,0,0,718,721,5,40,0,0,719,722,3,80,40,0,720,722,
  	5,20,0,0,721,719,1,0,0,0,721,720,1,0,0,0,722,723,1,0,0,0,723,724,5,41,
  	0,0,724,137,1,0,0,0,725,726,5,83,0,0,726,139,1,0,0,0,727,729,3,16,8,0,
  	728,727,1,0,0,0,728,729,1,0,0,0,729,730,1,0,0,0,730,731,3,138,69,0,731,
  	141,1,0,0,0,732,734,3,144,72,0,733,732,1,0,0,0,734,735,1,0,0,0,735,733,
  	1,0,0,0,735,736,1,0,0,0,736,143,1,0,0,0,737,738,5,40,0,0,738,739,3,142,
  	71,0,739,740,5,41,0,0,740,755,1,0,0,0,741,742,5,42,0,0,742,743,3,142,
  	71,0,743,744,5,43,0,0,744,755,1,0,0,0,745,746,5,44,0,0,746,747,3,142,
  	71,0,747,748,5,45,0,0,748,755,1,0,0,0,749,751,8,9,0,0,750,749,1,0,0,0,
  	751,752,1,0,0,0,752,750,1,0,0,0,752,753,1,0,0,0,753,755,1,0,0,0,754,737,
  	1,0,0,0,754,741,1,0,0,0,754,745,1,0,0,0,754,750,1,0,0,0,755,145,1,0,0,
  	0,756,761,3,148,74,0,757,758,5,77,0,0,758,760,3,148,74,0,759,757,1,0,
  	0,0,760,763,1,0,0,0,761,759,1,0,0,0,761,762,1,0,0,0,762,147,1,0,0,0,763,
  	761,1,0,0,0,764,766,5,83,0,0,765,767,3,168,84,0,766,765,1,0,0,0,766,767,
  	1,0,0,0,767,149,1,0,0,0,768,769,3,152,76,0,769,770,3,154,77,0,770,151,
  	1,0,0,0,771,772,6,76,-1,0,772,773,3,158,79,0,773,785,1,0,0,0,774,781,
  	10,1,0,0,775,782,3,154,77,0,776,778,5,42,0,0,777,779,3,82,41,0,778,777,
  	1,0,0,0,778,779,1,0,0,0,779,780,1,0,0,0,780,782,5,43,0,0,781,775,1,0,
  	0,0,781,776,1,0,0,0,782,784,1,0,0,0,783,774,1,0,0,0,784,787,1,0,0,0,785,
  	783,1,0,0,0,785,786,1,0,0,0,786,153,1,0,0,0,787,785,1,0,0,0,788,790,5,
  	40,0,0,789,791,3,162,81,0,790,789,1,0,0,0,790,791,1,0,0,0,791,792,1,0,
  	0,0,792,794,5,41,0,0,793,795,5,25,0,0,794,793,1,0,0,0,794,795,1,0,0,0,
  	795,797,1,0,0,0,796,798,3,156,78,0,797,796,1,0,0,0,797,798,1,0,0,0,798,
  	155,1,0,0,0,799,800,7,10,0,0,800,157,1,0,0,0,801,802,3,10,5,0,802,159,
  	1,0,0,0,803,804,3,128,64,0,804,161,1,0,0,0,805,806,3,164,82,0,806,163,
  	1,0,0,0,807,812,3,166,83,0,808,809,5,77,0,0,809,811,3,166,83,0,810,808,
  	1,0,0,0,811,814,1,0,0,0,812,810,1,0,0,0,812,813,1,0,0,0,813,165,1,0,0,
  	0,814,812,1,0,0,0,815,817,3,120,60,0,816,818,5,83,0,0,817,816,1,0,0,0,
  	817,818,1,0,0,0,818,821,1,0,0,0,819,820,5,56,0,0,820,822,3,172,86,0,821,
  	819,1,0,0,0,821,822,1,0,0,0,822,167,1,0,0,0,823,829,3,170,85,0,824,825,
  	5,40,0,0,825,826,3,38,19,0,826,827,5,41,0,0,827,829,1,0,0,0,828,823,1,
  	0,0,0,828,824,1,0,0,0,829,169,1,0,0,0,830,831,5,56,0,0,831,834,3,172,
  	86,0,832,834,3,176,88,0,833,830,1,0,0,0,833,832,1,0,0,0,834,171,1,0,0,
  	0,835,838,3,76,38,0,836,838,3,176,88,0,837,835,1,0,0,0,837,836,1,0,0,
  	0,838,173,1,0,0,0,839,844,3,172,86,0,840,841,5,77,0,0,841,843,3,172,86,
  	0,842,840,1,0,0,0,843,846,1,0,0,0,844,842,1,0,0,0,844,845,1,0,0,0,845,
  	848,1,0,0,0,846,844,1,0,0,0,847,849,5,77,0,0,848,847,1,0,0,0,848,849,
  	1,0,0,0,849,175,1,0,0,0,850,855,7,11,0,0,851,853,3,174,87,0,852,854,5,
  	77,0,0,853,852,1,0,0,0,853,854,1,0,0,0,854,856,1,0,0,0,855,851,1,0,0,
  	0,855,856,1,0,0,0,856,857,1,0,0,0,857,858,7,12,0,0,858,177,1,0,0,0,859,
  	900,1,0,0,0,860,900,5,46,0,0,861,900,5,47,0,0,862,900,5,48,0,0,863,900,
  	5,49,0,0,864,900,5,50,0,0,865,900,5,51,0,0,866,900,5,52,0,0,867,900,5,
  	53,0,0,868,900,5,54,0,0,869,900,5,55,0,0,870,900,5,56,0,0,871,900,5,58,
  	0,0,872,900,5,57,0,0,873,900,5,72,0,0,874,900,5,59,0,0,875,900,5,60,0,
  	0,876,900,5,61,0,0,877,900,5,63,0,0,878,900,5,64,0,0,879,900,5,65,0,0,
  	880,900,5,66,0,0,881,882,5,57,0,0,882,900,5,57,0,0,883,884,5,58,0,0,884,
  	900,5,58,0,0,885,900,5,68,0,0,886,900,5,67,0,0,887,900,5,69,0,0,888,900,
  	5,70,0,0,889,900,5,71,0,0,890,900,5,73,0,0,891,900,5,74,0,0,892,900,5,
  	75,0,0,893,900,5,76,0,0,894,900,5,77,0,0,895,896,5,40,0,0,896,900,5,41,
  	0,0,897,898,5,42,0,0,898,900,5,43,0,0,899,859,1,0,0,0,899,860,1,0,0,0,
  	899,861,1,0,0,0,899,862,1,0,0,0,899,863,1,0,0,0,899,864,1,0,0,0,899,865,
  	1,0,0,0,899,866,1,0,0,0,899,867,1,0,0,0,899,868,1,0,0,0,899,869,1,0,0,
  	0,899,870,1,0,0,0,899,871,1,0,0,0,899,872,1,0,0,0,899,873,1,0,0,0,899,
  	874,1,0,0,0,899,875,1,0,0,0,899,876,1,0,0,0,899,877,1,0,0,0,899,878,1,
  	0,0,0,899,879,1,0,0,0,899,880,1,0,0,0,899,881,1,0,0,0,899,883,1,0,0,0,
  	899,885,1,0,0,0,899,886,1,0,0,0,899,887,1,0,0,0,899,888,1,0,0,0,899,889,
  	1,0,0,0,899,890,1,0,0,0,899,891,1,0,0,0,899,892,1,0,0,0,899,893,1,0,0,
  	0,899,894,1,0,0,0,899,895,1,0,0,0,899,897,1,0,0,0,900,179,1,0,0,0,901,
  	909,5,1,0,0,902,909,5,2,0,0,903,909,5,3,0,0,904,909,5,4,0,0,905,909,3,
  	6,3,0,906,909,5,5,0,0,907,909,5,33,0,0,908,901,1,0,0,0,908,902,1,0,0,
  	0,908,903,1,0,0,0,908,904,1,0,0,0,908,905,1,0,0,0,908,906,1,0,0,0,908,
  	907,1,0,0,0,909,181,1,0,0,0,102,183,187,190,202,210,218,222,233,242,247,
  	253,261,263,272,277,280,285,292,301,305,318,322,328,335,342,346,348,354,
  	359,370,376,383,393,397,408,415,423,432,439,446,454,462,470,478,486,494,
  	503,510,519,530,532,538,544,550,557,566,574,582,584,604,608,614,619,623,
  	630,637,639,646,650,658,662,673,681,686,691,694,714,721,728,735,752,754,
  	761,766,778,781,785,790,794,797,812,817,821,828,833,837,844,848,853,855,
  	899,908
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

AngelscriptConsoleParser::ExpressionStatementContext* AngelscriptConsoleParser::ScriptContext::expressionStatement() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionStatementContext>(0);
}

AngelscriptConsoleParser::StatementSeqContext* AngelscriptConsoleParser::ScriptContext::statementSeq() {
  return getRuleContext<AngelscriptConsoleParser::StatementSeqContext>(0);
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
    setState(190);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(183);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 75)) & 355) != 0)) {
        setState(182);
        expressionStatement();
      }
      setState(185);
      match(AngelscriptConsoleParser::EOF);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(187);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 68070729920282622) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 75)) & 355) != 0)) {
        setState(186);
        statementSeq();
      }
      setState(189);
      match(AngelscriptConsoleParser::EOF);
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
  enterRule(_localctx, 2, AngelscriptConsoleParser::RuleAsGeneric);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(192);
    match(AngelscriptConsoleParser::Identifier);
    setState(193);
    match(AngelscriptConsoleParser::Less);
    setState(194);
    simpleTypeSpecifierList();
    setState(195);
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
  enterRule(_localctx, 4, AngelscriptConsoleParser::RuleSimpleTypeSpecifierList);
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
    setState(197);
    declSpecifierSeq();
    setState(202);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(198);
      match(AngelscriptConsoleParser::Comma);
      setState(199);
      declSpecifierSeq();
      setState(204);
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
  enterRule(_localctx, 6, AngelscriptConsoleParser::RuleBooleanLiteral);
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
    setState(205);
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
  enterRule(_localctx, 8, AngelscriptConsoleParser::RulePrimaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(218);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(208); 
      _errHandler->sync(this);
      alt = 1;
      do {
        switch (alt) {
          case 1: {
                setState(207);
                literal();
                break;
              }

        default:
          throw NoViableAltException(this);
        }
        setState(210); 
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx);
      } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(212);
      match(AngelscriptConsoleParser::LeftParen);
      setState(213);
      expression();
      setState(214);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(216);
      idExpression();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(217);
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
  enterRule(_localctx, 10, AngelscriptConsoleParser::RuleIdExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(222);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(220);
      unqualifiedId();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(221);
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
  enterRule(_localctx, 12, AngelscriptConsoleParser::RuleUnqualifiedId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(224);
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
  enterRule(_localctx, 14, AngelscriptConsoleParser::RuleQualifiedId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(226);
    nestedNameSpecifier(0);
    setState(227);
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
  size_t startState = 16;
  enterRecursionRule(_localctx, 16, AngelscriptConsoleParser::RuleNestedNameSpecifier, precedence);

    

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
    setState(233);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
    case 1: {
      setState(230);
      theTypeName();
      break;
    }

    case 2: {
      setState(231);
      namespaceName();
      break;
    }

    case 3: {
      setState(232);
      decltypeSpecifier();
      break;
    }

    default:
      break;
    }
    setState(235);
    match(AngelscriptConsoleParser::Doublecolon);
    _ctx->stop = _input->LT(-1);
    setState(242);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<NestedNameSpecifierContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleNestedNameSpecifier);
        setState(237);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(238);
        match(AngelscriptConsoleParser::Identifier);
        setState(239);
        match(AngelscriptConsoleParser::Doublecolon); 
      }
      setState(244);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
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
  enterRule(_localctx, 18, AngelscriptConsoleParser::RuleLambdaExpression);
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
    setState(245);
    lambdaIntroducer();
    setState(247);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::LeftParen) {
      setState(246);
      lambdaDeclarator();
    }
    setState(249);
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
  enterRule(_localctx, 20, AngelscriptConsoleParser::RuleLambdaIntroducer);
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
    setState(251);
    match(AngelscriptConsoleParser::LeftBracket);
    setState(253);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 52) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 52)) & 2147483665) != 0)) {
      setState(252);
      lambdaCapture();
    }
    setState(255);
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
  enterRule(_localctx, 22, AngelscriptConsoleParser::RuleLambdaCapture);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(263);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(257);
      captureList();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(258);
      captureDefault();
      setState(261);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Comma) {
        setState(259);
        match(AngelscriptConsoleParser::Comma);
        setState(260);
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
  enterRule(_localctx, 24, AngelscriptConsoleParser::RuleCaptureDefault);
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
  enterRule(_localctx, 26, AngelscriptConsoleParser::RuleCaptureList);
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
    setState(267);
    capture();
    setState(272);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(268);
      match(AngelscriptConsoleParser::Comma);
      setState(269);
      capture();
      setState(274);
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
  enterRule(_localctx, 28, AngelscriptConsoleParser::RuleCapture);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(277);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(275);
      simpleCapture();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(276);
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
  enterRule(_localctx, 30, AngelscriptConsoleParser::RuleSimpleCapture);
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
    setState(280);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::And) {
      setState(279);
      match(AngelscriptConsoleParser::And);
    }
    setState(282);
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
  enterRule(_localctx, 32, AngelscriptConsoleParser::RuleInitcapture);
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
    setState(285);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::And) {
      setState(284);
      match(AngelscriptConsoleParser::And);
    }
    setState(287);
    match(AngelscriptConsoleParser::Identifier);
    setState(288);
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
  enterRule(_localctx, 34, AngelscriptConsoleParser::RuleLambdaDeclarator);
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
    setState(290);
    match(AngelscriptConsoleParser::LeftParen);
    setState(292);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier) {
      setState(291);
      parameterDeclarationClause();
    }
    setState(294);
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
  size_t startState = 36;
  enterRecursionRule(_localctx, 36, AngelscriptConsoleParser::RulePostfixExpression, precedence);

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
    setState(322);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      setState(297);
      primaryExpression();
      break;
    }

    case 2: {
      setState(298);
      simpleTypeSpecifier();
      setState(305);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::LeftParen: {
          setState(299);
          match(AngelscriptConsoleParser::LeftParen);
          setState(301);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(300);
            expressionList();
          }
          setState(303);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case AngelscriptConsoleParser::LeftBracket:
        case AngelscriptConsoleParser::LeftBrace: {
          setState(304);
          bracedInitList();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      break;
    }

    case 3: {
      setState(307);
      match(AngelscriptConsoleParser::Cast);
      setState(308);
      match(AngelscriptConsoleParser::Less);
      setState(309);
      theTypeId();
      setState(310);
      match(AngelscriptConsoleParser::Greater);
      setState(311);
      match(AngelscriptConsoleParser::LeftParen);
      setState(312);
      expression();
      setState(313);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 4: {
      setState(315);
      match(AngelscriptConsoleParser::LeftParen);
      setState(318);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
      case 1: {
        setState(316);
        expression();
        break;
      }

      case 2: {
        setState(317);
        theTypeId();
        break;
      }

      default:
        break;
      }
      setState(320);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(348);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(346);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(324);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(325);
          match(AngelscriptConsoleParser::LeftBracket);
          setState(328);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
          case 1: {
            setState(326);
            expression();
            break;
          }

          case 2: {
            setState(327);
            bracedInitList();
            break;
          }

          default:
            break;
          }
          setState(330);
          match(AngelscriptConsoleParser::RightBracket);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(332);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(333);
          match(AngelscriptConsoleParser::LeftParen);
          setState(335);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(334);
            expressionList();
          }
          setState(337);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(338);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(339);
          match(AngelscriptConsoleParser::Dot);
          setState(342);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
          case 1: {
            setState(340);
            idExpression();
            break;
          }

          case 2: {
            setState(341);
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
          setState(344);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(345);
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
      setState(350);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx);
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
  enterRule(_localctx, 38, AngelscriptConsoleParser::RuleExpressionList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(351);
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
  enterRule(_localctx, 40, AngelscriptConsoleParser::RulePseudoDestructorName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(370);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(354);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
      case 1: {
        setState(353);
        nestedNameSpecifier(0);
        break;
      }

      default:
        break;
      }
      setState(359);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Identifier) {
        setState(356);
        theTypeName();
        setState(357);
        match(AngelscriptConsoleParser::Doublecolon);
      }
      setState(361);
      match(AngelscriptConsoleParser::Tilde);
      setState(362);
      theTypeName();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(363);
      nestedNameSpecifier(0);
      setState(364);
      match(AngelscriptConsoleParser::Doublecolon);
      setState(365);
      match(AngelscriptConsoleParser::Tilde);
      setState(366);
      theTypeName();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(368);
      match(AngelscriptConsoleParser::Tilde);
      setState(369);
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
  enterRule(_localctx, 42, AngelscriptConsoleParser::RuleUnaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(383);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(372);
      postfixExpression(0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(376);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::PlusPlus: {
          setState(373);
          match(AngelscriptConsoleParser::PlusPlus);
          break;
        }

        case AngelscriptConsoleParser::MinusMinus: {
          setState(374);
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
          setState(375);
          unaryOperator();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(378);
      unaryExpression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(379);
      match(AngelscriptConsoleParser::LeftParen);
      setState(380);
      theTypeId();
      setState(381);
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
  enterRule(_localctx, 44, AngelscriptConsoleParser::RuleUnaryOperator);
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
    setState(385);
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
  enterRule(_localctx, 46, AngelscriptConsoleParser::RuleNewPlacement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(387);
    match(AngelscriptConsoleParser::LeftParen);
    setState(388);
    expressionList();
    setState(389);
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
  enterRule(_localctx, 48, AngelscriptConsoleParser::RuleNewInitializer_);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(397);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 1);
        setState(391);
        match(AngelscriptConsoleParser::LeftParen);
        setState(393);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 75)) & 291) != 0)) {
          setState(392);
          expressionList();
        }
        setState(395);
        match(AngelscriptConsoleParser::RightParen);
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(396);
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
  enterRule(_localctx, 50, AngelscriptConsoleParser::RuleCastExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(408);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(399);
      unaryExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(400);
      match(AngelscriptConsoleParser::Cast);
      setState(401);
      match(AngelscriptConsoleParser::Less);
      setState(402);
      theTypeId();
      setState(403);
      match(AngelscriptConsoleParser::Greater);
      setState(404);
      match(AngelscriptConsoleParser::LeftParen);
      setState(405);
      castExpression();
      setState(406);
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
  enterRule(_localctx, 52, AngelscriptConsoleParser::RuleMultiplicativeExpression);
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
    setState(410);
    castExpression();
    setState(415);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1970324836974592) != 0)) {
      setState(411);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 1970324836974592) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(412);
      castExpression();
      setState(417);
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
  enterRule(_localctx, 54, AngelscriptConsoleParser::RuleAdditiveExpression);
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
    setState(418);
    multiplicativeExpression();
    setState(423);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Plus

    || _la == AngelscriptConsoleParser::Minus) {
      setState(419);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Plus

      || _la == AngelscriptConsoleParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(420);
      multiplicativeExpression();
      setState(425);
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
  enterRule(_localctx, 56, AngelscriptConsoleParser::RuleShiftExpression);

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
    setState(426);
    additiveExpression();
    setState(432);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(427);
        shiftOperator();
        setState(428);
        additiveExpression(); 
      }
      setState(434);
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
  enterRule(_localctx, 58, AngelscriptConsoleParser::RuleShiftOperator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(439);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Greater: {
        enterOuterAlt(_localctx, 1);
        setState(435);
        match(AngelscriptConsoleParser::Greater);
        setState(436);
        match(AngelscriptConsoleParser::Greater);
        break;
      }

      case AngelscriptConsoleParser::Less: {
        enterOuterAlt(_localctx, 2);
        setState(437);
        match(AngelscriptConsoleParser::Less);
        setState(438);
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
  enterRule(_localctx, 60, AngelscriptConsoleParser::RuleRelationalExpression);
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
    setState(441);
    shiftExpression();
    setState(446);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 57) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 57)) & 49155) != 0)) {
      setState(442);
      _la = _input->LA(1);
      if (!(((((_la - 57) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 57)) & 49155) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(443);
      shiftExpression();
      setState(448);
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
  enterRule(_localctx, 62, AngelscriptConsoleParser::RuleEqualityExpression);
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
    setState(449);
    relationalExpression();
    setState(454);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Equal

    || _la == AngelscriptConsoleParser::NotEqual) {
      setState(450);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Equal

      || _la == AngelscriptConsoleParser::NotEqual)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(451);
      relationalExpression();
      setState(456);
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
  enterRule(_localctx, 64, AngelscriptConsoleParser::RuleAndExpression);
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
    setState(457);
    equalityExpression();
    setState(462);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::And) {
      setState(458);
      match(AngelscriptConsoleParser::And);
      setState(459);
      equalityExpression();
      setState(464);
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
  enterRule(_localctx, 66, AngelscriptConsoleParser::RuleExclusiveOrExpression);
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
    setState(465);
    andExpression();
    setState(470);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Xor) {
      setState(466);
      match(AngelscriptConsoleParser::Xor);
      setState(467);
      andExpression();
      setState(472);
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
  enterRule(_localctx, 68, AngelscriptConsoleParser::RuleInclusiveOrExpression);
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
    setState(473);
    exclusiveOrExpression();
    setState(478);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Or) {
      setState(474);
      match(AngelscriptConsoleParser::Or);
      setState(475);
      exclusiveOrExpression();
      setState(480);
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
  enterRule(_localctx, 70, AngelscriptConsoleParser::RuleLogicalAndExpression);
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
    setState(481);
    inclusiveOrExpression();
    setState(486);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::AndAnd) {
      setState(482);
      match(AngelscriptConsoleParser::AndAnd);
      setState(483);
      inclusiveOrExpression();
      setState(488);
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
  enterRule(_localctx, 72, AngelscriptConsoleParser::RuleLogicalOrExpression);
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
    setState(489);
    logicalAndExpression();
    setState(494);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::OrOr) {
      setState(490);
      match(AngelscriptConsoleParser::OrOr);
      setState(491);
      logicalAndExpression();
      setState(496);
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
  enterRule(_localctx, 74, AngelscriptConsoleParser::RuleConditionalExpression);
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
    setState(497);
    logicalOrExpression();
    setState(503);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Question) {
      setState(498);
      match(AngelscriptConsoleParser::Question);
      setState(499);
      expression();
      setState(500);
      match(AngelscriptConsoleParser::Colon);
      setState(501);
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
  enterRule(_localctx, 76, AngelscriptConsoleParser::RuleAssignmentExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(510);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(505);
      conditionalExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(506);
      logicalOrExpression();
      setState(507);
      assignmentOperator();
      setState(508);
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
  enterRule(_localctx, 78, AngelscriptConsoleParser::RuleAssignmentOperator);
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
    setState(512);
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
  enterRule(_localctx, 80, AngelscriptConsoleParser::RuleExpression);
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
    setState(514);
    assignmentExpression();
    setState(519);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(515);
      match(AngelscriptConsoleParser::Comma);
      setState(516);
      assignmentExpression();
      setState(521);
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
  enterRule(_localctx, 82, AngelscriptConsoleParser::RuleConstantExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(522);
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
  enterRule(_localctx, 84, AngelscriptConsoleParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(532);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(524);
      labeledStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(530);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
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
        case AngelscriptConsoleParser::False_:
        case AngelscriptConsoleParser::Nullptr:
        case AngelscriptConsoleParser::True_:
        case AngelscriptConsoleParser::Void:
        case AngelscriptConsoleParser::LeftParen:
        case AngelscriptConsoleParser::LeftBracket:
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
        case AngelscriptConsoleParser::Semi:
        case AngelscriptConsoleParser::Identifier: {
          setState(525);
          expressionStatement();
          break;
        }

        case AngelscriptConsoleParser::LeftBrace: {
          setState(526);
          compoundStatement();
          break;
        }

        case AngelscriptConsoleParser::If:
        case AngelscriptConsoleParser::Switch: {
          setState(527);
          selectionStatement();
          break;
        }

        case AngelscriptConsoleParser::Do:
        case AngelscriptConsoleParser::For:
        case AngelscriptConsoleParser::While: {
          setState(528);
          iterationStatement();
          break;
        }

        case AngelscriptConsoleParser::Break:
        case AngelscriptConsoleParser::Continue:
        case AngelscriptConsoleParser::Return: {
          setState(529);
          jumpStatement();
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
  enterRule(_localctx, 86, AngelscriptConsoleParser::RuleLabeledStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(538);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Identifier: {
        setState(534);
        match(AngelscriptConsoleParser::Identifier);
        break;
      }

      case AngelscriptConsoleParser::Case: {
        setState(535);
        match(AngelscriptConsoleParser::Case);
        setState(536);
        constantExpression();
        break;
      }

      case AngelscriptConsoleParser::Default: {
        setState(537);
        match(AngelscriptConsoleParser::Default);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(540);
    match(AngelscriptConsoleParser::Colon);
    setState(541);
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
  enterRule(_localctx, 88, AngelscriptConsoleParser::RuleExpressionStatement);
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
    setState(544);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 291) != 0)) {
      setState(543);
      expression();
    }
    setState(546);
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
  enterRule(_localctx, 90, AngelscriptConsoleParser::RuleCompoundStatement);
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
    setState(548);
    match(AngelscriptConsoleParser::LeftBrace);
    setState(550);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070729920282622) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 355) != 0)) {
      setState(549);
      statementSeq();
    }
    setState(552);
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
  enterRule(_localctx, 92, AngelscriptConsoleParser::RuleStatementSeq);
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
    setState(555); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(554);
      statement();
      setState(557); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070729920282622) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
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
  enterRule(_localctx, 94, AngelscriptConsoleParser::RuleSelectionStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(574);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::If: {
        enterOuterAlt(_localctx, 1);
        setState(559);
        match(AngelscriptConsoleParser::If);
        setState(560);
        match(AngelscriptConsoleParser::LeftParen);
        setState(561);
        condition();
        setState(562);
        match(AngelscriptConsoleParser::RightParen);
        setState(563);
        statement();
        setState(566);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx)) {
        case 1: {
          setState(564);
          match(AngelscriptConsoleParser::Else);
          setState(565);
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
        setState(568);
        match(AngelscriptConsoleParser::Switch);
        setState(569);
        match(AngelscriptConsoleParser::LeftParen);
        setState(570);
        condition();
        setState(571);
        match(AngelscriptConsoleParser::RightParen);
        setState(572);
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
  enterRule(_localctx, 96, AngelscriptConsoleParser::RuleCondition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(584);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(576);
      expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(577);
      declSpecifierSeq();
      setState(578);
      declarator();
      setState(582);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::Assign: {
          setState(579);
          match(AngelscriptConsoleParser::Assign);
          setState(580);
          initializerClause();
          break;
        }

        case AngelscriptConsoleParser::LeftBracket:
        case AngelscriptConsoleParser::LeftBrace: {
          setState(581);
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
  enterRule(_localctx, 98, AngelscriptConsoleParser::RuleIterationStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(619);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::While: {
        enterOuterAlt(_localctx, 1);
        setState(586);
        match(AngelscriptConsoleParser::While);
        setState(587);
        match(AngelscriptConsoleParser::LeftParen);
        setState(588);
        condition();
        setState(589);
        match(AngelscriptConsoleParser::RightParen);
        setState(590);
        statement();
        break;
      }

      case AngelscriptConsoleParser::Do: {
        enterOuterAlt(_localctx, 2);
        setState(592);
        match(AngelscriptConsoleParser::Do);
        setState(593);
        statement();
        setState(594);
        match(AngelscriptConsoleParser::While);
        setState(595);
        match(AngelscriptConsoleParser::LeftParen);
        setState(596);
        expression();
        setState(597);
        match(AngelscriptConsoleParser::RightParen);
        setState(598);
        match(AngelscriptConsoleParser::Semi);
        break;
      }

      case AngelscriptConsoleParser::For: {
        enterOuterAlt(_localctx, 3);
        setState(600);
        match(AngelscriptConsoleParser::For);
        setState(601);
        match(AngelscriptConsoleParser::LeftParen);
        setState(614);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx)) {
        case 1: {
          setState(602);
          forInitStatement();
          setState(604);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68052495194128382) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(603);
            condition();
          }
          setState(606);
          match(AngelscriptConsoleParser::Semi);
          setState(608);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(607);
            expression();
          }
          break;
        }

        case 2: {
          setState(610);
          forRangeDeclaration();
          setState(611);
          match(AngelscriptConsoleParser::Colon);
          setState(612);
          forRangeInitializer();
          break;
        }

        default:
          break;
        }
        setState(616);
        match(AngelscriptConsoleParser::RightParen);
        setState(617);
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
  enterRule(_localctx, 100, AngelscriptConsoleParser::RuleForInitStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(623);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(621);
      expressionStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(622);
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
  enterRule(_localctx, 102, AngelscriptConsoleParser::RuleForRangeDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(625);
    declSpecifierSeq();
    setState(626);
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
  enterRule(_localctx, 104, AngelscriptConsoleParser::RuleForRangeInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(630);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(628);
      expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(629);
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

AngelscriptConsoleParser::ExpressionContext* AngelscriptConsoleParser::JumpStatementContext::expression() {
  return getRuleContext<AngelscriptConsoleParser::ExpressionContext>(0);
}

AngelscriptConsoleParser::BracedInitListContext* AngelscriptConsoleParser::JumpStatementContext::bracedInitList() {
  return getRuleContext<AngelscriptConsoleParser::BracedInitListContext>(0);
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
  enterRule(_localctx, 106, AngelscriptConsoleParser::RuleJumpStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(639);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Break: {
        setState(632);
        match(AngelscriptConsoleParser::Break);
        break;
      }

      case AngelscriptConsoleParser::Continue: {
        setState(633);
        match(AngelscriptConsoleParser::Continue);
        break;
      }

      case AngelscriptConsoleParser::Return: {
        setState(634);
        match(AngelscriptConsoleParser::Return);
        setState(637);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx)) {
        case 1: {
          setState(635);
          expression();
          break;
        }

        case 2: {
          setState(636);
          bracedInitList();
          break;
        }

        default:
          break;
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(641);
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
  enterRule(_localctx, 108, AngelscriptConsoleParser::RuleDeclarationseq);
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
    setState(644); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(643);
      declaration();
      setState(646); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68052495194128382) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 355) != 0));
   
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
  enterRule(_localctx, 110, AngelscriptConsoleParser::RuleDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(650);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 68, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(648);
      simpleDeclaration();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(649);
      emptyDeclaration_();
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
  enterRule(_localctx, 112, AngelscriptConsoleParser::RuleAliasDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(652);
    match(AngelscriptConsoleParser::Identifier);
    setState(653);
    match(AngelscriptConsoleParser::Assign);
    setState(654);
    theTypeId();
    setState(655);
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

tree::TerminalNode* AngelscriptConsoleParser::SimpleDeclarationContext::Semi() {
  return getToken(AngelscriptConsoleParser::Semi, 0);
}

AngelscriptConsoleParser::DeclSpecifierSeqContext* AngelscriptConsoleParser::SimpleDeclarationContext::declSpecifierSeq() {
  return getRuleContext<AngelscriptConsoleParser::DeclSpecifierSeqContext>(0);
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
  enterRule(_localctx, 114, AngelscriptConsoleParser::RuleSimpleDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(658);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx)) {
    case 1: {
      setState(657);
      declSpecifierSeq();
      break;
    }

    default:
      break;
    }
    setState(662);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 70, _ctx)) {
    case 1: {
      setState(660);
      initDeclaratorList();
      break;
    }

    case 2: {
      setState(661);
      assignmentExpression();
      break;
    }

    default:
      break;
    }
    setState(664);
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
  enterRule(_localctx, 116, AngelscriptConsoleParser::RuleEmptyDeclaration_);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(666);
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
  enterRule(_localctx, 118, AngelscriptConsoleParser::RuleDeclSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(668);
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
  enterRule(_localctx, 120, AngelscriptConsoleParser::RuleDeclSpecifierSeq);

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
    setState(671); 
    _errHandler->sync(this);
    alt = 1 + 1;
    do {
      switch (alt) {
        case 1 + 1: {
              setState(670);
              declSpecifier();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(673); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 71, _ctx);
    } while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
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
  enterRule(_localctx, 122, AngelscriptConsoleParser::RuleTypedefName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(675);
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
  enterRule(_localctx, 124, AngelscriptConsoleParser::RuleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(677);
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
  enterRule(_localctx, 126, AngelscriptConsoleParser::RuleTrailingTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(681);
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
        setState(679);
        simpleTypeSpecifier();
        break;
      }

      case AngelscriptConsoleParser::Const: {
        enterOuterAlt(_localctx, 2);
        setState(680);
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
  enterRule(_localctx, 128, AngelscriptConsoleParser::RuleTypeSpecifierSeq);
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
    setState(684); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(683);
      typeSpecifier();
      setState(686); 
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
  enterRule(_localctx, 130, AngelscriptConsoleParser::RuleTrailingTypeSpecifierSeq);
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
    setState(689); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(688);
      trailingTypeSpecifier();
      setState(691); 
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
  enterRule(_localctx, 132, AngelscriptConsoleParser::RuleSimpleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(714);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 76, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(694);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 75, _ctx)) {
      case 1: {
        setState(693);
        nestedNameSpecifier(0);
        break;
      }

      default:
        break;
      }
      setState(696);
      theTypeName();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(697);
      asGeneric();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(698);
      match(AngelscriptConsoleParser::Int);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(699);
      match(AngelscriptConsoleParser::Int8);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(700);
      match(AngelscriptConsoleParser::Int16);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(701);
      match(AngelscriptConsoleParser::Int32);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(702);
      match(AngelscriptConsoleParser::Int64);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(703);
      match(AngelscriptConsoleParser::UInt);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(704);
      match(AngelscriptConsoleParser::UInt8);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(705);
      match(AngelscriptConsoleParser::UInt16);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(706);
      match(AngelscriptConsoleParser::UInt32);
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(707);
      match(AngelscriptConsoleParser::UInt64);
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(708);
      match(AngelscriptConsoleParser::Float);
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(709);
      match(AngelscriptConsoleParser::Double);
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(710);
      match(AngelscriptConsoleParser::Bool);
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(711);
      match(AngelscriptConsoleParser::Void);
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(712);
      match(AngelscriptConsoleParser::Auto);
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(713);
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
  enterRule(_localctx, 134, AngelscriptConsoleParser::RuleTheTypeName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(716);
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
  enterRule(_localctx, 136, AngelscriptConsoleParser::RuleDecltypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(718);
    match(AngelscriptConsoleParser::LeftParen);
    setState(721);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 77, _ctx)) {
    case 1: {
      setState(719);
      expression();
      break;
    }

    case 2: {
      setState(720);
      match(AngelscriptConsoleParser::Auto);
      break;
    }

    default:
      break;
    }
    setState(723);
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
  enterRule(_localctx, 138, AngelscriptConsoleParser::RuleNamespaceName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(725);
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
  enterRule(_localctx, 140, AngelscriptConsoleParser::RuleQualifiednamespacespecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(728);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 78, _ctx)) {
    case 1: {
      setState(727);
      nestedNameSpecifier(0);
      break;
    }

    default:
      break;
    }
    setState(730);
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
  enterRule(_localctx, 142, AngelscriptConsoleParser::RuleBalancedTokenSeq);
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
    setState(733); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(732);
      balancedtoken();
      setState(735); 
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
  enterRule(_localctx, 144, AngelscriptConsoleParser::RuleBalancedtoken);
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
    setState(754);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 1);
        setState(737);
        match(AngelscriptConsoleParser::LeftParen);
        setState(738);
        balancedTokenSeq();
        setState(739);
        match(AngelscriptConsoleParser::RightParen);
        break;
      }

      case AngelscriptConsoleParser::LeftBracket: {
        enterOuterAlt(_localctx, 2);
        setState(741);
        match(AngelscriptConsoleParser::LeftBracket);
        setState(742);
        balancedTokenSeq();
        setState(743);
        match(AngelscriptConsoleParser::RightBracket);
        break;
      }

      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 3);
        setState(745);
        match(AngelscriptConsoleParser::LeftBrace);
        setState(746);
        balancedTokenSeq();
        setState(747);
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
        setState(750); 
        _errHandler->sync(this);
        alt = 1;
        do {
          switch (alt) {
            case 1: {
                  setState(749);
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
          setState(752); 
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 80, _ctx);
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
  enterRule(_localctx, 146, AngelscriptConsoleParser::RuleInitDeclaratorList);
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
    setState(756);
    initDeclarator();
    setState(761);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(757);
      match(AngelscriptConsoleParser::Comma);
      setState(758);
      initDeclarator();
      setState(763);
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
  enterRule(_localctx, 148, AngelscriptConsoleParser::RuleInitDeclarator);
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
    setState(764);
    match(AngelscriptConsoleParser::Identifier);
    setState(766);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72080683782111232) != 0)) {
      setState(765);
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
  enterRule(_localctx, 150, AngelscriptConsoleParser::RuleDeclarator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(768);
    declaratorDef(0);
    setState(769);
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
  size_t startState = 152;
  enterRecursionRule(_localctx, 152, AngelscriptConsoleParser::RuleDeclaratorDef, precedence);

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
    setState(772);
    declaratorid();
    _ctx->stop = _input->LT(-1);
    setState(785);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 86, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<DeclaratorDefContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleDeclaratorDef);
        setState(774);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(781);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case AngelscriptConsoleParser::LeftParen: {
            setState(775);
            parametersAndQualifiers();
            break;
          }

          case AngelscriptConsoleParser::LeftBracket: {
            setState(776);
            match(AngelscriptConsoleParser::LeftBracket);
            setState(778);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if ((((_la & ~ 0x3fULL) == 0) &&
              ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
              ((1ULL << (_la - 75)) & 291) != 0)) {
              setState(777);
              constantExpression();
            }
            setState(780);
            match(AngelscriptConsoleParser::RightBracket);
            break;
          }

        default:
          throw NoViableAltException(this);
        } 
      }
      setState(787);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 86, _ctx);
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
  enterRule(_localctx, 154, AngelscriptConsoleParser::RuleParametersAndQualifiers);
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
    setState(788);
    match(AngelscriptConsoleParser::LeftParen);
    setState(790);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier) {
      setState(789);
      parameterDeclarationClause();
    }
    setState(792);
    match(AngelscriptConsoleParser::RightParen);
    setState(794);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 88, _ctx)) {
    case 1: {
      setState(793);
      match(AngelscriptConsoleParser::Const);
      break;
    }

    default:
      break;
    }
    setState(797);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 89, _ctx)) {
    case 1: {
      setState(796);
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
  enterRule(_localctx, 156, AngelscriptConsoleParser::RuleRefqualifier);
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
    setState(799);
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
  enterRule(_localctx, 158, AngelscriptConsoleParser::RuleDeclaratorid);

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
  enterRule(_localctx, 160, AngelscriptConsoleParser::RuleTheTypeId);

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
  enterRule(_localctx, 162, AngelscriptConsoleParser::RuleParameterDeclarationClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(805);
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
  enterRule(_localctx, 164, AngelscriptConsoleParser::RuleParameterDeclarationList);
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
    setState(807);
    parameterDeclaration();
    setState(812);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(808);
      match(AngelscriptConsoleParser::Comma);
      setState(809);
      parameterDeclaration();
      setState(814);
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
  enterRule(_localctx, 166, AngelscriptConsoleParser::RuleParameterDeclaration);
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
    setState(815);
    declSpecifierSeq();
    setState(817);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Identifier) {
      setState(816);
      match(AngelscriptConsoleParser::Identifier);
    }
    setState(821);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Assign) {
      setState(819);
      match(AngelscriptConsoleParser::Assign);
      setState(820);
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
  enterRule(_localctx, 168, AngelscriptConsoleParser::RuleInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(828);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace:
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(823);
        braceOrEqualInitializer();
        break;
      }

      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 2);
        setState(824);
        match(AngelscriptConsoleParser::LeftParen);
        setState(825);
        expressionList();
        setState(826);
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
  enterRule(_localctx, 170, AngelscriptConsoleParser::RuleBraceOrEqualInitializer);

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
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(830);
        match(AngelscriptConsoleParser::Assign);
        setState(831);
        initializerClause();
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(832);
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
  enterRule(_localctx, 172, AngelscriptConsoleParser::RuleInitializerClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(837);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 95, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(835);
      assignmentExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(836);
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
  enterRule(_localctx, 174, AngelscriptConsoleParser::RuleInitializerList);

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
    setState(839);
    initializerClause();
    setState(844);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 96, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(840);
        match(AngelscriptConsoleParser::Comma);
        setState(841);
        initializerClause(); 
      }
      setState(846);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 96, _ctx);
    }
    setState(848);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 97, _ctx)) {
    case 1: {
      setState(847);
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
  enterRule(_localctx, 176, AngelscriptConsoleParser::RuleBracedInitList);
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
    setState(850);
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::LeftBracket

    || _la == AngelscriptConsoleParser::LeftBrace)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(855);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 291) != 0)) {
      setState(851);
      initializerList();
      setState(853);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Comma) {
        setState(852);
        match(AngelscriptConsoleParser::Comma);
      }
    }
    setState(857);
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
  enterRule(_localctx, 178, AngelscriptConsoleParser::RuleTheOperator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(899);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 100, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);

      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(860);
      match(AngelscriptConsoleParser::Plus);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(861);
      match(AngelscriptConsoleParser::Minus);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(862);
      match(AngelscriptConsoleParser::Star);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(863);
      match(AngelscriptConsoleParser::Div);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(864);
      match(AngelscriptConsoleParser::Mod);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(865);
      match(AngelscriptConsoleParser::Xor);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(866);
      match(AngelscriptConsoleParser::And);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(867);
      match(AngelscriptConsoleParser::Or);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(868);
      match(AngelscriptConsoleParser::Tilde);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(869);
      match(AngelscriptConsoleParser::Not);
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(870);
      match(AngelscriptConsoleParser::Assign);
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(871);
      match(AngelscriptConsoleParser::Greater);
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(872);
      match(AngelscriptConsoleParser::Less);
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(873);
      match(AngelscriptConsoleParser::GreaterEqual);
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(874);
      match(AngelscriptConsoleParser::PlusAssign);
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(875);
      match(AngelscriptConsoleParser::MinusAssign);
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(876);
      match(AngelscriptConsoleParser::StarAssign);
      break;
    }

    case 19: {
      enterOuterAlt(_localctx, 19);
      setState(877);
      match(AngelscriptConsoleParser::ModAssign);
      break;
    }

    case 20: {
      enterOuterAlt(_localctx, 20);
      setState(878);
      match(AngelscriptConsoleParser::XorAssign);
      break;
    }

    case 21: {
      enterOuterAlt(_localctx, 21);
      setState(879);
      match(AngelscriptConsoleParser::AndAssign);
      break;
    }

    case 22: {
      enterOuterAlt(_localctx, 22);
      setState(880);
      match(AngelscriptConsoleParser::OrAssign);
      break;
    }

    case 23: {
      enterOuterAlt(_localctx, 23);
      setState(881);
      match(AngelscriptConsoleParser::Less);
      setState(882);
      match(AngelscriptConsoleParser::Less);
      break;
    }

    case 24: {
      enterOuterAlt(_localctx, 24);
      setState(883);
      match(AngelscriptConsoleParser::Greater);
      setState(884);
      match(AngelscriptConsoleParser::Greater);
      break;
    }

    case 25: {
      enterOuterAlt(_localctx, 25);
      setState(885);
      match(AngelscriptConsoleParser::RightShiftAssign);
      break;
    }

    case 26: {
      enterOuterAlt(_localctx, 26);
      setState(886);
      match(AngelscriptConsoleParser::LeftShiftAssign);
      break;
    }

    case 27: {
      enterOuterAlt(_localctx, 27);
      setState(887);
      match(AngelscriptConsoleParser::Equal);
      break;
    }

    case 28: {
      enterOuterAlt(_localctx, 28);
      setState(888);
      match(AngelscriptConsoleParser::NotEqual);
      break;
    }

    case 29: {
      enterOuterAlt(_localctx, 29);
      setState(889);
      match(AngelscriptConsoleParser::LessEqual);
      break;
    }

    case 30: {
      enterOuterAlt(_localctx, 30);
      setState(890);
      match(AngelscriptConsoleParser::AndAnd);
      break;
    }

    case 31: {
      enterOuterAlt(_localctx, 31);
      setState(891);
      match(AngelscriptConsoleParser::OrOr);
      break;
    }

    case 32: {
      enterOuterAlt(_localctx, 32);
      setState(892);
      match(AngelscriptConsoleParser::PlusPlus);
      break;
    }

    case 33: {
      enterOuterAlt(_localctx, 33);
      setState(893);
      match(AngelscriptConsoleParser::MinusMinus);
      break;
    }

    case 34: {
      enterOuterAlt(_localctx, 34);
      setState(894);
      match(AngelscriptConsoleParser::Comma);
      break;
    }

    case 35: {
      enterOuterAlt(_localctx, 35);
      setState(895);
      match(AngelscriptConsoleParser::LeftParen);
      setState(896);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 36: {
      enterOuterAlt(_localctx, 36);
      setState(897);
      match(AngelscriptConsoleParser::LeftBracket);
      setState(898);
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
  enterRule(_localctx, 180, AngelscriptConsoleParser::RuleLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(908);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::IntegerLiteral: {
        enterOuterAlt(_localctx, 1);
        setState(901);
        match(AngelscriptConsoleParser::IntegerLiteral);
        break;
      }

      case AngelscriptConsoleParser::CharacterLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(902);
        match(AngelscriptConsoleParser::CharacterLiteral);
        break;
      }

      case AngelscriptConsoleParser::FloatingLiteral: {
        enterOuterAlt(_localctx, 3);
        setState(903);
        match(AngelscriptConsoleParser::FloatingLiteral);
        break;
      }

      case AngelscriptConsoleParser::StringLiteral: {
        enterOuterAlt(_localctx, 4);
        setState(904);
        match(AngelscriptConsoleParser::StringLiteral);
        break;
      }

      case AngelscriptConsoleParser::False_:
      case AngelscriptConsoleParser::True_: {
        enterOuterAlt(_localctx, 5);
        setState(905);
        booleanLiteral();
        break;
      }

      case AngelscriptConsoleParser::UserDefinedLiteral: {
        enterOuterAlt(_localctx, 6);
        setState(906);
        match(AngelscriptConsoleParser::UserDefinedLiteral);
        break;
      }

      case AngelscriptConsoleParser::Nullptr: {
        enterOuterAlt(_localctx, 7);
        setState(907);
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
    case 8: return nestedNameSpecifierSempred(antlrcpp::downCast<NestedNameSpecifierContext *>(context), predicateIndex);
    case 18: return postfixExpressionSempred(antlrcpp::downCast<PostfixExpressionContext *>(context), predicateIndex);
    case 76: return declaratorDefSempred(antlrcpp::downCast<DeclaratorDefContext *>(context), predicateIndex);

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
