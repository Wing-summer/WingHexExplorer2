#ifndef HISTORYDELDIALOG_H
#define HISTORYDELDIALOG_H

#include "class/recentfilemanager.h"
#include "framelessdialogbase.h"

namespace Ui {
class HistoryDelDialog;
}

class HistoryDelDialog : public QWidget {
    Q_OBJECT

public:
    explicit HistoryDelDialog(const QList<RecentFileManager::RecentInfo> &files,
                              bool fileNameOnly, QWidget *parent = nullptr);
    virtual ~HistoryDelDialog();

public:
    QVector<int> getResult() const;

public slots:
    int exec();

private slots:
    void on_lwHistory_currentRowChanged(int currentRow);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::HistoryDelDialog *ui;
    FramelessDialogBase *_dialog;

    QList<RecentFileManager::RecentInfo> _infos;
    bool _fileNameOnly;
};

#endif // HISTORYDELDIALOG_H
