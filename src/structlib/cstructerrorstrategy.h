#ifndef CSTRUCTERRORSTRATEGY_H
#define CSTRUCTERRORSTRATEGY_H

#include "DefaultErrorStrategy.h"
#include "Parser.h"

#include <QCoreApplication>

class CStructErrorStrategy : public antlr4::DefaultErrorStrategy {
    Q_DECLARE_TR_FUNCTIONS(CStructErrorListener)

public:
    CStructErrorStrategy();

public:
    void reportError(antlr4::Parser *recognizer,
                     const antlr4::RecognitionException &e) override;

protected:
    void
    reportNoViableAlternative(antlr4::Parser *recognizer,
                              const antlr4::NoViableAltException &e) override;

    void reportInputMismatch(antlr4::Parser *recognizer,
                             const antlr4::InputMismatchException &e) override;

    void
    reportFailedPredicate(antlr4::Parser *recognizer,
                          const antlr4::FailedPredicateException &e) override;

    void reportUnwantedToken(antlr4::Parser *recognizer) override;

    void reportMissingToken(antlr4::Parser *recognizer) override;
};

#endif // CSTRUCTERRORSTRATEGY_H
