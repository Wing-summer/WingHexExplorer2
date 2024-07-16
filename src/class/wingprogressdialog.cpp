#include "wingprogressdialog.h"

WingProgressDialog::WingProgressDialog(const QString &labelText,
                                       const QString &cancelButtonText,
                                       int minimum, int maximum,
                                       QWidget *parent) {
    m_dialog = new QProgressDialog(labelText, cancelButtonText, minimum,
                                   maximum, parent);
    m_dialog->setWindowFlag(Qt::Widget);

    m_d = new FramelessDialog(parent);
    m_d->buildUpContent(m_dialog);
    m_d->setWindowTitle(labelText);

    QObject::connect(m_dialog, &QProgressDialog::finished, m_d,
                     &FramelessDialog::done);
}

WingProgressDialog::~WingProgressDialog() {
    delete m_dialog;
    delete m_d;
}

FramelessDialog *WingProgressDialog::pdialog() const { return m_d; }

QProgressDialog *WingProgressDialog::dialog() const { return m_dialog; }
