
// Generated from DefEvaluator.g4 by ANTLR 4.13.2


#include "DefEvaluatorLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct DefEvaluatorLexerStaticData final {
  DefEvaluatorLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  DefEvaluatorLexerStaticData(const DefEvaluatorLexerStaticData&) = delete;
  DefEvaluatorLexerStaticData(DefEvaluatorLexerStaticData&&) = delete;
  DefEvaluatorLexerStaticData& operator=(const DefEvaluatorLexerStaticData&) = delete;
  DefEvaluatorLexerStaticData& operator=(DefEvaluatorLexerStaticData&&) = delete;

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

::antlr4::internal::OnceFlag defevaluatorlexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<DefEvaluatorLexerStaticData> defevaluatorlexerLexerStaticData = nullptr;

void defevaluatorlexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (defevaluatorlexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(defevaluatorlexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<DefEvaluatorLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
      "LeftParen", "RightParen", "LeftShift", "RightShift", "Plus", "Minus", 
      "Star", "Div", "Mod", "And", "Or", "Caret", "Not", "Tilde", "LessThan", 
      "Digit", "HexQuad", "Constant", "IntegerConstant", "BinaryConstant", 
      "DecimalConstant", "OctalConstant", "HexadecimalConstant", "HexadecimalPrefix", 
      "NonzeroDigit", "OctalDigit", "FloatingSuffix", "HexadecimalFractionalConstant", 
      "HexadecimalDigit", "FloatingConstant", "DecimalFloatingConstant", 
      "FractionalConstant", "ExponentPart", "Sign", "DigitSequence", "HexadecimalDigitSequence", 
      "Whitespace"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'>'", "'<='", "'>='", "'=='", "'!='", "'&&'", "'||'", "'\\u003F'", 
      "':'", "'('", "')'", "'<<'", "'>>'", "'+'", "'-'", "'*'", "'/'", "'%'", 
      "'&'", "'|'", "'^'", "'!'", "'~'", "'<'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "LeftParen", "RightParen", 
      "LeftShift", "RightShift", "Plus", "Minus", "Star", "Div", "Mod", 
      "And", "Or", "Caret", "Not", "Tilde", "LessThan", "Constant", "IntegerConstant", 
      "Whitespace"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,27,265,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
  	7,28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,
  	7,35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,
  	7,42,2,43,7,43,2,44,7,44,2,45,7,45,1,0,1,0,1,1,1,1,1,1,1,2,1,2,1,2,1,
  	3,1,3,1,3,1,4,1,4,1,4,1,5,1,5,1,5,1,6,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,
  	1,10,1,10,1,11,1,11,1,11,1,12,1,12,1,12,1,13,1,13,1,14,1,14,1,15,1,15,
  	1,16,1,16,1,17,1,17,1,18,1,18,1,19,1,19,1,20,1,20,1,21,1,21,1,22,1,22,
  	1,23,1,23,1,24,1,24,1,25,1,25,1,25,1,25,1,25,1,26,1,26,3,26,159,8,26,
  	1,27,1,27,1,27,1,27,3,27,165,8,27,1,28,1,28,1,28,4,28,170,8,28,11,28,
  	12,28,171,1,29,1,29,5,29,176,8,29,10,29,12,29,179,9,29,1,30,1,30,5,30,
  	183,8,30,10,30,12,30,186,9,30,1,31,1,31,4,31,190,8,31,11,31,12,31,191,
  	1,32,1,32,1,32,1,33,1,33,1,34,1,34,1,35,1,35,1,36,3,36,204,8,36,1,36,
  	1,36,1,36,1,36,1,36,3,36,211,8,36,1,37,1,37,1,38,1,38,1,39,1,39,3,39,
  	219,8,39,1,39,3,39,222,8,39,1,39,1,39,1,39,3,39,227,8,39,3,39,229,8,39,
  	1,40,3,40,232,8,40,1,40,1,40,1,40,1,40,1,40,3,40,239,8,40,1,41,1,41,3,
  	41,243,8,41,1,41,1,41,1,42,1,42,1,43,4,43,250,8,43,11,43,12,43,251,1,
  	44,4,44,255,8,44,11,44,12,44,256,1,45,4,45,260,8,45,11,45,12,45,261,1,
  	45,1,45,0,0,46,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,
  	12,25,13,27,14,29,15,31,16,33,17,35,18,37,19,39,20,41,21,43,22,45,23,
  	47,24,49,0,51,0,53,25,55,26,57,0,59,0,61,0,63,0,65,0,67,0,69,0,71,0,73,
  	0,75,0,77,0,79,0,81,0,83,0,85,0,87,0,89,0,91,27,1,0,11,1,0,48,57,2,0,
  	66,66,98,98,1,0,48,49,2,0,88,88,120,120,1,0,49,57,1,0,48,55,4,0,70,70,
  	76,76,102,102,108,108,3,0,48,57,65,70,97,102,2,0,69,69,101,101,2,0,43,
  	43,45,45,2,0,9,9,32,32,265,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,
  	0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,
  	0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,29,
  	1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,0,39,1,0,
  	0,0,0,41,1,0,0,0,0,43,1,0,0,0,0,45,1,0,0,0,0,47,1,0,0,0,0,53,1,0,0,0,
  	0,55,1,0,0,0,0,91,1,0,0,0,1,93,1,0,0,0,3,95,1,0,0,0,5,98,1,0,0,0,7,101,
  	1,0,0,0,9,104,1,0,0,0,11,107,1,0,0,0,13,110,1,0,0,0,15,113,1,0,0,0,17,
  	115,1,0,0,0,19,117,1,0,0,0,21,119,1,0,0,0,23,121,1,0,0,0,25,124,1,0,0,
  	0,27,127,1,0,0,0,29,129,1,0,0,0,31,131,1,0,0,0,33,133,1,0,0,0,35,135,
  	1,0,0,0,37,137,1,0,0,0,39,139,1,0,0,0,41,141,1,0,0,0,43,143,1,0,0,0,45,
  	145,1,0,0,0,47,147,1,0,0,0,49,149,1,0,0,0,51,151,1,0,0,0,53,158,1,0,0,
  	0,55,164,1,0,0,0,57,166,1,0,0,0,59,173,1,0,0,0,61,180,1,0,0,0,63,187,
  	1,0,0,0,65,193,1,0,0,0,67,196,1,0,0,0,69,198,1,0,0,0,71,200,1,0,0,0,73,
  	210,1,0,0,0,75,212,1,0,0,0,77,214,1,0,0,0,79,228,1,0,0,0,81,238,1,0,0,
  	0,83,240,1,0,0,0,85,246,1,0,0,0,87,249,1,0,0,0,89,254,1,0,0,0,91,259,
  	1,0,0,0,93,94,5,62,0,0,94,2,1,0,0,0,95,96,5,60,0,0,96,97,5,61,0,0,97,
  	4,1,0,0,0,98,99,5,62,0,0,99,100,5,61,0,0,100,6,1,0,0,0,101,102,5,61,0,
  	0,102,103,5,61,0,0,103,8,1,0,0,0,104,105,5,33,0,0,105,106,5,61,0,0,106,
  	10,1,0,0,0,107,108,5,38,0,0,108,109,5,38,0,0,109,12,1,0,0,0,110,111,5,
  	124,0,0,111,112,5,124,0,0,112,14,1,0,0,0,113,114,5,63,0,0,114,16,1,0,
  	0,0,115,116,5,58,0,0,116,18,1,0,0,0,117,118,5,40,0,0,118,20,1,0,0,0,119,
  	120,5,41,0,0,120,22,1,0,0,0,121,122,5,60,0,0,122,123,5,60,0,0,123,24,
  	1,0,0,0,124,125,5,62,0,0,125,126,5,62,0,0,126,26,1,0,0,0,127,128,5,43,
  	0,0,128,28,1,0,0,0,129,130,5,45,0,0,130,30,1,0,0,0,131,132,5,42,0,0,132,
  	32,1,0,0,0,133,134,5,47,0,0,134,34,1,0,0,0,135,136,5,37,0,0,136,36,1,
  	0,0,0,137,138,5,38,0,0,138,38,1,0,0,0,139,140,5,124,0,0,140,40,1,0,0,
  	0,141,142,5,94,0,0,142,42,1,0,0,0,143,144,5,33,0,0,144,44,1,0,0,0,145,
  	146,5,126,0,0,146,46,1,0,0,0,147,148,5,60,0,0,148,48,1,0,0,0,149,150,
  	7,0,0,0,150,50,1,0,0,0,151,152,3,75,37,0,152,153,3,75,37,0,153,154,3,
  	75,37,0,154,155,3,75,37,0,155,52,1,0,0,0,156,159,3,55,27,0,157,159,3,
  	77,38,0,158,156,1,0,0,0,158,157,1,0,0,0,159,54,1,0,0,0,160,165,3,59,29,
  	0,161,165,3,61,30,0,162,165,3,63,31,0,163,165,3,57,28,0,164,160,1,0,0,
  	0,164,161,1,0,0,0,164,162,1,0,0,0,164,163,1,0,0,0,165,56,1,0,0,0,166,
  	167,5,48,0,0,167,169,7,1,0,0,168,170,7,2,0,0,169,168,1,0,0,0,170,171,
  	1,0,0,0,171,169,1,0,0,0,171,172,1,0,0,0,172,58,1,0,0,0,173,177,3,67,33,
  	0,174,176,3,49,24,0,175,174,1,0,0,0,176,179,1,0,0,0,177,175,1,0,0,0,177,
  	178,1,0,0,0,178,60,1,0,0,0,179,177,1,0,0,0,180,184,5,48,0,0,181,183,3,
  	69,34,0,182,181,1,0,0,0,183,186,1,0,0,0,184,182,1,0,0,0,184,185,1,0,0,
  	0,185,62,1,0,0,0,186,184,1,0,0,0,187,189,3,65,32,0,188,190,3,75,37,0,
  	189,188,1,0,0,0,190,191,1,0,0,0,191,189,1,0,0,0,191,192,1,0,0,0,192,64,
  	1,0,0,0,193,194,5,48,0,0,194,195,7,3,0,0,195,66,1,0,0,0,196,197,7,4,0,
  	0,197,68,1,0,0,0,198,199,7,5,0,0,199,70,1,0,0,0,200,201,7,6,0,0,201,72,
  	1,0,0,0,202,204,3,89,44,0,203,202,1,0,0,0,203,204,1,0,0,0,204,205,1,0,
  	0,0,205,206,5,46,0,0,206,211,3,89,44,0,207,208,3,89,44,0,208,209,5,46,
  	0,0,209,211,1,0,0,0,210,203,1,0,0,0,210,207,1,0,0,0,211,74,1,0,0,0,212,
  	213,7,7,0,0,213,76,1,0,0,0,214,215,3,79,39,0,215,78,1,0,0,0,216,218,3,
  	81,40,0,217,219,3,83,41,0,218,217,1,0,0,0,218,219,1,0,0,0,219,221,1,0,
  	0,0,220,222,3,71,35,0,221,220,1,0,0,0,221,222,1,0,0,0,222,229,1,0,0,0,
  	223,224,3,87,43,0,224,226,3,83,41,0,225,227,3,71,35,0,226,225,1,0,0,0,
  	226,227,1,0,0,0,227,229,1,0,0,0,228,216,1,0,0,0,228,223,1,0,0,0,229,80,
  	1,0,0,0,230,232,3,87,43,0,231,230,1,0,0,0,231,232,1,0,0,0,232,233,1,0,
  	0,0,233,234,5,46,0,0,234,239,3,87,43,0,235,236,3,87,43,0,236,237,5,46,
  	0,0,237,239,1,0,0,0,238,231,1,0,0,0,238,235,1,0,0,0,239,82,1,0,0,0,240,
  	242,7,8,0,0,241,243,3,85,42,0,242,241,1,0,0,0,242,243,1,0,0,0,243,244,
  	1,0,0,0,244,245,3,87,43,0,245,84,1,0,0,0,246,247,7,9,0,0,247,86,1,0,0,
  	0,248,250,3,49,24,0,249,248,1,0,0,0,250,251,1,0,0,0,251,249,1,0,0,0,251,
  	252,1,0,0,0,252,88,1,0,0,0,253,255,3,75,37,0,254,253,1,0,0,0,255,256,
  	1,0,0,0,256,254,1,0,0,0,256,257,1,0,0,0,257,90,1,0,0,0,258,260,7,10,0,
  	0,259,258,1,0,0,0,260,261,1,0,0,0,261,259,1,0,0,0,261,262,1,0,0,0,262,
  	263,1,0,0,0,263,264,6,45,0,0,264,92,1,0,0,0,19,0,158,164,171,177,184,
  	191,203,210,218,221,226,228,231,238,242,251,256,261,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  defevaluatorlexerLexerStaticData = std::move(staticData);
}

}

DefEvaluatorLexer::DefEvaluatorLexer(CharStream *input) : Lexer(input) {
  DefEvaluatorLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *defevaluatorlexerLexerStaticData->atn, defevaluatorlexerLexerStaticData->decisionToDFA, defevaluatorlexerLexerStaticData->sharedContextCache);
}

DefEvaluatorLexer::~DefEvaluatorLexer() {
  delete _interpreter;
}

std::string DefEvaluatorLexer::getGrammarFileName() const {
  return "DefEvaluator.g4";
}

const std::vector<std::string>& DefEvaluatorLexer::getRuleNames() const {
  return defevaluatorlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& DefEvaluatorLexer::getChannelNames() const {
  return defevaluatorlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& DefEvaluatorLexer::getModeNames() const {
  return defevaluatorlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& DefEvaluatorLexer::getVocabulary() const {
  return defevaluatorlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView DefEvaluatorLexer::getSerializedATN() const {
  return defevaluatorlexerLexerStaticData->serializedATN;
}

const atn::ATN& DefEvaluatorLexer::getATN() const {
  return *defevaluatorlexerLexerStaticData->atn;
}




void DefEvaluatorLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  defevaluatorlexerLexerInitialize();
#else
  ::antlr4::internal::call_once(defevaluatorlexerLexerOnceFlag, defevaluatorlexerLexerInitialize);
#endif
}
