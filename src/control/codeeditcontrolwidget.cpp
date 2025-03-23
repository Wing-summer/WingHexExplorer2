#include "codeeditcontrolwidget.h"

#include <QKeyEvent>
#include <QPainter>

CodeEditControlWidget::CodeEditControlWidget(QWidget *parent)
    : QWidget{parent} {}

QSize CodeEditControlWidget::sizeHint() const {
    // Make the default just a bit wider
    const QSize hint = QWidget::sizeHint();
    return {(hint.width() * 5) / 4, hint.height()};
}

void CodeEditControlWidget::accept() {
    if (m_defaultButton) {
        m_defaultButton->animateClick();
    }
}

void CodeEditControlWidget::cancel() { close(); }

void CodeEditControlWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    // Round the corners of this widget.  Not strictly necessary, but
    // it looks nicer...
    const int arc = 4;
    const int circ = 8;
    const int h = height() - 1;
    const int w = width() - 1;
    painter.setPen(palette().color(QPalette::Mid));
    const QColor windowColor = palette().color(QPalette::Window);
    painter.setBrush(windowColor);
    painter.drawEllipse(0, h - circ, circ, circ);
    painter.drawEllipse(w - circ, h - circ, circ, circ);
    painter.fillRect(0, 0, arc, h - arc, windowColor);
    painter.fillRect(w - arc, 0, arc, h - arc, windowColor);
    painter.fillRect(arc, h - arc, w - circ, arc, windowColor);
    painter.fillRect(arc, 0, w - circ, h - arc, windowColor);
    painter.drawLine(0, 0, 0, h - arc);
    painter.drawLine(arc, h, w - arc, h);
    painter.drawLine(w, 0, w, h - arc);
}

void CodeEditControlWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        cancel();
    } else if (event->key() == Qt::Key_Enter) {
        accept();
    } else {
        QWidget::keyPressEvent(event);
    }
}

QAbstractButton *CodeEditControlWidget::defaultButton() const {
    return m_defaultButton;
}

void CodeEditControlWidget::setDefaultButton(
    QAbstractButton *newDefaultButton) {
    m_defaultButton = newDefaultButton;
}
