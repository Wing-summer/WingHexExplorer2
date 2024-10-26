#ifndef _AS_COMPLETION_H_
#define _AS_COMPLETION_H_

#include "angelscript.h"
#include "control/qcodecompletionwidget.h"
#include "qcodecompletionengine.h"

#include <QHash>

class QByteArray;

class AsCompletion : public QCodeCompletionEngine {
    Q_OBJECT
public:
    explicit AsCompletion(asIScriptEngine *engine, QObject *p = nullptr);

    virtual ~AsCompletion();

    virtual QCodeCompletionEngine *clone() override;

    virtual QString language() const override;
    virtual QStringList extensions() const override;

protected:
    virtual void complete(const QDocumentCursor &c,
                          const QString &trigger) override;

private:
    asIScriptEngine *_engine;
    QCodeCompletionWidget *pPopup;
    QPointer<QCodeModel> pModel;
};

#endif // _CPP_COMPLETION_H_
