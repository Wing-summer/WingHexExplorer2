#ifndef REMOVECOMMAND_H
#define REMOVECOMMAND_H

#include "hexcommand.h"

class RemoveCommand : public HexCommand {
public:
    RemoveCommand(QHexDocument *doc, qsizetype offset, qsizetype length,
                  QHexCursor *cursor, int nibbleindex,
                  QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
};

#endif // REMOVECOMMAND_H
