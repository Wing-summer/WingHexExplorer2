#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#include "hexcommand.h"

class InsertCommand : public HexCommand {
public:
    InsertCommand(QHexDocument *doc, QHexCursor *cursor, qsizetype offset,
                  const QByteArray &data, int nibbleindex,
                  QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
};

#endif // INSERTCOMMAND_H
