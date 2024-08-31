#include "bookmarkcommand.h"

BookMarkCommand::BookMarkCommand(QHexDocument *doc, qsizetype pos, QString comment,
                                 QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_comment(comment), m_pos(pos) {}
