#ifndef _AS_COMPLETION_H_
#define _AS_COMPLETION_H_

#include "control/qcodecompletionwidget.h"
#include "qcodecompletionengine.h"
#include "qcodenode.h"

#include <QHash>

class QByteArray;

class QCodeCompletionBackend {
    friend class AsCompletion;

public:
    QCodeCompletionBackend();
    virtual ~QCodeCompletionBackend();

    QList<QCodeNode *> rootNodes() const;
    QCodeNode *findNode(const QByteArray &type) const;

    void init();

private:
    QCodeModel *pModel;
    QStringList m_buffer, m_pathlist;
};

class AsCompletion : public QCodeCompletionEngine {
public:
    AsCompletion(QObject *p = 0);
    AsCompletion(QCodeModel *m, QObject *p = 0);

    virtual ~AsCompletion();

    virtual QCodeCompletionEngine *clone();

    virtual QString language() const;
    virtual QStringList extensions() const;

    void init();

    QCodeCompletionBackend *backend() const;

protected:
    virtual void setCodeModel(QCodeModel *m);
    virtual void complete(QCodeStream *s, const QString &trigger);

public:
    void hierarchy(QCodeNode *n, QList<QCodeNode *> &l,
                   QHash<QByteArray, QByteArray> &tpl);

    QCodeNode *lookup(const QByteArray &t);
    QCodeNode *nsAwareLookup(const QByteArray &t);
    QCodeNode *lookup(QCodeNode *n, const QByteArray &t,
                      QList<QCodeNode *> *extra = 0);

    QCodeNode *localLookup(const QList<QCodeNode *> &cxt, QByteArray &tt,
                           QByteArray &type, bool &ptr,
                           QHash<QByteArray, QByteArray> &tpl);

    QCodeNode *decrementalLookup(const QList<QCodeNode *> &cxt, QByteArray &tt,
                                 QByteArray &type, bool &ptr,
                                 QHash<QByteArray, QByteArray> &tpl,
                                 int k = -1);

    void getMembers(QList<QByteArray>::const_iterator beg,
                    QList<QByteArray>::const_iterator end,
                    const QHash<QByteArray, QByteArray> &variables,
                    QList<QByteArray> cxt, QList<QCodeNode *> &l, int *filter);

    QByteArray functionLookup(QCodeNode *n, const QByteArray &s);
    QByteArray functionLookup(QCodeModel *m, const QByteArray &s);

private:
    QCodeCompletionWidget *pPopup;
    QPointer<QCodeModel> pModel;

    static unsigned long instances;
    static QCodeCompletionBackend *pBackend;

    QCodeNode *m_locals;
    QList<QByteArray> context;
    QList<QByteArray> m_namespaces;
    QHash<QByteArray, QByteArray> variables;
    bool scope_local, scope_system, scope_file;
};

#endif // _CPP_COMPLETION_H_
