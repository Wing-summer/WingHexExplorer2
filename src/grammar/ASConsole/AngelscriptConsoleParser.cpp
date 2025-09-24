
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
      "typedefName", "typeSpecifier", "arraySpecifier", "trailingTypeSpecifier", 
      "typeSpecifierSeq", "trailingTypeSpecifierSeq", "simpleTypeSpecifier", 
      "theTypeName", "decltypeSpecifier", "namespaceName", "qualifiednamespacespecifier", 
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
  	4,1,96,927,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
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
  	91,2,92,7,92,1,0,5,0,188,8,0,10,0,12,0,191,9,0,1,0,1,0,1,1,1,1,1,1,1,
  	1,1,1,3,1,200,8,1,1,2,1,2,1,2,1,2,1,2,1,3,1,3,1,3,5,3,210,8,3,10,3,12,
  	3,213,9,3,1,4,1,4,1,5,4,5,218,8,5,11,5,12,5,219,1,5,1,5,1,5,1,5,1,5,1,
  	5,3,5,228,8,5,1,6,1,6,3,6,232,8,6,1,7,1,7,1,8,1,8,1,8,1,9,1,9,1,9,1,9,
  	3,9,243,8,9,1,9,1,9,1,9,1,9,1,9,5,9,250,8,9,10,9,12,9,253,9,9,1,10,1,
  	10,3,10,257,8,10,1,10,1,10,1,11,1,11,3,11,263,8,11,1,11,1,11,1,12,1,12,
  	1,12,1,12,3,12,271,8,12,3,12,273,8,12,1,13,1,13,1,14,1,14,1,14,5,14,280,
  	8,14,10,14,12,14,283,9,14,1,15,1,15,3,15,287,8,15,1,16,3,16,290,8,16,
  	1,16,1,16,1,17,3,17,295,8,17,1,17,1,17,1,17,1,18,1,18,3,18,302,8,18,1,
  	18,1,18,1,19,1,19,1,19,1,19,5,19,310,8,19,10,19,12,19,313,9,19,1,19,1,
  	19,3,19,317,8,19,1,19,1,19,3,19,321,8,19,1,19,1,19,1,19,1,19,1,19,1,19,
  	1,19,1,19,1,19,1,19,1,19,3,19,334,8,19,1,19,1,19,3,19,338,8,19,1,19,1,
  	19,1,19,1,19,3,19,344,8,19,1,19,1,19,1,19,1,19,1,19,3,19,351,8,19,1,19,
  	1,19,1,19,1,19,1,19,3,19,358,8,19,1,19,1,19,5,19,362,8,19,10,19,12,19,
  	365,9,19,1,20,1,20,1,21,3,21,370,8,21,1,21,1,21,1,21,3,21,375,8,21,1,
  	21,1,21,1,21,1,21,1,21,1,21,1,21,1,21,1,21,3,21,386,8,21,1,22,1,22,1,
  	22,1,22,3,22,392,8,22,1,22,1,22,1,22,1,22,1,22,3,22,399,8,22,1,23,1,23,
  	1,24,1,24,1,24,1,24,1,25,1,25,3,25,409,8,25,1,25,1,25,3,25,413,8,25,1,
  	26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,1,26,3,26,424,8,26,1,27,1,27,1,
  	27,5,27,429,8,27,10,27,12,27,432,9,27,1,28,1,28,1,28,5,28,437,8,28,10,
  	28,12,28,440,9,28,1,29,1,29,1,29,1,29,5,29,446,8,29,10,29,12,29,449,9,
  	29,1,30,1,30,1,30,1,30,3,30,455,8,30,1,31,1,31,1,31,5,31,460,8,31,10,
  	31,12,31,463,9,31,1,32,1,32,1,32,5,32,468,8,32,10,32,12,32,471,9,32,1,
  	33,1,33,1,33,5,33,476,8,33,10,33,12,33,479,9,33,1,34,1,34,1,34,5,34,484,
  	8,34,10,34,12,34,487,9,34,1,35,1,35,1,35,5,35,492,8,35,10,35,12,35,495,
  	9,35,1,36,1,36,1,36,5,36,500,8,36,10,36,12,36,503,9,36,1,37,1,37,1,37,
  	5,37,508,8,37,10,37,12,37,511,9,37,1,38,1,38,1,38,1,38,1,38,1,38,3,38,
  	519,8,38,1,39,1,39,1,39,1,39,1,39,3,39,526,8,39,1,40,1,40,1,41,1,41,1,
  	41,5,41,533,8,41,10,41,12,41,536,9,41,1,42,1,42,1,43,1,43,1,43,1,43,1,
  	43,1,43,1,43,3,43,547,8,43,1,44,1,44,1,44,1,44,3,44,553,8,44,1,44,1,44,
  	1,44,1,45,3,45,559,8,45,1,45,1,45,1,46,1,46,3,46,565,8,46,1,46,1,46,1,
  	47,4,47,570,8,47,11,47,12,47,571,1,48,1,48,1,48,1,48,1,48,1,48,1,48,3,
  	48,581,8,48,1,48,1,48,1,48,1,48,1,48,1,48,3,48,589,8,48,1,49,1,49,1,49,
  	1,49,1,49,1,49,3,49,597,8,49,3,49,599,8,49,1,50,1,50,1,50,1,50,1,50,1,
  	50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,1,50,3,50,619,
  	8,50,1,50,1,50,3,50,623,8,50,1,50,1,50,1,50,1,50,3,50,629,8,50,1,50,1,
  	50,1,50,3,50,634,8,50,1,51,1,51,3,51,638,8,51,1,52,1,52,1,52,1,53,1,53,
  	3,53,645,8,53,1,54,1,54,1,54,1,55,4,55,651,8,55,11,55,12,55,652,1,56,
  	1,56,3,56,657,8,56,1,57,1,57,1,57,1,57,1,57,1,58,1,58,1,58,3,58,667,8,
  	58,1,58,1,58,1,59,1,59,1,60,1,60,1,61,4,61,676,8,61,11,61,12,61,677,1,
  	62,1,62,1,63,1,63,1,64,1,64,3,64,686,8,64,1,64,1,64,1,65,1,65,5,65,692,
  	8,65,10,65,12,65,695,9,65,1,65,3,65,698,8,65,1,66,4,66,701,8,66,11,66,
  	12,66,702,1,67,4,67,706,8,67,11,67,12,67,707,1,68,3,68,711,8,68,1,68,
  	1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,1,68,
  	1,68,1,68,1,68,3,68,731,8,68,1,69,1,69,1,70,1,70,1,70,3,70,738,8,70,1,
  	70,1,70,1,71,1,71,1,72,3,72,745,8,72,1,72,1,72,1,73,4,73,750,8,73,11,
  	73,12,73,751,1,74,1,74,1,74,1,74,1,74,1,74,1,74,1,74,1,74,1,74,1,74,1,
  	74,1,74,4,74,767,8,74,11,74,12,74,768,3,74,771,8,74,1,75,1,75,1,75,5,
  	75,776,8,75,10,75,12,75,779,9,75,1,76,1,76,3,76,783,8,76,1,77,1,77,1,
  	77,1,78,1,78,1,78,1,78,1,78,1,78,1,78,3,78,795,8,78,1,78,3,78,798,8,78,
  	5,78,800,8,78,10,78,12,78,803,9,78,1,79,1,79,3,79,807,8,79,1,79,1,79,
  	3,79,811,8,79,1,79,3,79,814,8,79,1,80,1,80,1,81,1,81,1,82,1,82,1,83,1,
  	83,1,84,1,84,1,84,5,84,827,8,84,10,84,12,84,830,9,84,1,85,1,85,3,85,834,
  	8,85,1,85,1,85,3,85,838,8,85,1,86,1,86,1,86,1,86,1,86,3,86,845,8,86,1,
  	87,1,87,1,87,3,87,850,8,87,1,88,1,88,3,88,854,8,88,1,89,1,89,1,89,5,89,
  	859,8,89,10,89,12,89,862,9,89,1,89,3,89,865,8,89,1,90,1,90,1,90,3,90,
  	870,8,90,3,90,872,8,90,1,90,1,90,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,
  	91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,
  	91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,91,1,
  	91,1,91,1,91,1,91,1,91,3,91,916,8,91,1,92,1,92,1,92,1,92,1,92,1,92,1,
  	92,3,92,925,8,92,1,92,0,3,18,38,156,93,0,2,4,6,8,10,12,14,16,18,20,22,
  	24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,
  	70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,
  	112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,
  	148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,
  	184,0,14,2,0,30,30,37,37,2,0,52,52,56,56,1,0,75,76,2,0,46,48,52,55,1,
  	0,48,50,1,0,46,47,2,0,57,58,71,72,1,0,69,70,2,0,56,56,59,68,3,0,21,21,
  	26,26,34,34,1,0,40,45,2,0,52,52,73,73,2,0,42,42,44,44,2,0,43,43,45,45,
  	1009,0,189,1,0,0,0,2,199,1,0,0,0,4,201,1,0,0,0,6,206,1,0,0,0,8,214,1,
  	0,0,0,10,227,1,0,0,0,12,231,1,0,0,0,14,233,1,0,0,0,16,235,1,0,0,0,18,
  	238,1,0,0,0,20,254,1,0,0,0,22,260,1,0,0,0,24,272,1,0,0,0,26,274,1,0,0,
  	0,28,276,1,0,0,0,30,286,1,0,0,0,32,289,1,0,0,0,34,294,1,0,0,0,36,299,
  	1,0,0,0,38,337,1,0,0,0,40,366,1,0,0,0,42,385,1,0,0,0,44,398,1,0,0,0,46,
  	400,1,0,0,0,48,402,1,0,0,0,50,412,1,0,0,0,52,423,1,0,0,0,54,425,1,0,0,
  	0,56,433,1,0,0,0,58,441,1,0,0,0,60,454,1,0,0,0,62,456,1,0,0,0,64,464,
  	1,0,0,0,66,472,1,0,0,0,68,480,1,0,0,0,70,488,1,0,0,0,72,496,1,0,0,0,74,
  	504,1,0,0,0,76,512,1,0,0,0,78,525,1,0,0,0,80,527,1,0,0,0,82,529,1,0,0,
  	0,84,537,1,0,0,0,86,546,1,0,0,0,88,552,1,0,0,0,90,558,1,0,0,0,92,562,
  	1,0,0,0,94,569,1,0,0,0,96,588,1,0,0,0,98,598,1,0,0,0,100,633,1,0,0,0,
  	102,637,1,0,0,0,104,639,1,0,0,0,106,644,1,0,0,0,108,646,1,0,0,0,110,650,
  	1,0,0,0,112,656,1,0,0,0,114,658,1,0,0,0,116,663,1,0,0,0,118,670,1,0,0,
  	0,120,672,1,0,0,0,122,675,1,0,0,0,124,679,1,0,0,0,126,681,1,0,0,0,128,
  	683,1,0,0,0,130,697,1,0,0,0,132,700,1,0,0,0,134,705,1,0,0,0,136,730,1,
  	0,0,0,138,732,1,0,0,0,140,734,1,0,0,0,142,741,1,0,0,0,144,744,1,0,0,0,
  	146,749,1,0,0,0,148,770,1,0,0,0,150,772,1,0,0,0,152,780,1,0,0,0,154,784,
  	1,0,0,0,156,787,1,0,0,0,158,804,1,0,0,0,160,815,1,0,0,0,162,817,1,0,0,
  	0,164,819,1,0,0,0,166,821,1,0,0,0,168,823,1,0,0,0,170,831,1,0,0,0,172,
  	844,1,0,0,0,174,849,1,0,0,0,176,853,1,0,0,0,178,855,1,0,0,0,180,866,1,
  	0,0,0,182,915,1,0,0,0,184,924,1,0,0,0,186,188,3,2,1,0,187,186,1,0,0,0,
  	188,191,1,0,0,0,189,187,1,0,0,0,189,190,1,0,0,0,190,192,1,0,0,0,191,189,
  	1,0,0,0,192,193,5,0,0,1,193,1,1,0,0,0,194,195,3,122,61,0,195,196,3,150,
  	75,0,196,197,5,81,0,0,197,200,1,0,0,0,198,200,3,86,43,0,199,194,1,0,0,
  	0,199,198,1,0,0,0,200,3,1,0,0,0,201,202,5,83,0,0,202,203,5,57,0,0,203,
  	204,3,6,3,0,204,205,5,58,0,0,205,5,1,0,0,0,206,211,3,122,61,0,207,208,
  	5,77,0,0,208,210,3,122,61,0,209,207,1,0,0,0,210,213,1,0,0,0,211,209,1,
  	0,0,0,211,212,1,0,0,0,212,7,1,0,0,0,213,211,1,0,0,0,214,215,7,0,0,0,215,
  	9,1,0,0,0,216,218,3,184,92,0,217,216,1,0,0,0,218,219,1,0,0,0,219,217,
  	1,0,0,0,219,220,1,0,0,0,220,228,1,0,0,0,221,222,5,40,0,0,222,223,3,82,
  	41,0,223,224,5,41,0,0,224,228,1,0,0,0,225,228,3,12,6,0,226,228,3,20,10,
  	0,227,217,1,0,0,0,227,221,1,0,0,0,227,225,1,0,0,0,227,226,1,0,0,0,228,
  	11,1,0,0,0,229,232,3,14,7,0,230,232,3,16,8,0,231,229,1,0,0,0,231,230,
  	1,0,0,0,232,13,1,0,0,0,233,234,5,83,0,0,234,15,1,0,0,0,235,236,3,18,9,
  	0,236,237,3,14,7,0,237,17,1,0,0,0,238,242,6,9,-1,0,239,243,3,138,69,0,
  	240,243,3,142,71,0,241,243,3,140,70,0,242,239,1,0,0,0,242,240,1,0,0,0,
  	242,241,1,0,0,0,242,243,1,0,0,0,243,244,1,0,0,0,244,245,5,80,0,0,245,
  	251,1,0,0,0,246,247,10,1,0,0,247,248,5,83,0,0,248,250,5,80,0,0,249,246,
  	1,0,0,0,250,253,1,0,0,0,251,249,1,0,0,0,251,252,1,0,0,0,252,19,1,0,0,
  	0,253,251,1,0,0,0,254,256,3,22,11,0,255,257,3,36,18,0,256,255,1,0,0,0,
  	256,257,1,0,0,0,257,258,1,0,0,0,258,259,3,92,46,0,259,21,1,0,0,0,260,
  	262,5,42,0,0,261,263,3,24,12,0,262,261,1,0,0,0,262,263,1,0,0,0,263,264,
  	1,0,0,0,264,265,5,43,0,0,265,23,1,0,0,0,266,273,3,28,14,0,267,270,3,26,
  	13,0,268,269,5,77,0,0,269,271,3,28,14,0,270,268,1,0,0,0,270,271,1,0,0,
  	0,271,273,1,0,0,0,272,266,1,0,0,0,272,267,1,0,0,0,273,25,1,0,0,0,274,
  	275,7,1,0,0,275,27,1,0,0,0,276,281,3,30,15,0,277,278,5,77,0,0,278,280,
  	3,30,15,0,279,277,1,0,0,0,280,283,1,0,0,0,281,279,1,0,0,0,281,282,1,0,
  	0,0,282,29,1,0,0,0,283,281,1,0,0,0,284,287,3,32,16,0,285,287,3,34,17,
  	0,286,284,1,0,0,0,286,285,1,0,0,0,287,31,1,0,0,0,288,290,5,52,0,0,289,
  	288,1,0,0,0,289,290,1,0,0,0,290,291,1,0,0,0,291,292,5,83,0,0,292,33,1,
  	0,0,0,293,295,5,52,0,0,294,293,1,0,0,0,294,295,1,0,0,0,295,296,1,0,0,
  	0,296,297,5,83,0,0,297,298,3,172,86,0,298,35,1,0,0,0,299,301,5,40,0,0,
  	300,302,3,166,83,0,301,300,1,0,0,0,301,302,1,0,0,0,302,303,1,0,0,0,303,
  	304,5,41,0,0,304,37,1,0,0,0,305,306,6,19,-1,0,306,338,3,10,5,0,307,311,
  	3,136,68,0,308,310,3,128,64,0,309,308,1,0,0,0,310,313,1,0,0,0,311,309,
  	1,0,0,0,311,312,1,0,0,0,312,320,1,0,0,0,313,311,1,0,0,0,314,316,5,40,
  	0,0,315,317,3,40,20,0,316,315,1,0,0,0,316,317,1,0,0,0,317,318,1,0,0,0,
  	318,321,5,41,0,0,319,321,3,180,90,0,320,314,1,0,0,0,320,319,1,0,0,0,321,
  	338,1,0,0,0,322,323,5,6,0,0,323,324,5,57,0,0,324,325,3,164,82,0,325,326,
  	5,58,0,0,326,327,5,40,0,0,327,328,3,82,41,0,328,329,5,41,0,0,329,338,
  	1,0,0,0,330,333,5,40,0,0,331,334,3,82,41,0,332,334,3,164,82,0,333,331,
  	1,0,0,0,333,332,1,0,0,0,334,335,1,0,0,0,335,336,5,41,0,0,336,338,1,0,
  	0,0,337,305,1,0,0,0,337,307,1,0,0,0,337,322,1,0,0,0,337,330,1,0,0,0,338,
  	363,1,0,0,0,339,340,10,7,0,0,340,343,5,42,0,0,341,344,3,82,41,0,342,344,
  	3,180,90,0,343,341,1,0,0,0,343,342,1,0,0,0,344,345,1,0,0,0,345,346,5,
  	43,0,0,346,362,1,0,0,0,347,348,10,6,0,0,348,350,5,40,0,0,349,351,3,40,
  	20,0,350,349,1,0,0,0,350,351,1,0,0,0,351,352,1,0,0,0,352,362,5,41,0,0,
  	353,354,10,4,0,0,354,357,5,82,0,0,355,358,3,12,6,0,356,358,3,42,21,0,
  	357,355,1,0,0,0,357,356,1,0,0,0,358,362,1,0,0,0,359,360,10,3,0,0,360,
  	362,7,2,0,0,361,339,1,0,0,0,361,347,1,0,0,0,361,353,1,0,0,0,361,359,1,
  	0,0,0,362,365,1,0,0,0,363,361,1,0,0,0,363,364,1,0,0,0,364,39,1,0,0,0,
  	365,363,1,0,0,0,366,367,3,178,89,0,367,41,1,0,0,0,368,370,3,18,9,0,369,
  	368,1,0,0,0,369,370,1,0,0,0,370,374,1,0,0,0,371,372,3,138,69,0,372,373,
  	5,80,0,0,373,375,1,0,0,0,374,371,1,0,0,0,374,375,1,0,0,0,375,376,1,0,
  	0,0,376,377,5,54,0,0,377,386,3,138,69,0,378,379,3,18,9,0,379,380,5,80,
  	0,0,380,381,5,54,0,0,381,382,3,138,69,0,382,386,1,0,0,0,383,384,5,54,
  	0,0,384,386,3,140,70,0,385,369,1,0,0,0,385,378,1,0,0,0,385,383,1,0,0,
  	0,386,43,1,0,0,0,387,399,3,38,19,0,388,392,5,75,0,0,389,392,5,76,0,0,
  	390,392,3,46,23,0,391,388,1,0,0,0,391,389,1,0,0,0,391,390,1,0,0,0,392,
  	393,1,0,0,0,393,399,3,44,22,0,394,395,5,40,0,0,395,396,3,164,82,0,396,
  	397,5,41,0,0,397,399,1,0,0,0,398,387,1,0,0,0,398,391,1,0,0,0,398,394,
  	1,0,0,0,399,45,1,0,0,0,400,401,7,3,0,0,401,47,1,0,0,0,402,403,5,40,0,
  	0,403,404,3,40,20,0,404,405,5,41,0,0,405,49,1,0,0,0,406,408,5,40,0,0,
  	407,409,3,40,20,0,408,407,1,0,0,0,408,409,1,0,0,0,409,410,1,0,0,0,410,
  	413,5,41,0,0,411,413,3,180,90,0,412,406,1,0,0,0,412,411,1,0,0,0,413,51,
  	1,0,0,0,414,424,3,44,22,0,415,416,5,6,0,0,416,417,5,57,0,0,417,418,3,
  	164,82,0,418,419,5,58,0,0,419,420,5,40,0,0,420,421,3,52,26,0,421,422,
  	5,41,0,0,422,424,1,0,0,0,423,414,1,0,0,0,423,415,1,0,0,0,424,53,1,0,0,
  	0,425,430,3,52,26,0,426,427,7,4,0,0,427,429,3,52,26,0,428,426,1,0,0,0,
  	429,432,1,0,0,0,430,428,1,0,0,0,430,431,1,0,0,0,431,55,1,0,0,0,432,430,
  	1,0,0,0,433,438,3,54,27,0,434,435,7,5,0,0,435,437,3,54,27,0,436,434,1,
  	0,0,0,437,440,1,0,0,0,438,436,1,0,0,0,438,439,1,0,0,0,439,57,1,0,0,0,
  	440,438,1,0,0,0,441,447,3,56,28,0,442,443,3,60,30,0,443,444,3,56,28,0,
  	444,446,1,0,0,0,445,442,1,0,0,0,446,449,1,0,0,0,447,445,1,0,0,0,447,448,
  	1,0,0,0,448,59,1,0,0,0,449,447,1,0,0,0,450,451,5,58,0,0,451,455,5,58,
  	0,0,452,453,5,57,0,0,453,455,5,57,0,0,454,450,1,0,0,0,454,452,1,0,0,0,
  	455,61,1,0,0,0,456,461,3,58,29,0,457,458,7,6,0,0,458,460,3,58,29,0,459,
  	457,1,0,0,0,460,463,1,0,0,0,461,459,1,0,0,0,461,462,1,0,0,0,462,63,1,
  	0,0,0,463,461,1,0,0,0,464,469,3,62,31,0,465,466,7,7,0,0,466,468,3,62,
  	31,0,467,465,1,0,0,0,468,471,1,0,0,0,469,467,1,0,0,0,469,470,1,0,0,0,
  	470,65,1,0,0,0,471,469,1,0,0,0,472,477,3,64,32,0,473,474,5,52,0,0,474,
  	476,3,64,32,0,475,473,1,0,0,0,476,479,1,0,0,0,477,475,1,0,0,0,477,478,
  	1,0,0,0,478,67,1,0,0,0,479,477,1,0,0,0,480,485,3,66,33,0,481,482,5,51,
  	0,0,482,484,3,66,33,0,483,481,1,0,0,0,484,487,1,0,0,0,485,483,1,0,0,0,
  	485,486,1,0,0,0,486,69,1,0,0,0,487,485,1,0,0,0,488,493,3,68,34,0,489,
  	490,5,53,0,0,490,492,3,68,34,0,491,489,1,0,0,0,492,495,1,0,0,0,493,491,
  	1,0,0,0,493,494,1,0,0,0,494,71,1,0,0,0,495,493,1,0,0,0,496,501,3,70,35,
  	0,497,498,5,73,0,0,498,500,3,70,35,0,499,497,1,0,0,0,500,503,1,0,0,0,
  	501,499,1,0,0,0,501,502,1,0,0,0,502,73,1,0,0,0,503,501,1,0,0,0,504,509,
  	3,72,36,0,505,506,5,74,0,0,506,508,3,72,36,0,507,505,1,0,0,0,508,511,
  	1,0,0,0,509,507,1,0,0,0,509,510,1,0,0,0,510,75,1,0,0,0,511,509,1,0,0,
  	0,512,518,3,74,37,0,513,514,5,78,0,0,514,515,3,82,41,0,515,516,5,79,0,
  	0,516,517,3,78,39,0,517,519,1,0,0,0,518,513,1,0,0,0,518,519,1,0,0,0,519,
  	77,1,0,0,0,520,526,3,76,38,0,521,522,3,74,37,0,522,523,3,80,40,0,523,
  	524,3,176,88,0,524,526,1,0,0,0,525,520,1,0,0,0,525,521,1,0,0,0,526,79,
  	1,0,0,0,527,528,7,8,0,0,528,81,1,0,0,0,529,534,3,78,39,0,530,531,5,77,
  	0,0,531,533,3,78,39,0,532,530,1,0,0,0,533,536,1,0,0,0,534,532,1,0,0,0,
  	534,535,1,0,0,0,535,83,1,0,0,0,536,534,1,0,0,0,537,538,3,76,38,0,538,
  	85,1,0,0,0,539,547,3,88,44,0,540,547,3,112,56,0,541,547,3,90,45,0,542,
  	547,3,92,46,0,543,547,3,96,48,0,544,547,3,100,50,0,545,547,3,108,54,0,
  	546,539,1,0,0,0,546,540,1,0,0,0,546,541,1,0,0,0,546,542,1,0,0,0,546,543,
  	1,0,0,0,546,544,1,0,0,0,546,545,1,0,0,0,547,87,1,0,0,0,548,553,5,83,0,
  	0,549,550,5,22,0,0,550,553,3,84,42,0,551,553,5,27,0,0,552,548,1,0,0,0,
  	552,549,1,0,0,0,552,551,1,0,0,0,553,554,1,0,0,0,554,555,5,79,0,0,555,
  	556,3,86,43,0,556,89,1,0,0,0,557,559,3,82,41,0,558,557,1,0,0,0,558,559,
  	1,0,0,0,559,560,1,0,0,0,560,561,5,81,0,0,561,91,1,0,0,0,562,564,5,44,
  	0,0,563,565,3,94,47,0,564,563,1,0,0,0,564,565,1,0,0,0,565,566,1,0,0,0,
  	566,567,5,45,0,0,567,93,1,0,0,0,568,570,3,86,43,0,569,568,1,0,0,0,570,
  	571,1,0,0,0,571,569,1,0,0,0,571,572,1,0,0,0,572,95,1,0,0,0,573,574,5,
  	32,0,0,574,575,5,40,0,0,575,576,3,98,49,0,576,577,5,41,0,0,577,580,3,
  	86,43,0,578,579,5,29,0,0,579,581,3,86,43,0,580,578,1,0,0,0,580,581,1,
  	0,0,0,581,589,1,0,0,0,582,583,5,36,0,0,583,584,5,40,0,0,584,585,3,98,
  	49,0,585,586,5,41,0,0,586,587,3,86,43,0,587,589,1,0,0,0,588,573,1,0,0,
  	0,588,582,1,0,0,0,589,97,1,0,0,0,590,599,3,82,41,0,591,592,3,122,61,0,
  	592,596,3,154,77,0,593,594,5,56,0,0,594,597,3,176,88,0,595,597,3,180,
  	90,0,596,593,1,0,0,0,596,595,1,0,0,0,597,599,1,0,0,0,598,590,1,0,0,0,
  	598,591,1,0,0,0,599,99,1,0,0,0,600,601,5,39,0,0,601,602,5,40,0,0,602,
  	603,3,98,49,0,603,604,5,41,0,0,604,605,3,86,43,0,605,634,1,0,0,0,606,
  	607,5,28,0,0,607,608,3,86,43,0,608,609,5,39,0,0,609,610,5,40,0,0,610,
  	611,3,82,41,0,611,612,5,41,0,0,612,613,5,81,0,0,613,634,1,0,0,0,614,615,
  	5,31,0,0,615,628,5,40,0,0,616,618,3,102,51,0,617,619,3,98,49,0,618,617,
  	1,0,0,0,618,619,1,0,0,0,619,620,1,0,0,0,620,622,5,81,0,0,621,623,3,82,
  	41,0,622,621,1,0,0,0,622,623,1,0,0,0,623,629,1,0,0,0,624,625,3,104,52,
  	0,625,626,5,79,0,0,626,627,3,106,53,0,627,629,1,0,0,0,628,616,1,0,0,0,
  	628,624,1,0,0,0,629,630,1,0,0,0,630,631,5,41,0,0,631,632,3,86,43,0,632,
  	634,1,0,0,0,633,600,1,0,0,0,633,606,1,0,0,0,633,614,1,0,0,0,634,101,1,
  	0,0,0,635,638,3,90,45,0,636,638,3,116,58,0,637,635,1,0,0,0,637,636,1,
  	0,0,0,638,103,1,0,0,0,639,640,3,122,61,0,640,641,5,83,0,0,641,105,1,0,
  	0,0,642,645,3,82,41,0,643,645,3,180,90,0,644,642,1,0,0,0,644,643,1,0,
  	0,0,645,107,1,0,0,0,646,647,7,9,0,0,647,648,5,81,0,0,648,109,1,0,0,0,
  	649,651,3,112,56,0,650,649,1,0,0,0,651,652,1,0,0,0,652,650,1,0,0,0,652,
  	653,1,0,0,0,653,111,1,0,0,0,654,657,3,116,58,0,655,657,3,118,59,0,656,
  	654,1,0,0,0,656,655,1,0,0,0,657,113,1,0,0,0,658,659,5,83,0,0,659,660,
  	5,56,0,0,660,661,3,164,82,0,661,662,5,81,0,0,662,115,1,0,0,0,663,666,
  	3,122,61,0,664,667,3,150,75,0,665,667,3,78,39,0,666,664,1,0,0,0,666,665,
  	1,0,0,0,666,667,1,0,0,0,667,668,1,0,0,0,668,669,5,81,0,0,669,117,1,0,
  	0,0,670,671,5,81,0,0,671,119,1,0,0,0,672,673,3,126,63,0,673,121,1,0,0,
  	0,674,676,3,120,60,0,675,674,1,0,0,0,676,677,1,0,0,0,677,675,1,0,0,0,
  	677,678,1,0,0,0,678,123,1,0,0,0,679,680,5,83,0,0,680,125,1,0,0,0,681,
  	682,3,130,65,0,682,127,1,0,0,0,683,685,5,42,0,0,684,686,3,84,42,0,685,
  	684,1,0,0,0,685,686,1,0,0,0,686,687,1,0,0,0,687,688,5,43,0,0,688,129,
  	1,0,0,0,689,693,3,136,68,0,690,692,3,128,64,0,691,690,1,0,0,0,692,695,
  	1,0,0,0,693,691,1,0,0,0,693,694,1,0,0,0,694,698,1,0,0,0,695,693,1,0,0,
  	0,696,698,5,25,0,0,697,689,1,0,0,0,697,696,1,0,0,0,698,131,1,0,0,0,699,
  	701,3,126,63,0,700,699,1,0,0,0,701,702,1,0,0,0,702,700,1,0,0,0,702,703,
  	1,0,0,0,703,133,1,0,0,0,704,706,3,130,65,0,705,704,1,0,0,0,706,707,1,
  	0,0,0,707,705,1,0,0,0,707,708,1,0,0,0,708,135,1,0,0,0,709,711,3,18,9,
  	0,710,709,1,0,0,0,710,711,1,0,0,0,711,712,1,0,0,0,712,731,3,138,69,0,
  	713,731,3,4,2,0,714,731,5,7,0,0,715,731,5,8,0,0,716,731,5,9,0,0,717,731,
  	5,10,0,0,718,731,5,11,0,0,719,731,5,12,0,0,720,731,5,13,0,0,721,731,5,
  	14,0,0,722,731,5,15,0,0,723,731,5,16,0,0,724,731,5,17,0,0,725,731,5,18,
  	0,0,726,731,5,19,0,0,727,731,5,38,0,0,728,731,5,20,0,0,729,731,3,140,
  	70,0,730,710,1,0,0,0,730,713,1,0,0,0,730,714,1,0,0,0,730,715,1,0,0,0,
  	730,716,1,0,0,0,730,717,1,0,0,0,730,718,1,0,0,0,730,719,1,0,0,0,730,720,
  	1,0,0,0,730,721,1,0,0,0,730,722,1,0,0,0,730,723,1,0,0,0,730,724,1,0,0,
  	0,730,725,1,0,0,0,730,726,1,0,0,0,730,727,1,0,0,0,730,728,1,0,0,0,730,
  	729,1,0,0,0,731,137,1,0,0,0,732,733,3,124,62,0,733,139,1,0,0,0,734,737,
  	5,40,0,0,735,738,3,82,41,0,736,738,5,20,0,0,737,735,1,0,0,0,737,736,1,
  	0,0,0,738,739,1,0,0,0,739,740,5,41,0,0,740,141,1,0,0,0,741,742,5,83,0,
  	0,742,143,1,0,0,0,743,745,3,18,9,0,744,743,1,0,0,0,744,745,1,0,0,0,745,
  	746,1,0,0,0,746,747,3,142,71,0,747,145,1,0,0,0,748,750,3,148,74,0,749,
  	748,1,0,0,0,750,751,1,0,0,0,751,749,1,0,0,0,751,752,1,0,0,0,752,147,1,
  	0,0,0,753,754,5,40,0,0,754,755,3,146,73,0,755,756,5,41,0,0,756,771,1,
  	0,0,0,757,758,5,42,0,0,758,759,3,146,73,0,759,760,5,43,0,0,760,771,1,
  	0,0,0,761,762,5,44,0,0,762,763,3,146,73,0,763,764,5,45,0,0,764,771,1,
  	0,0,0,765,767,8,10,0,0,766,765,1,0,0,0,767,768,1,0,0,0,768,766,1,0,0,
  	0,768,769,1,0,0,0,769,771,1,0,0,0,770,753,1,0,0,0,770,757,1,0,0,0,770,
  	761,1,0,0,0,770,766,1,0,0,0,771,149,1,0,0,0,772,777,3,152,76,0,773,774,
  	5,77,0,0,774,776,3,152,76,0,775,773,1,0,0,0,776,779,1,0,0,0,777,775,1,
  	0,0,0,777,778,1,0,0,0,778,151,1,0,0,0,779,777,1,0,0,0,780,782,5,83,0,
  	0,781,783,3,172,86,0,782,781,1,0,0,0,782,783,1,0,0,0,783,153,1,0,0,0,
  	784,785,3,156,78,0,785,786,3,158,79,0,786,155,1,0,0,0,787,788,6,78,-1,
  	0,788,789,3,162,81,0,789,801,1,0,0,0,790,797,10,1,0,0,791,798,3,158,79,
  	0,792,794,5,42,0,0,793,795,3,84,42,0,794,793,1,0,0,0,794,795,1,0,0,0,
  	795,796,1,0,0,0,796,798,5,43,0,0,797,791,1,0,0,0,797,792,1,0,0,0,798,
  	800,1,0,0,0,799,790,1,0,0,0,800,803,1,0,0,0,801,799,1,0,0,0,801,802,1,
  	0,0,0,802,157,1,0,0,0,803,801,1,0,0,0,804,806,5,40,0,0,805,807,3,166,
  	83,0,806,805,1,0,0,0,806,807,1,0,0,0,807,808,1,0,0,0,808,810,5,41,0,0,
  	809,811,5,25,0,0,810,809,1,0,0,0,810,811,1,0,0,0,811,813,1,0,0,0,812,
  	814,3,160,80,0,813,812,1,0,0,0,813,814,1,0,0,0,814,159,1,0,0,0,815,816,
  	7,11,0,0,816,161,1,0,0,0,817,818,3,12,6,0,818,163,1,0,0,0,819,820,3,132,
  	66,0,820,165,1,0,0,0,821,822,3,168,84,0,822,167,1,0,0,0,823,828,3,170,
  	85,0,824,825,5,77,0,0,825,827,3,170,85,0,826,824,1,0,0,0,827,830,1,0,
  	0,0,828,826,1,0,0,0,828,829,1,0,0,0,829,169,1,0,0,0,830,828,1,0,0,0,831,
  	833,3,122,61,0,832,834,5,83,0,0,833,832,1,0,0,0,833,834,1,0,0,0,834,837,
  	1,0,0,0,835,836,5,56,0,0,836,838,3,176,88,0,837,835,1,0,0,0,837,838,1,
  	0,0,0,838,171,1,0,0,0,839,845,3,174,87,0,840,841,5,40,0,0,841,842,3,40,
  	20,0,842,843,5,41,0,0,843,845,1,0,0,0,844,839,1,0,0,0,844,840,1,0,0,0,
  	845,173,1,0,0,0,846,847,5,56,0,0,847,850,3,176,88,0,848,850,3,180,90,
  	0,849,846,1,0,0,0,849,848,1,0,0,0,850,175,1,0,0,0,851,854,3,78,39,0,852,
  	854,3,180,90,0,853,851,1,0,0,0,853,852,1,0,0,0,854,177,1,0,0,0,855,860,
  	3,176,88,0,856,857,5,77,0,0,857,859,3,176,88,0,858,856,1,0,0,0,859,862,
  	1,0,0,0,860,858,1,0,0,0,860,861,1,0,0,0,861,864,1,0,0,0,862,860,1,0,0,
  	0,863,865,5,77,0,0,864,863,1,0,0,0,864,865,1,0,0,0,865,179,1,0,0,0,866,
  	871,7,12,0,0,867,869,3,178,89,0,868,870,5,77,0,0,869,868,1,0,0,0,869,
  	870,1,0,0,0,870,872,1,0,0,0,871,867,1,0,0,0,871,872,1,0,0,0,872,873,1,
  	0,0,0,873,874,7,13,0,0,874,181,1,0,0,0,875,916,1,0,0,0,876,916,5,46,0,
  	0,877,916,5,47,0,0,878,916,5,48,0,0,879,916,5,49,0,0,880,916,5,50,0,0,
  	881,916,5,51,0,0,882,916,5,52,0,0,883,916,5,53,0,0,884,916,5,54,0,0,885,
  	916,5,55,0,0,886,916,5,56,0,0,887,916,5,58,0,0,888,916,5,57,0,0,889,916,
  	5,72,0,0,890,916,5,59,0,0,891,916,5,60,0,0,892,916,5,61,0,0,893,916,5,
  	63,0,0,894,916,5,64,0,0,895,916,5,65,0,0,896,916,5,66,0,0,897,898,5,57,
  	0,0,898,916,5,57,0,0,899,900,5,58,0,0,900,916,5,58,0,0,901,916,5,68,0,
  	0,902,916,5,67,0,0,903,916,5,69,0,0,904,916,5,70,0,0,905,916,5,71,0,0,
  	906,916,5,73,0,0,907,916,5,74,0,0,908,916,5,75,0,0,909,916,5,76,0,0,910,
  	916,5,77,0,0,911,912,5,40,0,0,912,916,5,41,0,0,913,914,5,42,0,0,914,916,
  	5,43,0,0,915,875,1,0,0,0,915,876,1,0,0,0,915,877,1,0,0,0,915,878,1,0,
  	0,0,915,879,1,0,0,0,915,880,1,0,0,0,915,881,1,0,0,0,915,882,1,0,0,0,915,
  	883,1,0,0,0,915,884,1,0,0,0,915,885,1,0,0,0,915,886,1,0,0,0,915,887,1,
  	0,0,0,915,888,1,0,0,0,915,889,1,0,0,0,915,890,1,0,0,0,915,891,1,0,0,0,
  	915,892,1,0,0,0,915,893,1,0,0,0,915,894,1,0,0,0,915,895,1,0,0,0,915,896,
  	1,0,0,0,915,897,1,0,0,0,915,899,1,0,0,0,915,901,1,0,0,0,915,902,1,0,0,
  	0,915,903,1,0,0,0,915,904,1,0,0,0,915,905,1,0,0,0,915,906,1,0,0,0,915,
  	907,1,0,0,0,915,908,1,0,0,0,915,909,1,0,0,0,915,910,1,0,0,0,915,911,1,
  	0,0,0,915,913,1,0,0,0,916,183,1,0,0,0,917,925,5,1,0,0,918,925,5,2,0,0,
  	919,925,5,3,0,0,920,925,5,4,0,0,921,925,3,8,4,0,922,925,5,5,0,0,923,925,
  	5,33,0,0,924,917,1,0,0,0,924,918,1,0,0,0,924,919,1,0,0,0,924,920,1,0,
  	0,0,924,921,1,0,0,0,924,922,1,0,0,0,924,923,1,0,0,0,925,185,1,0,0,0,100,
  	189,199,211,219,227,231,242,251,256,262,270,272,281,286,289,294,301,311,
  	316,320,333,337,343,350,357,361,363,369,374,385,391,398,408,412,423,430,
  	438,447,454,461,469,477,485,493,501,509,518,525,534,546,552,558,564,571,
  	580,588,596,598,618,622,628,633,637,644,652,656,666,677,685,693,697,702,
  	707,710,730,737,744,751,768,770,777,782,794,797,801,806,810,813,828,833,
  	837,844,849,853,860,864,869,871,915,924
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
    setState(189);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070729953837054) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 355) != 0)) {
      setState(186);
      scriptItem();
      setState(191);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(192);
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
    setState(199);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(194);
      declSpecifierSeq();
      setState(195);
      initDeclaratorList();
      setState(196);
      match(AngelscriptConsoleParser::Semi);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(198);
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
    setState(201);
    match(AngelscriptConsoleParser::Identifier);
    setState(202);
    match(AngelscriptConsoleParser::Less);
    setState(203);
    simpleTypeSpecifierList();
    setState(204);
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
    setState(206);
    declSpecifierSeq();
    setState(211);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(207);
      match(AngelscriptConsoleParser::Comma);
      setState(208);
      declSpecifierSeq();
      setState(213);
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
    setState(214);
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
    setState(227);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 4, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(217); 
      _errHandler->sync(this);
      alt = 1;
      do {
        switch (alt) {
          case 1: {
                setState(216);
                literal();
                break;
              }

        default:
          throw NoViableAltException(this);
        }
        setState(219); 
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx);
      } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(221);
      match(AngelscriptConsoleParser::LeftParen);
      setState(222);
      expression();
      setState(223);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(225);
      idExpression();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(226);
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
    setState(231);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(229);
      unqualifiedId();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(230);
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
    setState(233);
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
    setState(235);
    nestedNameSpecifier(0);
    setState(236);
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
    setState(242);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 6, _ctx)) {
    case 1: {
      setState(239);
      theTypeName();
      break;
    }

    case 2: {
      setState(240);
      namespaceName();
      break;
    }

    case 3: {
      setState(241);
      decltypeSpecifier();
      break;
    }

    default:
      break;
    }
    setState(244);
    match(AngelscriptConsoleParser::Doublecolon);
    _ctx->stop = _input->LT(-1);
    setState(251);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<NestedNameSpecifierContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleNestedNameSpecifier);
        setState(246);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(247);
        match(AngelscriptConsoleParser::Identifier);
        setState(248);
        match(AngelscriptConsoleParser::Doublecolon); 
      }
      setState(253);
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
    setState(254);
    lambdaIntroducer();
    setState(256);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::LeftParen) {
      setState(255);
      lambdaDeclarator();
    }
    setState(258);
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
    setState(260);
    match(AngelscriptConsoleParser::LeftBracket);
    setState(262);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 52) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 52)) & 2147483665) != 0)) {
      setState(261);
      lambdaCapture();
    }
    setState(264);
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
    setState(272);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(266);
      captureList();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(267);
      captureDefault();
      setState(270);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Comma) {
        setState(268);
        match(AngelscriptConsoleParser::Comma);
        setState(269);
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
    setState(274);
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
    setState(276);
    capture();
    setState(281);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(277);
      match(AngelscriptConsoleParser::Comma);
      setState(278);
      capture();
      setState(283);
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
    setState(286);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(284);
      simpleCapture();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(285);
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
    setState(289);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::And) {
      setState(288);
      match(AngelscriptConsoleParser::And);
    }
    setState(291);
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
    setState(294);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::And) {
      setState(293);
      match(AngelscriptConsoleParser::And);
    }
    setState(296);
    match(AngelscriptConsoleParser::Identifier);
    setState(297);
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
    setState(299);
    match(AngelscriptConsoleParser::LeftParen);
    setState(301);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier) {
      setState(300);
      parameterDeclarationClause();
    }
    setState(303);
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
    setState(337);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx)) {
    case 1: {
      setState(306);
      primaryExpression();
      break;
    }

    case 2: {
      setState(307);
      simpleTypeSpecifier();
      setState(311);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(308);
          arraySpecifier(); 
        }
        setState(313);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
      }
      setState(320);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::LeftParen: {
          setState(314);
          match(AngelscriptConsoleParser::LeftParen);
          setState(316);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(315);
            expressionList();
          }
          setState(318);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case AngelscriptConsoleParser::LeftBracket:
        case AngelscriptConsoleParser::LeftBrace: {
          setState(319);
          bracedInitList();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      break;
    }

    case 3: {
      setState(322);
      match(AngelscriptConsoleParser::Cast);
      setState(323);
      match(AngelscriptConsoleParser::Less);
      setState(324);
      theTypeId();
      setState(325);
      match(AngelscriptConsoleParser::Greater);
      setState(326);
      match(AngelscriptConsoleParser::LeftParen);
      setState(327);
      expression();
      setState(328);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 4: {
      setState(330);
      match(AngelscriptConsoleParser::LeftParen);
      setState(333);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
      case 1: {
        setState(331);
        expression();
        break;
      }

      case 2: {
        setState(332);
        theTypeId();
        break;
      }

      default:
        break;
      }
      setState(335);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(363);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(361);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(339);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(340);
          match(AngelscriptConsoleParser::LeftBracket);
          setState(343);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
          case 1: {
            setState(341);
            expression();
            break;
          }

          case 2: {
            setState(342);
            bracedInitList();
            break;
          }

          default:
            break;
          }
          setState(345);
          match(AngelscriptConsoleParser::RightBracket);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(347);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(348);
          match(AngelscriptConsoleParser::LeftParen);
          setState(350);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(349);
            expressionList();
          }
          setState(352);
          match(AngelscriptConsoleParser::RightParen);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<PostfixExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RulePostfixExpression);
          setState(353);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(354);
          match(AngelscriptConsoleParser::Dot);
          setState(357);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx)) {
          case 1: {
            setState(355);
            idExpression();
            break;
          }

          case 2: {
            setState(356);
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
          setState(359);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(360);
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
      setState(365);
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
    setState(366);
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
    setState(385);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 29, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(369);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
      case 1: {
        setState(368);
        nestedNameSpecifier(0);
        break;
      }

      default:
        break;
      }
      setState(374);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Identifier) {
        setState(371);
        theTypeName();
        setState(372);
        match(AngelscriptConsoleParser::Doublecolon);
      }
      setState(376);
      match(AngelscriptConsoleParser::Tilde);
      setState(377);
      theTypeName();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(378);
      nestedNameSpecifier(0);
      setState(379);
      match(AngelscriptConsoleParser::Doublecolon);
      setState(380);
      match(AngelscriptConsoleParser::Tilde);
      setState(381);
      theTypeName();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(383);
      match(AngelscriptConsoleParser::Tilde);
      setState(384);
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
    setState(398);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(387);
      postfixExpression(0);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(391);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::PlusPlus: {
          setState(388);
          match(AngelscriptConsoleParser::PlusPlus);
          break;
        }

        case AngelscriptConsoleParser::MinusMinus: {
          setState(389);
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
          setState(390);
          unaryOperator();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(393);
      unaryExpression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(394);
      match(AngelscriptConsoleParser::LeftParen);
      setState(395);
      theTypeId();
      setState(396);
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
    setState(400);
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
    setState(402);
    match(AngelscriptConsoleParser::LeftParen);
    setState(403);
    expressionList();
    setState(404);
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
    setState(412);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 1);
        setState(406);
        match(AngelscriptConsoleParser::LeftParen);
        setState(408);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 75)) & 291) != 0)) {
          setState(407);
          expressionList();
        }
        setState(410);
        match(AngelscriptConsoleParser::RightParen);
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(411);
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
    setState(423);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(414);
      unaryExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(415);
      match(AngelscriptConsoleParser::Cast);
      setState(416);
      match(AngelscriptConsoleParser::Less);
      setState(417);
      theTypeId();
      setState(418);
      match(AngelscriptConsoleParser::Greater);
      setState(419);
      match(AngelscriptConsoleParser::LeftParen);
      setState(420);
      castExpression();
      setState(421);
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
    setState(425);
    castExpression();
    setState(430);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1970324836974592) != 0)) {
      setState(426);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 1970324836974592) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(427);
      castExpression();
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
    setState(433);
    multiplicativeExpression();
    setState(438);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Plus

    || _la == AngelscriptConsoleParser::Minus) {
      setState(434);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Plus

      || _la == AngelscriptConsoleParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(435);
      multiplicativeExpression();
      setState(440);
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
    setState(441);
    additiveExpression();
    setState(447);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(442);
        shiftOperator();
        setState(443);
        additiveExpression(); 
      }
      setState(449);
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
  enterRule(_localctx, 60, AngelscriptConsoleParser::RuleShiftOperator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(454);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Greater: {
        enterOuterAlt(_localctx, 1);
        setState(450);
        match(AngelscriptConsoleParser::Greater);
        setState(451);
        match(AngelscriptConsoleParser::Greater);
        break;
      }

      case AngelscriptConsoleParser::Less: {
        enterOuterAlt(_localctx, 2);
        setState(452);
        match(AngelscriptConsoleParser::Less);
        setState(453);
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
    setState(456);
    shiftExpression();
    setState(461);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 57) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 57)) & 49155) != 0)) {
      setState(457);
      _la = _input->LA(1);
      if (!(((((_la - 57) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 57)) & 49155) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(458);
      shiftExpression();
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
    setState(464);
    relationalExpression();
    setState(469);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Equal

    || _la == AngelscriptConsoleParser::NotEqual) {
      setState(465);
      _la = _input->LA(1);
      if (!(_la == AngelscriptConsoleParser::Equal

      || _la == AngelscriptConsoleParser::NotEqual)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(466);
      relationalExpression();
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
    setState(472);
    equalityExpression();
    setState(477);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::And) {
      setState(473);
      match(AngelscriptConsoleParser::And);
      setState(474);
      equalityExpression();
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
    setState(480);
    andExpression();
    setState(485);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Xor) {
      setState(481);
      match(AngelscriptConsoleParser::Xor);
      setState(482);
      andExpression();
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
    setState(488);
    exclusiveOrExpression();
    setState(493);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Or) {
      setState(489);
      match(AngelscriptConsoleParser::Or);
      setState(490);
      exclusiveOrExpression();
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
    setState(496);
    inclusiveOrExpression();
    setState(501);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::AndAnd) {
      setState(497);
      match(AngelscriptConsoleParser::AndAnd);
      setState(498);
      inclusiveOrExpression();
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
    setState(504);
    logicalAndExpression();
    setState(509);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::OrOr) {
      setState(505);
      match(AngelscriptConsoleParser::OrOr);
      setState(506);
      logicalAndExpression();
      setState(511);
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
    setState(512);
    logicalOrExpression();
    setState(518);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Question) {
      setState(513);
      match(AngelscriptConsoleParser::Question);
      setState(514);
      expression();
      setState(515);
      match(AngelscriptConsoleParser::Colon);
      setState(516);
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
    setState(525);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(520);
      conditionalExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(521);
      logicalOrExpression();
      setState(522);
      assignmentOperator();
      setState(523);
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
    setState(527);
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
    setState(529);
    assignmentExpression();
    setState(534);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(530);
      match(AngelscriptConsoleParser::Comma);
      setState(531);
      assignmentExpression();
      setState(536);
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
    setState(537);
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
    setState(546);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 49, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(539);
      labeledStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(540);
      declaration();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(541);
      expressionStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(542);
      compoundStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(543);
      selectionStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(544);
      iterationStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(545);
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
    setState(552);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Identifier: {
        setState(548);
        match(AngelscriptConsoleParser::Identifier);
        break;
      }

      case AngelscriptConsoleParser::Case: {
        setState(549);
        match(AngelscriptConsoleParser::Case);
        setState(550);
        constantExpression();
        break;
      }

      case AngelscriptConsoleParser::Default: {
        setState(551);
        match(AngelscriptConsoleParser::Default);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(554);
    match(AngelscriptConsoleParser::Colon);
    setState(555);
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
    setState(558);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 291) != 0)) {
      setState(557);
      expression();
    }
    setState(560);
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
    setState(562);
    match(AngelscriptConsoleParser::LeftBrace);
    setState(564);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070729953837054) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 355) != 0)) {
      setState(563);
      statementSeq();
    }
    setState(566);
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
    setState(569); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(568);
      statement();
      setState(571); 
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
    setState(588);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::If: {
        enterOuterAlt(_localctx, 1);
        setState(573);
        match(AngelscriptConsoleParser::If);
        setState(574);
        match(AngelscriptConsoleParser::LeftParen);
        setState(575);
        condition();
        setState(576);
        match(AngelscriptConsoleParser::RightParen);
        setState(577);
        statement();
        setState(580);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx)) {
        case 1: {
          setState(578);
          match(AngelscriptConsoleParser::Else);
          setState(579);
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
        setState(582);
        match(AngelscriptConsoleParser::Switch);
        setState(583);
        match(AngelscriptConsoleParser::LeftParen);
        setState(584);
        condition();
        setState(585);
        match(AngelscriptConsoleParser::RightParen);
        setState(586);
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
    setState(598);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 57, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(590);
      expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(591);
      declSpecifierSeq();
      setState(592);
      declarator();
      setState(596);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case AngelscriptConsoleParser::Assign: {
          setState(593);
          match(AngelscriptConsoleParser::Assign);
          setState(594);
          initializerClause();
          break;
        }

        case AngelscriptConsoleParser::LeftBracket:
        case AngelscriptConsoleParser::LeftBrace: {
          setState(595);
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
    setState(633);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::While: {
        enterOuterAlt(_localctx, 1);
        setState(600);
        match(AngelscriptConsoleParser::While);
        setState(601);
        match(AngelscriptConsoleParser::LeftParen);
        setState(602);
        condition();
        setState(603);
        match(AngelscriptConsoleParser::RightParen);
        setState(604);
        statement();
        break;
      }

      case AngelscriptConsoleParser::Do: {
        enterOuterAlt(_localctx, 2);
        setState(606);
        match(AngelscriptConsoleParser::Do);
        setState(607);
        statement();
        setState(608);
        match(AngelscriptConsoleParser::While);
        setState(609);
        match(AngelscriptConsoleParser::LeftParen);
        setState(610);
        expression();
        setState(611);
        match(AngelscriptConsoleParser::RightParen);
        setState(612);
        match(AngelscriptConsoleParser::Semi);
        break;
      }

      case AngelscriptConsoleParser::For: {
        enterOuterAlt(_localctx, 3);
        setState(614);
        match(AngelscriptConsoleParser::For);
        setState(615);
        match(AngelscriptConsoleParser::LeftParen);
        setState(628);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 60, _ctx)) {
        case 1: {
          setState(616);
          forInitStatement();
          setState(618);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68052495194128382) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(617);
            condition();
          }
          setState(620);
          match(AngelscriptConsoleParser::Semi);
          setState(622);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 75)) & 291) != 0)) {
            setState(621);
            expression();
          }
          break;
        }

        case 2: {
          setState(624);
          forRangeDeclaration();
          setState(625);
          match(AngelscriptConsoleParser::Colon);
          setState(626);
          forRangeInitializer();
          break;
        }

        default:
          break;
        }
        setState(630);
        match(AngelscriptConsoleParser::RightParen);
        setState(631);
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
    setState(637);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(635);
      expressionStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(636);
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
    setState(639);
    declSpecifierSeq();
    setState(640);
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
    setState(644);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(642);
      expression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(643);
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
    setState(646);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 17249075200) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(647);
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
    setState(650); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(649);
      declaration();
      setState(652); 
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
    setState(656);
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
        setState(654);
        simpleDeclaration();
        break;
      }

      case AngelscriptConsoleParser::Semi: {
        enterOuterAlt(_localctx, 2);
        setState(655);
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
    setState(658);
    match(AngelscriptConsoleParser::Identifier);
    setState(659);
    match(AngelscriptConsoleParser::Assign);
    setState(660);
    theTypeId();
    setState(661);
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
    setState(663);
    declSpecifierSeq();
    setState(666);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 66, _ctx)) {
    case 1: {
      setState(664);
      initDeclaratorList();
      break;
    }

    case 2: {
      setState(665);
      assignmentExpression();
      break;
    }

    default:
      break;
    }
    setState(668);
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
    setState(670);
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
    setState(672);
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
    setState(675); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(674);
              declSpecifier();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(677); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx);
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
    setState(679);
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
    setState(681);
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
  enterRule(_localctx, 128, AngelscriptConsoleParser::RuleArraySpecifier);
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
    setState(683);
    match(AngelscriptConsoleParser::LeftBracket);
    setState(685);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 291) != 0)) {
      setState(684);
      constantExpression();
    }
    setState(687);
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
  enterRule(_localctx, 130, AngelscriptConsoleParser::RuleTrailingTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(697);
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
        setState(689);
        simpleTypeSpecifier();
        setState(693);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
        while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
          if (alt == 1) {
            setState(690);
            arraySpecifier(); 
          }
          setState(695);
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx);
        }
        break;
      }

      case AngelscriptConsoleParser::Const: {
        enterOuterAlt(_localctx, 2);
        setState(696);
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
  enterRule(_localctx, 132, AngelscriptConsoleParser::RuleTypeSpecifierSeq);
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
    setState(700); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(699);
      typeSpecifier();
      setState(702); 
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
  enterRule(_localctx, 134, AngelscriptConsoleParser::RuleTrailingTypeSpecifierSeq);
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
    setState(705); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(704);
      trailingTypeSpecifier();
      setState(707); 
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
  enterRule(_localctx, 136, AngelscriptConsoleParser::RuleSimpleTypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(730);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(710);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 73, _ctx)) {
      case 1: {
        setState(709);
        nestedNameSpecifier(0);
        break;
      }

      default:
        break;
      }
      setState(712);
      theTypeName();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(713);
      asGeneric();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(714);
      match(AngelscriptConsoleParser::Int);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(715);
      match(AngelscriptConsoleParser::Int8);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(716);
      match(AngelscriptConsoleParser::Int16);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(717);
      match(AngelscriptConsoleParser::Int32);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(718);
      match(AngelscriptConsoleParser::Int64);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(719);
      match(AngelscriptConsoleParser::UInt);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(720);
      match(AngelscriptConsoleParser::UInt8);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(721);
      match(AngelscriptConsoleParser::UInt16);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(722);
      match(AngelscriptConsoleParser::UInt32);
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(723);
      match(AngelscriptConsoleParser::UInt64);
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(724);
      match(AngelscriptConsoleParser::Float);
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(725);
      match(AngelscriptConsoleParser::Double);
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(726);
      match(AngelscriptConsoleParser::Bool);
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(727);
      match(AngelscriptConsoleParser::Void);
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(728);
      match(AngelscriptConsoleParser::Auto);
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(729);
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
  enterRule(_localctx, 138, AngelscriptConsoleParser::RuleTheTypeName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(732);
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
  enterRule(_localctx, 140, AngelscriptConsoleParser::RuleDecltypeSpecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(734);
    match(AngelscriptConsoleParser::LeftParen);
    setState(737);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 75, _ctx)) {
    case 1: {
      setState(735);
      expression();
      break;
    }

    case 2: {
      setState(736);
      match(AngelscriptConsoleParser::Auto);
      break;
    }

    default:
      break;
    }
    setState(739);
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
  enterRule(_localctx, 142, AngelscriptConsoleParser::RuleNamespaceName);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(741);
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
  enterRule(_localctx, 144, AngelscriptConsoleParser::RuleQualifiednamespacespecifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(744);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 76, _ctx)) {
    case 1: {
      setState(743);
      nestedNameSpecifier(0);
      break;
    }

    default:
      break;
    }
    setState(746);
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
  enterRule(_localctx, 146, AngelscriptConsoleParser::RuleBalancedTokenSeq);
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
    setState(749); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(748);
      balancedtoken();
      setState(751); 
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
  enterRule(_localctx, 148, AngelscriptConsoleParser::RuleBalancedtoken);
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
    setState(770);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 1);
        setState(753);
        match(AngelscriptConsoleParser::LeftParen);
        setState(754);
        balancedTokenSeq();
        setState(755);
        match(AngelscriptConsoleParser::RightParen);
        break;
      }

      case AngelscriptConsoleParser::LeftBracket: {
        enterOuterAlt(_localctx, 2);
        setState(757);
        match(AngelscriptConsoleParser::LeftBracket);
        setState(758);
        balancedTokenSeq();
        setState(759);
        match(AngelscriptConsoleParser::RightBracket);
        break;
      }

      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 3);
        setState(761);
        match(AngelscriptConsoleParser::LeftBrace);
        setState(762);
        balancedTokenSeq();
        setState(763);
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
        setState(766); 
        _errHandler->sync(this);
        alt = 1;
        do {
          switch (alt) {
            case 1: {
                  setState(765);
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
          setState(768); 
          _errHandler->sync(this);
          alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 78, _ctx);
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
  enterRule(_localctx, 150, AngelscriptConsoleParser::RuleInitDeclaratorList);
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
    setState(772);
    initDeclarator();
    setState(777);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(773);
      match(AngelscriptConsoleParser::Comma);
      setState(774);
      initDeclarator();
      setState(779);
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
  enterRule(_localctx, 152, AngelscriptConsoleParser::RuleInitDeclarator);
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
    setState(780);
    match(AngelscriptConsoleParser::Identifier);
    setState(782);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 72080683782111232) != 0)) {
      setState(781);
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
  enterRule(_localctx, 154, AngelscriptConsoleParser::RuleDeclarator);

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
    declaratorDef(0);
    setState(785);
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
  size_t startState = 156;
  enterRecursionRule(_localctx, 156, AngelscriptConsoleParser::RuleDeclaratorDef, precedence);

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
    setState(788);
    declaratorid();
    _ctx->stop = _input->LT(-1);
    setState(801);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 84, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<DeclaratorDefContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleDeclaratorDef);
        setState(790);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(797);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case AngelscriptConsoleParser::LeftParen: {
            setState(791);
            parametersAndQualifiers();
            break;
          }

          case AngelscriptConsoleParser::LeftBracket: {
            setState(792);
            match(AngelscriptConsoleParser::LeftBracket);
            setState(794);
            _errHandler->sync(this);

            _la = _input->LA(1);
            if ((((_la & ~ 0x3fULL) == 0) &&
              ((1ULL << _la) & 68052495160573950) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
              ((1ULL << (_la - 75)) & 291) != 0)) {
              setState(793);
              constantExpression();
            }
            setState(796);
            match(AngelscriptConsoleParser::RightBracket);
            break;
          }

        default:
          throw NoViableAltException(this);
        } 
      }
      setState(803);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 84, _ctx);
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
  enterRule(_localctx, 158, AngelscriptConsoleParser::RuleParametersAndQualifiers);
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
    setState(804);
    match(AngelscriptConsoleParser::LeftParen);
    setState(806);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1374425186176) != 0) || _la == AngelscriptConsoleParser::Doublecolon

    || _la == AngelscriptConsoleParser::Identifier) {
      setState(805);
      parameterDeclarationClause();
    }
    setState(808);
    match(AngelscriptConsoleParser::RightParen);
    setState(810);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 86, _ctx)) {
    case 1: {
      setState(809);
      match(AngelscriptConsoleParser::Const);
      break;
    }

    default:
      break;
    }
    setState(813);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 87, _ctx)) {
    case 1: {
      setState(812);
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
  enterRule(_localctx, 160, AngelscriptConsoleParser::RuleRefqualifier);
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
  enterRule(_localctx, 162, AngelscriptConsoleParser::RuleDeclaratorid);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(817);
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
  enterRule(_localctx, 164, AngelscriptConsoleParser::RuleTheTypeId);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(819);
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
  enterRule(_localctx, 166, AngelscriptConsoleParser::RuleParameterDeclarationClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(821);
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
  enterRule(_localctx, 168, AngelscriptConsoleParser::RuleParameterDeclarationList);
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
    setState(823);
    parameterDeclaration();
    setState(828);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptConsoleParser::Comma) {
      setState(824);
      match(AngelscriptConsoleParser::Comma);
      setState(825);
      parameterDeclaration();
      setState(830);
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
  enterRule(_localctx, 170, AngelscriptConsoleParser::RuleParameterDeclaration);
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
    setState(831);
    declSpecifierSeq();
    setState(833);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Identifier) {
      setState(832);
      match(AngelscriptConsoleParser::Identifier);
    }
    setState(837);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == AngelscriptConsoleParser::Assign) {
      setState(835);
      match(AngelscriptConsoleParser::Assign);
      setState(836);
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
  enterRule(_localctx, 172, AngelscriptConsoleParser::RuleInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(844);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace:
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(839);
        braceOrEqualInitializer();
        break;
      }

      case AngelscriptConsoleParser::LeftParen: {
        enterOuterAlt(_localctx, 2);
        setState(840);
        match(AngelscriptConsoleParser::LeftParen);
        setState(841);
        expressionList();
        setState(842);
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
  enterRule(_localctx, 174, AngelscriptConsoleParser::RuleBraceOrEqualInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(849);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::Assign: {
        enterOuterAlt(_localctx, 1);
        setState(846);
        match(AngelscriptConsoleParser::Assign);
        setState(847);
        initializerClause();
        break;
      }

      case AngelscriptConsoleParser::LeftBracket:
      case AngelscriptConsoleParser::LeftBrace: {
        enterOuterAlt(_localctx, 2);
        setState(848);
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
  enterRule(_localctx, 176, AngelscriptConsoleParser::RuleInitializerClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(853);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 93, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(851);
      assignmentExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(852);
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
  enterRule(_localctx, 178, AngelscriptConsoleParser::RuleInitializerList);

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
    setState(855);
    initializerClause();
    setState(860);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 94, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(856);
        match(AngelscriptConsoleParser::Comma);
        setState(857);
        initializerClause(); 
      }
      setState(862);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 94, _ctx);
    }
    setState(864);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 95, _ctx)) {
    case 1: {
      setState(863);
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
  enterRule(_localctx, 180, AngelscriptConsoleParser::RuleBracedInitList);
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
    setState(866);
    _la = _input->LA(1);
    if (!(_la == AngelscriptConsoleParser::LeftBracket

    || _la == AngelscriptConsoleParser::LeftBrace)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(871);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 68070087346618366) != 0) || ((((_la - 75) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 75)) & 291) != 0)) {
      setState(867);
      initializerList();
      setState(869);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == AngelscriptConsoleParser::Comma) {
        setState(868);
        match(AngelscriptConsoleParser::Comma);
      }
    }
    setState(873);
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
  enterRule(_localctx, 182, AngelscriptConsoleParser::RuleTheOperator);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(915);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 98, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);

      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(876);
      match(AngelscriptConsoleParser::Plus);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(877);
      match(AngelscriptConsoleParser::Minus);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(878);
      match(AngelscriptConsoleParser::Star);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(879);
      match(AngelscriptConsoleParser::Div);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(880);
      match(AngelscriptConsoleParser::Mod);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(881);
      match(AngelscriptConsoleParser::Xor);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(882);
      match(AngelscriptConsoleParser::And);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(883);
      match(AngelscriptConsoleParser::Or);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(884);
      match(AngelscriptConsoleParser::Tilde);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(885);
      match(AngelscriptConsoleParser::Not);
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(886);
      match(AngelscriptConsoleParser::Assign);
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(887);
      match(AngelscriptConsoleParser::Greater);
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(888);
      match(AngelscriptConsoleParser::Less);
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(889);
      match(AngelscriptConsoleParser::GreaterEqual);
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(890);
      match(AngelscriptConsoleParser::PlusAssign);
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(891);
      match(AngelscriptConsoleParser::MinusAssign);
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(892);
      match(AngelscriptConsoleParser::StarAssign);
      break;
    }

    case 19: {
      enterOuterAlt(_localctx, 19);
      setState(893);
      match(AngelscriptConsoleParser::ModAssign);
      break;
    }

    case 20: {
      enterOuterAlt(_localctx, 20);
      setState(894);
      match(AngelscriptConsoleParser::XorAssign);
      break;
    }

    case 21: {
      enterOuterAlt(_localctx, 21);
      setState(895);
      match(AngelscriptConsoleParser::AndAssign);
      break;
    }

    case 22: {
      enterOuterAlt(_localctx, 22);
      setState(896);
      match(AngelscriptConsoleParser::OrAssign);
      break;
    }

    case 23: {
      enterOuterAlt(_localctx, 23);
      setState(897);
      match(AngelscriptConsoleParser::Less);
      setState(898);
      match(AngelscriptConsoleParser::Less);
      break;
    }

    case 24: {
      enterOuterAlt(_localctx, 24);
      setState(899);
      match(AngelscriptConsoleParser::Greater);
      setState(900);
      match(AngelscriptConsoleParser::Greater);
      break;
    }

    case 25: {
      enterOuterAlt(_localctx, 25);
      setState(901);
      match(AngelscriptConsoleParser::RightShiftAssign);
      break;
    }

    case 26: {
      enterOuterAlt(_localctx, 26);
      setState(902);
      match(AngelscriptConsoleParser::LeftShiftAssign);
      break;
    }

    case 27: {
      enterOuterAlt(_localctx, 27);
      setState(903);
      match(AngelscriptConsoleParser::Equal);
      break;
    }

    case 28: {
      enterOuterAlt(_localctx, 28);
      setState(904);
      match(AngelscriptConsoleParser::NotEqual);
      break;
    }

    case 29: {
      enterOuterAlt(_localctx, 29);
      setState(905);
      match(AngelscriptConsoleParser::LessEqual);
      break;
    }

    case 30: {
      enterOuterAlt(_localctx, 30);
      setState(906);
      match(AngelscriptConsoleParser::AndAnd);
      break;
    }

    case 31: {
      enterOuterAlt(_localctx, 31);
      setState(907);
      match(AngelscriptConsoleParser::OrOr);
      break;
    }

    case 32: {
      enterOuterAlt(_localctx, 32);
      setState(908);
      match(AngelscriptConsoleParser::PlusPlus);
      break;
    }

    case 33: {
      enterOuterAlt(_localctx, 33);
      setState(909);
      match(AngelscriptConsoleParser::MinusMinus);
      break;
    }

    case 34: {
      enterOuterAlt(_localctx, 34);
      setState(910);
      match(AngelscriptConsoleParser::Comma);
      break;
    }

    case 35: {
      enterOuterAlt(_localctx, 35);
      setState(911);
      match(AngelscriptConsoleParser::LeftParen);
      setState(912);
      match(AngelscriptConsoleParser::RightParen);
      break;
    }

    case 36: {
      enterOuterAlt(_localctx, 36);
      setState(913);
      match(AngelscriptConsoleParser::LeftBracket);
      setState(914);
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
  enterRule(_localctx, 184, AngelscriptConsoleParser::RuleLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(924);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptConsoleParser::IntegerLiteral: {
        enterOuterAlt(_localctx, 1);
        setState(917);
        match(AngelscriptConsoleParser::IntegerLiteral);
        break;
      }

      case AngelscriptConsoleParser::CharacterLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(918);
        match(AngelscriptConsoleParser::CharacterLiteral);
        break;
      }

      case AngelscriptConsoleParser::FloatingLiteral: {
        enterOuterAlt(_localctx, 3);
        setState(919);
        match(AngelscriptConsoleParser::FloatingLiteral);
        break;
      }

      case AngelscriptConsoleParser::StringLiteral: {
        enterOuterAlt(_localctx, 4);
        setState(920);
        match(AngelscriptConsoleParser::StringLiteral);
        break;
      }

      case AngelscriptConsoleParser::False_:
      case AngelscriptConsoleParser::True_: {
        enterOuterAlt(_localctx, 5);
        setState(921);
        booleanLiteral();
        break;
      }

      case AngelscriptConsoleParser::UserDefinedLiteral: {
        enterOuterAlt(_localctx, 6);
        setState(922);
        match(AngelscriptConsoleParser::UserDefinedLiteral);
        break;
      }

      case AngelscriptConsoleParser::Nullptr: {
        enterOuterAlt(_localctx, 7);
        setState(923);
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
    case 78: return declaratorDefSempred(antlrcpp::downCast<DeclaratorDefContext *>(context), predicateIndex);

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
