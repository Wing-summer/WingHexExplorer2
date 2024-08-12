#include "metareplacecommand.h"

MetaReplaceCommand::MetaReplaceCommand(QHexMetadata *hexmeta,
                                       QHexMetadataAbsoluteItem &meta,
                                       QHexMetadataAbsoluteItem &oldmeta,
                                       QUndoCommand *parent)
    : MetaCommand(hexmeta, meta, parent), m_old(oldmeta) {}

void MetaReplaceCommand::undo() { m_hexmeta->modifyMetadata(m_meta, m_old); }

void MetaReplaceCommand::redo() { m_hexmeta->modifyMetadata(m_old, m_meta); }
