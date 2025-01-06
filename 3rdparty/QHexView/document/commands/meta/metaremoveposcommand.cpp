#include "metaremoveposcommand.h"

MetaRemovePosCommand::MetaRemovePosCommand(QHexMetadata *hexmeta, qsizetype pos,
                                           QUndoCommand *parent)
    : QUndoCommand(parent), m_hexmeta(hexmeta), m_pos(pos) {
    auto po = m_hexmeta->get(pos);
    if (po.has_value()) {
        oldmeta = po.value();
    }
}

void MetaRemovePosCommand::redo() { m_hexmeta->removeMetadata(m_pos); }

void MetaRemovePosCommand::undo() {
    m_hexmeta->metadata(oldmeta.begin, oldmeta.end, oldmeta.foreground,
                        oldmeta.background, oldmeta.comment);
}
