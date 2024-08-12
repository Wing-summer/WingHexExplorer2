#include "encodingchangecommand.h"

EncodingChangeCommand::EncodingChangeCommand(QHexRenderer *render, QString olde,
                                             QString newe, QUndoCommand *parent)
    : QUndoCommand(parent), m_render(render), m_olde(olde), m_newe(newe) {}

void EncodingChangeCommand::redo() { m_render->setEncoding(m_newe); }

void EncodingChangeCommand::undo() { m_render->setEncoding(m_olde); }
