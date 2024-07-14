#ifndef METACOMMAND_H
#define METACOMMAND_H

#include <QObject>
#include <QUndoCommand>

#include "../../qhexmetadata.h"

// this class is newed by wingsummer

class MetaCommand : public QUndoCommand {
public:
  MetaCommand(QHexMetadata *hexmeta, QHexMetadataAbsoluteItem &meta,
              QUndoCommand *parent = nullptr);

protected:
  QHexMetadata *m_hexmeta;
  QHexMetadataAbsoluteItem m_meta;
};

#endif // METACOMMAND_H
