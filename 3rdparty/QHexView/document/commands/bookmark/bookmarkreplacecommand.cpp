#include "bookmarkreplacecommand.h"

BookMarkReplaceCommand::BookMarkReplaceCommand(QHexDocument *doc, qsizetype pos,
                                               QString comment,
                                               QString oldcomment,
                                               QUndoCommand *parent)
    : BookMarkCommand(doc, pos, comment, parent), m_oldcomment(oldcomment) {}

void BookMarkReplaceCommand::redo() { m_doc->modBookMark(m_pos, m_comment); }

void BookMarkReplaceCommand::undo() { m_doc->modBookMark(m_pos, m_oldcomment); }
