#include "metashowcommand.h"

MetaShowCommand::MetaShowCommand(QHexDocument *doc, ShowType type, bool show,
                                 QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_type(type), m_show(show) {}

void MetaShowCommand::redo() {
  switch (m_type) {
  case ShowType::All:
    m_doc->setMetabgVisible(m_show);
    m_doc->setMetafgVisible(m_show);
    m_doc->setMetaCommentVisible(m_show);
    break;
  case ShowType::BgColor:
    m_doc->setMetabgVisible(m_show);
    break;
  case ShowType::FgColor:
    m_doc->setMetafgVisible(m_show);
    break;
  case ShowType::Comment:
    m_doc->setMetaCommentVisible(m_show);
    break;
  }
}

void MetaShowCommand::undo() {
  switch (m_type) {
  case ShowType::All:
    m_doc->setMetabgVisible(!m_show);
    m_doc->setMetafgVisible(!m_show);
    m_doc->setMetaCommentVisible(!m_show);
    break;
  case ShowType::BgColor:
    m_doc->setMetabgVisible(!m_show);
    break;
  case ShowType::FgColor:
    m_doc->setMetafgVisible(!m_show);
    break;
  case ShowType::Comment:
    m_doc->setMetaCommentVisible(!m_show);
    break;
  }
}
