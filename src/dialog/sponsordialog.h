#ifndef SPONSORDIALOG_H
#define SPONSORDIALOG_H

#include "framelessdialog.h"
#include <QObject>

class SponsorDialog : public FramelessDialog {
    Q_OBJECT
public:
    explicit SponsorDialog(QWidget *parent = nullptr);
};

#endif // SPONSORDIALOG_H
