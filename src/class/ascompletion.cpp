#include "ascompletion.h"

#include "qasparser.h"
#include "qcalltip.h"
#include "qdocumentcursor.h"
#include "qdocumentline.h"

// #include "plugin.h"

#include <QDir>
#include <QLibraryInfo>
#include <QQueue>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QtDebug>

AsCompletion::AsCompletion(asIScriptEngine *engine, QObject *p)
    : QCodeCompletionEngine(p), _engine(engine) {
    Q_ASSERT(engine);

    addTrigger(".");
    addTrigger("::");

    // unleash the power of call tips
    addTrigger("(");
}

AsCompletion::~AsCompletion() {}

QCodeCompletionEngine *AsCompletion::clone() {
    AsCompletion *e = new AsCompletion(_engine);

    for (auto &t : triggers())
        e->addTrigger(t);

    emit cloned(e);

    return e;
}

QString AsCompletion::language() const { return "AngelScript"; }

QStringList AsCompletion::extensions() const {
    QStringList l;

    l << "as"
      << "angelscript";

    return l;
}

void AsCompletion::complete(const QDocumentCursor &c, const QString &trigger) {
    auto codes = c.document()->text(true, false);
    QAsParser parser(_engine);
    parser.parse(codes, this->editor()->fileName());

    QList<QCodeNode *> nodes = parser.codeNodes();

    // TODO
    QByteArray fn;
    QList<QCodeNode *> temp; // internal CodeNodes
    int filter = QCodeCompletionWidget::FilterFlag::KeepAll;

    if (nodes.count()) {
        if (trigger == "(") {
            QStringList tips;

            // qDebug("fn %s", fn.constData());

            for (auto &n : nodes) {
                for (auto &f : n->children) {
                    if (f->type() != QCodeNode::Function ||
                        f->role(QCodeNode::Name) != fn)
                        continue;

                    auto tip = QString::fromUtf8(f->role(QCodeNode::Arguments))
                                   .prepend('(')
                                   .append(')');

                    if (!tips.contains(tip))
                        tips << tip;
                }
            }

            if (!tips.isEmpty()) {
                QRect r = editor()->cursorRect();
                QDocumentCursor cursor = editor()->cursor();
                QDocumentLine line = cursor.line();

                int hx = editor()->horizontalOffset(),
                    cx = line.cursorToX(cursor.columnNumber());

                auto ct = new QCallTip(editor()->viewport());
                ct->move(cx - hx, r.y() + r.height());
                ct->setTips(tips);
                ct->show();
                ct->setFocus();

#ifdef TRACE_COMPLETION
                qDebug(
                    "parsing + scoping + search + pre-display : elapsed %i ms",
                    time.elapsed());
#endif
            }
        } else {
            pPopup->setTemporaryNodes(temp);
            pPopup->setFilter(QCodeCompletionWidget::Filter(filter));
            pPopup->setCompletions(nodes);

#ifdef TRACE_COMPLETION
            qDebug("parsing + scoping + search + pre-display : elapsed %i ms",
                   time.elapsed());
#endif

            pPopup->popup();
        }
    } else {
        qDeleteAll(temp);
        qDebug("completion failed");
    }
}
