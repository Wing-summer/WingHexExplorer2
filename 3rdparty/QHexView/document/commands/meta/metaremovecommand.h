#ifndef METAREMOVECOMMAND_H
#define METAREMOVECOMMAND_H

#include "metacommand.h"
#include <QList>
#include <QObject>

// this class is newed by wingsummer

class MetaRemoveCommand : public MetaCommand {
public:
  MetaRemoveCommand(QHexMetadata *hexmeta, QHexMetadataAbsoluteItem &meta,
                    QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;
};

#endif // METAREMOVECOMMAND_H
