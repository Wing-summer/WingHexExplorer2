#include "bookmarkremovecommand.h"

BookMarkRemoveCommand::BookMarkRemoveCommand(QHexDocument *doc, qint64 pos,
                                             QString comment,
                                             QUndoCommand *parent)
    : BookMarkCommand(doc, pos, comment, parent) {}

void BookMarkRemoveCommand::redo() { m_doc->removeBookMark(m_pos); }

void BookMarkRemoveCommand::undo() { m_doc->addBookMark(m_pos, m_comment); }
