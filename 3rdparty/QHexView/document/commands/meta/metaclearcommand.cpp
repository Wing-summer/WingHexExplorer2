#include "metaclearcommand.h"

MetaClearCommand::MetaClearCommand(QHexMetadata *hexmeta,
                                   const QVector<QHexMetadataItem> &metas,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), m_hexmeta(hexmeta), m_metas(metas) {}

void MetaClearCommand::redo() { m_hexmeta->clear(); }

void MetaClearCommand::undo() { m_hexmeta->applyMetas(m_metas); }
