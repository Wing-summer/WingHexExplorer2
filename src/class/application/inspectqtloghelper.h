#ifndef INSPECTQTLOGHELPER_H
#define INSPECTQTLOGHELPER_H

#include <QCoreApplication>
#include <QFile>
#include <QTextBrowser>

#include "dialog/framelessdialogbase.h"

class InspectQtLogHelper {
    Q_DECLARE_TR_FUNCTIONS(InspectQtLogHelper)

public:
    static InspectQtLogHelper &instance();

    void init();
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
    FramelessDialogBase *m_logger;
    QTextBrowser *m_ctx;

    QSharedPointer<QFile> m_file = nullptr;
    QSharedPointer<QTextStream> m_stream = nullptr;
};

#endif // INSPECTQTLOGHELPER_H
