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

#ifndef INSPECTQTLOGHELPER_H
#define INSPECTQTLOGHELPER_H

#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QTextBrowser>

#include "dialog/framelessdialogbase.h"

class InspectQtLogHelper {
    Q_DECLARE_TR_FUNCTIONS(InspectQtLogHelper)

public:
    static InspectQtLogHelper &instance();

    void init();
    void initConsole();
    void destory();

    void showLogWidget();

private:
    InspectQtLogHelper();
    ~InspectQtLogHelper();
    Q_DISABLE_COPY_MOVE(InspectQtLogHelper)

private:
    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &message);

private:
    FramelessDialogBase *m_logger = nullptr;
    QTextBrowser *m_ctx = nullptr;
    QtMessageHandler m_previousHandler = nullptr;
    bool m_installed = false;
    QMutex m_mutex;

    QSharedPointer<QFile> m_file = nullptr;
    QSharedPointer<QTextStream> m_stream = nullptr;
};

#endif // INSPECTQTLOGHELPER_H
