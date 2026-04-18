
// Generated from AngelscriptExprParser.g4 by ANTLR 4.13.2


#include "AngelscriptExprParserVisitor.h"

#include "AngelscriptExprParserParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct AngelscriptExprParserParserStaticData final {
  AngelscriptExprParserParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  AngelscriptExprParserParserStaticData(const AngelscriptExprParserParserStaticData&) = delete;
  AngelscriptExprParserParserStaticData(AngelscriptExprParserParserStaticData&&) = delete;
  AngelscriptExprParserParserStaticData& operator=(const AngelscriptExprParserParserStaticData&) = delete;
  AngelscriptExprParserParserStaticData& operator=(AngelscriptExprParserParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag angelscriptexprparserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<AngelscriptExprParserParserStaticData> angelscriptexprparserParserStaticData = nullptr;

void angelscriptexprparserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (angelscriptexprparserParserStaticData != nullptr) {
    return;
  }
#else
  assert(angelscriptexprparserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<AngelscriptExprParserParserStaticData>(
    std::vector<std::string>{
      "expr", "logicalOrExpr", "logicalAndExpr", "bitOrExpr", "bitXorExpr", 
      "bitAndExpr", "equalityExpr", "relationalExpr", "shiftExpr", "additiveExpr", 
      "multiplicativeExpr", "unaryExpr", "primaryExpr", "literal"
    },
    std::vector<std::string>{
      "", "'true'", "'false'", "'null'", "'('", "')'", "'+'", "'-'", "'*'", 
      "'/'", "'%'", "'^'", "'&'", "'|'", "'~'", "'!'", "'<'", "'>'", "'=='", 
      "'!='", "'<='", "'>='", "'&&'", "'||'"
    },
    std::vector<std::string>{
      "", "True_", "False_", "Nullptr", "LeftParen", "RightParen", "Plus", 
      "Minus", "Star", "Div", "Mod", "Xor", "And", "Or", "Tilde", "Not", 
      "Less", "Greater", "Equal", "NotEqual", "LessEqual", "GreaterEqual", 
      "AndAnd", "OrOr", "IntegerLiteral", "FloatingLiteral", "CharacterLiteral", 
      "StringLiteral", "Whitespace", "Newline", "BlockComment", "LineComment"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,31,137,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,1,0,1,0,
  	1,0,1,1,1,1,1,1,5,1,35,8,1,10,1,12,1,38,9,1,1,2,1,2,1,2,5,2,43,8,2,10,
  	2,12,2,46,9,2,1,3,1,3,1,3,5,3,51,8,3,10,3,12,3,54,9,3,1,4,1,4,1,4,5,4,
  	59,8,4,10,4,12,4,62,9,4,1,5,1,5,1,5,5,5,67,8,5,10,5,12,5,70,9,5,1,6,1,
  	6,1,6,5,6,75,8,6,10,6,12,6,78,9,6,1,7,1,7,1,7,5,7,83,8,7,10,7,12,7,86,
  	9,7,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,3,8,96,8,8,1,8,5,8,99,8,8,10,8,12,
  	8,102,9,8,1,9,1,9,1,9,5,9,107,8,9,10,9,12,9,110,9,9,1,10,1,10,1,10,5,
  	10,115,8,10,10,10,12,10,118,9,10,1,11,5,11,121,8,11,10,11,12,11,124,9,
  	11,1,11,1,11,1,12,1,12,1,12,1,12,1,12,3,12,133,8,12,1,13,1,13,1,13,0,
  	0,14,0,2,4,6,8,10,12,14,16,18,20,22,24,26,0,6,1,0,18,19,2,0,16,17,20,
  	21,1,0,6,7,1,0,8,10,2,0,6,7,14,15,2,0,1,3,24,27,136,0,28,1,0,0,0,2,31,
  	1,0,0,0,4,39,1,0,0,0,6,47,1,0,0,0,8,55,1,0,0,0,10,63,1,0,0,0,12,71,1,
  	0,0,0,14,79,1,0,0,0,16,87,1,0,0,0,18,103,1,0,0,0,20,111,1,0,0,0,22,122,
  	1,0,0,0,24,132,1,0,0,0,26,134,1,0,0,0,28,29,3,2,1,0,29,30,5,0,0,1,30,
  	1,1,0,0,0,31,36,3,4,2,0,32,33,5,23,0,0,33,35,3,4,2,0,34,32,1,0,0,0,35,
  	38,1,0,0,0,36,34,1,0,0,0,36,37,1,0,0,0,37,3,1,0,0,0,38,36,1,0,0,0,39,
  	44,3,6,3,0,40,41,5,22,0,0,41,43,3,6,3,0,42,40,1,0,0,0,43,46,1,0,0,0,44,
  	42,1,0,0,0,44,45,1,0,0,0,45,5,1,0,0,0,46,44,1,0,0,0,47,52,3,8,4,0,48,
  	49,5,13,0,0,49,51,3,8,4,0,50,48,1,0,0,0,51,54,1,0,0,0,52,50,1,0,0,0,52,
  	53,1,0,0,0,53,7,1,0,0,0,54,52,1,0,0,0,55,60,3,10,5,0,56,57,5,11,0,0,57,
  	59,3,10,5,0,58,56,1,0,0,0,59,62,1,0,0,0,60,58,1,0,0,0,60,61,1,0,0,0,61,
  	9,1,0,0,0,62,60,1,0,0,0,63,68,3,12,6,0,64,65,5,12,0,0,65,67,3,12,6,0,
  	66,64,1,0,0,0,67,70,1,0,0,0,68,66,1,0,0,0,68,69,1,0,0,0,69,11,1,0,0,0,
  	70,68,1,0,0,0,71,76,3,14,7,0,72,73,7,0,0,0,73,75,3,14,7,0,74,72,1,0,0,
  	0,75,78,1,0,0,0,76,74,1,0,0,0,76,77,1,0,0,0,77,13,1,0,0,0,78,76,1,0,0,
  	0,79,84,3,16,8,0,80,81,7,1,0,0,81,83,3,16,8,0,82,80,1,0,0,0,83,86,1,0,
  	0,0,84,82,1,0,0,0,84,85,1,0,0,0,85,15,1,0,0,0,86,84,1,0,0,0,87,100,3,
  	18,9,0,88,89,5,17,0,0,89,90,5,17,0,0,90,96,5,17,0,0,91,92,5,17,0,0,92,
  	96,5,17,0,0,93,94,5,16,0,0,94,96,5,16,0,0,95,88,1,0,0,0,95,91,1,0,0,0,
  	95,93,1,0,0,0,96,97,1,0,0,0,97,99,3,18,9,0,98,95,1,0,0,0,99,102,1,0,0,
  	0,100,98,1,0,0,0,100,101,1,0,0,0,101,17,1,0,0,0,102,100,1,0,0,0,103,108,
  	3,20,10,0,104,105,7,2,0,0,105,107,3,20,10,0,106,104,1,0,0,0,107,110,1,
  	0,0,0,108,106,1,0,0,0,108,109,1,0,0,0,109,19,1,0,0,0,110,108,1,0,0,0,
  	111,116,3,22,11,0,112,113,7,3,0,0,113,115,3,22,11,0,114,112,1,0,0,0,115,
  	118,1,0,0,0,116,114,1,0,0,0,116,117,1,0,0,0,117,21,1,0,0,0,118,116,1,
  	0,0,0,119,121,7,4,0,0,120,119,1,0,0,0,121,124,1,0,0,0,122,120,1,0,0,0,
  	122,123,1,0,0,0,123,125,1,0,0,0,124,122,1,0,0,0,125,126,3,24,12,0,126,
  	23,1,0,0,0,127,128,5,4,0,0,128,129,3,0,0,0,129,130,5,5,0,0,130,133,1,
  	0,0,0,131,133,3,26,13,0,132,127,1,0,0,0,132,131,1,0,0,0,133,25,1,0,0,
  	0,134,135,7,5,0,0,135,27,1,0,0,0,13,36,44,52,60,68,76,84,95,100,108,116,
  	122,132
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  angelscriptexprparserParserStaticData = std::move(staticData);
}

}

AngelscriptExprParserParser::AngelscriptExprParserParser(TokenStream *input) : AngelscriptExprParserParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

AngelscriptExprParserParser::AngelscriptExprParserParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  AngelscriptExprParserParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *angelscriptexprparserParserStaticData->atn, angelscriptexprparserParserStaticData->decisionToDFA, angelscriptexprparserParserStaticData->sharedContextCache, options);
}

AngelscriptExprParserParser::~AngelscriptExprParserParser() {
  delete _interpreter;
}

const atn::ATN& AngelscriptExprParserParser::getATN() const {
  return *angelscriptexprparserParserStaticData->atn;
}

std::string AngelscriptExprParserParser::getGrammarFileName() const {
  return "AngelscriptExprParser.g4";
}

const std::vector<std::string>& AngelscriptExprParserParser::getRuleNames() const {
  return angelscriptexprparserParserStaticData->ruleNames;
}

const dfa::Vocabulary& AngelscriptExprParserParser::getVocabulary() const {
  return angelscriptexprparserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView AngelscriptExprParserParser::getSerializedATN() const {
  return angelscriptexprparserParserStaticData->serializedATN;
}


//----------------- ExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::ExprContext::ExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptExprParserParser::LogicalOrExprContext* AngelscriptExprParserParser::ExprContext::logicalOrExpr() {
  return getRuleContext<AngelscriptExprParserParser::LogicalOrExprContext>(0);
}

tree::TerminalNode* AngelscriptExprParserParser::ExprContext::EOF() {
  return getToken(AngelscriptExprParserParser::EOF, 0);
}


size_t AngelscriptExprParserParser::ExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleExpr;
}


std::any AngelscriptExprParserParser::ExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::ExprContext* AngelscriptExprParserParser::expr() {
  ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, getState());
  enterRule(_localctx, 0, AngelscriptExprParserParser::RuleExpr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(28);
    logicalOrExpr();
    setState(29);
    match(AngelscriptExprParserParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LogicalOrExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::LogicalOrExprContext::LogicalOrExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::LogicalAndExprContext *> AngelscriptExprParserParser::LogicalOrExprContext::logicalAndExpr() {
  return getRuleContexts<AngelscriptExprParserParser::LogicalAndExprContext>();
}

AngelscriptExprParserParser::LogicalAndExprContext* AngelscriptExprParserParser::LogicalOrExprContext::logicalAndExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::LogicalAndExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::LogicalOrExprContext::OrOr() {
  return getTokens(AngelscriptExprParserParser::OrOr);
}

tree::TerminalNode* AngelscriptExprParserParser::LogicalOrExprContext::OrOr(size_t i) {
  return getToken(AngelscriptExprParserParser::OrOr, i);
}


size_t AngelscriptExprParserParser::LogicalOrExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleLogicalOrExpr;
}


std::any AngelscriptExprParserParser::LogicalOrExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitLogicalOrExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::LogicalOrExprContext* AngelscriptExprParserParser::logicalOrExpr() {
  LogicalOrExprContext *_localctx = _tracker.createInstance<LogicalOrExprContext>(_ctx, getState());
  enterRule(_localctx, 2, AngelscriptExprParserParser::RuleLogicalOrExpr);
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
    setState(31);
    logicalAndExpr();
    setState(36);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptExprParserParser::OrOr) {
      setState(32);
      match(AngelscriptExprParserParser::OrOr);
      setState(33);
      logicalAndExpr();
      setState(38);
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

//----------------- LogicalAndExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::LogicalAndExprContext::LogicalAndExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::BitOrExprContext *> AngelscriptExprParserParser::LogicalAndExprContext::bitOrExpr() {
  return getRuleContexts<AngelscriptExprParserParser::BitOrExprContext>();
}

AngelscriptExprParserParser::BitOrExprContext* AngelscriptExprParserParser::LogicalAndExprContext::bitOrExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::BitOrExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::LogicalAndExprContext::AndAnd() {
  return getTokens(AngelscriptExprParserParser::AndAnd);
}

tree::TerminalNode* AngelscriptExprParserParser::LogicalAndExprContext::AndAnd(size_t i) {
  return getToken(AngelscriptExprParserParser::AndAnd, i);
}


size_t AngelscriptExprParserParser::LogicalAndExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleLogicalAndExpr;
}


std::any AngelscriptExprParserParser::LogicalAndExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitLogicalAndExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::LogicalAndExprContext* AngelscriptExprParserParser::logicalAndExpr() {
  LogicalAndExprContext *_localctx = _tracker.createInstance<LogicalAndExprContext>(_ctx, getState());
  enterRule(_localctx, 4, AngelscriptExprParserParser::RuleLogicalAndExpr);
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
    setState(39);
    bitOrExpr();
    setState(44);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptExprParserParser::AndAnd) {
      setState(40);
      match(AngelscriptExprParserParser::AndAnd);
      setState(41);
      bitOrExpr();
      setState(46);
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

//----------------- BitOrExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::BitOrExprContext::BitOrExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::BitXorExprContext *> AngelscriptExprParserParser::BitOrExprContext::bitXorExpr() {
  return getRuleContexts<AngelscriptExprParserParser::BitXorExprContext>();
}

AngelscriptExprParserParser::BitXorExprContext* AngelscriptExprParserParser::BitOrExprContext::bitXorExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::BitXorExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::BitOrExprContext::Or() {
  return getTokens(AngelscriptExprParserParser::Or);
}

tree::TerminalNode* AngelscriptExprParserParser::BitOrExprContext::Or(size_t i) {
  return getToken(AngelscriptExprParserParser::Or, i);
}


size_t AngelscriptExprParserParser::BitOrExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleBitOrExpr;
}


std::any AngelscriptExprParserParser::BitOrExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitBitOrExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::BitOrExprContext* AngelscriptExprParserParser::bitOrExpr() {
  BitOrExprContext *_localctx = _tracker.createInstance<BitOrExprContext>(_ctx, getState());
  enterRule(_localctx, 6, AngelscriptExprParserParser::RuleBitOrExpr);
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
    setState(47);
    bitXorExpr();
    setState(52);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptExprParserParser::Or) {
      setState(48);
      match(AngelscriptExprParserParser::Or);
      setState(49);
      bitXorExpr();
      setState(54);
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

//----------------- BitXorExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::BitXorExprContext::BitXorExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::BitAndExprContext *> AngelscriptExprParserParser::BitXorExprContext::bitAndExpr() {
  return getRuleContexts<AngelscriptExprParserParser::BitAndExprContext>();
}

AngelscriptExprParserParser::BitAndExprContext* AngelscriptExprParserParser::BitXorExprContext::bitAndExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::BitAndExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::BitXorExprContext::Xor() {
  return getTokens(AngelscriptExprParserParser::Xor);
}

tree::TerminalNode* AngelscriptExprParserParser::BitXorExprContext::Xor(size_t i) {
  return getToken(AngelscriptExprParserParser::Xor, i);
}


size_t AngelscriptExprParserParser::BitXorExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleBitXorExpr;
}


std::any AngelscriptExprParserParser::BitXorExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitBitXorExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::BitXorExprContext* AngelscriptExprParserParser::bitXorExpr() {
  BitXorExprContext *_localctx = _tracker.createInstance<BitXorExprContext>(_ctx, getState());
  enterRule(_localctx, 8, AngelscriptExprParserParser::RuleBitXorExpr);
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
    setState(55);
    bitAndExpr();
    setState(60);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptExprParserParser::Xor) {
      setState(56);
      match(AngelscriptExprParserParser::Xor);
      setState(57);
      bitAndExpr();
      setState(62);
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

//----------------- BitAndExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::BitAndExprContext::BitAndExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::EqualityExprContext *> AngelscriptExprParserParser::BitAndExprContext::equalityExpr() {
  return getRuleContexts<AngelscriptExprParserParser::EqualityExprContext>();
}

AngelscriptExprParserParser::EqualityExprContext* AngelscriptExprParserParser::BitAndExprContext::equalityExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::EqualityExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::BitAndExprContext::And() {
  return getTokens(AngelscriptExprParserParser::And);
}

tree::TerminalNode* AngelscriptExprParserParser::BitAndExprContext::And(size_t i) {
  return getToken(AngelscriptExprParserParser::And, i);
}


size_t AngelscriptExprParserParser::BitAndExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleBitAndExpr;
}


std::any AngelscriptExprParserParser::BitAndExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitBitAndExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::BitAndExprContext* AngelscriptExprParserParser::bitAndExpr() {
  BitAndExprContext *_localctx = _tracker.createInstance<BitAndExprContext>(_ctx, getState());
  enterRule(_localctx, 10, AngelscriptExprParserParser::RuleBitAndExpr);
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
    setState(63);
    equalityExpr();
    setState(68);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptExprParserParser::And) {
      setState(64);
      match(AngelscriptExprParserParser::And);
      setState(65);
      equalityExpr();
      setState(70);
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

//----------------- EqualityExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::EqualityExprContext::EqualityExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::RelationalExprContext *> AngelscriptExprParserParser::EqualityExprContext::relationalExpr() {
  return getRuleContexts<AngelscriptExprParserParser::RelationalExprContext>();
}

AngelscriptExprParserParser::RelationalExprContext* AngelscriptExprParserParser::EqualityExprContext::relationalExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::RelationalExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::EqualityExprContext::Equal() {
  return getTokens(AngelscriptExprParserParser::Equal);
}

tree::TerminalNode* AngelscriptExprParserParser::EqualityExprContext::Equal(size_t i) {
  return getToken(AngelscriptExprParserParser::Equal, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::EqualityExprContext::NotEqual() {
  return getTokens(AngelscriptExprParserParser::NotEqual);
}

tree::TerminalNode* AngelscriptExprParserParser::EqualityExprContext::NotEqual(size_t i) {
  return getToken(AngelscriptExprParserParser::NotEqual, i);
}


size_t AngelscriptExprParserParser::EqualityExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleEqualityExpr;
}


std::any AngelscriptExprParserParser::EqualityExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitEqualityExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::EqualityExprContext* AngelscriptExprParserParser::equalityExpr() {
  EqualityExprContext *_localctx = _tracker.createInstance<EqualityExprContext>(_ctx, getState());
  enterRule(_localctx, 12, AngelscriptExprParserParser::RuleEqualityExpr);
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
    setState(71);
    relationalExpr();
    setState(76);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptExprParserParser::Equal

    || _la == AngelscriptExprParserParser::NotEqual) {
      setState(72);
      _la = _input->LA(1);
      if (!(_la == AngelscriptExprParserParser::Equal

      || _la == AngelscriptExprParserParser::NotEqual)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(73);
      relationalExpr();
      setState(78);
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

//----------------- RelationalExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::RelationalExprContext::RelationalExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::ShiftExprContext *> AngelscriptExprParserParser::RelationalExprContext::shiftExpr() {
  return getRuleContexts<AngelscriptExprParserParser::ShiftExprContext>();
}

AngelscriptExprParserParser::ShiftExprContext* AngelscriptExprParserParser::RelationalExprContext::shiftExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::ShiftExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::RelationalExprContext::Less() {
  return getTokens(AngelscriptExprParserParser::Less);
}

tree::TerminalNode* AngelscriptExprParserParser::RelationalExprContext::Less(size_t i) {
  return getToken(AngelscriptExprParserParser::Less, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::RelationalExprContext::LessEqual() {
  return getTokens(AngelscriptExprParserParser::LessEqual);
}

tree::TerminalNode* AngelscriptExprParserParser::RelationalExprContext::LessEqual(size_t i) {
  return getToken(AngelscriptExprParserParser::LessEqual, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::RelationalExprContext::Greater() {
  return getTokens(AngelscriptExprParserParser::Greater);
}

tree::TerminalNode* AngelscriptExprParserParser::RelationalExprContext::Greater(size_t i) {
  return getToken(AngelscriptExprParserParser::Greater, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::RelationalExprContext::GreaterEqual() {
  return getTokens(AngelscriptExprParserParser::GreaterEqual);
}

tree::TerminalNode* AngelscriptExprParserParser::RelationalExprContext::GreaterEqual(size_t i) {
  return getToken(AngelscriptExprParserParser::GreaterEqual, i);
}


size_t AngelscriptExprParserParser::RelationalExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleRelationalExpr;
}


std::any AngelscriptExprParserParser::RelationalExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitRelationalExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::RelationalExprContext* AngelscriptExprParserParser::relationalExpr() {
  RelationalExprContext *_localctx = _tracker.createInstance<RelationalExprContext>(_ctx, getState());
  enterRule(_localctx, 14, AngelscriptExprParserParser::RuleRelationalExpr);
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
    setState(79);
    shiftExpr();
    setState(84);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 3342336) != 0)) {
      setState(80);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 3342336) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(81);
      shiftExpr();
      setState(86);
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

//----------------- ShiftExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::ShiftExprContext::ShiftExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::AdditiveExprContext *> AngelscriptExprParserParser::ShiftExprContext::additiveExpr() {
  return getRuleContexts<AngelscriptExprParserParser::AdditiveExprContext>();
}

AngelscriptExprParserParser::AdditiveExprContext* AngelscriptExprParserParser::ShiftExprContext::additiveExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::AdditiveExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::ShiftExprContext::Greater() {
  return getTokens(AngelscriptExprParserParser::Greater);
}

tree::TerminalNode* AngelscriptExprParserParser::ShiftExprContext::Greater(size_t i) {
  return getToken(AngelscriptExprParserParser::Greater, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::ShiftExprContext::Less() {
  return getTokens(AngelscriptExprParserParser::Less);
}

tree::TerminalNode* AngelscriptExprParserParser::ShiftExprContext::Less(size_t i) {
  return getToken(AngelscriptExprParserParser::Less, i);
}


size_t AngelscriptExprParserParser::ShiftExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleShiftExpr;
}


std::any AngelscriptExprParserParser::ShiftExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitShiftExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::ShiftExprContext* AngelscriptExprParserParser::shiftExpr() {
  ShiftExprContext *_localctx = _tracker.createInstance<ShiftExprContext>(_ctx, getState());
  enterRule(_localctx, 16, AngelscriptExprParserParser::RuleShiftExpr);

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
    setState(87);
    additiveExpr();
    setState(100);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(95);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
        case 1: {
          setState(88);
          match(AngelscriptExprParserParser::Greater);
          setState(89);
          match(AngelscriptExprParserParser::Greater);
          setState(90);
          match(AngelscriptExprParserParser::Greater);
          break;
        }

        case 2: {
          setState(91);
          match(AngelscriptExprParserParser::Greater);
          setState(92);
          match(AngelscriptExprParserParser::Greater);
          break;
        }

        case 3: {
          setState(93);
          match(AngelscriptExprParserParser::Less);
          setState(94);
          match(AngelscriptExprParserParser::Less);
          break;
        }

        default:
          break;
        }
        setState(97);
        additiveExpr(); 
      }
      setState(102);
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

//----------------- AdditiveExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::AdditiveExprContext::AdditiveExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::MultiplicativeExprContext *> AngelscriptExprParserParser::AdditiveExprContext::multiplicativeExpr() {
  return getRuleContexts<AngelscriptExprParserParser::MultiplicativeExprContext>();
}

AngelscriptExprParserParser::MultiplicativeExprContext* AngelscriptExprParserParser::AdditiveExprContext::multiplicativeExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::MultiplicativeExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::AdditiveExprContext::Plus() {
  return getTokens(AngelscriptExprParserParser::Plus);
}

tree::TerminalNode* AngelscriptExprParserParser::AdditiveExprContext::Plus(size_t i) {
  return getToken(AngelscriptExprParserParser::Plus, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::AdditiveExprContext::Minus() {
  return getTokens(AngelscriptExprParserParser::Minus);
}

tree::TerminalNode* AngelscriptExprParserParser::AdditiveExprContext::Minus(size_t i) {
  return getToken(AngelscriptExprParserParser::Minus, i);
}


size_t AngelscriptExprParserParser::AdditiveExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleAdditiveExpr;
}


std::any AngelscriptExprParserParser::AdditiveExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::AdditiveExprContext* AngelscriptExprParserParser::additiveExpr() {
  AdditiveExprContext *_localctx = _tracker.createInstance<AdditiveExprContext>(_ctx, getState());
  enterRule(_localctx, 18, AngelscriptExprParserParser::RuleAdditiveExpr);
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
    setState(103);
    multiplicativeExpr();
    setState(108);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == AngelscriptExprParserParser::Plus

    || _la == AngelscriptExprParserParser::Minus) {
      setState(104);
      _la = _input->LA(1);
      if (!(_la == AngelscriptExprParserParser::Plus

      || _la == AngelscriptExprParserParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(105);
      multiplicativeExpr();
      setState(110);
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

//----------------- MultiplicativeExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::MultiplicativeExprContext::MultiplicativeExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<AngelscriptExprParserParser::UnaryExprContext *> AngelscriptExprParserParser::MultiplicativeExprContext::unaryExpr() {
  return getRuleContexts<AngelscriptExprParserParser::UnaryExprContext>();
}

AngelscriptExprParserParser::UnaryExprContext* AngelscriptExprParserParser::MultiplicativeExprContext::unaryExpr(size_t i) {
  return getRuleContext<AngelscriptExprParserParser::UnaryExprContext>(i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::MultiplicativeExprContext::Star() {
  return getTokens(AngelscriptExprParserParser::Star);
}

tree::TerminalNode* AngelscriptExprParserParser::MultiplicativeExprContext::Star(size_t i) {
  return getToken(AngelscriptExprParserParser::Star, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::MultiplicativeExprContext::Div() {
  return getTokens(AngelscriptExprParserParser::Div);
}

tree::TerminalNode* AngelscriptExprParserParser::MultiplicativeExprContext::Div(size_t i) {
  return getToken(AngelscriptExprParserParser::Div, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::MultiplicativeExprContext::Mod() {
  return getTokens(AngelscriptExprParserParser::Mod);
}

tree::TerminalNode* AngelscriptExprParserParser::MultiplicativeExprContext::Mod(size_t i) {
  return getToken(AngelscriptExprParserParser::Mod, i);
}


size_t AngelscriptExprParserParser::MultiplicativeExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleMultiplicativeExpr;
}


std::any AngelscriptExprParserParser::MultiplicativeExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::MultiplicativeExprContext* AngelscriptExprParserParser::multiplicativeExpr() {
  MultiplicativeExprContext *_localctx = _tracker.createInstance<MultiplicativeExprContext>(_ctx, getState());
  enterRule(_localctx, 20, AngelscriptExprParserParser::RuleMultiplicativeExpr);
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
    unaryExpr();
    setState(116);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1792) != 0)) {
      setState(112);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 1792) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(113);
      unaryExpr();
      setState(118);
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

//----------------- UnaryExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::UnaryExprContext::UnaryExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

AngelscriptExprParserParser::PrimaryExprContext* AngelscriptExprParserParser::UnaryExprContext::primaryExpr() {
  return getRuleContext<AngelscriptExprParserParser::PrimaryExprContext>(0);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::UnaryExprContext::Plus() {
  return getTokens(AngelscriptExprParserParser::Plus);
}

tree::TerminalNode* AngelscriptExprParserParser::UnaryExprContext::Plus(size_t i) {
  return getToken(AngelscriptExprParserParser::Plus, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::UnaryExprContext::Minus() {
  return getTokens(AngelscriptExprParserParser::Minus);
}

tree::TerminalNode* AngelscriptExprParserParser::UnaryExprContext::Minus(size_t i) {
  return getToken(AngelscriptExprParserParser::Minus, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::UnaryExprContext::Not() {
  return getTokens(AngelscriptExprParserParser::Not);
}

tree::TerminalNode* AngelscriptExprParserParser::UnaryExprContext::Not(size_t i) {
  return getToken(AngelscriptExprParserParser::Not, i);
}

std::vector<tree::TerminalNode *> AngelscriptExprParserParser::UnaryExprContext::Tilde() {
  return getTokens(AngelscriptExprParserParser::Tilde);
}

tree::TerminalNode* AngelscriptExprParserParser::UnaryExprContext::Tilde(size_t i) {
  return getToken(AngelscriptExprParserParser::Tilde, i);
}


size_t AngelscriptExprParserParser::UnaryExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleUnaryExpr;
}


std::any AngelscriptExprParserParser::UnaryExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitUnaryExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::UnaryExprContext* AngelscriptExprParserParser::unaryExpr() {
  UnaryExprContext *_localctx = _tracker.createInstance<UnaryExprContext>(_ctx, getState());
  enterRule(_localctx, 22, AngelscriptExprParserParser::RuleUnaryExpr);
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
    setState(122);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 49344) != 0)) {
      setState(119);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 49344) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(124);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(125);
    primaryExpr();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExprContext ------------------------------------------------------------------

AngelscriptExprParserParser::PrimaryExprContext::PrimaryExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptExprParserParser::PrimaryExprContext::LeftParen() {
  return getToken(AngelscriptExprParserParser::LeftParen, 0);
}

AngelscriptExprParserParser::ExprContext* AngelscriptExprParserParser::PrimaryExprContext::expr() {
  return getRuleContext<AngelscriptExprParserParser::ExprContext>(0);
}

tree::TerminalNode* AngelscriptExprParserParser::PrimaryExprContext::RightParen() {
  return getToken(AngelscriptExprParserParser::RightParen, 0);
}

AngelscriptExprParserParser::LiteralContext* AngelscriptExprParserParser::PrimaryExprContext::literal() {
  return getRuleContext<AngelscriptExprParserParser::LiteralContext>(0);
}


size_t AngelscriptExprParserParser::PrimaryExprContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RulePrimaryExpr;
}


std::any AngelscriptExprParserParser::PrimaryExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpr(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::PrimaryExprContext* AngelscriptExprParserParser::primaryExpr() {
  PrimaryExprContext *_localctx = _tracker.createInstance<PrimaryExprContext>(_ctx, getState());
  enterRule(_localctx, 24, AngelscriptExprParserParser::RulePrimaryExpr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(132);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case AngelscriptExprParserParser::LeftParen: {
        enterOuterAlt(_localctx, 1);
        setState(127);
        match(AngelscriptExprParserParser::LeftParen);
        setState(128);
        expr();
        setState(129);
        match(AngelscriptExprParserParser::RightParen);
        break;
      }

      case AngelscriptExprParserParser::True_:
      case AngelscriptExprParserParser::False_:
      case AngelscriptExprParserParser::Nullptr:
      case AngelscriptExprParserParser::IntegerLiteral:
      case AngelscriptExprParserParser::FloatingLiteral:
      case AngelscriptExprParserParser::CharacterLiteral:
      case AngelscriptExprParserParser::StringLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(131);
        literal();
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

//----------------- LiteralContext ------------------------------------------------------------------

AngelscriptExprParserParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* AngelscriptExprParserParser::LiteralContext::IntegerLiteral() {
  return getToken(AngelscriptExprParserParser::IntegerLiteral, 0);
}

tree::TerminalNode* AngelscriptExprParserParser::LiteralContext::FloatingLiteral() {
  return getToken(AngelscriptExprParserParser::FloatingLiteral, 0);
}

tree::TerminalNode* AngelscriptExprParserParser::LiteralContext::CharacterLiteral() {
  return getToken(AngelscriptExprParserParser::CharacterLiteral, 0);
}

tree::TerminalNode* AngelscriptExprParserParser::LiteralContext::StringLiteral() {
  return getToken(AngelscriptExprParserParser::StringLiteral, 0);
}

tree::TerminalNode* AngelscriptExprParserParser::LiteralContext::True_() {
  return getToken(AngelscriptExprParserParser::True_, 0);
}

tree::TerminalNode* AngelscriptExprParserParser::LiteralContext::False_() {
  return getToken(AngelscriptExprParserParser::False_, 0);
}

tree::TerminalNode* AngelscriptExprParserParser::LiteralContext::Nullptr() {
  return getToken(AngelscriptExprParserParser::Nullptr, 0);
}


size_t AngelscriptExprParserParser::LiteralContext::getRuleIndex() const {
  return AngelscriptExprParserParser::RuleLiteral;
}


std::any AngelscriptExprParserParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<AngelscriptExprParserVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

AngelscriptExprParserParser::LiteralContext* AngelscriptExprParserParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 26, AngelscriptExprParserParser::RuleLiteral);
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
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 251658254) != 0))) {
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

void AngelscriptExprParserParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  angelscriptexprparserParserInitialize();
#else
  ::antlr4::internal::call_once(angelscriptexprparserParserOnceFlag, angelscriptexprparserParserInitialize);
#endif
}
