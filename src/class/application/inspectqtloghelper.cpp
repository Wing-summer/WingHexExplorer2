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

#include "inspectqtloghelper.h"

#include <cstdio>

#include <QMenu>
#include <QMutexLocker>
#include <QScrollBar>
#include <QTextCharFormat>
#include <QTextCursor>

#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "utilities.h"

namespace {
void appendLogMessage(QTextBrowser *browser, QtMsgType type,
                      const QString &message) {
    auto scrollBar = browser->verticalScrollBar();
    const bool follow = scrollBar->value() == scrollBar->maximum();
    QTextCursor cursor(browser->document());
    cursor.movePosition(QTextCursor::End);
    if (!browser->document()->isEmpty()) {
        cursor.insertBlock();
    }
    QTextCharFormat format;
    switch (type) {
    case QtWarningMsg:
        format.setForeground(QColor(Qt::darkYellow));
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        format.setForeground(QColor(Qt::red));
        break;
    case QtInfoMsg:
        format.setForeground(QColor(Qt::darkGreen));
        break;
    case QtDebugMsg:
        break;
    }
    cursor.insertText(message, format);
    if (follow) {
        scrollBar->setValue(scrollBar->maximum());
    }
}
} // namespace

InspectQtLogHelper &InspectQtLogHelper::instance() {
    static InspectQtLogHelper ins;
    return ins;
}

void InspectQtLogHelper::init() {
    if (m_logger) {
        return;
    }
    m_logger = new FramelessDialogBase;
    m_ctx = new QTextBrowser(m_logger);
    m_ctx->setObjectName(QStringLiteral("inspectQtLog.output"));
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

    initConsole();
}

void InspectQtLogHelper::initConsole() {
    if (m_installed) {
        return;
    }
    qSetMessagePattern(QStringLiteral("%{if-debug}[Debug]%{endif}"
                                      "%{if-warning}[Warn]%{endif}"
                                      "%{if-info}[Info]%{endif}"
                                      "%{if-critical}[Error]%{endif}"
                                      "%{if-fatal}[Fatal]%{endif}"
                                      " %{message}"));
    m_previousHandler = qInstallMessageHandler(messageHandler);
    m_installed = true;
}

void InspectQtLogHelper::destory() {
    if (m_installed) {
        qInstallMessageHandler(m_previousHandler);
        m_installed = false;
    }
    FramelessDialogBase *window;
    {
        QMutexLocker lock(&m_mutex);
        window = m_logger;
        m_logger = nullptr;
        m_ctx = nullptr;
    }
    delete window;
}

void InspectQtLogHelper::showLogWidget() {
    if (m_logger) {
        m_logger->show();
    }
}

InspectQtLogHelper::InspectQtLogHelper() : m_stream(new QTextStream(stderr)) {
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
    auto msg = qFormatLogMessage(type, context, message);
    // The receiver owns the record terminator. Preserve internal line breaks
    // and indentation, but never append a second newline to an existing one.
    while (msg.endsWith(QChar('\n')) || msg.endsWith(QChar('\r'))) {
        msg.chop(1);
    }
    {
        QMutexLocker lock(&ins.m_mutex);
        if (ins.m_file->isOpen()) {
            *ins.m_stream << msg << QChar('\n');
            ins.m_stream->flush();
        }
        const auto bytes = msg.toUtf8();
        std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stderr);
        std::fputc('\n', stderr);
        std::fflush(stderr);
        if (auto logger = ins.m_ctx) {
            QMetaObject::invokeMethod(
                logger,
                [logger, type, msg] { appendLogMessage(logger, type, msg); },
                Qt::QueuedConnection);
        }
    }
}
