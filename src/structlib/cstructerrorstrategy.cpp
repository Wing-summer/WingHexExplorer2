#include "cstructerrorstrategy.h"

#include "FailedPredicateException.h"
#include "InputMismatchException.h"
#include "NoViableAltException.h"
#include "ParserRuleContext.h"

CStructErrorStrategy::CStructErrorStrategy() {}

void CStructErrorStrategy::reportError(antlr4::Parser *recognizer,
                                       const antlr4::RecognitionException &e) {
    // If we've already reported an error and have not matched a token
    // yet successfully, don't report any errors.
    if (inErrorRecoveryMode(recognizer)) {
        return; // don't report spurious errors
    }

    beginErrorCondition(recognizer);
    if (antlrcpp::is<const antlr4::NoViableAltException *>(&e)) {
        reportNoViableAlternative(
            recognizer, static_cast<const antlr4::NoViableAltException &>(e));
    } else if (antlrcpp::is<const antlr4::InputMismatchException *>(&e)) {
        reportInputMismatch(
            recognizer, static_cast<const antlr4::InputMismatchException &>(e));
    } else if (antlrcpp::is<const antlr4::FailedPredicateException *>(&e)) {
        reportFailedPredicate(
            recognizer,
            static_cast<const antlr4::FailedPredicateException &>(e));
    } else if (antlrcpp::is<const antlr4::RecognitionException *>(&e)) {
        // other unknown exceptions?
        recognizer->notifyErrorListeners(e.getOffendingToken(), e.what(),
                                         std::current_exception());
    }
}

void CStructErrorStrategy::reportNoViableAlternative(
    antlr4::Parser *recognizer, const antlr4::NoViableAltException &e) {
    antlr4::TokenStream *tokens = recognizer->getTokenStream();
    QString input;
    if (tokens != nullptr) {
        if (e.getStartToken()->getType() == antlr4::Token::EOF) {
            input = tr("<EOF>");
        } else {
            input = QString::fromStdString(escapeWSAndQuote(
                tokens->getText(e.getStartToken(), e.getOffendingToken())));
        }
    } else {
        input = tr("<unknown input>");
    }
    auto msg = tr("no viable alternative at input %1").arg(input);
    recognizer->notifyErrorListeners(e.getOffendingToken(), msg.toStdString(),
                                     std::make_exception_ptr(e));
}

void CStructErrorStrategy::reportInputMismatch(
    antlr4::Parser *recognizer, const antlr4::InputMismatchException &e) {
    auto msg = tr("mismatched input %1 expecting %2")
                   .arg(QString::fromStdString(
                            getTokenErrorDisplay(e.getOffendingToken())),
                        QString::fromStdString(e.getExpectedTokens().toString(
                            recognizer->getVocabulary())));
    recognizer->notifyErrorListeners(e.getOffendingToken(), msg.toStdString(),
                                     std::make_exception_ptr(e));
}

void CStructErrorStrategy::reportFailedPredicate(
    antlr4::Parser *recognizer, const antlr4::FailedPredicateException &e) {
    const std::string &ruleName =
        recognizer->getRuleNames()[recognizer->getContext()->getRuleIndex()];

    auto msg = tr("rule %1 failed predicate: %2?")
                   .arg(QString::fromStdString(ruleName),
                        QString::fromStdString(
                            const_cast<antlr4::FailedPredicateException &>(e)
                                .getPredicate()));

    recognizer->notifyErrorListeners(e.getOffendingToken(), msg.toStdString(),
                                     std::make_exception_ptr(e));
}

void CStructErrorStrategy::reportUnwantedToken(antlr4::Parser *recognizer) {
    if (inErrorRecoveryMode(recognizer)) {
        return;
    }

    beginErrorCondition(recognizer);

    antlr4::Token *t = recognizer->getCurrentToken();
    std::string tokenName = getTokenErrorDisplay(t);
    antlr4::misc::IntervalSet expecting = getExpectedTokens(recognizer);

    auto msg = tr("extraneous input %1 expecting %2")
                   .arg(QString::fromStdString(tokenName),
                        QString::fromStdString(
                            expecting.toString(recognizer->getVocabulary())));

    recognizer->notifyErrorListeners(t, msg.toStdString(), nullptr);
}

void CStructErrorStrategy::reportMissingToken(antlr4::Parser *recognizer) {
    if (inErrorRecoveryMode(recognizer)) {
        return;
    }

    beginErrorCondition(recognizer);

    antlr4::Token *t = recognizer->getCurrentToken();
    antlr4::misc::IntervalSet expecting = getExpectedTokens(recognizer);
    std::string expectedText = expecting.toString(recognizer->getVocabulary());

    auto msg = tr("missing %1 at %2")
                   .arg(QString::fromStdString(expectedText),
                        QString::fromStdString(getTokenErrorDisplay(t)));

    recognizer->notifyErrorListeners(t, msg.toStdString(), nullptr);
}
