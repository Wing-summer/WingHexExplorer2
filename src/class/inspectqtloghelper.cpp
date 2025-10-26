#include "inspectqtloghelper.h"

#include "settingmanager.h"
#include "utilities.h"

#include <QMenu>

#include <iostream>

#define INFOLOG(msg) "<font color=\"green\">" + msg + "</font>"
#define ERRLOG(msg) "<font color=\"red\">" + msg + "</font>"
#define WARNLOG(msg) "<font color=\"gold\">" + msg + "</font>"

InspectQtLogHelper &InspectQtLogHelper::instance() {
    static InspectQtLogHelper ins;
    return ins;
}

void InspectQtLogHelper::init() {
    _logger = new FramelessDialogBase;
    _ctx = new QTextBrowser(_logger);
    _ctx->setContextMenuPolicy(Qt::CustomContextMenu);
    _ctx->setUndoRedoEnabled(false);
    _ctx->setOpenExternalLinks(true);

    auto menu = _ctx->createStandardContextMenu();
    auto ma = menu->actions();

    menu->addSeparator();
    auto a = new QAction(tr("Clear"), menu);
    QObject::connect(a, &QAction::triggered, qApp, [this]() { _ctx->clear(); });
    menu->addAction(a);

    auto af = ma.at(0);
    af = menu->insertSeparator(af);
    a = new QAction(tr("TopMost"), menu);
    a->setCheckable(true);
    QObject::connect(a, &QAction::toggled, qApp, [this](bool b) {
        _logger->setWindowFlag(Qt::WindowStaysOnTopHint, b);
        if (!_logger->isVisible()) {
            _logger->setVisible(true);
        }
    });
    menu->insertAction(af, a);

    for (auto &a : ma) {
        a->setEnabled(true);
    }

    QObject::connect(_ctx, &QTextBrowser::customContextMenuRequested, qApp,
                     [menu, this](const QPoint &pos) {
                         menu->popup(_ctx->mapToGlobal(pos));
                     });

    _logger->buildUpContent(_ctx);
    _logger->setWindowTitle(tr("Inspect"));
    _logger->setWindowIcon(ICONRES("qtloginspect"));
    _logger->setMinimumSize(400, 350);

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
    delete _logger;
}

void InspectQtLogHelper::showLogWidget() { _logger->show(); }

InspectQtLogHelper::InspectQtLogHelper() : _stream(new QTextStream(stdout)) {
    _stream->device()->setTextModeEnabled(true);

    QString newFileName = QStringLiteral("%1_%2_%3.log")
                              .arg(APP_NAME, WINGHEX_VERSION,
                                   QDateTime::currentDateTime().toString(
                                       QStringLiteral("yyyyMMdd_hhmmsss")));
    auto logPath = Utilities::getAppDataPath() + QDir::separator() +
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
    _file = QSharedPointer<QFile>(new QFile(path));
    if (_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        _stream->setDevice(_file.get());
    }
}

InspectQtLogHelper::~InspectQtLogHelper() {
    _stream->flush();
    _file->close();
}

void InspectQtLogHelper::messageHandler(QtMsgType type,
                                        const QMessageLogContext &context,
                                        const QString &message) {
    auto &ins = InspectQtLogHelper::instance();
    auto stream = ins._stream;
    auto logger = ins._ctx;
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
