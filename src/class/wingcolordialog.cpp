#include "wingcolordialog.h"
#include "../dialog/framelessdialog.h"

#include <QColorDialog>

QColor WingColorDialog::getColor(const QString &caption, QWidget *parent) {
    auto dialog = new QColorDialog(parent);
    dialog->setWindowFlag(Qt::Widget);

    FramelessDialog d(parent);
    d.buildUpContent(dialog);
    d.setWindowTitle(caption);

    QObject::connect(dialog, &QColorDialog::finished, &d,
                     &FramelessDialog::done);

    if (d.exec() == QDialog::Accepted)
        return dialog->currentColor();
    return QColor();
}

WingColorDialog::WingColorDialog() {}
