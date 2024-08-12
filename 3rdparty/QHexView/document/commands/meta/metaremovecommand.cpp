#include "metaremovecommand.h"

MetaRemoveCommand::MetaRemoveCommand(QHexMetadata *hexmeta,
                                     QHexMetadataAbsoluteItem &meta,
                                     QUndoCommand *parent)
    : MetaCommand(hexmeta, meta, parent) {}

void MetaRemoveCommand::redo() { m_hexmeta->removeMetadata(m_meta); }

void MetaRemoveCommand::undo() {
  m_hexmeta->metadata(m_meta.begin, m_meta.end, m_meta.foreground,
                      m_meta.background, m_meta.comment);
}
