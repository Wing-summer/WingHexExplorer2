#ifndef OPENREGIONDIALOG_H
#define OPENREGIONDIALOG_H

#include <QDialog>
#include <QMainWindow>
#include <QSpinBox>

#include "QPathEdit/qpathedit.h"
#include "framelessdialog.h"

struct RegionFileResult {
    QString filename;
    qsizetype start;
    qsizetype length;
};

class OpenRegionDialog : public FramelessDialog {
    Q_OBJECT
public:
    OpenRegionDialog(const QString &filename, int start = 0, int length = 1024,
                     QWidget *parent = nullptr);
    RegionFileResult getResult();

private:
    void on_accept();
    void on_reject();

private:
    QPathEdit *filepath;
    QSpinBox *sbStart, *sbLength;

    RegionFileResult res;
};

#endif // OPENREGIONDIALOG_H
