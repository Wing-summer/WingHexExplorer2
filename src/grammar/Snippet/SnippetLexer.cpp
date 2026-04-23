
// Generated from Snippet.g4 by ANTLR 4.13.2


#include "SnippetLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct SnippetLexerStaticData final {
  SnippetLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SnippetLexerStaticData(const SnippetLexerStaticData&) = delete;
  SnippetLexerStaticData(SnippetLexerStaticData&&) = delete;
  SnippetLexerStaticData& operator=(const SnippetLexerStaticData&) = delete;
  SnippetLexerStaticData& operator=(SnippetLexerStaticData&&) = delete;

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

::antlr4::internal::OnceFlag snippetlexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<SnippetLexerStaticData> snippetlexerLexerStaticData = nullptr;

void snippetlexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (snippetlexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(snippetlexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<SnippetLexerStaticData>(
    std::vector<std::string>{
      "TEXT_CONTENT", "ESCAPED_CHAR", "VARIABLE", "VARIABLE_BRACED", "VARIABLE_WITH_DEFAULT", 
      "PLACEHOLDER", "TABSTOP", "TABSTOP_BRACED", "TABSTOP_WITH_DEFAULT", 
      "CHOICE", "CHOICE_OPTIONS", "IDENTIFIER", "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "'$0'"
    },
    std::vector<std::string>{
      "", "TEXT_CONTENT", "ESCAPED_CHAR", "VARIABLE", "VARIABLE_BRACED", 
      "VARIABLE_WITH_DEFAULT", "PLACEHOLDER", "TABSTOP", "TABSTOP_BRACED", 
      "TABSTOP_WITH_DEFAULT", "CHOICE", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,11,141,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,1,0,4,0,29,8,
  	0,11,0,12,0,30,1,1,1,1,1,1,1,2,1,2,1,2,1,3,1,3,1,3,1,3,1,3,1,3,1,4,1,
  	4,1,4,1,4,1,4,1,4,5,4,51,8,4,10,4,12,4,54,9,4,1,4,1,4,1,5,1,5,1,5,1,6,
  	1,6,1,6,5,6,64,8,6,10,6,12,6,67,9,6,1,7,1,7,1,7,1,7,1,7,5,7,74,8,7,10,
  	7,12,7,77,9,7,1,7,1,7,1,8,1,8,1,8,1,8,1,8,5,8,86,8,8,10,8,12,8,89,9,8,
  	1,8,1,8,5,8,93,8,8,10,8,12,8,96,9,8,1,8,1,8,1,9,1,9,1,9,1,9,1,9,5,9,105,
  	8,9,10,9,12,9,108,9,9,1,9,1,9,1,9,1,9,1,10,4,10,115,8,10,11,10,12,10,
  	116,1,10,1,10,4,10,121,8,10,11,10,12,10,122,5,10,125,8,10,10,10,12,10,
  	128,9,10,1,11,1,11,5,11,132,8,11,10,11,12,11,135,9,11,1,12,4,12,138,8,
  	12,11,12,12,12,139,0,0,13,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,
  	10,21,0,23,0,25,11,1,0,9,4,0,10,10,13,13,36,36,92,92,4,0,36,36,91,93,
  	123,123,125,125,1,0,125,125,1,0,49,57,1,0,48,57,3,0,10,10,13,13,124,125,
  	3,0,65,90,95,95,97,122,4,0,48,57,65,90,95,95,97,122,3,0,9,10,13,13,32,
  	32,150,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,
  	11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,25,1,
  	0,0,0,1,28,1,0,0,0,3,32,1,0,0,0,5,35,1,0,0,0,7,38,1,0,0,0,9,44,1,0,0,
  	0,11,57,1,0,0,0,13,60,1,0,0,0,15,68,1,0,0,0,17,80,1,0,0,0,19,99,1,0,0,
  	0,21,114,1,0,0,0,23,129,1,0,0,0,25,137,1,0,0,0,27,29,8,0,0,0,28,27,1,
  	0,0,0,29,30,1,0,0,0,30,28,1,0,0,0,30,31,1,0,0,0,31,2,1,0,0,0,32,33,5,
  	92,0,0,33,34,7,1,0,0,34,4,1,0,0,0,35,36,5,36,0,0,36,37,3,23,11,0,37,6,
  	1,0,0,0,38,39,5,36,0,0,39,40,5,123,0,0,40,41,1,0,0,0,41,42,3,23,11,0,
  	42,43,5,125,0,0,43,8,1,0,0,0,44,45,5,36,0,0,45,46,5,123,0,0,46,47,1,0,
  	0,0,47,48,3,23,11,0,48,52,5,58,0,0,49,51,8,2,0,0,50,49,1,0,0,0,51,54,
  	1,0,0,0,52,50,1,0,0,0,52,53,1,0,0,0,53,55,1,0,0,0,54,52,1,0,0,0,55,56,
  	5,125,0,0,56,10,1,0,0,0,57,58,5,36,0,0,58,59,5,48,0,0,59,12,1,0,0,0,60,
  	61,5,36,0,0,61,65,7,3,0,0,62,64,7,4,0,0,63,62,1,0,0,0,64,67,1,0,0,0,65,
  	63,1,0,0,0,65,66,1,0,0,0,66,14,1,0,0,0,67,65,1,0,0,0,68,69,5,36,0,0,69,
  	70,5,123,0,0,70,71,1,0,0,0,71,75,7,3,0,0,72,74,7,4,0,0,73,72,1,0,0,0,
  	74,77,1,0,0,0,75,73,1,0,0,0,75,76,1,0,0,0,76,78,1,0,0,0,77,75,1,0,0,0,
  	78,79,5,125,0,0,79,16,1,0,0,0,80,81,5,36,0,0,81,82,5,123,0,0,82,83,1,
  	0,0,0,83,87,7,3,0,0,84,86,7,4,0,0,85,84,1,0,0,0,86,89,1,0,0,0,87,85,1,
  	0,0,0,87,88,1,0,0,0,88,90,1,0,0,0,89,87,1,0,0,0,90,94,5,58,0,0,91,93,
  	8,2,0,0,92,91,1,0,0,0,93,96,1,0,0,0,94,92,1,0,0,0,94,95,1,0,0,0,95,97,
  	1,0,0,0,96,94,1,0,0,0,97,98,5,125,0,0,98,18,1,0,0,0,99,100,5,36,0,0,100,
  	101,5,123,0,0,101,102,1,0,0,0,102,106,7,3,0,0,103,105,7,4,0,0,104,103,
  	1,0,0,0,105,108,1,0,0,0,106,104,1,0,0,0,106,107,1,0,0,0,107,109,1,0,0,
  	0,108,106,1,0,0,0,109,110,5,124,0,0,110,111,3,21,10,0,111,112,5,125,0,
  	0,112,20,1,0,0,0,113,115,8,5,0,0,114,113,1,0,0,0,115,116,1,0,0,0,116,
  	114,1,0,0,0,116,117,1,0,0,0,117,126,1,0,0,0,118,120,5,44,0,0,119,121,
  	8,5,0,0,120,119,1,0,0,0,121,122,1,0,0,0,122,120,1,0,0,0,122,123,1,0,0,
  	0,123,125,1,0,0,0,124,118,1,0,0,0,125,128,1,0,0,0,126,124,1,0,0,0,126,
  	127,1,0,0,0,127,22,1,0,0,0,128,126,1,0,0,0,129,133,7,6,0,0,130,132,7,
  	7,0,0,131,130,1,0,0,0,132,135,1,0,0,0,133,131,1,0,0,0,133,134,1,0,0,0,
  	134,24,1,0,0,0,135,133,1,0,0,0,136,138,7,8,0,0,137,136,1,0,0,0,138,139,
  	1,0,0,0,139,137,1,0,0,0,139,140,1,0,0,0,140,26,1,0,0,0,13,0,30,52,65,
  	75,87,94,106,116,122,126,133,139,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  snippetlexerLexerStaticData = std::move(staticData);
}

}

SnippetLexer::SnippetLexer(CharStream *input) : Lexer(input) {
  SnippetLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *snippetlexerLexerStaticData->atn, snippetlexerLexerStaticData->decisionToDFA, snippetlexerLexerStaticData->sharedContextCache);
}

SnippetLexer::~SnippetLexer() {
  delete _interpreter;
}

std::string SnippetLexer::getGrammarFileName() const {
  return "Snippet.g4";
}

const std::vector<std::string>& SnippetLexer::getRuleNames() const {
  return snippetlexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& SnippetLexer::getChannelNames() const {
  return snippetlexerLexerStaticData->channelNames;
}

const std::vector<std::string>& SnippetLexer::getModeNames() const {
  return snippetlexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& SnippetLexer::getVocabulary() const {
  return snippetlexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SnippetLexer::getSerializedATN() const {
  return snippetlexerLexerStaticData->serializedATN;
}

const atn::ATN& SnippetLexer::getATN() const {
  return *snippetlexerLexerStaticData->atn;
}




void SnippetLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  snippetlexerLexerInitialize();
#else
  ::antlr4::internal::call_once(snippetlexerLexerOnceFlag, snippetlexerLexerInitialize);
#endif
}
