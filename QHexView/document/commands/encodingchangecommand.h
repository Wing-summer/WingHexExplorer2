#ifndef ENCODINGCHANGECOMMAND_H
#define ENCODINGCHANGECOMMAND_H

#include "document/qhexrenderer.h"
#include <QUndoCommand>

class EncodingChangeCommand : public QUndoCommand {
public:
  EncodingChangeCommand(QHexRenderer *render, QString olde, QString newe,
                        QUndoCommand *parent = nullptr);
  void redo() override;
  void undo() override;

private:
  QHexRenderer *m_render;
  QString m_olde, m_newe;
};

#endif // ENCODINGCHANGECOMMAND_H
