#ifndef BOOKMARKCOMMAND_H
#define BOOKMARKCOMMAND_H

#include "document/qhexdocument.h"
#include <QObject>
#include <QUndoCommand>

class BookMarkCommand : public QUndoCommand {
public:
  BookMarkCommand(QHexDocument *doc, qsizetype pos, QString comment,
                  QUndoCommand *parent = nullptr);

protected:
  QHexDocument *m_doc;
  QString m_comment;
  qsizetype m_pos;
};

#endif // BOOKMARKCOMMAND_H
