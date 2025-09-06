
// Generated from NumCal.g4 by ANTLR 4.13.2


#include "NumCalLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct NumCalLexerStaticData final {
  NumCalLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  NumCalLexerStaticData(const NumCalLexerStaticData&) = delete;
  NumCalLexerStaticData(NumCalLexerStaticData&&) = delete;
  NumCalLexerStaticData& operator=(const NumCalLexerStaticData&) = delete;
  NumCalLexerStaticData& operator=(NumCalLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag numcallexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<NumCalLexerStaticData> numcallexerLexerStaticData = nullptr;

void numcallexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (numcallexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(numcallexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<NumCalLexerStaticData>(
    std::vector<std::string>{
      "LeftParen", "RightParen", "LeftShift", "RightShift", "Plus", "Minus", 
      "Star", "Div", "Mod", "And", "Or", "Caret", "Tilde", "LessThan", "Colon", 
      "LeftBracket", "RightBracket", "Digit", "HexQuad", "IntegerConstant", 
      "BinaryConstant", "DecimalConstant", "OctalConstant", "HexadecimalConstant", 
      "HexadecimalPrefix", "NonzeroDigit", "OctalDigit", "HexadecimalDigit", 
      "Sign", "DigitSequence", "HexadecimalDigitSequence", "Whitespace"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'('", "')'", "'<<'", "'>>'", "'+'", "'-'", "'*'", "'/'", "'%'", 
      "'&'", "'|'", "'^'", "'~'", "'<'", "':'", "'['", "']'"
    },
    std::vector<std::string>{
      "", "LeftParen", "RightParen", "LeftShift", "RightShift", "Plus", 
      "Minus", "Star", "Div", "Mod", "And", "Or", "Caret", "Tilde", "LessThan", 
      "Colon", "LeftBracket", "RightBracket", "IntegerConstant", "Whitespace"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,19,169,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
  	7,28,2,29,7,29,2,30,7,30,2,31,7,31,1,0,1,0,1,1,1,1,1,2,1,2,1,2,1,3,1,
  	3,1,3,1,4,1,4,1,5,1,5,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,11,
  	1,11,1,12,1,12,1,13,1,13,1,14,1,14,1,15,1,15,1,16,1,16,1,17,1,17,1,18,
  	1,18,1,18,1,18,1,18,1,19,1,19,1,19,1,19,3,19,113,8,19,1,20,1,20,1,20,
  	4,20,118,8,20,11,20,12,20,119,1,21,1,21,5,21,124,8,21,10,21,12,21,127,
  	9,21,1,22,1,22,5,22,131,8,22,10,22,12,22,134,9,22,1,23,1,23,4,23,138,
  	8,23,11,23,12,23,139,1,24,1,24,1,24,1,25,1,25,1,26,1,26,1,27,1,27,1,28,
  	1,28,1,29,4,29,154,8,29,11,29,12,29,155,1,30,4,30,159,8,30,11,30,12,30,
  	160,1,31,4,31,164,8,31,11,31,12,31,165,1,31,1,31,0,0,32,1,1,3,2,5,3,7,
  	4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,27,14,29,15,31,16,33,
  	17,35,0,37,0,39,18,41,0,43,0,45,0,47,0,49,0,51,0,53,0,55,0,57,0,59,0,
  	61,0,63,19,1,0,9,1,0,48,57,2,0,66,66,98,98,1,0,48,49,2,0,88,88,120,120,
  	1,0,49,57,1,0,48,55,3,0,48,57,65,70,97,102,2,0,43,43,45,45,2,0,9,9,32,
  	32,165,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,
  	11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,
  	0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,
  	0,0,33,1,0,0,0,0,39,1,0,0,0,0,63,1,0,0,0,1,65,1,0,0,0,3,67,1,0,0,0,5,
  	69,1,0,0,0,7,72,1,0,0,0,9,75,1,0,0,0,11,77,1,0,0,0,13,79,1,0,0,0,15,81,
  	1,0,0,0,17,83,1,0,0,0,19,85,1,0,0,0,21,87,1,0,0,0,23,89,1,0,0,0,25,91,
  	1,0,0,0,27,93,1,0,0,0,29,95,1,0,0,0,31,97,1,0,0,0,33,99,1,0,0,0,35,101,
  	1,0,0,0,37,103,1,0,0,0,39,112,1,0,0,0,41,114,1,0,0,0,43,121,1,0,0,0,45,
  	128,1,0,0,0,47,135,1,0,0,0,49,141,1,0,0,0,51,144,1,0,0,0,53,146,1,0,0,
  	0,55,148,1,0,0,0,57,150,1,0,0,0,59,153,1,0,0,0,61,158,1,0,0,0,63,163,
  	1,0,0,0,65,66,5,40,0,0,66,2,1,0,0,0,67,68,5,41,0,0,68,4,1,0,0,0,69,70,
  	5,60,0,0,70,71,5,60,0,0,71,6,1,0,0,0,72,73,5,62,0,0,73,74,5,62,0,0,74,
  	8,1,0,0,0,75,76,5,43,0,0,76,10,1,0,0,0,77,78,5,45,0,0,78,12,1,0,0,0,79,
  	80,5,42,0,0,80,14,1,0,0,0,81,82,5,47,0,0,82,16,1,0,0,0,83,84,5,37,0,0,
  	84,18,1,0,0,0,85,86,5,38,0,0,86,20,1,0,0,0,87,88,5,124,0,0,88,22,1,0,
  	0,0,89,90,5,94,0,0,90,24,1,0,0,0,91,92,5,126,0,0,92,26,1,0,0,0,93,94,
  	5,60,0,0,94,28,1,0,0,0,95,96,5,58,0,0,96,30,1,0,0,0,97,98,5,91,0,0,98,
  	32,1,0,0,0,99,100,5,93,0,0,100,34,1,0,0,0,101,102,7,0,0,0,102,36,1,0,
  	0,0,103,104,3,55,27,0,104,105,3,55,27,0,105,106,3,55,27,0,106,107,3,55,
  	27,0,107,38,1,0,0,0,108,113,3,43,21,0,109,113,3,45,22,0,110,113,3,47,
  	23,0,111,113,3,41,20,0,112,108,1,0,0,0,112,109,1,0,0,0,112,110,1,0,0,
  	0,112,111,1,0,0,0,113,40,1,0,0,0,114,115,5,48,0,0,115,117,7,1,0,0,116,
  	118,7,2,0,0,117,116,1,0,0,0,118,119,1,0,0,0,119,117,1,0,0,0,119,120,1,
  	0,0,0,120,42,1,0,0,0,121,125,3,51,25,0,122,124,3,35,17,0,123,122,1,0,
  	0,0,124,127,1,0,0,0,125,123,1,0,0,0,125,126,1,0,0,0,126,44,1,0,0,0,127,
  	125,1,0,0,0,128,132,5,48,0,0,129,131,3,53,26,0,130,129,1,0,0,0,131,134,
  	1,0,0,0,132,130,1,0,0,0,132,133,1,0,0,0,133,46,1,0,0,0,134,132,1,0,0,
  	0,135,137,3,49,24,0,136,138,3,55,27,0,137,136,1,0,0,0,138,139,1,0,0,0,
  	139,137,1,0,0,0,139,140,1,0,0,0,140,48,1,0,0,0,141,142,5,48,0,0,142,143,
  	7,3,0,0,143,50,1,0,0,0,144,145,7,4,0,0,145,52,1,0,0,0,146,147,7,5,0,0,
  	147,54,1,0,0,0,148,149,7,6,0,0,149,56,1,0,0,0,150,151,7,7,0,0,151,58,
  	1,0,0,0,152,154,3,35,17,0,153,152,1,0,0,0,154,155,1,0,0,0,155,153,1,0,
  	0,0,155,156,1,0,0,0,156,60,1,0,0,0,157,159,3,55,27,0,158,157,1,0,0,0,
  	159,160,1,0,0,0,160,158,1,0,0,0,160,161,1,0,0,0,161,62,1,0,0,0,162,164,
  	7,8,0,0,163,162,1,0,0,0,164,165,1,0,0,0,165,163,1,0,0,0,165,166,1,0,0,
  	0,166,167,1,0,0,0,167,168,6,31,0,0,168,64,1,0,0,0,9,0,112,119,125,132,
  	139,155,160,165,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  numcallexerLexerStaticData = std::move(staticData);
}

}

NumCalLexer::NumCalLexer(CharStream *input) : Lexer(input) {
  NumCalLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *numcallexerLexerStaticData->atn, numcallexerLexerStaticData->decisionToDFA, numcallexerLexerStaticData->sharedContextCache);
}

NumCalLexer::~NumCalLexer() {
  delete _interpreter;
}

std::string NumCalLexer::getGrammarFileName() const {
  return "NumCal.g4";
}

const std::vector<std::string>& NumCalLexer::getRuleNames() const {
  return numcallexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& NumCalLexer::getChannelNames() const {
  return numcallexerLexerStaticData->channelNames;
}

const std::vector<std::string>& NumCalLexer::getModeNames() const {
  return numcallexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& NumCalLexer::getVocabulary() const {
  return numcallexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView NumCalLexer::getSerializedATN() const {
  return numcallexerLexerStaticData->serializedATN;
}

const atn::ATN& NumCalLexer::getATN() const {
  return *numcallexerLexerStaticData->atn;
}




void NumCalLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  numcallexerLexerInitialize();
#else
  ::antlr4::internal::call_once(numcallexerLexerOnceFlag, numcallexerLexerInitialize);
#endif
}
