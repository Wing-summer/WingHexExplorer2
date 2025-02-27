/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QCOMPLETION_ENGINE_H_
#define _QCOMPLETION_ENGINE_H_

#include "qce-config.h"

/*!
        \file qcompletionengine.h
        \brief Definition of the QCodeCompletionEngine class.
*/

#include "qdocumentcursor.h"

#include <QObject>
#include <QPointer>
#include <QStringList>

class QEditor;
class QAction;
class QKeyEvent;
class QCodeModel;
class QCodeStream;

class QCE_EXPORT QCodeCompletionEngine : public QObject {
    Q_OBJECT

public:
    QCodeCompletionEngine(QObject *p = nullptr);
    virtual ~QCodeCompletionEngine();

    virtual QCodeCompletionEngine *clone() = 0;

    virtual QString language() const = 0;
    virtual QStringList extensions() const = 0;

    QAction *triggerAction() const;

    QEditor *editor() const;
    virtual void setEditor(QEditor *e);

    QStringList triggers() const;

    void addTrigger(const QString &s);
    void removeTrigger(const QString &s);

    virtual void retranslate();

    qsizetype trigWordLen() const;
    void setTrigWordLen(qsizetype newTrigWordLen);

signals:
    void popup();
    void cloned(QCodeCompletionEngine *e);
    void completionTriggered(const QString &s);

public slots:
    void complete(const QString &trigger = {});
    // make it public
    virtual void complete(const QDocumentCursor &c, const QString &trigger);

    void textEdited(QKeyEvent *e);

protected:
    virtual void run();
    virtual bool eventFilter(QObject *o, QEvent *e);

private:
    void triggerWordLenComplete();

private:
    qsizetype m_max;
    qsizetype m_trigWordLen;

    QString m_trig;
    QDocumentCursor m_cur;
    QAction *pForcedTrigger;

    QStringList m_triggers;

    QPointer<QEditor> pEdit;
};

#endif // _QCOMPLETION_ENGINE_H_
