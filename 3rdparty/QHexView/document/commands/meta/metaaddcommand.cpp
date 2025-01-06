#include "metaaddcommand.h"

MetaAddCommand::MetaAddCommand(QHexMetadata *hexmeta,
                               const QHexMetadataItem &meta,
                               QUndoCommand *parent)
    : MetaCommand(hexmeta, meta, parent) {
    _brokenMetas = m_hexmeta->mayBrokenMetaData(meta.begin, meta.end);
}

void MetaAddCommand::redo() {
    m_hexmeta->metadata(m_meta.begin, m_meta.end, m_meta.foreground,
                        m_meta.background, m_meta.comment);
}

void MetaAddCommand::undo() {
    m_hexmeta->removeMetadata(m_meta.begin, m_meta.end);
    for (auto &meta : _brokenMetas) {
        m_hexmeta->metadata(meta.begin, meta.end, meta.foreground,
                            meta.background, meta.comment);
    }
}
