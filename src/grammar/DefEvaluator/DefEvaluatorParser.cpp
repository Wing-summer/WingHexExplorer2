
// Generated from DefEvaluator.g4 by ANTLR 4.13.2


#include "DefEvaluatorVisitor.h"

#include "DefEvaluatorParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct DefEvaluatorParserStaticData final {
  DefEvaluatorParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  DefEvaluatorParserStaticData(const DefEvaluatorParserStaticData&) = delete;
  DefEvaluatorParserStaticData(DefEvaluatorParserStaticData&&) = delete;
  DefEvaluatorParserStaticData& operator=(const DefEvaluatorParserStaticData&) = delete;
  DefEvaluatorParserStaticData& operator=(DefEvaluatorParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag defevaluatorParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<DefEvaluatorParserStaticData> defevaluatorParserStaticData = nullptr;

void defevaluatorParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (defevaluatorParserStaticData != nullptr) {
    return;
  }
#else
  assert(defevaluatorParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<DefEvaluatorParserStaticData>(
    std::vector<std::string>{
      "primaryExpression", "unaryExpression", "unaryOperator", "castExpression", 
      "relationalExpression", "equalityExpression", "multiplicativeExpression", 
      "additiveExpression", "shiftExpression", "andExpression", "exclusiveOrExpression", 
      "inclusiveOrExpression", "logicalAndExpression", "logicalOrExpression", 
      "conditionalExpression", "assignmentExpression", "entryExpression"
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
  	4,1,27,153,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,1,0,1,0,1,0,1,0,1,0,3,0,40,8,0,1,1,1,1,1,1,1,1,
  	3,1,46,8,1,1,2,1,2,1,3,1,3,3,3,52,8,3,1,4,1,4,1,4,5,4,57,8,4,10,4,12,
  	4,60,9,4,1,5,1,5,1,5,5,5,65,8,5,10,5,12,5,68,9,5,1,6,1,6,1,6,5,6,73,8,
  	6,10,6,12,6,76,9,6,1,7,1,7,1,7,5,7,81,8,7,10,7,12,7,84,9,7,1,8,1,8,1,
  	8,5,8,89,8,8,10,8,12,8,92,9,8,1,9,1,9,1,9,5,9,97,8,9,10,9,12,9,100,9,
  	9,1,10,1,10,1,10,5,10,105,8,10,10,10,12,10,108,9,10,1,11,1,11,1,11,5,
  	11,113,8,11,10,11,12,11,116,9,11,1,12,1,12,1,12,5,12,121,8,12,10,12,12,
  	12,124,9,12,1,13,1,13,1,13,5,13,129,8,13,10,13,12,13,132,9,13,1,14,1,
  	14,1,14,1,14,1,14,1,14,3,14,140,8,14,1,15,1,15,3,15,144,8,15,1,16,1,16,
  	1,16,1,16,1,16,3,16,151,8,16,1,16,0,0,17,0,2,4,6,8,10,12,14,16,18,20,
  	22,24,26,28,30,32,0,6,2,0,14,15,22,23,2,0,1,3,24,24,1,0,4,5,1,0,16,18,
  	1,0,14,15,1,0,12,13,151,0,39,1,0,0,0,2,45,1,0,0,0,4,47,1,0,0,0,6,51,1,
  	0,0,0,8,53,1,0,0,0,10,61,1,0,0,0,12,69,1,0,0,0,14,77,1,0,0,0,16,85,1,
  	0,0,0,18,93,1,0,0,0,20,101,1,0,0,0,22,109,1,0,0,0,24,117,1,0,0,0,26,125,
  	1,0,0,0,28,133,1,0,0,0,30,143,1,0,0,0,32,150,1,0,0,0,34,40,5,25,0,0,35,
  	36,5,10,0,0,36,37,3,30,15,0,37,38,5,11,0,0,38,40,1,0,0,0,39,34,1,0,0,
  	0,39,35,1,0,0,0,40,1,1,0,0,0,41,46,3,0,0,0,42,43,3,4,2,0,43,44,3,6,3,
  	0,44,46,1,0,0,0,45,41,1,0,0,0,45,42,1,0,0,0,46,3,1,0,0,0,47,48,7,0,0,
  	0,48,5,1,0,0,0,49,52,3,2,1,0,50,52,5,25,0,0,51,49,1,0,0,0,51,50,1,0,0,
  	0,52,7,1,0,0,0,53,58,3,16,8,0,54,55,7,1,0,0,55,57,3,16,8,0,56,54,1,0,
  	0,0,57,60,1,0,0,0,58,56,1,0,0,0,58,59,1,0,0,0,59,9,1,0,0,0,60,58,1,0,
  	0,0,61,66,3,8,4,0,62,63,7,2,0,0,63,65,3,8,4,0,64,62,1,0,0,0,65,68,1,0,
  	0,0,66,64,1,0,0,0,66,67,1,0,0,0,67,11,1,0,0,0,68,66,1,0,0,0,69,74,3,6,
  	3,0,70,71,7,3,0,0,71,73,3,6,3,0,72,70,1,0,0,0,73,76,1,0,0,0,74,72,1,0,
  	0,0,74,75,1,0,0,0,75,13,1,0,0,0,76,74,1,0,0,0,77,82,3,12,6,0,78,79,7,
  	4,0,0,79,81,3,12,6,0,80,78,1,0,0,0,81,84,1,0,0,0,82,80,1,0,0,0,82,83,
  	1,0,0,0,83,15,1,0,0,0,84,82,1,0,0,0,85,90,3,14,7,0,86,87,7,5,0,0,87,89,
  	3,14,7,0,88,86,1,0,0,0,89,92,1,0,0,0,90,88,1,0,0,0,90,91,1,0,0,0,91,17,
  	1,0,0,0,92,90,1,0,0,0,93,98,3,16,8,0,94,95,5,19,0,0,95,97,3,16,8,0,96,
  	94,1,0,0,0,97,100,1,0,0,0,98,96,1,0,0,0,98,99,1,0,0,0,99,19,1,0,0,0,100,
  	98,1,0,0,0,101,106,3,18,9,0,102,103,5,21,0,0,103,105,3,18,9,0,104,102,
  	1,0,0,0,105,108,1,0,0,0,106,104,1,0,0,0,106,107,1,0,0,0,107,21,1,0,0,
  	0,108,106,1,0,0,0,109,114,3,20,10,0,110,111,5,20,0,0,111,113,3,20,10,
  	0,112,110,1,0,0,0,113,116,1,0,0,0,114,112,1,0,0,0,114,115,1,0,0,0,115,
  	23,1,0,0,0,116,114,1,0,0,0,117,122,3,22,11,0,118,119,5,6,0,0,119,121,
  	3,22,11,0,120,118,1,0,0,0,121,124,1,0,0,0,122,120,1,0,0,0,122,123,1,0,
  	0,0,123,25,1,0,0,0,124,122,1,0,0,0,125,130,3,24,12,0,126,127,5,7,0,0,
  	127,129,3,24,12,0,128,126,1,0,0,0,129,132,1,0,0,0,130,128,1,0,0,0,130,
  	131,1,0,0,0,131,27,1,0,0,0,132,130,1,0,0,0,133,139,3,26,13,0,134,135,
  	5,8,0,0,135,136,3,0,0,0,136,137,5,9,0,0,137,138,3,28,14,0,138,140,1,0,
  	0,0,139,134,1,0,0,0,139,140,1,0,0,0,140,29,1,0,0,0,141,144,3,28,14,0,
  	142,144,3,22,11,0,143,141,1,0,0,0,143,142,1,0,0,0,144,31,1,0,0,0,145,
  	146,5,25,0,0,146,151,5,0,0,1,147,148,3,30,15,0,148,149,5,0,0,1,149,151,
  	1,0,0,0,150,145,1,0,0,0,150,147,1,0,0,0,151,33,1,0,0,0,16,39,45,51,58,
  	66,74,82,90,98,106,114,122,130,139,143,150
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  defevaluatorParserStaticData = std::move(staticData);
}

}

DefEvaluatorParser::DefEvaluatorParser(TokenStream *input) : DefEvaluatorParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

DefEvaluatorParser::DefEvaluatorParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  DefEvaluatorParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *defevaluatorParserStaticData->atn, defevaluatorParserStaticData->decisionToDFA, defevaluatorParserStaticData->sharedContextCache, options);
}

DefEvaluatorParser::~DefEvaluatorParser() {
  delete _interpreter;
}

const atn::ATN& DefEvaluatorParser::getATN() const {
  return *defevaluatorParserStaticData->atn;
}

std::string DefEvaluatorParser::getGrammarFileName() const {
  return "DefEvaluator.g4";
}

const std::vector<std::string>& DefEvaluatorParser::getRuleNames() const {
  return defevaluatorParserStaticData->ruleNames;
}

const dfa::Vocabulary& DefEvaluatorParser::getVocabulary() const {
  return defevaluatorParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView DefEvaluatorParser::getSerializedATN() const {
  return defevaluatorParserStaticData->serializedATN;
}


//----------------- PrimaryExpressionContext ------------------------------------------------------------------

DefEvaluatorParser::PrimaryExpressionContext::PrimaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* DefEvaluatorParser::PrimaryExpressionContext::Constant() {
  return getToken(DefEvaluatorParser::Constant, 0);
}

tree::TerminalNode* DefEvaluatorParser::PrimaryExpressionContext::LeftParen() {
  return getToken(DefEvaluatorParser::LeftParen, 0);
}

DefEvaluatorParser::AssignmentExpressionContext* DefEvaluatorParser::PrimaryExpressionContext::assignmentExpression() {
  return getRuleContext<DefEvaluatorParser::AssignmentExpressionContext>(0);
}

tree::TerminalNode* DefEvaluatorParser::PrimaryExpressionContext::RightParen() {
  return getToken(DefEvaluatorParser::RightParen, 0);
}


size_t DefEvaluatorParser::PrimaryExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RulePrimaryExpression;
}


std::any DefEvaluatorParser::PrimaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::PrimaryExpressionContext* DefEvaluatorParser::primaryExpression() {
  PrimaryExpressionContext *_localctx = _tracker.createInstance<PrimaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 0, DefEvaluatorParser::RulePrimaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(39);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case DefEvaluatorParser::Constant: {
        enterOuterAlt(_localctx, 1);
        setState(34);
        match(DefEvaluatorParser::Constant);
        break;
      }

      case DefEvaluatorParser::LeftParen: {
        enterOuterAlt(_localctx, 2);
        setState(35);
        match(DefEvaluatorParser::LeftParen);
        setState(36);
        assignmentExpression();
        setState(37);
        match(DefEvaluatorParser::RightParen);
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

DefEvaluatorParser::UnaryExpressionContext::UnaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

DefEvaluatorParser::PrimaryExpressionContext* DefEvaluatorParser::UnaryExpressionContext::primaryExpression() {
  return getRuleContext<DefEvaluatorParser::PrimaryExpressionContext>(0);
}

DefEvaluatorParser::UnaryOperatorContext* DefEvaluatorParser::UnaryExpressionContext::unaryOperator() {
  return getRuleContext<DefEvaluatorParser::UnaryOperatorContext>(0);
}

DefEvaluatorParser::CastExpressionContext* DefEvaluatorParser::UnaryExpressionContext::castExpression() {
  return getRuleContext<DefEvaluatorParser::CastExpressionContext>(0);
}


size_t DefEvaluatorParser::UnaryExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleUnaryExpression;
}


std::any DefEvaluatorParser::UnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::UnaryExpressionContext* DefEvaluatorParser::unaryExpression() {
  UnaryExpressionContext *_localctx = _tracker.createInstance<UnaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 2, DefEvaluatorParser::RuleUnaryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(45);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case DefEvaluatorParser::LeftParen:
      case DefEvaluatorParser::Constant: {
        enterOuterAlt(_localctx, 1);
        setState(41);
        primaryExpression();
        break;
      }

      case DefEvaluatorParser::Plus:
      case DefEvaluatorParser::Minus:
      case DefEvaluatorParser::Not:
      case DefEvaluatorParser::Tilde: {
        enterOuterAlt(_localctx, 2);
        setState(42);
        unaryOperator();
        setState(43);
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

DefEvaluatorParser::UnaryOperatorContext::UnaryOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* DefEvaluatorParser::UnaryOperatorContext::Plus() {
  return getToken(DefEvaluatorParser::Plus, 0);
}

tree::TerminalNode* DefEvaluatorParser::UnaryOperatorContext::Minus() {
  return getToken(DefEvaluatorParser::Minus, 0);
}

tree::TerminalNode* DefEvaluatorParser::UnaryOperatorContext::Tilde() {
  return getToken(DefEvaluatorParser::Tilde, 0);
}

tree::TerminalNode* DefEvaluatorParser::UnaryOperatorContext::Not() {
  return getToken(DefEvaluatorParser::Not, 0);
}


size_t DefEvaluatorParser::UnaryOperatorContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleUnaryOperator;
}


std::any DefEvaluatorParser::UnaryOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitUnaryOperator(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::UnaryOperatorContext* DefEvaluatorParser::unaryOperator() {
  UnaryOperatorContext *_localctx = _tracker.createInstance<UnaryOperatorContext>(_ctx, getState());
  enterRule(_localctx, 4, DefEvaluatorParser::RuleUnaryOperator);
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
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 12632064) != 0))) {
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

DefEvaluatorParser::CastExpressionContext::CastExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

DefEvaluatorParser::UnaryExpressionContext* DefEvaluatorParser::CastExpressionContext::unaryExpression() {
  return getRuleContext<DefEvaluatorParser::UnaryExpressionContext>(0);
}

tree::TerminalNode* DefEvaluatorParser::CastExpressionContext::Constant() {
  return getToken(DefEvaluatorParser::Constant, 0);
}


size_t DefEvaluatorParser::CastExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleCastExpression;
}


std::any DefEvaluatorParser::CastExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitCastExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::CastExpressionContext* DefEvaluatorParser::castExpression() {
  CastExpressionContext *_localctx = _tracker.createInstance<CastExpressionContext>(_ctx, getState());
  enterRule(_localctx, 6, DefEvaluatorParser::RuleCastExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(51);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(49);
      unaryExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(50);
      match(DefEvaluatorParser::Constant);
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

//----------------- RelationalExpressionContext ------------------------------------------------------------------

DefEvaluatorParser::RelationalExpressionContext::RelationalExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::ShiftExpressionContext *> DefEvaluatorParser::RelationalExpressionContext::shiftExpression() {
  return getRuleContexts<DefEvaluatorParser::ShiftExpressionContext>();
}

DefEvaluatorParser::ShiftExpressionContext* DefEvaluatorParser::RelationalExpressionContext::shiftExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::ShiftExpressionContext>(i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::RelationalExpressionContext::LessThan() {
  return getTokens(DefEvaluatorParser::LessThan);
}

tree::TerminalNode* DefEvaluatorParser::RelationalExpressionContext::LessThan(size_t i) {
  return getToken(DefEvaluatorParser::LessThan, i);
}


size_t DefEvaluatorParser::RelationalExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleRelationalExpression;
}


std::any DefEvaluatorParser::RelationalExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitRelationalExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::RelationalExpressionContext* DefEvaluatorParser::relationalExpression() {
  RelationalExpressionContext *_localctx = _tracker.createInstance<RelationalExpressionContext>(_ctx, getState());
  enterRule(_localctx, 8, DefEvaluatorParser::RuleRelationalExpression);
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
    shiftExpression();
    setState(58);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 16777230) != 0)) {
      setState(54);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 16777230) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(55);
      shiftExpression();
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

//----------------- EqualityExpressionContext ------------------------------------------------------------------

DefEvaluatorParser::EqualityExpressionContext::EqualityExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::RelationalExpressionContext *> DefEvaluatorParser::EqualityExpressionContext::relationalExpression() {
  return getRuleContexts<DefEvaluatorParser::RelationalExpressionContext>();
}

DefEvaluatorParser::RelationalExpressionContext* DefEvaluatorParser::EqualityExpressionContext::relationalExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::RelationalExpressionContext>(i);
}


size_t DefEvaluatorParser::EqualityExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleEqualityExpression;
}


std::any DefEvaluatorParser::EqualityExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitEqualityExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::EqualityExpressionContext* DefEvaluatorParser::equalityExpression() {
  EqualityExpressionContext *_localctx = _tracker.createInstance<EqualityExpressionContext>(_ctx, getState());
  enterRule(_localctx, 10, DefEvaluatorParser::RuleEqualityExpression);
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
    relationalExpression();
    setState(66);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::T__3

    || _la == DefEvaluatorParser::T__4) {
      setState(62);
      _la = _input->LA(1);
      if (!(_la == DefEvaluatorParser::T__3

      || _la == DefEvaluatorParser::T__4)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(63);
      relationalExpression();
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

//----------------- MultiplicativeExpressionContext ------------------------------------------------------------------

DefEvaluatorParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::CastExpressionContext *> DefEvaluatorParser::MultiplicativeExpressionContext::castExpression() {
  return getRuleContexts<DefEvaluatorParser::CastExpressionContext>();
}

DefEvaluatorParser::CastExpressionContext* DefEvaluatorParser::MultiplicativeExpressionContext::castExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::CastExpressionContext>(i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::MultiplicativeExpressionContext::Star() {
  return getTokens(DefEvaluatorParser::Star);
}

tree::TerminalNode* DefEvaluatorParser::MultiplicativeExpressionContext::Star(size_t i) {
  return getToken(DefEvaluatorParser::Star, i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::MultiplicativeExpressionContext::Div() {
  return getTokens(DefEvaluatorParser::Div);
}

tree::TerminalNode* DefEvaluatorParser::MultiplicativeExpressionContext::Div(size_t i) {
  return getToken(DefEvaluatorParser::Div, i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::MultiplicativeExpressionContext::Mod() {
  return getTokens(DefEvaluatorParser::Mod);
}

tree::TerminalNode* DefEvaluatorParser::MultiplicativeExpressionContext::Mod(size_t i) {
  return getToken(DefEvaluatorParser::Mod, i);
}


size_t DefEvaluatorParser::MultiplicativeExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleMultiplicativeExpression;
}


std::any DefEvaluatorParser::MultiplicativeExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::MultiplicativeExpressionContext* DefEvaluatorParser::multiplicativeExpression() {
  MultiplicativeExpressionContext *_localctx = _tracker.createInstance<MultiplicativeExpressionContext>(_ctx, getState());
  enterRule(_localctx, 12, DefEvaluatorParser::RuleMultiplicativeExpression);
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
    castExpression();
    setState(74);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 458752) != 0)) {
      setState(70);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 458752) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(71);
      castExpression();
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

//----------------- AdditiveExpressionContext ------------------------------------------------------------------

DefEvaluatorParser::AdditiveExpressionContext::AdditiveExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::MultiplicativeExpressionContext *> DefEvaluatorParser::AdditiveExpressionContext::multiplicativeExpression() {
  return getRuleContexts<DefEvaluatorParser::MultiplicativeExpressionContext>();
}

DefEvaluatorParser::MultiplicativeExpressionContext* DefEvaluatorParser::AdditiveExpressionContext::multiplicativeExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::MultiplicativeExpressionContext>(i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::AdditiveExpressionContext::Plus() {
  return getTokens(DefEvaluatorParser::Plus);
}

tree::TerminalNode* DefEvaluatorParser::AdditiveExpressionContext::Plus(size_t i) {
  return getToken(DefEvaluatorParser::Plus, i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::AdditiveExpressionContext::Minus() {
  return getTokens(DefEvaluatorParser::Minus);
}

tree::TerminalNode* DefEvaluatorParser::AdditiveExpressionContext::Minus(size_t i) {
  return getToken(DefEvaluatorParser::Minus, i);
}


size_t DefEvaluatorParser::AdditiveExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleAdditiveExpression;
}


std::any DefEvaluatorParser::AdditiveExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::AdditiveExpressionContext* DefEvaluatorParser::additiveExpression() {
  AdditiveExpressionContext *_localctx = _tracker.createInstance<AdditiveExpressionContext>(_ctx, getState());
  enterRule(_localctx, 14, DefEvaluatorParser::RuleAdditiveExpression);
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
    multiplicativeExpression();
    setState(82);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::Plus

    || _la == DefEvaluatorParser::Minus) {
      setState(78);
      _la = _input->LA(1);
      if (!(_la == DefEvaluatorParser::Plus

      || _la == DefEvaluatorParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(79);
      multiplicativeExpression();
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

//----------------- ShiftExpressionContext ------------------------------------------------------------------

DefEvaluatorParser::ShiftExpressionContext::ShiftExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::AdditiveExpressionContext *> DefEvaluatorParser::ShiftExpressionContext::additiveExpression() {
  return getRuleContexts<DefEvaluatorParser::AdditiveExpressionContext>();
}

DefEvaluatorParser::AdditiveExpressionContext* DefEvaluatorParser::ShiftExpressionContext::additiveExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::AdditiveExpressionContext>(i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::ShiftExpressionContext::LeftShift() {
  return getTokens(DefEvaluatorParser::LeftShift);
}

tree::TerminalNode* DefEvaluatorParser::ShiftExpressionContext::LeftShift(size_t i) {
  return getToken(DefEvaluatorParser::LeftShift, i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::ShiftExpressionContext::RightShift() {
  return getTokens(DefEvaluatorParser::RightShift);
}

tree::TerminalNode* DefEvaluatorParser::ShiftExpressionContext::RightShift(size_t i) {
  return getToken(DefEvaluatorParser::RightShift, i);
}


size_t DefEvaluatorParser::ShiftExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleShiftExpression;
}


std::any DefEvaluatorParser::ShiftExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitShiftExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::ShiftExpressionContext* DefEvaluatorParser::shiftExpression() {
  ShiftExpressionContext *_localctx = _tracker.createInstance<ShiftExpressionContext>(_ctx, getState());
  enterRule(_localctx, 16, DefEvaluatorParser::RuleShiftExpression);
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
    additiveExpression();
    setState(90);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::LeftShift

    || _la == DefEvaluatorParser::RightShift) {
      setState(86);
      _la = _input->LA(1);
      if (!(_la == DefEvaluatorParser::LeftShift

      || _la == DefEvaluatorParser::RightShift)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(87);
      additiveExpression();
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

//----------------- AndExpressionContext ------------------------------------------------------------------

DefEvaluatorParser::AndExpressionContext::AndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::ShiftExpressionContext *> DefEvaluatorParser::AndExpressionContext::shiftExpression() {
  return getRuleContexts<DefEvaluatorParser::ShiftExpressionContext>();
}

DefEvaluatorParser::ShiftExpressionContext* DefEvaluatorParser::AndExpressionContext::shiftExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::ShiftExpressionContext>(i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::AndExpressionContext::And() {
  return getTokens(DefEvaluatorParser::And);
}

tree::TerminalNode* DefEvaluatorParser::AndExpressionContext::And(size_t i) {
  return getToken(DefEvaluatorParser::And, i);
}


size_t DefEvaluatorParser::AndExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleAndExpression;
}


std::any DefEvaluatorParser::AndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitAndExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::AndExpressionContext* DefEvaluatorParser::andExpression() {
  AndExpressionContext *_localctx = _tracker.createInstance<AndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 18, DefEvaluatorParser::RuleAndExpression);
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
    setState(93);
    shiftExpression();
    setState(98);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::And) {
      setState(94);
      match(DefEvaluatorParser::And);
      setState(95);
      shiftExpression();
      setState(100);
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

DefEvaluatorParser::ExclusiveOrExpressionContext::ExclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::AndExpressionContext *> DefEvaluatorParser::ExclusiveOrExpressionContext::andExpression() {
  return getRuleContexts<DefEvaluatorParser::AndExpressionContext>();
}

DefEvaluatorParser::AndExpressionContext* DefEvaluatorParser::ExclusiveOrExpressionContext::andExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::AndExpressionContext>(i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::ExclusiveOrExpressionContext::Caret() {
  return getTokens(DefEvaluatorParser::Caret);
}

tree::TerminalNode* DefEvaluatorParser::ExclusiveOrExpressionContext::Caret(size_t i) {
  return getToken(DefEvaluatorParser::Caret, i);
}


size_t DefEvaluatorParser::ExclusiveOrExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleExclusiveOrExpression;
}


std::any DefEvaluatorParser::ExclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitExclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::ExclusiveOrExpressionContext* DefEvaluatorParser::exclusiveOrExpression() {
  ExclusiveOrExpressionContext *_localctx = _tracker.createInstance<ExclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 20, DefEvaluatorParser::RuleExclusiveOrExpression);
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
    setState(101);
    andExpression();
    setState(106);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::Caret) {
      setState(102);
      match(DefEvaluatorParser::Caret);
      setState(103);
      andExpression();
      setState(108);
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

DefEvaluatorParser::InclusiveOrExpressionContext::InclusiveOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::ExclusiveOrExpressionContext *> DefEvaluatorParser::InclusiveOrExpressionContext::exclusiveOrExpression() {
  return getRuleContexts<DefEvaluatorParser::ExclusiveOrExpressionContext>();
}

DefEvaluatorParser::ExclusiveOrExpressionContext* DefEvaluatorParser::InclusiveOrExpressionContext::exclusiveOrExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::ExclusiveOrExpressionContext>(i);
}

std::vector<tree::TerminalNode *> DefEvaluatorParser::InclusiveOrExpressionContext::Or() {
  return getTokens(DefEvaluatorParser::Or);
}

tree::TerminalNode* DefEvaluatorParser::InclusiveOrExpressionContext::Or(size_t i) {
  return getToken(DefEvaluatorParser::Or, i);
}


size_t DefEvaluatorParser::InclusiveOrExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleInclusiveOrExpression;
}


std::any DefEvaluatorParser::InclusiveOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitInclusiveOrExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::InclusiveOrExpressionContext* DefEvaluatorParser::inclusiveOrExpression() {
  InclusiveOrExpressionContext *_localctx = _tracker.createInstance<InclusiveOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 22, DefEvaluatorParser::RuleInclusiveOrExpression);
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
    setState(109);
    exclusiveOrExpression();
    setState(114);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::Or) {
      setState(110);
      match(DefEvaluatorParser::Or);
      setState(111);
      exclusiveOrExpression();
      setState(116);
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

DefEvaluatorParser::LogicalAndExpressionContext::LogicalAndExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::InclusiveOrExpressionContext *> DefEvaluatorParser::LogicalAndExpressionContext::inclusiveOrExpression() {
  return getRuleContexts<DefEvaluatorParser::InclusiveOrExpressionContext>();
}

DefEvaluatorParser::InclusiveOrExpressionContext* DefEvaluatorParser::LogicalAndExpressionContext::inclusiveOrExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::InclusiveOrExpressionContext>(i);
}


size_t DefEvaluatorParser::LogicalAndExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleLogicalAndExpression;
}


std::any DefEvaluatorParser::LogicalAndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitLogicalAndExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::LogicalAndExpressionContext* DefEvaluatorParser::logicalAndExpression() {
  LogicalAndExpressionContext *_localctx = _tracker.createInstance<LogicalAndExpressionContext>(_ctx, getState());
  enterRule(_localctx, 24, DefEvaluatorParser::RuleLogicalAndExpression);
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
    setState(117);
    inclusiveOrExpression();
    setState(122);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::T__5) {
      setState(118);
      match(DefEvaluatorParser::T__5);
      setState(119);
      inclusiveOrExpression();
      setState(124);
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

DefEvaluatorParser::LogicalOrExpressionContext::LogicalOrExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<DefEvaluatorParser::LogicalAndExpressionContext *> DefEvaluatorParser::LogicalOrExpressionContext::logicalAndExpression() {
  return getRuleContexts<DefEvaluatorParser::LogicalAndExpressionContext>();
}

DefEvaluatorParser::LogicalAndExpressionContext* DefEvaluatorParser::LogicalOrExpressionContext::logicalAndExpression(size_t i) {
  return getRuleContext<DefEvaluatorParser::LogicalAndExpressionContext>(i);
}


size_t DefEvaluatorParser::LogicalOrExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleLogicalOrExpression;
}


std::any DefEvaluatorParser::LogicalOrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitLogicalOrExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::LogicalOrExpressionContext* DefEvaluatorParser::logicalOrExpression() {
  LogicalOrExpressionContext *_localctx = _tracker.createInstance<LogicalOrExpressionContext>(_ctx, getState());
  enterRule(_localctx, 26, DefEvaluatorParser::RuleLogicalOrExpression);
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
    setState(125);
    logicalAndExpression();
    setState(130);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == DefEvaluatorParser::T__6) {
      setState(126);
      match(DefEvaluatorParser::T__6);
      setState(127);
      logicalAndExpression();
      setState(132);
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

DefEvaluatorParser::ConditionalExpressionContext::ConditionalExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

DefEvaluatorParser::LogicalOrExpressionContext* DefEvaluatorParser::ConditionalExpressionContext::logicalOrExpression() {
  return getRuleContext<DefEvaluatorParser::LogicalOrExpressionContext>(0);
}

DefEvaluatorParser::PrimaryExpressionContext* DefEvaluatorParser::ConditionalExpressionContext::primaryExpression() {
  return getRuleContext<DefEvaluatorParser::PrimaryExpressionContext>(0);
}

DefEvaluatorParser::ConditionalExpressionContext* DefEvaluatorParser::ConditionalExpressionContext::conditionalExpression() {
  return getRuleContext<DefEvaluatorParser::ConditionalExpressionContext>(0);
}


size_t DefEvaluatorParser::ConditionalExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleConditionalExpression;
}


std::any DefEvaluatorParser::ConditionalExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitConditionalExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::ConditionalExpressionContext* DefEvaluatorParser::conditionalExpression() {
  ConditionalExpressionContext *_localctx = _tracker.createInstance<ConditionalExpressionContext>(_ctx, getState());
  enterRule(_localctx, 28, DefEvaluatorParser::RuleConditionalExpression);
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
    setState(133);
    logicalOrExpression();
    setState(139);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == DefEvaluatorParser::T__7) {
      setState(134);
      match(DefEvaluatorParser::T__7);
      setState(135);
      primaryExpression();
      setState(136);
      match(DefEvaluatorParser::T__8);
      setState(137);
      conditionalExpression();
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

DefEvaluatorParser::AssignmentExpressionContext::AssignmentExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

DefEvaluatorParser::ConditionalExpressionContext* DefEvaluatorParser::AssignmentExpressionContext::conditionalExpression() {
  return getRuleContext<DefEvaluatorParser::ConditionalExpressionContext>(0);
}

DefEvaluatorParser::InclusiveOrExpressionContext* DefEvaluatorParser::AssignmentExpressionContext::inclusiveOrExpression() {
  return getRuleContext<DefEvaluatorParser::InclusiveOrExpressionContext>(0);
}


size_t DefEvaluatorParser::AssignmentExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleAssignmentExpression;
}


std::any DefEvaluatorParser::AssignmentExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitAssignmentExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::AssignmentExpressionContext* DefEvaluatorParser::assignmentExpression() {
  AssignmentExpressionContext *_localctx = _tracker.createInstance<AssignmentExpressionContext>(_ctx, getState());
  enterRule(_localctx, 30, DefEvaluatorParser::RuleAssignmentExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(143);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(141);
      conditionalExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(142);
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

DefEvaluatorParser::EntryExpressionContext::EntryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* DefEvaluatorParser::EntryExpressionContext::Constant() {
  return getToken(DefEvaluatorParser::Constant, 0);
}

tree::TerminalNode* DefEvaluatorParser::EntryExpressionContext::EOF() {
  return getToken(DefEvaluatorParser::EOF, 0);
}

DefEvaluatorParser::AssignmentExpressionContext* DefEvaluatorParser::EntryExpressionContext::assignmentExpression() {
  return getRuleContext<DefEvaluatorParser::AssignmentExpressionContext>(0);
}


size_t DefEvaluatorParser::EntryExpressionContext::getRuleIndex() const {
  return DefEvaluatorParser::RuleEntryExpression;
}


std::any DefEvaluatorParser::EntryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<DefEvaluatorVisitor*>(visitor))
    return parserVisitor->visitEntryExpression(this);
  else
    return visitor->visitChildren(this);
}

DefEvaluatorParser::EntryExpressionContext* DefEvaluatorParser::entryExpression() {
  EntryExpressionContext *_localctx = _tracker.createInstance<EntryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 32, DefEvaluatorParser::RuleEntryExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(150);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(145);
      match(DefEvaluatorParser::Constant);
      setState(146);
      match(DefEvaluatorParser::EOF);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(147);
      assignmentExpression();
      setState(148);
      match(DefEvaluatorParser::EOF);
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

void DefEvaluatorParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  defevaluatorParserInitialize();
#else
  ::antlr4::internal::call_once(defevaluatorParserOnceFlag, defevaluatorParserInitialize);
#endif
}
