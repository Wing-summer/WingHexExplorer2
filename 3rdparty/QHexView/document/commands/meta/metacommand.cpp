#include "metacommand.h"

MetaCommand::MetaCommand(QHexMetadata *hexmeta, const QHexMetadataItem &meta,
                         QUndoCommand *parent)
    : QUndoCommand(parent), m_hexmeta(hexmeta), m_meta(meta) {}
