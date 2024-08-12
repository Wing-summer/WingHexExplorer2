#ifndef METAREPLACECOMMAND_H
#define METAREPLACECOMMAND_H

#include "metacommand.h"
#include <QObject>

class MetaReplaceCommand : public MetaCommand {
public:
  MetaReplaceCommand(QHexMetadata *hexmeta, QHexMetadataAbsoluteItem &meta,
                     QHexMetadataAbsoluteItem &oldmeta,
                     QUndoCommand *parent = nullptr);
  void undo() override;
  void redo() override;

private:
  QHexMetadataAbsoluteItem m_old;
};

#endif // METAREPLACECOMMAND_H
