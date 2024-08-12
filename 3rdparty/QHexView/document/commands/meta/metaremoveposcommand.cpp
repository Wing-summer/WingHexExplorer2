#include "metaremoveposcommand.h"

MetaRemovePosCommand::MetaRemovePosCommand(QHexMetadata *hexmeta, qint64 pos,
                                           QUndoCommand *parent)
    : QUndoCommand(parent), m_hexmeta(hexmeta), m_pos(pos) {
  olditems = m_hexmeta->gets(pos);
}

void MetaRemovePosCommand::redo() { m_hexmeta->removeMetadata(m_pos); }

void MetaRemovePosCommand::undo() {
  for (auto item : olditems)
    m_hexmeta->metadata(item.begin, item.end, item.foreground, item.background,
                        item.comment);
}
