#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#include "hexcommand.h"

class InsertCommand : public HexCommand {
public:
    InsertCommand(QHexBuffer *buffer, qsizetype offset, const QByteArray &data,
                  QHexCursor *cursor, int nibbleindex,
                  QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
};

#endif // INSERTCOMMAND_H
