
// Generated from NumCal.g4 by ANTLR 4.13.2


#include "NumCalVisitor.h"

#include "NumCalParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct NumCalParserStaticData final {
  NumCalParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  NumCalParserStaticData(const NumCalParserStaticData&) = delete;
  NumCalParserStaticData(NumCalParserStaticData&&) = delete;
  NumCalParserStaticData& operator=(const NumCalParserStaticData&) = delete;
  NumCalParserStaticData& operator=(NumCalParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag numcalParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<NumCalParserStaticData> numcalParserStaticData = nullptr;

void numcalParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (numcalParserStaticData != nullptr) {
    return;
  }
#else
  assert(numcalParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<NumCalParserStaticData>(
    std::vector<std::string>{
      "primaryExpression", "unaryExpression", "unaryOperator", "castExpression", 
      "multiplicativeExpression", "additiveExpression", "shiftExpression", 
      "andExpression", "exclusiveOrExpression", "inclusiveOrExpression", 
      "assignmentExpression", "entryExpression", "prefixGoto"
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
  	4,1,19,127,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,1,0,1,0,1,0,1,0,1,
  	0,3,0,32,8,0,1,1,1,1,1,1,1,1,3,1,38,8,1,1,2,1,2,1,3,1,3,3,3,44,8,3,1,
  	4,1,4,1,4,5,4,49,8,4,10,4,12,4,52,9,4,1,5,1,5,1,5,5,5,57,8,5,10,5,12,
  	5,60,9,5,1,6,1,6,1,6,5,6,65,8,6,10,6,12,6,68,9,6,1,7,1,7,1,7,5,7,73,8,
  	7,10,7,12,7,76,9,7,1,8,1,8,1,8,5,8,81,8,8,10,8,12,8,84,9,8,1,9,1,9,1,
  	9,5,9,89,8,9,10,9,12,9,92,9,9,1,10,1,10,3,10,96,8,10,1,11,3,11,99,8,11,
  	1,11,1,11,1,11,1,11,1,11,3,11,106,8,11,1,11,1,11,1,11,1,11,1,11,1,11,
  	1,11,1,11,1,11,1,11,1,11,1,11,1,11,1,11,1,11,3,11,123,8,11,1,12,1,12,
  	1,12,0,0,13,0,2,4,6,8,10,12,14,16,18,20,22,24,0,5,2,0,5,6,13,13,1,0,7,
  	9,1,0,5,6,1,0,3,4,2,0,5,6,14,14,128,0,31,1,0,0,0,2,37,1,0,0,0,4,39,1,
  	0,0,0,6,43,1,0,0,0,8,45,1,0,0,0,10,53,1,0,0,0,12,61,1,0,0,0,14,69,1,0,
  	0,0,16,77,1,0,0,0,18,85,1,0,0,0,20,95,1,0,0,0,22,122,1,0,0,0,24,124,1,
  	0,0,0,26,32,5,18,0,0,27,28,5,1,0,0,28,29,3,20,10,0,29,30,5,2,0,0,30,32,
  	1,0,0,0,31,26,1,0,0,0,31,27,1,0,0,0,32,1,1,0,0,0,33,38,3,0,0,0,34,35,
  	3,4,2,0,35,36,3,6,3,0,36,38,1,0,0,0,37,33,1,0,0,0,37,34,1,0,0,0,38,3,
  	1,0,0,0,39,40,7,0,0,0,40,5,1,0,0,0,41,44,3,2,1,0,42,44,5,18,0,0,43,41,
  	1,0,0,0,43,42,1,0,0,0,44,7,1,0,0,0,45,50,3,6,3,0,46,47,7,1,0,0,47,49,
  	3,6,3,0,48,46,1,0,0,0,49,52,1,0,0,0,50,48,1,0,0,0,50,51,1,0,0,0,51,9,
  	1,0,0,0,52,50,1,0,0,0,53,58,3,8,4,0,54,55,7,2,0,0,55,57,3,8,4,0,56,54,
  	1,0,0,0,57,60,1,0,0,0,58,56,1,0,0,0,58,59,1,0,0,0,59,11,1,0,0,0,60,58,
  	1,0,0,0,61,66,3,10,5,0,62,63,7,3,0,0,63,65,3,10,5,0,64,62,1,0,0,0,65,
  	68,1,0,0,0,66,64,1,0,0,0,66,67,1,0,0,0,67,13,1,0,0,0,68,66,1,0,0,0,69,
  	74,3,12,6,0,70,71,5,10,0,0,71,73,3,12,6,0,72,70,1,0,0,0,73,76,1,0,0,0,
  	74,72,1,0,0,0,74,75,1,0,0,0,75,15,1,0,0,0,76,74,1,0,0,0,77,82,3,14,7,
  	0,78,79,5,12,0,0,79,81,3,14,7,0,80,78,1,0,0,0,81,84,1,0,0,0,82,80,1,0,
  	0,0,82,83,1,0,0,0,83,17,1,0,0,0,84,82,1,0,0,0,85,90,3,16,8,0,86,87,5,
  	11,0,0,87,89,3,16,8,0,88,86,1,0,0,0,89,92,1,0,0,0,90,88,1,0,0,0,90,91,
  	1,0,0,0,91,19,1,0,0,0,92,90,1,0,0,0,93,96,5,18,0,0,94,96,3,18,9,0,95,
  	93,1,0,0,0,95,94,1,0,0,0,96,21,1,0,0,0,97,99,3,24,12,0,98,97,1,0,0,0,
  	98,99,1,0,0,0,99,100,1,0,0,0,100,101,5,18,0,0,101,123,5,0,0,1,102,103,
  	3,24,12,0,103,104,5,15,0,0,104,106,1,0,0,0,105,102,1,0,0,0,105,106,1,
  	0,0,0,106,107,1,0,0,0,107,108,3,20,10,0,108,109,5,0,0,1,109,123,1,0,0,
  	0,110,111,3,24,12,0,111,112,5,16,0,0,112,113,3,20,10,0,113,114,5,17,0,
  	0,114,115,5,0,0,1,115,123,1,0,0,0,116,117,3,24,12,0,117,118,5,1,0,0,118,
  	119,3,20,10,0,119,120,5,2,0,0,120,121,5,0,0,1,121,123,1,0,0,0,122,98,
  	1,0,0,0,122,105,1,0,0,0,122,110,1,0,0,0,122,116,1,0,0,0,123,23,1,0,0,
  	0,124,125,7,4,0,0,125,25,1,0,0,0,13,31,37,43,50,58,66,74,82,90,95,98,
  	105,122
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  numcalParserStaticData = std::move(staticData);
}

}

NumCalParser::NumCalParser(TokenStream *input) : NumCalParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

NumCalParser::NumCalParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  NumCalParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *numcalParserStaticData->atn, numcalParserStaticData->decisionToDFA, numcalParserStaticData->sharedContextCache, options);
}

NumCalParser::~NumCalParser() {
  delete _interpreter;
}

const atn::ATN& NumCalParser::getATN() const {
  return *numcalParserStaticData->atn;
}

std::string NumCalParser::getGrammarFileName() const {
  return "NumCal.g4";
}

const std::vector<std::string>& NumCalParser::getRuleNames() const {
  return numcalParserStaticData->ruleNames;
}

const dfa::Vocabulary& NumCalParser::getVocabulary() const {
  return numcalParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView NumCalParser::getSerializedATN() const {
  return numcalParserStaticData->serializedATN;
}


//----------------- PrimaryExpressionContext ------------------------------------------------------------------

NumCalParser::PrimaryExpressionContext::PrimaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* NumCalParser::PrimaryExpressionContext::IntegerConstant() {
  return getToken(NumCalParser::IntegerConstant, 0);
}

tree::TerminalNode* NumCalParser::PrimaryExpressionContext::LeftParen() {
  return getToken(NumCalParser::LeftParen, 0);
}

NumCalParser::AssignmentExpressionContext* NumCalParser::PrimaryExpressionContext::assignmentExpression() {
  return getRuleContext<NumCalParser::AssignmentExpressionContext>(0);
}

tree::TerminalNode* NumCalParser::PrimaryExpressionContext::RightParen() {
  return getToken(NumCalParser::RightParen, 0);
}


size_t NumCalParser::PrimaryExpressionContext::getRuleIndex() const {
  return NumCalParser::RulePrimaryExpression;
}


std::any NumCalParser::PrimaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::PrimaryExpressionContext* NumCalParser::primaryExpression() {
  PrimaryExpressionContext *_localctx = _tracker.createInstance<PrimaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 0, NumCalParser::RulePrimaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(31);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case NumCalParser::IntegerConstant: {
        enterOuterAlt(_localctx, 1);
        setState(26);
        match(NumCalParser::IntegerConstant);
        break;
      }

      case NumCalParser::LeftParen: {
        enterOuterAlt(_localctx, 2);
        setState(27);
        match(NumCalParser::LeftParen);
        setState(28);
        assignmentExpression();
        setState(29);
        match(NumCalParser::RightParen);
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

//----------------- UnaryExpressionContext ------------------------------------------------------------------

NumCalParser::UnaryExpressionContext::UnaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

NumCalParser::PrimaryExpressionContext* NumCalParser::UnaryExpressionContext::primaryExpression() {
  return getRuleContext<NumCalParser::PrimaryExpressionContext>(0);
}

NumCalParser::UnaryOperatorContext* NumCalParser::UnaryExpressionContext::unaryOperator() {
  return getRuleContext<NumCalParser::UnaryOperatorContext>(0);
}

NumCalParser::CastExpressionContext* NumCalParser::UnaryExpressionContext::castExpression() {
  return getRuleContext<NumCalParser::CastExpressionContext>(0);
}


size_t NumCalParser::UnaryExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleUnaryExpression;
}


std::any NumCalParser::UnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::UnaryExpressionContext* NumCalParser::unaryExpression() {
  UnaryExpressionContext *_localctx = _tracker.createInstance<UnaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 2, NumCalParser::RuleUnaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(37);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case NumCalParser::LeftParen:
      case NumCalParser::IntegerConstant: {
        enterOuterAlt(_localctx, 1);
        setState(33);
        primaryExpression();
        break;
      }

      case NumCalParser::Plus:
      case NumCalParser::Minus:
      case NumCalParser::Tilde: {
        enterOuterAlt(_localctx, 2);
        setState(34);
        unaryOperator();
        setState(35);
        castExpression();
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

//----------------- UnaryOperatorContext ------------------------------------------------------------------

NumCalParser::UnaryOperatorContext::UnaryOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* NumCalParser::UnaryOperatorContext::Plus() {
  return getToken(NumCalParser::Plus, 0);
}

tree::TerminalNode* NumCalParser::UnaryOperatorContext::Minus() {
  return getToken(NumCalParser::Minus, 0);
}

tree::TerminalNode* NumCalParser::UnaryOperatorContext::Tilde() {
  return getToken(NumCalParser::Tilde, 0);
}


size_t NumCalParser::UnaryOperatorContext::getRuleIndex() const {
  return NumCalParser::RuleUnaryOperator;
}


std::any NumCalParser::UnaryOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitUnaryOperator(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::UnaryOperatorContext* NumCalParser::unaryOperator() {
  UnaryOperatorContext *_localctx = _tracker.createInstance<UnaryOperatorContext>(_ctx, getState());
  enterRule(_localctx, 4, NumCalParser::RuleUnaryOperator);
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
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 8288) != 0))) {
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

NumCalParser::CastExpressionContext::CastExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

NumCalParser::UnaryExpressionContext* NumCalParser::CastExpressionContext::unaryExpression() {
  return getRuleContext<NumCalParser::UnaryExpressionContext>(0);
}

tree::TerminalNode* NumCalParser::CastExpressionContext::IntegerConstant() {
  return getToken(NumCalParser::IntegerConstant, 0);
}


size_t NumCalParser::CastExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleCastExpression;
}


std::any NumCalParser::CastExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitCastExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::CastExpressionContext* NumCalParser::castExpression() {
  CastExpressionContext *_localctx = _tracker.createInstance<CastExpressionContext>(_ctx, getState());
  enterRule(_localctx, 6, NumCalParser::RuleCastExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(43);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(41);
      unaryExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(42);
      match(NumCalParser::IntegerConstant);
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

NumCalParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<NumCalParser::CastExpressionContext *> NumCalParser::MultiplicativeExpressionContext::castExpression() {
  return getRuleContexts<NumCalParser::CastExpressionContext>();
}

NumCalParser::CastExpressionContext* NumCalParser::MultiplicativeExpressionContext::castExpression(size_t i) {
  return getRuleContext<NumCalParser::CastExpressionContext>(i);
}

std::vector<tree::TerminalNode *> NumCalParser::MultiplicativeExpressionContext::Star() {
  return getTokens(NumCalParser::Star);
}

tree::TerminalNode* NumCalParser::MultiplicativeExpressionContext::Star(size_t i) {
  return getToken(NumCalParser::Star, i);
}

std::vector<tree::TerminalNode *> NumCalParser::MultiplicativeExpressionContext::Div() {
  return getTokens(NumCalParser::Div);
}

tree::TerminalNode* NumCalParser::MultiplicativeExpressionContext::Div(size_t i) {
  return getToken(NumCalParser::Div, i);
}

std::vector<tree::TerminalNode *> NumCalParser::MultiplicativeExpressionContext::Mod() {
  return getTokens(NumCalParser::Mod);
}

tree::TerminalNode* NumCalParser::MultiplicativeExpressionContext::Mod(size_t i) {
  return getToken(NumCalParser::Mod, i);
}


size_t NumCalParser::MultiplicativeExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleMultiplicativeExpression;
}


std::any NumCalParser::MultiplicativeExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::MultiplicativeExpressionContext* NumCalParser::multiplicativeExpression() {
  MultiplicativeExpressionContext *_localctx = _tracker.createInstance<MultiplicativeExpressionContext>(_ctx, getState());
  enterRule(_localctx, 8, NumCalParser::RuleMultiplicativeExpression);
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
    setState(45);
    castExpression();
    setState(50);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 896) != 0)) {
      setState(46);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 896) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(47);
      castExpression();
      setState(52);
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

NumCalParser::AdditiveExpressionContext::AdditiveExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<NumCalParser::MultiplicativeExpressionContext *> NumCalParser::AdditiveExpressionContext::multiplicativeExpression() {
  return getRuleContexts<NumCalParser::MultiplicativeExpressionContext>();
}

NumCalParser::MultiplicativeExpressionContext* NumCalParser::AdditiveExpressionContext::multiplicativeExpression(size_t i) {
  return getRuleContext<NumCalParser::MultiplicativeExpressionContext>(i);
}

std::vector<tree::TerminalNode *> NumCalParser::AdditiveExpressionContext::Plus() {
  return getTokens(NumCalParser::Plus);
}

tree::TerminalNode* NumCalParser::AdditiveExpressionContext::Plus(size_t i) {
  return getToken(NumCalParser::Plus, i);
}

std::vector<tree::TerminalNode *> NumCalParser::AdditiveExpressionContext::Minus() {
  return getTokens(NumCalParser::Minus);
}

tree::TerminalNode* NumCalParser::AdditiveExpressionContext::Minus(size_t i) {
  return getToken(NumCalParser::Minus, i);
}


size_t NumCalParser::AdditiveExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleAdditiveExpression;
}


std::any NumCalParser::AdditiveExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::AdditiveExpressionContext* NumCalParser::additiveExpression() {
  AdditiveExpressionContext *_localctx = _tracker.createInstance<AdditiveExpressionContext>(_ctx, getState());
  enterRule(_localctx, 10, NumCalParser::RuleAdditiveExpression);
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
    setState(53);
    multiplicativeExpression();
    setState(58);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == NumCalParser::Plus

    || _la == NumCalParser::Minus) {
      setState(54);
      _la = _input->LA(1);
      if (!(_la == NumCalParser::Plus

      || _la == NumCalParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(55);
      multiplicativeExpression();
      setState(60);
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

NumCalParser::ShiftExpressionContext::ShiftExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<NumCalParser::AdditiveExpressionContext *> NumCalParser::ShiftExpressionContext::additiveExpression() {
  return getRuleContexts<NumCalParser::AdditiveExpressionContext>();
}

NumCalParser::AdditiveExpressionContext* NumCalParser::ShiftExpressionContext::additiveExpression(size_t i) {
  return getRuleContext<NumCalParser::AdditiveExpressionContext>(i);
}

std::vector<tree::TerminalNode *> NumCalParser::ShiftExpressionContext::LeftShift() {
  return getTokens(NumCalParser::LeftShift);
}

tree::TerminalNode* NumCalParser::ShiftExpressionContext::LeftShift(size_t i) {
  return getToken(NumCalParser::LeftShift, i);
}

std::vector<tree::TerminalNode *> NumCalParser::ShiftExpressionContext::RightShift() {
  return getTokens(NumCalParser::RightShift);
}

tree::TerminalNode* NumCalParser::ShiftExpressionContext::RightShift(size_t i) {
  return getToken(NumCalParser::RightShift, i);
}


size_t NumCalParser::ShiftExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleShiftExpression;
}


std::any NumCalParser::ShiftExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitShiftExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::ShiftExpressionContext* NumCalParser::shiftExpression() {
  ShiftExpressionContext *_localctx = _tracker.createInstance<ShiftExpressionContext>(_ctx, getState());
  enterRule(_localctx, 12, NumCalParser::RuleShiftExpression);
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
    setState(61);
    additiveExpression();
    setState(66);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == NumCalParser::LeftShift

    || _la == NumCalParser::RightShift) {
      setState(62);
      _la = _input->LA(1);
      if (!(_la == NumCalParser::LeftShift

      || _la == NumCalParser::RightShift)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(63);
      additiveExpression();
      setState(68);
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

NumCalParser::AndExpressionContext::AndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<NumCalParser::ShiftExpressionContext *> NumCalParser::AndExpressionContext::shiftExpression() {
  return getRuleContexts<NumCalParser::ShiftExpressionContext>();
}

NumCalParser::ShiftExpressionContext* NumCalParser::AndExpressionContext::shiftExpression(size_t i) {
  return getRuleContext<NumCalParser::ShiftExpressionContext>(i);
}

std::vector<tree::TerminalNode *> NumCalParser::AndExpressionContext::And() {
  return getTokens(NumCalParser::And);
}

tree::TerminalNode* NumCalParser::AndExpressionContext::And(size_t i) {
  return getToken(NumCalParser::And, i);
}


size_t NumCalParser::AndExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleAndExpression;
}


std::any NumCalParser::AndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitAndExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::AndExpressionContext* NumCalParser::andExpression() {
  AndExpressionContext *_localctx = _tracker.createInstance<AndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 14, NumCalParser::RuleAndExpression);
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
    setState(69);
    shiftExpression();
    setState(74);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == NumCalParser::And) {
      setState(70);
      match(NumCalParser::And);
      setState(71);
      shiftExpression();
      setState(76);
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

NumCalParser::ExclusiveOrExpressionContext::ExclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<NumCalParser::AndExpressionContext *> NumCalParser::ExclusiveOrExpressionContext::andExpression() {
  return getRuleContexts<NumCalParser::AndExpressionContext>();
}

NumCalParser::AndExpressionContext* NumCalParser::ExclusiveOrExpressionContext::andExpression(size_t i) {
  return getRuleContext<NumCalParser::AndExpressionContext>(i);
}

std::vector<tree::TerminalNode *> NumCalParser::ExclusiveOrExpressionContext::Caret() {
  return getTokens(NumCalParser::Caret);
}

tree::TerminalNode* NumCalParser::ExclusiveOrExpressionContext::Caret(size_t i) {
  return getToken(NumCalParser::Caret, i);
}


size_t NumCalParser::ExclusiveOrExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleExclusiveOrExpression;
}


std::any NumCalParser::ExclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitExclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::ExclusiveOrExpressionContext* NumCalParser::exclusiveOrExpression() {
  ExclusiveOrExpressionContext *_localctx = _tracker.createInstance<ExclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 16, NumCalParser::RuleExclusiveOrExpression);
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
    setState(77);
    andExpression();
    setState(82);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == NumCalParser::Caret) {
      setState(78);
      match(NumCalParser::Caret);
      setState(79);
      andExpression();
      setState(84);
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

NumCalParser::InclusiveOrExpressionContext::InclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<NumCalParser::ExclusiveOrExpressionContext *> NumCalParser::InclusiveOrExpressionContext::exclusiveOrExpression() {
  return getRuleContexts<NumCalParser::ExclusiveOrExpressionContext>();
}

NumCalParser::ExclusiveOrExpressionContext* NumCalParser::InclusiveOrExpressionContext::exclusiveOrExpression(size_t i) {
  return getRuleContext<NumCalParser::ExclusiveOrExpressionContext>(i);
}

std::vector<tree::TerminalNode *> NumCalParser::InclusiveOrExpressionContext::Or() {
  return getTokens(NumCalParser::Or);
}

tree::TerminalNode* NumCalParser::InclusiveOrExpressionContext::Or(size_t i) {
  return getToken(NumCalParser::Or, i);
}


size_t NumCalParser::InclusiveOrExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleInclusiveOrExpression;
}


std::any NumCalParser::InclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitInclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::InclusiveOrExpressionContext* NumCalParser::inclusiveOrExpression() {
  InclusiveOrExpressionContext *_localctx = _tracker.createInstance<InclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 18, NumCalParser::RuleInclusiveOrExpression);
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
    setState(85);
    exclusiveOrExpression();
    setState(90);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == NumCalParser::Or) {
      setState(86);
      match(NumCalParser::Or);
      setState(87);
      exclusiveOrExpression();
      setState(92);
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

NumCalParser::AssignmentExpressionContext::AssignmentExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* NumCalParser::AssignmentExpressionContext::IntegerConstant() {
  return getToken(NumCalParser::IntegerConstant, 0);
}

NumCalParser::InclusiveOrExpressionContext* NumCalParser::AssignmentExpressionContext::inclusiveOrExpression() {
  return getRuleContext<NumCalParser::InclusiveOrExpressionContext>(0);
}


size_t NumCalParser::AssignmentExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleAssignmentExpression;
}


std::any NumCalParser::AssignmentExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitAssignmentExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::AssignmentExpressionContext* NumCalParser::assignmentExpression() {
  AssignmentExpressionContext *_localctx = _tracker.createInstance<AssignmentExpressionContext>(_ctx, getState());
  enterRule(_localctx, 20, NumCalParser::RuleAssignmentExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(95);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(93);
      match(NumCalParser::IntegerConstant);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(94);
      inclusiveOrExpression();
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

//----------------- EntryExpressionContext ------------------------------------------------------------------

NumCalParser::EntryExpressionContext::EntryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* NumCalParser::EntryExpressionContext::IntegerConstant() {
  return getToken(NumCalParser::IntegerConstant, 0);
}

tree::TerminalNode* NumCalParser::EntryExpressionContext::EOF() {
  return getToken(NumCalParser::EOF, 0);
}

NumCalParser::PrefixGotoContext* NumCalParser::EntryExpressionContext::prefixGoto() {
  return getRuleContext<NumCalParser::PrefixGotoContext>(0);
}

NumCalParser::AssignmentExpressionContext* NumCalParser::EntryExpressionContext::assignmentExpression() {
  return getRuleContext<NumCalParser::AssignmentExpressionContext>(0);
}

tree::TerminalNode* NumCalParser::EntryExpressionContext::Colon() {
  return getToken(NumCalParser::Colon, 0);
}

tree::TerminalNode* NumCalParser::EntryExpressionContext::LeftBracket() {
  return getToken(NumCalParser::LeftBracket, 0);
}

tree::TerminalNode* NumCalParser::EntryExpressionContext::RightBracket() {
  return getToken(NumCalParser::RightBracket, 0);
}

tree::TerminalNode* NumCalParser::EntryExpressionContext::LeftParen() {
  return getToken(NumCalParser::LeftParen, 0);
}

tree::TerminalNode* NumCalParser::EntryExpressionContext::RightParen() {
  return getToken(NumCalParser::RightParen, 0);
}


size_t NumCalParser::EntryExpressionContext::getRuleIndex() const {
  return NumCalParser::RuleEntryExpression;
}


std::any NumCalParser::EntryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitEntryExpression(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::EntryExpressionContext* NumCalParser::entryExpression() {
  EntryExpressionContext *_localctx = _tracker.createInstance<EntryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 22, NumCalParser::RuleEntryExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(122);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(98);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 16480) != 0)) {
        setState(97);
        prefixGoto();
      }
      setState(100);
      match(NumCalParser::IntegerConstant);
      setState(101);
      match(NumCalParser::EOF);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(105);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx)) {
      case 1: {
        setState(102);
        prefixGoto();
        setState(103);
        match(NumCalParser::Colon);
        break;
      }

      default:
        break;
      }
      setState(107);
      assignmentExpression();
      setState(108);
      match(NumCalParser::EOF);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(110);
      prefixGoto();
      setState(111);
      match(NumCalParser::LeftBracket);
      setState(112);
      assignmentExpression();
      setState(113);
      match(NumCalParser::RightBracket);
      setState(114);
      match(NumCalParser::EOF);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(116);
      prefixGoto();
      setState(117);
      match(NumCalParser::LeftParen);
      setState(118);
      assignmentExpression();
      setState(119);
      match(NumCalParser::RightParen);
      setState(120);
      match(NumCalParser::EOF);
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

//----------------- PrefixGotoContext ------------------------------------------------------------------

NumCalParser::PrefixGotoContext::PrefixGotoContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* NumCalParser::PrefixGotoContext::Plus() {
  return getToken(NumCalParser::Plus, 0);
}

tree::TerminalNode* NumCalParser::PrefixGotoContext::Minus() {
  return getToken(NumCalParser::Minus, 0);
}

tree::TerminalNode* NumCalParser::PrefixGotoContext::LessThan() {
  return getToken(NumCalParser::LessThan, 0);
}


size_t NumCalParser::PrefixGotoContext::getRuleIndex() const {
  return NumCalParser::RulePrefixGoto;
}


std::any NumCalParser::PrefixGotoContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<NumCalVisitor*>(visitor))
    return parserVisitor->visitPrefixGoto(this);
  else
    return visitor->visitChildren(this);
}

NumCalParser::PrefixGotoContext* NumCalParser::prefixGoto() {
  PrefixGotoContext *_localctx = _tracker.createInstance<PrefixGotoContext>(_ctx, getState());
  enterRule(_localctx, 24, NumCalParser::RulePrefixGoto);
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
    setState(124);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 16480) != 0))) {
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

void NumCalParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  numcalParserInitialize();
#else
  ::antlr4::internal::call_once(numcalParserOnceFlag, numcalParserInitialize);
#endif
}
