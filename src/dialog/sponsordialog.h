#ifndef SPONSORDIALOG_H
#define SPONSORDIALOG_H

#include "framelessdialogbase.h"
#include <QObject>

class SponsorDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    explicit SponsorDialog(QWidget *parent = nullptr);
};

#endif // SPONSORDIALOG_H
