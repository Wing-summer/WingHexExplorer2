#ifndef ASDEBUGGER_H
#define ASDEBUGGER_H

#include "AngelScript/add_on/debugger/debugger.h"

#include <QHash>
#include <QObject>

class asDebugger : public QObject, public CDebugger {
    Q_OBJECT
public:
    asDebugger(QObject *parent = nullptr);

public:
    virtual void Output(const std::string &str) override;
    virtual void Output(const QString &str);
    virtual void PrintHelp() override;

    virtual void AddFileBreakPoint(const std::string &file,
                                   int lineNbr) override;
    virtual void AddFuncBreakPoint(const std::string &func) override;
    virtual void ListBreakPoints() override;
    virtual bool CheckBreakPoint(asIScriptContext *ctx) override;

    virtual void ListStatistics(asIScriptContext *ctx) override;

private:
    bool strcasecmp(const std::string &str1, const std::string &str2);

signals:
    void onOutput(const QString &message);
};

#endif // ASDEBUGGER_H
