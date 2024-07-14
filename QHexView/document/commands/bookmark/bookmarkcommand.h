#ifndef BOOKMARKCOMMAND_H
#define BOOKMARKCOMMAND_H

#include "document/qhexdocument.h"
#include <QObject>
#include <QUndoCommand>

class BookMarkCommand : public QUndoCommand {
public:
  BookMarkCommand(QHexDocument *doc, qint64 pos, QString comment,
                  QUndoCommand *parent = nullptr);

protected:
  QHexDocument *m_doc;
  QString m_comment;
  qint64 m_pos;
};

#endif // BOOKMARKCOMMAND_H
