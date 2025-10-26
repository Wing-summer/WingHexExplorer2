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

#ifndef ANGELLSP_H
#define ANGELLSP_H

#include "lsp.h"

#include <QEventLoop>
#include <QHash>
#include <QJsonValue>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

class AngelLsp : public QObject {
    Q_OBJECT

public:
    enum class TraceMode { off = 0, messages = 1, verbose = 2 };
    Q_ENUM(TraceMode);

public:
    static AngelLsp &instance();
    virtual ~AngelLsp();

public:
    // Start/stop server (returns true if started)
    bool start();

public slots:
    // convenience: synchronous shutdown
    void shutdownAndExit();

    bool restart();
    void restartWithGUI(QWidget *parent);

    bool isActive() const;

private:
    void stop();

public:
    // Synchronous (blocking) initialize
    QJsonValue initializeSync(int timeoutMs = 5000);

    void initialized();

    // Basic document lifecycle helpers
    void openDocument(const QString &uri, qint64 version, const QString &text);

    void changeDocument(const QString &uri, qint64 version,
                        const QString &fullText);

    void closeDocument(const QString &uri);

public:
    // High-level feature wrappers (sync)
    QJsonValue requestDocumentSymbol(const QString &uri, int timeoutMs = 3000);
    QJsonValue requestCompletion(const QString &uri, int line, int character,
                                 const QString &triggerChar,
                                 int timeoutMs = 3000);
    QJsonValue requestHover(const QString &uri, int line, int character,
                            int timeoutMs = 3000);
    QJsonValue requestSignatureHelp(const QString &uri, int line, int character,
                                    int timeoutMs = 3000);
    QJsonValue requestResolve(const QJsonValue &symbol, int timeoutMs = 3000);

    QJsonValue requestFormat(const QString &uri, int timeoutMs = 3000);

private:
    void reloadConfigure();

    // Generic request/notification
    // Returns request id
    qint64 sendRequest(const QString &method,
                       const QJsonValue &params = QJsonValue(),
                       int /*timeoutMs*/ = 0);

    // Blocking request
    QJsonValue sendRequestSync(const QString &method,
                               const QJsonValue &params = QJsonValue(),
                               int timeoutMs = 3000);
    void sendNotification(const QString &method,
                          const QJsonValue &params = QJsonValue());

    // Send LSP cancellation notification for a request id (client -> server)
    // LSP uses the JSON-RPC extension "$/cancelRequest" with { id: <id> }.
    void sendCancelRequest(int id);

signals:
    // emitted when server sends diagnostics
    void diagnosticsPublished(const QString &url,
                              const QList<LSP::Diagnostics> &diagnostics);

    // server process events
    void serverStarted();
    void serverExited();

private slots:
    void handleStdout();
    void handleStderr();
    void handleProcessFinished();

private:
    AngelLsp();

    void sendMessage(const QJsonObject &msg);
    void handleIncomingMessage(const QJsonObject &msg);

public:
    static LSP::Range readLSPDocRange(const QJsonObject &items);
    static LSP::Location readLSPDocLocation(const QJsonObject &items);

    static QJsonObject jsonLSPDocRange(const LSP::Range &range);
    static QJsonObject jsonLSPDocLocation(const LSP::Location &loc);

public:
    TraceMode traceMode() const;
    void setTraceMode(const TraceMode &newTraceMode);

    int indentSpace() const;
    void setIndentSpace(int newIndentSpace);

    bool useTabIndent() const;
    void setUseTabIndent(bool newUseTabIndent);

    void resetSettings();

    bool enabled() const;
    void setEnabled(bool newEnable);

    bool autofmt() const;
    void setAutofmt(bool newAutofmt);

private:
    QProcess *m_proc = nullptr;
    QByteArray m_stdoutBuffer;

    qint64 m_nextId = 1;
    QMap<qint64, QEventLoop *> m_pendingLoops;
    QMap<qint64, QJsonValue> m_pendingResponses;

    QMap<qint64, QString> m_outstandingRequests; // map request id -> method

    // Debounce/coalesce document change support
    QHash<QString, QTimer *> m_docTimers; // per-document debouncers

private:
    // configures
    TraceMode _traceMode = TraceMode::off;
    int _indentSpace = 4;
    bool _useTabIndent = false;
    bool _enabled = true;
    bool _autofmt = true;
};

#endif // ANGELLSP_H
