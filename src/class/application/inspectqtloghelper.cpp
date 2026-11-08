#include "inspectqtloghelper.h"

#include <iostream>

#include <QMenu>

#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "utilities.h"

#define INFOLOG(msg) "<font color=\"green\">" + msg + "</font>"
#define ERRLOG(msg) "<font color=\"red\">" + msg + "</font>"
#define WARNLOG(msg) "<font color=\"gold\">" + msg + "</font>"

InspectQtLogHelper &InspectQtLogHelper::instance() {
    static InspectQtLogHelper ins;
    return ins;
}

void InspectQtLogHelper::init() {
    m_logger = new FramelessDialogBase;
    m_ctx = new QTextBrowser(m_logger);
    m_ctx->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ctx->setUndoRedoEnabled(false);
    m_ctx->setOpenExternalLinks(true);

    auto menu = m_ctx->createStandardContextMenu();
    const auto ma = menu->actions();

    menu->addSeparator();
    auto a = new QAction(tr("Clear"), menu);
    QObject::connect(a, &QAction::triggered, qApp,
                     [this]() { m_ctx->clear(); });
    menu->addAction(a);

    for (const auto &a : ma) {
        a->setEnabled(true);
    }

    QObject::connect(m_ctx, &QTextBrowser::customContextMenuRequested, qApp,
                     [menu, this](const QPoint &pos) {
                         menu->popup(m_ctx->mapToGlobal(pos));
                     });

    m_logger->buildUpContent(m_ctx);
    m_logger->setWindowTitle(tr("Inspect"));
    m_logger->setWindowIcon(ThemeManager::instance().standardIcon(
        ThemeManager::StandardIcon::InspectQtLog));
    m_logger->setMinimumSize(400, 350);

    qSetMessagePattern(QStringLiteral("%{if-debug}[Debug]%{endif}"
                                      "%{if-warning}[Warn]%{endif}"
                                      "%{if-info}[Info]%{endif}"
                                      "%{if-critical}[Error]%{endif}"
                                      "%{if-fatal}[Fatal]%{endif}"
                                      " %{message}"));
    qInstallMessageHandler(messageHandler);
}

void InspectQtLogHelper::destory() {
    qInstallMessageHandler(nullptr);
    delete m_logger;
}

void InspectQtLogHelper::showLogWidget() { m_logger->show(); }

InspectQtLogHelper::InspectQtLogHelper() : m_stream(new QTextStream(stdout)) {
    m_stream->device()->setTextModeEnabled(true);

    QString newFileName = QStringLiteral("%1_%2_%3.log")
                              .arg(APP_NAME, WINGHEX_VERSION,
                                   QDateTime::currentDateTime().toString(
                                       QStringLiteral("yyyyMMdd_hhmmsss")));
    QString logPath = Utilities::getAppDataPath() + QDir::separator() +
                      QStringLiteral("qtlog");

    QDir logDir;
    logDir.mkpath(logPath);
    logDir.setPath(logPath);

    // clean up log files if too much
    auto logs = logDir.entryInfoList({"*.log"}, QDir::Files, QDir::Time);
    auto total = logs.size();
    for (decltype(total) i = SettingManager::instance().logCount() - 1;
         i < total; ++i) {
        QFile::remove(logs.at(i).absoluteFilePath());
    }

    auto path = logDir.absoluteFilePath(newFileName);
    m_file = QSharedPointer<QFile>(new QFile(path));
    if (m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_stream->setDevice(m_file.get());
    }
}

InspectQtLogHelper::~InspectQtLogHelper() {
    m_stream->flush();
    m_file->close();
}

void InspectQtLogHelper::messageHandler(QtMsgType type,
                                        const QMessageLogContext &context,
                                        const QString &message) {
    auto &ins = InspectQtLogHelper::instance();
    auto stream = ins.m_stream;
    auto logger = ins.m_ctx;
    Q_ASSERT(logger);

    auto msg = qFormatLogMessage(type, context, message);
    *stream << msg << Qt::endl;

    switch (type) {
    case QtDebugMsg:
        logger->append(msg);
        std::cout << qUtf8Printable(msg) << std::endl;
        break;
    case QtWarningMsg:
        logger->append(INFOLOG(msg));
        std::cout << qUtf8Printable(msg) << std::endl;
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        logger->append(ERRLOG(msg));
        std::cerr << qUtf8Printable(msg) << std::endl;
        break;
    case QtInfoMsg:
        logger->append(INFOLOG(msg));
        std::cout << qUtf8Printable(msg) << std::endl;
        break;
    }
}
