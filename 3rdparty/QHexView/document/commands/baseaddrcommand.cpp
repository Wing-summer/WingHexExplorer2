#include "baseaddrcommand.h"

BaseAddrCommand::BaseAddrCommand(QHexDocument *doc, quint64 oldaddr,
                                 quint64 newaddr, QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_old(oldaddr), m_new(newaddr) {}

void BaseAddrCommand::redo() { m_doc->setBaseAddress(m_new); }

void BaseAddrCommand::undo() { m_doc->setBaseAddress(m_old); }
