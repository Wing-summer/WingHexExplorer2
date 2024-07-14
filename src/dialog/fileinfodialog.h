#ifndef FILEINFODIALOG_H
#define FILEINFODIALOG_H

#include "framelessdialog.h"

class FileInfoDialog : public FramelessDialog {
    Q_OBJECT
public:
    FileInfoDialog(QString filename, bool isRegionFile,
                   QWidget *parent = nullptr);
    virtual ~FileInfoDialog();
};

#endif // FILEINFODIALOG_H
