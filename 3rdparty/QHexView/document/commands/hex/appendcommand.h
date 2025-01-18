#ifndef APPENDCOMMAND_H
#define APPENDCOMMAND_H

#include "hexcommand.h"

class AppendCommand : public HexCommand {
public:
    AppendCommand(QHexDocument *doc, QHexCursor *cursor, const QByteArray &data,
                  int nibbleindex, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
};

#endif // APPENDCOMMAND_H
