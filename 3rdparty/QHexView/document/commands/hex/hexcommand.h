#ifndef HEXCOMMAND_H
#define HEXCOMMAND_H

#include "document/qhexcursor.h"
#include "document/qhexdocument.h"

#include <QUndoCommand>

class HexCommand : public QUndoCommand {
public:
    HexCommand(QHexDocument *doc, QHexCursor *cursor, int nibbleindex,
               QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

protected:
    QHexDocument *m_doc;
    QHexCursor *m_cursor;

    qsizetype m_offset;
    qsizetype m_length;
    QByteArray m_data;

    int m_nibbleindex;
};

#endif // HEXCOMMAND_H
