/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include <QPointer>

#include "qtsingleapplication/src/qtsingleapplication.h"

#include "dialog/mainwindow.h"

class AppManager : public QtSingleApplication {
    Q_OBJECT
public:
    explicit AppManager(int &argc, char *argv[]);
    virtual ~AppManager();

    void initialize(const QStringList &files);

    static AppManager *instance();

    MainWindow *mainWindow() const;

    inline void translation() {
        QApplication::tr("wingsummer");
        QApplication::tr("WingCloudStudio");
    }

public slots:
    WingHex::ErrFile openFile(const QString &file, bool autoDetect,
                              bool skipScripting, bool *isWorkSpace);

private:
    MainWindow *m_w = nullptr;
    QPointer<SplashDialog> m_splash;

    static AppManager *m_instance;
};

#endif // APPMANAGER_H
