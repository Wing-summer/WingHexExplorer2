#ifndef REPLACECOMMAND_H
#define REPLACECOMMAND_H

#include "hexcommand.h"

class ReplaceCommand : public HexCommand {
public:
    ReplaceCommand(QHexBuffer *buffer, qsizetype offset, const QByteArray &data,
                   QHexCursor *cursor, int nibbleindex,
                   QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QByteArray m_olddata;
};

#endif // REPLACECOMMAND_H
