#ifndef METAADDCOMMAND_H
#define METAADDCOMMAND_H

#include "metacommand.h"
#include <QObject>

class MetaAddCommand : public MetaCommand {
public:
    MetaAddCommand(QHexMetadata *hexmeta, const QHexMetadataItem &meta,
                   QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QVector<QHexMetadataItem> _brokenMetas;
};

#endif // METAADDCOMMAND_H
