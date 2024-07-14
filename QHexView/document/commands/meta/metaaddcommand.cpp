#include "metaaddcommand.h"

MetaAddCommand::MetaAddCommand(QHexMetadata *hexmeta,
                               QHexMetadataAbsoluteItem &meta,
                               QUndoCommand *parent)
    : MetaCommand(hexmeta, meta, parent) {}

void MetaAddCommand::redo() {
  m_hexmeta->metadata(m_meta.begin, m_meta.end, m_meta.foreground,
                      m_meta.background, m_meta.comment);
}

void MetaAddCommand::undo() { m_hexmeta->removeMetadata(m_meta); }
