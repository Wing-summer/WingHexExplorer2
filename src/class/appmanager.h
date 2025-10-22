/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "dialog/mainwindow.h"
#include "qtsingleapplication/src/qtsingleapplication.h"

#include <QElapsedTimer>

class AppManager : public QtSingleApplication {
    Q_OBJECT
public:
    explicit AppManager(int &argc, char *argv[]);
    virtual ~AppManager();

    static AppManager *instance();

    MainWindow *mainWindow() const;

    void translation() {
        QApplication::tr("wingsummer");
        QApplication::tr("WingCloudStudio");
    }

    quint64 currentMSecsSinceEpoch();

public slots:
    ErrFile openFile(const QString &file, bool autoDetect, bool skipScripting,
                     bool *isWorkSpace);

private:
    MainWindow *_w = nullptr;
    QElapsedTimer _timer;
    SplashDialog *splash = nullptr;

    static AppManager *_instance;
};

#endif // APPMANAGER_H
