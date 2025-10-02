
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
      "TEXT_CONTENT", "ESCAPED_CHAR", "VARIABLE", "VARIABLE_WITH_DEFAULT", 
      "PLACEHOLDER", "TABSTOP", "TABSTOP_WITH_DEFAULT", "CHOICE", "CHOICE_OPTIONS", 
      "IDENTIFIER", "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "", "", "", "", "'$0'"
    },
    std::vector<std::string>{
      "", "TEXT_CONTENT", "ESCAPED_CHAR", "VARIABLE", "VARIABLE_WITH_DEFAULT", 
      "PLACEHOLDER", "TABSTOP", "TABSTOP_WITH_DEFAULT", "CHOICE", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,9,121,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,1,0,4,0,25,8,0,11,0,12,0,26,1,1,1,
  	1,1,1,1,2,1,2,1,2,1,3,1,3,1,3,1,3,1,3,1,3,5,3,41,8,3,10,3,12,3,44,9,3,
  	1,3,1,3,1,4,1,4,1,4,1,5,1,5,1,5,5,5,54,8,5,10,5,12,5,57,9,5,1,6,1,6,1,
  	6,1,6,1,6,5,6,64,8,6,10,6,12,6,67,9,6,1,6,1,6,5,6,71,8,6,10,6,12,6,74,
  	9,6,1,6,1,6,1,7,1,7,1,7,1,7,1,7,5,7,83,8,7,10,7,12,7,86,9,7,1,7,1,7,1,
  	7,1,7,1,8,4,8,93,8,8,11,8,12,8,94,1,8,1,8,4,8,99,8,8,11,8,12,8,100,5,
  	8,103,8,8,10,8,12,8,106,9,8,1,9,1,9,5,9,110,8,9,10,9,12,9,113,9,9,1,10,
  	4,10,116,8,10,11,10,12,10,117,1,10,1,10,0,0,11,1,1,3,2,5,3,7,4,9,5,11,
  	6,13,7,15,8,17,0,19,0,21,9,1,0,9,4,0,10,10,13,13,36,36,92,92,4,0,36,36,
  	91,93,123,123,125,125,1,0,125,125,1,0,49,57,1,0,48,57,3,0,10,10,13,13,
  	124,125,3,0,65,90,95,95,97,122,4,0,48,57,65,90,95,95,97,122,3,0,9,10,
  	13,13,32,32,129,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,
  	0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,21,1,0,0,0,1,24,1,0,0,
  	0,3,28,1,0,0,0,5,31,1,0,0,0,7,34,1,0,0,0,9,47,1,0,0,0,11,50,1,0,0,0,13,
  	58,1,0,0,0,15,77,1,0,0,0,17,92,1,0,0,0,19,107,1,0,0,0,21,115,1,0,0,0,
  	23,25,8,0,0,0,24,23,1,0,0,0,25,26,1,0,0,0,26,24,1,0,0,0,26,27,1,0,0,0,
  	27,2,1,0,0,0,28,29,5,92,0,0,29,30,7,1,0,0,30,4,1,0,0,0,31,32,5,36,0,0,
  	32,33,3,19,9,0,33,6,1,0,0,0,34,35,5,36,0,0,35,36,5,123,0,0,36,37,1,0,
  	0,0,37,38,3,19,9,0,38,42,5,58,0,0,39,41,8,2,0,0,40,39,1,0,0,0,41,44,1,
  	0,0,0,42,40,1,0,0,0,42,43,1,0,0,0,43,45,1,0,0,0,44,42,1,0,0,0,45,46,5,
  	125,0,0,46,8,1,0,0,0,47,48,5,36,0,0,48,49,5,48,0,0,49,10,1,0,0,0,50,51,
  	5,36,0,0,51,55,7,3,0,0,52,54,7,4,0,0,53,52,1,0,0,0,54,57,1,0,0,0,55,53,
  	1,0,0,0,55,56,1,0,0,0,56,12,1,0,0,0,57,55,1,0,0,0,58,59,5,36,0,0,59,60,
  	5,123,0,0,60,61,1,0,0,0,61,65,7,3,0,0,62,64,7,4,0,0,63,62,1,0,0,0,64,
  	67,1,0,0,0,65,63,1,0,0,0,65,66,1,0,0,0,66,68,1,0,0,0,67,65,1,0,0,0,68,
  	72,5,58,0,0,69,71,8,2,0,0,70,69,1,0,0,0,71,74,1,0,0,0,72,70,1,0,0,0,72,
  	73,1,0,0,0,73,75,1,0,0,0,74,72,1,0,0,0,75,76,5,125,0,0,76,14,1,0,0,0,
  	77,78,5,36,0,0,78,79,5,123,0,0,79,80,1,0,0,0,80,84,7,3,0,0,81,83,7,4,
  	0,0,82,81,1,0,0,0,83,86,1,0,0,0,84,82,1,0,0,0,84,85,1,0,0,0,85,87,1,0,
  	0,0,86,84,1,0,0,0,87,88,5,124,0,0,88,89,3,17,8,0,89,90,5,125,0,0,90,16,
  	1,0,0,0,91,93,8,5,0,0,92,91,1,0,0,0,93,94,1,0,0,0,94,92,1,0,0,0,94,95,
  	1,0,0,0,95,104,1,0,0,0,96,98,5,44,0,0,97,99,8,5,0,0,98,97,1,0,0,0,99,
  	100,1,0,0,0,100,98,1,0,0,0,100,101,1,0,0,0,101,103,1,0,0,0,102,96,1,0,
  	0,0,103,106,1,0,0,0,104,102,1,0,0,0,104,105,1,0,0,0,105,18,1,0,0,0,106,
  	104,1,0,0,0,107,111,7,6,0,0,108,110,7,7,0,0,109,108,1,0,0,0,110,113,1,
  	0,0,0,111,109,1,0,0,0,111,112,1,0,0,0,112,20,1,0,0,0,113,111,1,0,0,0,
  	114,116,7,8,0,0,115,114,1,0,0,0,116,117,1,0,0,0,117,115,1,0,0,0,117,118,
  	1,0,0,0,118,119,1,0,0,0,119,120,6,10,0,0,120,22,1,0,0,0,12,0,26,42,55,
  	65,72,84,94,100,104,111,117,1,6,0,0
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
