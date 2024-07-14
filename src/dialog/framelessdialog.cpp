#include "framelessdialog.h"

FramelessDialog::FramelessDialog(QWidget *parent)
    : FramelessWindowBase(parent) {
    m_winEffect = new QGraphicsDropShadowEffect(this);
    m_winEffect->setObjectName(QStringLiteral("WIN_BORDER_BLUR"));
    m_winEffect->setColor(QColor(2, 122, 255, 255));
    m_winEffect->setBlurRadius(15);
    m_winEffect->setOffset(0.0);

    Q_ASSERT(WIN_WIDGET);
    WIN_WIDGET->setGraphicsEffect(m_winEffect);

    this->setContentsMargins(15, 15, 15, 15);
    setResizeable(false, this->contentsMargins());
}

FramelessDialog::~FramelessDialog() {}

void FramelessDialog::buildUpContent(QWidget *content) {
    FramelessWindowBase::buildUpContent(TitleBar::TITLE_DIALOG, content);
}

void FramelessDialog::onDealedWithBorder(bool needHide) {
    m_winEffect->setEnabled(!needHide);
}
