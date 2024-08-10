#ifndef FILEINFODIALOG_H
#define FILEINFODIALOG_H

#include "framelessdialogbase.h"

class FileInfoDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    FileInfoDialog(QString filename, bool isRegionFile,
                   QWidget *parent = nullptr);
    virtual ~FileInfoDialog();
};

#endif // FILEINFODIALOG_H
