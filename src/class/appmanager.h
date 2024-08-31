#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "SingleApplication/singleapplication.h"
#include "dialog/mainwindow.h"

class AppManager : public SingleApplication {
    Q_OBJECT
public:
    explicit AppManager(int &argc, char *argv[]);
    virtual ~AppManager();

    static AppManager *instance();

    MainWindow *mainWindow() const;

public slots:
    void openFile(const QString &file, bool autoDetect = true,
                  qsizetype start = -1, qsizetype stop = -1);
    void openRawFile(const QString &file);
    void openDriver(const QString &driver);
    void openRegionFile(const QString &region, qsizetype start = -1,
                        qsizetype length = -1);
    void openWorkSpace(const QString &ws);

private:
    MainWindow *_w = nullptr;
    static AppManager *_instance;
};

#endif // APPMANAGER_H
