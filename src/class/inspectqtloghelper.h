#ifndef INSPECTQTLOGHELPER_H
#define INSPECTQTLOGHELPER_H

#include "dialog/framelessdialogbase.h"

#include <QCoreApplication>
#include <QTextBrowser>

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
    FramelessDialogBase *_logger;
    QTextBrowser *_ctx;
};

#endif // INSPECTQTLOGHELPER_H
