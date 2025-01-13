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

private:
    QVector<QHexMetadataItem> _rmMetas;
    QMap<qsizetype, QString> _rmbms;
};

#endif // REMOVECOMMAND_H
