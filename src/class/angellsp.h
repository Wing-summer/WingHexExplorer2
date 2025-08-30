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
    static AngelLsp &instance();
    virtual ~AngelLsp();

public:
    // Start/stop server (returns true if started)
    bool start();
    void stop();

public:
    // Synchronous (blocking) initialize
    QJsonValue initializeSync(int timeoutMs = 5000);

    void initialized();

    // Basic document lifecycle helpers
    void openDocument(const QString &uri, qint64 version, const QString &text);

    void changeDocument(const QString &uri, qint64 version,
                        const LSP::TextDocumentContentChangeEvent &e);

    void closeDocument(const QString &uri);

public:
    // High-level feature wrappers (sync)
    QJsonValue requestDocumentSymbol(const QString &uri, int timeoutMs = 3000);
    QJsonValue requestSemanticTokensFull(const QString &uri,
                                         int timeoutMs = 3000);
    QJsonValue requestCompletion(const QString &uri, int line, int character,
                                 int timeoutMs = 3000);
    QJsonValue requestHover(const QString &uri, int line, int character,
                            int timeoutMs = 3000);
    QJsonValue requestDefinition(const QString &uri, int line, int character,
                                 int timeoutMs = 3000);
    QJsonValue requestReferences(const QString &uri, int line, int character,
                                 bool includeDeclaration = true,
                                 int timeoutMs = 3000);
    QJsonValue requestSignatureHelp(const QString &uri, int line, int character,
                                    int timeoutMs = 3000);
    QJsonValue requestResolve(const QJsonValue &symbol, int timeoutMs = 3000);

private:
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
    // emitted when a (previously sent) request finishes (async mode)
    void requestFinished(int id, const QJsonValue &result);

    // emitted when server sends diagnostics
    void diagnosticsPublished(const QString &uri,
                              const QList<LSP::Diagnostics> &diagnostics);

    // log message notifications
    void logMessage(LSP::MessageType type, const QString &message);

    // other notifications from server
    void notificationReceived(const QString &method, const QJsonValue &params);

    // server process events
    void serverStarted();
    void serverExited(int exitCode, QProcess::ExitStatus status);

    // emitted when server sends a request (method with id) and before we
    // auto-respond. If you want to handle request yourself, connect to this
    // signal and do your reply; otherwise AngelLSP will auto-respond with
    // default handlers.
    void serverRequestReceived(const QString &method, const QJsonValue &params,
                               int id);

public slots:
    // convenience: synchronous shutdown
    void shutdownAndExit();

private slots:
    void handleStdout();
    void handleStderr();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    AngelLsp();

    void sendMessage(const QJsonObject &msg);
    void handleIncomingMessage(const QJsonObject &msg);

private:
    LSP::Range readLSPDocRange(const QJsonObject &items);
    LSP::Location readLSPDocLocation(const QJsonObject &items);

    QJsonObject jsonLSPDocRange(const LSP::Range &range);
    QJsonObject jsonLSPDocLocation(const LSP::Location &loc);

private:
    QProcess *m_proc = nullptr;
    QByteArray m_stdoutBuffer;

    qint64 m_nextId = 1;
    QMap<qint64, QEventLoop *> m_pendingLoops;
    QMap<qint64, QJsonValue> m_pendingResponses;

    QMap<qint64, QString> m_outstandingRequests; // map request id -> method

    // Debounce/coalesce document change support
    QHash<QString, QTimer *> m_docTimers; // per-document debouncers
};

#endif // ANGELLSP_H
