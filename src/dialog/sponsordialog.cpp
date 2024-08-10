#include "sponsordialog.h"

#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

SponsorDialog::SponsorDialog(QWidget *parent) : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addSpacing(5);
    layout->addWidget(
        new QLabel("❤️ " + tr("ThanksForSponsor") + " ❤️", this), 0,
        Qt::AlignCenter);
    layout->addSpacing(5);

    QPixmap sponsor(":/com.wingsummer.winghex/images/sponsorqr.png");
    auto l = new QLabel(this);
    l->setPixmap(sponsor);
    l->setScaledContents(true);
    layout->addWidget(l);

    buildUpContent(widget);

    setWindowTitle(tr("Sponsor"));
}
