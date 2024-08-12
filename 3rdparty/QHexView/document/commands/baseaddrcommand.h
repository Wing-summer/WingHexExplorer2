#ifndef BASEADDRCOMMAND_H
#define BASEADDRCOMMAND_H

#include "document/qhexdocument.h"
#include <QUndoCommand>

class BaseAddrCommand : public QUndoCommand {
public:
  BaseAddrCommand(QHexDocument *doc, quint64 oldaddr, quint64 newaddr,
                  QUndoCommand *parent = nullptr);

  void redo() override;
  void undo() override;

private:
  QHexDocument *m_doc;
  quint64 m_old, m_new;
};

#endif // BASEADDRCOMMAND_H
