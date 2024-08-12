#ifndef BOOKMARKREMOVECOMMAND_H
#define BOOKMARKREMOVECOMMAND_H

#include "bookmarkcommand.h"

class BookMarkRemoveCommand : public BookMarkCommand {
public:
  BookMarkRemoveCommand(QHexDocument *doc, qint64 pos, QString comment,
                        QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;
};

#endif // BOOKMARKREMOVECOMMAND_H
