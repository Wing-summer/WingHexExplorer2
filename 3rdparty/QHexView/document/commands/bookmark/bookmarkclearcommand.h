#ifndef BOOKMARKCLEARCOMMAND_H
#define BOOKMARKCLEARCOMMAND_H

#include "document/qhexdocument.h"
#include <QMap>
#include <QObject>
#include <QUndoCommand>

class BookMarkClearCommand : public QUndoCommand {
public:
    BookMarkClearCommand(QHexDocument *doc,
                         const QMap<qsizetype, QString> &bookmarks,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    QHexDocument *m_doc;
    QMap<qsizetype, QString> m_bookmarks;
};

#endif // BOOKMARKCLEARCOMMAND_H
