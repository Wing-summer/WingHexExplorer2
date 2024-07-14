#ifndef METAREMOVEPOSCOMMAND_H
#define METAREMOVEPOSCOMMAND_H

#include "../../qhexmetadata.h"
#include <QList>
#include <QObject>
#include <QUndoCommand>
#include <QUndoStack>

class MetaRemovePosCommand : public QUndoCommand {
public:
  MetaRemovePosCommand(QHexMetadata *hexmeta, qint64 pos,
                       QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

protected:
  QHexMetadata *m_hexmeta;
  qint64 m_pos;
  QList<QHexMetadataAbsoluteItem> olditems;
};

#endif // METAREMOVEPOSCOMMAND_H
