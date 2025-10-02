
// Generated from Snippet.g4 by ANTLR 4.13.2


#include "SnippetVisitor.h"

#include "SnippetParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SnippetParserStaticData final {
  SnippetParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SnippetParserStaticData(const SnippetParserStaticData&) = delete;
  SnippetParserStaticData(SnippetParserStaticData&&) = delete;
  SnippetParserStaticData& operator=(const SnippetParserStaticData&) = delete;
  SnippetParserStaticData& operator=(SnippetParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag snippetParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<SnippetParserStaticData> snippetParserStaticData = nullptr;

void snippetParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (snippetParserStaticData != nullptr) {
    return;
  }
#else
  assert(snippetParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<SnippetParserStaticData>(
    std::vector<std::string>{
      "snippet", "part"
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
  	4,1,9,23,2,0,7,0,2,1,7,1,1,0,5,0,6,8,0,10,0,12,0,9,9,0,1,0,1,0,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,21,8,1,1,1,0,0,2,0,2,0,0,28,0,7,1,0,0,0,
  	2,20,1,0,0,0,4,6,3,2,1,0,5,4,1,0,0,0,6,9,1,0,0,0,7,5,1,0,0,0,7,8,1,0,
  	0,0,8,10,1,0,0,0,9,7,1,0,0,0,10,11,5,0,0,1,11,1,1,0,0,0,12,21,5,1,0,0,
  	13,21,5,2,0,0,14,21,5,3,0,0,15,21,5,4,0,0,16,21,5,5,0,0,17,21,5,6,0,0,
  	18,21,5,7,0,0,19,21,5,8,0,0,20,12,1,0,0,0,20,13,1,0,0,0,20,14,1,0,0,0,
  	20,15,1,0,0,0,20,16,1,0,0,0,20,17,1,0,0,0,20,18,1,0,0,0,20,19,1,0,0,0,
  	21,3,1,0,0,0,2,7,20
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  snippetParserStaticData = std::move(staticData);
}

}

SnippetParser::SnippetParser(TokenStream *input) : SnippetParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SnippetParser::SnippetParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SnippetParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *snippetParserStaticData->atn, snippetParserStaticData->decisionToDFA, snippetParserStaticData->sharedContextCache, options);
}

SnippetParser::~SnippetParser() {
  delete _interpreter;
}

const atn::ATN& SnippetParser::getATN() const {
  return *snippetParserStaticData->atn;
}

std::string SnippetParser::getGrammarFileName() const {
  return "Snippet.g4";
}

const std::vector<std::string>& SnippetParser::getRuleNames() const {
  return snippetParserStaticData->ruleNames;
}

const dfa::Vocabulary& SnippetParser::getVocabulary() const {
  return snippetParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SnippetParser::getSerializedATN() const {
  return snippetParserStaticData->serializedATN;
}


//----------------- SnippetContext ------------------------------------------------------------------

SnippetParser::SnippetContext::SnippetContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SnippetParser::SnippetContext::EOF() {
  return getToken(SnippetParser::EOF, 0);
}

std::vector<SnippetParser::PartContext *> SnippetParser::SnippetContext::part() {
  return getRuleContexts<SnippetParser::PartContext>();
}

SnippetParser::PartContext* SnippetParser::SnippetContext::part(size_t i) {
  return getRuleContext<SnippetParser::PartContext>(i);
}


size_t SnippetParser::SnippetContext::getRuleIndex() const {
  return SnippetParser::RuleSnippet;
}


std::any SnippetParser::SnippetContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitSnippet(this);
  else
    return visitor->visitChildren(this);
}

SnippetParser::SnippetContext* SnippetParser::snippet() {
  SnippetContext *_localctx = _tracker.createInstance<SnippetContext>(_ctx, getState());
  enterRule(_localctx, 0, SnippetParser::RuleSnippet);
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
    setState(7);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 510) != 0)) {
      setState(4);
      part();
      setState(9);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(10);
    match(SnippetParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PartContext ------------------------------------------------------------------

SnippetParser::PartContext::PartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SnippetParser::PartContext::getRuleIndex() const {
  return SnippetParser::RulePart;
}

void SnippetParser::PartContext::copyFrom(PartContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- PlaceholderContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::PlaceholderContext::PLACEHOLDER() {
  return getToken(SnippetParser::PLACEHOLDER, 0);
}

SnippetParser::PlaceholderContext::PlaceholderContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::PlaceholderContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitPlaceholder(this);
  else
    return visitor->visitChildren(this);
}
//----------------- VariableContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::VariableContext::VARIABLE() {
  return getToken(SnippetParser::VARIABLE, 0);
}

SnippetParser::VariableContext::VariableContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::VariableContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitVariable(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ChoiceContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::ChoiceContext::CHOICE() {
  return getToken(SnippetParser::CHOICE, 0);
}

SnippetParser::ChoiceContext::ChoiceContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::ChoiceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitChoice(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EscapedCharContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::EscapedCharContext::ESCAPED_CHAR() {
  return getToken(SnippetParser::ESCAPED_CHAR, 0);
}

SnippetParser::EscapedCharContext::EscapedCharContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::EscapedCharContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitEscapedChar(this);
  else
    return visitor->visitChildren(this);
}
//----------------- TextContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::TextContext::TEXT_CONTENT() {
  return getToken(SnippetParser::TEXT_CONTENT, 0);
}

SnippetParser::TextContext::TextContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::TextContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitText(this);
  else
    return visitor->visitChildren(this);
}
//----------------- VariableWithDefaultContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::VariableWithDefaultContext::VARIABLE_WITH_DEFAULT() {
  return getToken(SnippetParser::VARIABLE_WITH_DEFAULT, 0);
}

SnippetParser::VariableWithDefaultContext::VariableWithDefaultContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::VariableWithDefaultContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitVariableWithDefault(this);
  else
    return visitor->visitChildren(this);
}
//----------------- TabstopContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::TabstopContext::TABSTOP() {
  return getToken(SnippetParser::TABSTOP, 0);
}

SnippetParser::TabstopContext::TabstopContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::TabstopContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitTabstop(this);
  else
    return visitor->visitChildren(this);
}
//----------------- TabstopWithDefaultContext ------------------------------------------------------------------

tree::TerminalNode* SnippetParser::TabstopWithDefaultContext::TABSTOP_WITH_DEFAULT() {
  return getToken(SnippetParser::TABSTOP_WITH_DEFAULT, 0);
}

SnippetParser::TabstopWithDefaultContext::TabstopWithDefaultContext(PartContext *ctx) { copyFrom(ctx); }


std::any SnippetParser::TabstopWithDefaultContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SnippetVisitor*>(visitor))
    return parserVisitor->visitTabstopWithDefault(this);
  else
    return visitor->visitChildren(this);
}
SnippetParser::PartContext* SnippetParser::part() {
  PartContext *_localctx = _tracker.createInstance<PartContext>(_ctx, getState());
  enterRule(_localctx, 2, SnippetParser::RulePart);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(20);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SnippetParser::TEXT_CONTENT: {
        _localctx = _tracker.createInstance<SnippetParser::TextContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(12);
        match(SnippetParser::TEXT_CONTENT);
        break;
      }

      case SnippetParser::ESCAPED_CHAR: {
        _localctx = _tracker.createInstance<SnippetParser::EscapedCharContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(13);
        match(SnippetParser::ESCAPED_CHAR);
        break;
      }

      case SnippetParser::VARIABLE: {
        _localctx = _tracker.createInstance<SnippetParser::VariableContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(14);
        match(SnippetParser::VARIABLE);
        break;
      }

      case SnippetParser::VARIABLE_WITH_DEFAULT: {
        _localctx = _tracker.createInstance<SnippetParser::VariableWithDefaultContext>(_localctx);
        enterOuterAlt(_localctx, 4);
        setState(15);
        match(SnippetParser::VARIABLE_WITH_DEFAULT);
        break;
      }

      case SnippetParser::PLACEHOLDER: {
        _localctx = _tracker.createInstance<SnippetParser::PlaceholderContext>(_localctx);
        enterOuterAlt(_localctx, 5);
        setState(16);
        match(SnippetParser::PLACEHOLDER);
        break;
      }

      case SnippetParser::TABSTOP: {
        _localctx = _tracker.createInstance<SnippetParser::TabstopContext>(_localctx);
        enterOuterAlt(_localctx, 6);
        setState(17);
        match(SnippetParser::TABSTOP);
        break;
      }

      case SnippetParser::TABSTOP_WITH_DEFAULT: {
        _localctx = _tracker.createInstance<SnippetParser::TabstopWithDefaultContext>(_localctx);
        enterOuterAlt(_localctx, 7);
        setState(18);
        match(SnippetParser::TABSTOP_WITH_DEFAULT);
        break;
      }

      case SnippetParser::CHOICE: {
        _localctx = _tracker.createInstance<SnippetParser::ChoiceContext>(_localctx);
        enterOuterAlt(_localctx, 8);
        setState(19);
        match(SnippetParser::CHOICE);
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

void SnippetParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  snippetParserInitialize();
#else
  ::antlr4::internal::call_once(snippetParserOnceFlag, snippetParserInitialize);
#endif
}
