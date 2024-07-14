#ifndef METASHOWCOMMAND_H
#define METASHOWCOMMAND_H

#include "document/qhexdocument.h"
#include <QUndoCommand>

enum class ShowType { All, BgColor, FgColor, Comment };

class MetaShowCommand : public QUndoCommand {
public:
  MetaShowCommand(QHexDocument *doc, ShowType type, bool show,
                  QUndoCommand *parent = nullptr);

  void redo() override;
  void undo() override;

private:
  QHexDocument *m_doc;
  ShowType m_type;
  bool m_show;
};

#endif // METASHOWCOMMAND_H
