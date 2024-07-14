#ifndef BOOKMARKCLEARCOMMAND_H
#define BOOKMARKCLEARCOMMAND_H

#include "document/qhexdocument.h"
#include <QObject>
#include <QUndoCommand>

class BookMarkClearCommand : public QUndoCommand {
public:
  BookMarkClearCommand(QHexDocument *doc, QList<BookMarkStruct> bookmarks,
                       QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

protected:
  QHexDocument *m_doc;
  QList<BookMarkStruct> m_bookmarks;
};

#endif // BOOKMARKCLEARCOMMAND_H
