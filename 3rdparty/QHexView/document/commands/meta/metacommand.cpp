#include "metacommand.h"

MetaCommand::MetaCommand(QHexMetadata *hexmeta, QHexMetadataAbsoluteItem &meta,
                         QUndoCommand *parent)
    : QUndoCommand(parent), m_hexmeta(hexmeta), m_meta(meta) {}
