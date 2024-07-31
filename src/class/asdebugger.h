#ifndef ASDEBUGGER_H
#define ASDEBUGGER_H

#include "AngelScript/add_on/debugger/debugger.h"

#include <QObject>

class asDebugger : public QObject, public CDebugger {
    Q_OBJECT
public:
    asDebugger(QObject *parent = nullptr);

public:
    virtual void Output(const std::string &str) override;
    virtual void PrintHelp() override;

signals:
    void onOutput(const QString &message);
};

#endif // ASDEBUGGER_H
