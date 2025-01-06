#ifndef METAREMOVEPOSCOMMAND_H
#define METAREMOVEPOSCOMMAND_H

#include "../../qhexmetadata.h"
#include <QList>
#include <QObject>
#include <QUndoCommand>
#include <QUndoStack>

class MetaRemovePosCommand : public QUndoCommand {
public:
    MetaRemovePosCommand(QHexMetadata *hexmeta, qsizetype pos,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    QHexMetadata *m_hexmeta;
    qsizetype m_pos;
    QHexMetadataItem oldmeta;
};

#endif // METAREMOVEPOSCOMMAND_H
