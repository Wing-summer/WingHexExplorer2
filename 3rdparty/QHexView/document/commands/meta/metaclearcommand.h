#ifndef METACLEARCOMMAND_H
#define METACLEARCOMMAND_H

#include "../../qhexmetadata.h"
#include <QList>
#include <QObject>
#include <QUndoCommand>
#include <QUndoStack>

// this class is newed by wingsummer

class MetaClearCommand : public QUndoCommand {
public:
  MetaClearCommand(QHexMetadata *hexmeta, QList<QHexMetadataAbsoluteItem> metas,
                   QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

protected:
  QHexMetadata *m_hexmeta;
  QList<QHexMetadataAbsoluteItem> m_metas;
};

#endif // METACLEARCOMMAND_H
