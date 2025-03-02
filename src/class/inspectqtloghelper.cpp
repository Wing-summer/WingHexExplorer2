#include "inspectqtloghelper.h"
#include "utilities.h"

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
    connect(a, &QAction::triggered, this, [this]() { _ctx->clear(); });
    menu->addAction(a);

    auto af = ma.at(0);
    af = menu->insertSeparator(af);
    a = new QAction(tr("TopMost"), menu);
    a->setCheckable(true);
    connect(a, &QAction::toggled, this, [this](bool b) {
        _logger->setWindowFlag(Qt::WindowStaysOnTopHint, b);
        if (!_logger->isVisible()) {
            _logger->setVisible(true);
        }
    });
    menu->insertAction(af, a);

    connect(_ctx, &QTextBrowser::customContextMenuRequested, this,
            [menu, this](const QPoint &pos) {
                menu->popup(_ctx->mapToGlobal(pos));
            });

    _logger->buildUpContent(_ctx);
    _logger->setWindowTitle(tr("Inspect"));
    _logger->setWindowIcon(ICONRES("qtloginspect"));

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

InspectQtLogHelper::InspectQtLogHelper() {}

InspectQtLogHelper::~InspectQtLogHelper() {}

void InspectQtLogHelper::messageHandler(QtMsgType type,
                                        const QMessageLogContext &context,
                                        const QString &message) {
    auto logger = InspectQtLogHelper::instance()._ctx;
    auto msg = qFormatLogMessage(type, context, message);
    Q_ASSERT(logger);
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
