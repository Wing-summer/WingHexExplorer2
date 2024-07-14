#ifndef METAADDCOMMAND_H
#define METAADDCOMMAND_H

#include "metacommand.h"
#include <QObject>

class MetaAddCommand : public MetaCommand {
public:
  MetaAddCommand(QHexMetadata *hexmeta, QHexMetadataAbsoluteItem &meta,
                 QUndoCommand *parent = nullptr);
  void undo() override;
  void redo() override;
};

#endif // METAADDCOMMAND_H
