#include "angellsp.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

AngelLsp::AngelLsp() {}

AngelLsp &AngelLsp::instance() {
    static AngelLsp ins;
    return ins;
}

AngelLsp::~AngelLsp() { stop(); }

bool AngelLsp::start() {
    if (m_proc)
        stop();

    QDir appDir(qApp->applicationDirPath());

#ifdef Q_OS_WIN
    static auto LSP_FILENAME = QStringLiteral("angelscript-ls.exe");
#else
    static auto LSP_FILENAME = QStringLiteral("angelscript-ls");
#endif

    if (!appDir.cd(QStringLiteral("lsp"))) {
        qWarning("AngelLSP: server not found");
        return false;
    }

    if (!appDir.exists(LSP_FILENAME)) {
        qWarning("AngelLSP: server not found");
        return false;
    }

    static QStringList args{"--stdio"};

    m_proc = new QProcess(this);

    m_proc->setProgram(appDir.absoluteFilePath(LSP_FILENAME));
    m_proc->setArguments(args);
    m_proc->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_proc, &QProcess::readyReadStandardOutput, this,
            &AngelLsp::handleStdout);
    connect(m_proc, &QProcess::readyReadStandardError, this,
            &AngelLsp::handleStderr);
    connect(m_proc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &AngelLsp::handleProcessFinished);

    m_proc->start();
    if (!m_proc->waitForStarted(3000)) {
        qWarning("AngelLSP: failed to start server");
        delete m_proc;
        m_proc = nullptr;
        return false;
    }

    Q_EMIT serverStarted();
    return true;
}

void AngelLsp::stop() {
    if (!m_proc)
        return;

    // destroy timers for documents
    for (auto &t : m_docTimers) {
        if (t) {
            t->stop();
            t->deleteLater();
        }
    }
    m_docTimers.clear();

    if (m_proc->state() == QProcess::Running) {
        m_proc->terminate();
        m_proc->waitForFinished(1000);
        if (m_proc->state() != QProcess::NotRunning) {
            m_proc->kill();
            m_proc->waitForFinished(1000);
        }
    }
    delete m_proc;
    m_proc = nullptr;
}

QJsonValue AngelLsp::initializeSync(int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject params;
    params["processId"] = QCoreApplication::applicationPid();

    // capabilities minimal
    QJsonObject caps;
    QJsonObject workspace;
    workspace["configuration"] = false;
    workspace["workspaceFolders"] = false;
    caps["workspace"] = workspace;
    QJsonObject td;
    QJsonObject sync;
    sync["dynamicRegistration"] = false;
    td["synchronization"] = sync;
    caps["textDocument"] = td;
    params["capabilities"] = caps;

    return sendRequestSync("initialize", params, timeoutMs);
}

void AngelLsp::initialized() { sendNotification("initialized", {}); }

void AngelLsp::openDocument(const QString &uri, qint64 version,
                            const QString &text) {
    if (!m_proc)
        return;

    QJsonObject textDoc;
    textDoc["uri"] = uri;
    textDoc["languageId"] = QStringLiteral("angelscript");
    textDoc["version"] = version;
    textDoc["text"] = text;
    QJsonObject params;
    params["textDocument"] = textDoc;
    sendNotification("textDocument/didOpen", params);
}

void AngelLsp::closeDocument(const QString &uri) {
    if (!m_proc)
        return;

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}};
    sendNotification("textDocument/didClose", params);
}

QJsonValue AngelLsp::requestDocumentSymbol(const QString &uri, int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject p;
    p["textDocument"] = QJsonObject{{"uri", uri}};
    return sendRequestSync("textDocument/documentSymbol", p, timeoutMs);
}

QJsonValue AngelLsp::requestSemanticTokensFull(const QString &uri,
                                               int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject p;
    p["textDocument"] = QJsonObject{{"uri", uri}};
    return sendRequestSync("textDocument/semanticTokens/full", p, timeoutMs);
}

QJsonValue AngelLsp::requestCompletion(const QString &uri, int line,
                                       int character, int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;
    QJsonObject p;
    p["textDocument"] = QJsonObject{{"uri", uri}};
    p["position"] = pos;
    return sendRequestSync("textDocument/completion", p, timeoutMs);
}

QJsonValue AngelLsp::requestHover(const QString &uri, int line, int character,
                                  int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;
    QJsonObject p;
    p["textDocument"] = QJsonObject{{"uri", uri}};
    p["position"] = pos;
    return sendRequestSync("textDocument/hover", p, timeoutMs);
}

QJsonValue AngelLsp::requestDefinition(const QString &uri, int line,
                                       int character, int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;
    QJsonObject p;
    p["textDocument"] = QJsonObject{{"uri", uri}};
    p["position"] = pos;
    return sendRequestSync("textDocument/definition", p, timeoutMs);
}

QJsonValue AngelLsp::requestReferences(const QString &uri, int line,
                                       int character, bool includeDeclaration,
                                       int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;
    QJsonObject context;
    context["includeDeclaration"] = includeDeclaration;
    QJsonObject p;
    p["textDocument"] = QJsonObject{{"uri", uri}};
    p["position"] = pos;
    p["context"] = context;
    return sendRequestSync("textDocument/references", p, timeoutMs);
}

QJsonValue AngelLsp::requestSignatureHelp(const QString &uri, int line,
                                          int character, int timeoutMs) {
    if (!m_proc)
        return {};

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;
    QJsonObject p;
    p["textDocument"] = QJsonObject{{"uri", uri}};
    p["position"] = pos;
    return sendRequestSync("textDocument/signatureHelp", p, timeoutMs);
}

QJsonValue AngelLsp::requestResolve(const QJsonValue &symbol, int timeoutMs) {
    if (!m_proc)
        return {};

    return sendRequestSync("completionItem/resolve", symbol, timeoutMs);
}

qint64 AngelLsp::sendRequest(const QString &method, const QJsonValue &params,
                             int) {
    if (!m_proc)
        return -1;

    qint64 id = m_nextId++;
    QJsonObject obj;
    obj["jsonrpc"] = "2.0";
    obj["id"] = id;
    obj["method"] = method;
    if (!params.isNull())
        obj["params"] = params;
    // record method for this id
    m_outstandingRequests[id] = method;
    sendMessage(obj);
    return id;
}

void AngelLsp::changeDocument(const QString &uri, qint64 version,
                              const LSP::TextDocumentContentChangeEvent &e) {
    if (!m_proc)
        return;

    QJsonArray changes;

    QJsonObject v;
    v["range"] = jsonLSPDocRange(e.range);
    v["rangeLength"] = e.rangeLength;
    v["text"] = e.text;
    changes.append(v);

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", uri}, {"version", version}};
    params["contentChanges"] = changes;

    sendNotification("textDocument/didChange", params);
}

QJsonValue AngelLsp::sendRequestSync(const QString &method,
                                     const QJsonValue &params, int timeoutMs) {
    if (!m_proc)
        return {};

    qint64 id = m_nextId++;
    QJsonObject obj;
    obj["jsonrpc"] = "2.0";
    obj["id"] = id;
    obj["method"] = method;
    if (!params.isNull())
        obj["params"] = params;

    // record method for this id
    m_outstandingRequests[id] = method;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    m_pendingLoops[id] = &loop;
    sendMessage(obj);
    timer.start(timeoutMs);
    loop.exec();
    m_pendingLoops.remove(id);

    if (timer.isActive()) {
        if (m_pendingResponses.contains(id)) {
            QJsonValue val = m_pendingResponses.take(id);
            // also clean outstandingRequests if present
            if (m_outstandingRequests.contains(id))
                m_outstandingRequests.remove(id);
            return val;
        }
        if (m_outstandingRequests.contains(id))
            m_outstandingRequests.remove(id);
        return {};
    } else {
        // timeout: remove outstandingRequests to avoid leak
        if (m_outstandingRequests.contains(id))
            m_outstandingRequests.remove(id);
        qWarning("AngelLSP: request timeout for %s id %lld",
                 qUtf8Printable(method), id);
        return {};
    }
}

void AngelLsp::sendNotification(const QString &method,
                                const QJsonValue &params) {
    if (!m_proc)
        return;

    QJsonObject obj;
    obj["jsonrpc"] = "2.0";
    obj["method"] = method;
    if (!params.isNull()) {
        obj["params"] = params;
    }
    sendMessage(obj);
}

void AngelLsp::sendCancelRequest(int id) {
    if (!m_proc)
        return;

    if (id <= 0)
        return;
    QJsonObject p;
    p["id"] = id;
    sendNotification("$/cancelRequest", p);
    qDebug("AngelLSP: sent cancelRequest for id %d", id);
}

void AngelLsp::shutdownAndExit() {
    if (!m_proc)
        return;

    sendRequestSync("shutdown", {}, 2000);
    sendNotification("exit", {});
    stop();
}

void AngelLsp::handleStdout() {
    if (!m_proc)
        return;
    QByteArray chunk = m_proc->readAllStandardOutput();
    if (chunk.isEmpty())
        return;
    m_stdoutBuffer.append(chunk);

    // process possibly multiple messages (handle sticky packets and
    // fragmentation)
    while (true) {
        // find header/body separator (try CRLF CRLF first, then LF LF)
        int headerEnd = m_stdoutBuffer.indexOf("\r\n\r\n");
        int sepLen = 4;
        if (headerEnd == -1) {
            headerEnd = m_stdoutBuffer.indexOf("\n\n");
            sepLen = 2;
        }
        if (headerEnd == -1) {
            // no full header yet
            break;
        }

        QByteArray headerBytes = m_stdoutBuffer.left(headerEnd);
        QString headerStr = QString::fromUtf8(headerBytes);

        // find Content-Length header (case-insensitive)
        int contentLength = -1;
        int idx = -1;
        idx = headerBytes.indexOf("Content-Length:");
        if (idx == -1)
            idx = headerBytes.indexOf("content-length:");
        if (idx == -1)
            idx = headerBytes.indexOf("Content-length:");
        if (idx == -1)
            idx = headerBytes.indexOf("content-Length:");
        if (idx == -1) {
            qWarning("AngelLSP: no Content-Length header found in: %s",
                     qUtf8Printable(headerStr.sliced(200)));
            // consume header portion and continue
            m_stdoutBuffer.remove(0, headerEnd + sepLen);
            continue;
        }

        int lenStart = idx + QByteArrayLiteral("Content-Length:").size();
        // determine end-of-line after lenStart
        int crlfPos = headerBytes.indexOf("\r\n", lenStart);
        int lfPos = headerBytes.indexOf("\n", lenStart);
        int lenEnd = -1;
        if (crlfPos != -1)
            lenEnd = crlfPos;
        else if (lfPos != -1)
            lenEnd = lfPos;
        else
            lenEnd = headerBytes.size();

        QByteArray lenBytes =
            headerBytes.sliced(lenStart, lenEnd - lenStart).trimmed();
        bool ok = false;
        int len = lenBytes.toInt(&ok);
        if (!ok || len < 0) {
            qWarning() << "AngelLSP: invalid Content-Length value:"
                       << QString::fromUtf8(lenBytes);
            m_stdoutBuffer.remove(0, headerEnd + sepLen);
            continue;
        }
        contentLength = len;

        int totalNeeded = headerEnd + sepLen + contentLength;
        if (m_stdoutBuffer.size() < totalNeeded) {
            // body not yet complete; wait for more data
            break;
        }

        QByteArray body =
            m_stdoutBuffer.sliced(headerEnd + sepLen, contentLength);
        m_stdoutBuffer.remove(0, totalNeeded);

        QJsonParseError parseErr{};
        QJsonDocument doc = QJsonDocument::fromJson(body, &parseErr);
        if (parseErr.error != QJsonParseError::NoError) {
            qWarning() << "AngelLSP: JSON parse error:"
                       << parseErr.errorString()
                       << "body:" << QString::fromUtf8(body.left(200));
            // continue to next message if any
            continue;
        }
        if (!doc.isObject()) {
            qWarning("AngelLSP: unexpected JSON (not object) body");
            continue;
        }
        QJsonObject msg = doc.object();
        // delegate to existing handler
        handleIncomingMessage(msg);
        // loop to handle next message in buffer
    }
}

void AngelLsp::handleStderr() {
    if (!m_proc)
        return;
    QByteArray chunk = m_proc->readAllStandardError();
    if (chunk.isEmpty())
        return;
    QString s = QString::fromUtf8(chunk);
    qWarning().noquote() << "[server-stderr]" << s;
}

void AngelLsp::handleProcessFinished(int exitCode,
                                     QProcess::ExitStatus status) {
    Q_EMIT serverExited(exitCode, status);
}

void AngelLsp::sendMessage(const QJsonObject &msg) {
    if (!m_proc || m_proc->state() != QProcess::Running) {
        qWarning() << "AngelLSP: sendMessage but process not running";
        return;
    }
    QJsonDocument doc(msg);
    QByteArray body = doc.toJson(QJsonDocument::Compact);
    QByteArray header = QByteArrayLiteral("Content-Length: ") +
                        QByteArray::number(body.size()) +
                        QByteArrayLiteral("\r\n\r\n");
    QByteArray out = header + body;
    m_proc->write(out);
    m_proc->waitForBytesWritten(1000);
    qDebug() << "[AngelLSP->server]" << QString::fromUtf8(body);
}

void AngelLsp::handleIncomingMessage(const QJsonObject &msg) {
    // Response (id + result/error)
    if (msg.contains("id") &&
        (msg.contains("result") || msg.contains("error"))) {
        int id = msg["id"].toInt();
        QJsonValue payload;
        if (msg.contains("result")) {
            payload = msg["result"];
            m_pendingResponses[id] = payload;
        } else {
            payload = msg["error"];
            m_pendingResponses[id] = payload;
        }

        // determine method if we recorded it
        QString method;
        if (m_outstandingRequests.contains(id)) {
            method = m_outstandingRequests.value(id);
            m_outstandingRequests.remove(id);
        }

        if (m_pendingLoops.contains(id)) {
            QEventLoop *loop = m_pendingLoops[id];
            if (loop)
                loop->quit();
        }
        Q_EMIT requestFinished(id, payload);
        return;
    }

    // Server -> client request (has method & id)
    if (msg.contains("method") && msg.contains("id")) {
        QString method = msg["method"].toString();
        int id = msg["id"].toInt();
        auto params = msg.value("params");

        // emit notification that server requested something (allow external
        // handling)
        Q_EMIT serverRequestReceived(method, params, id);

        // Implement standard minimal replies the server expects
        if (method == QStringLiteral("client/registerCapability")) {
            // server registers notifications/requests; respond with null result
            QJsonObject resp;
            resp["jsonrpc"] = "2.0";
            resp["id"] = id;
            resp["result"] = QJsonValue();
            sendMessage(resp);
            return;
        }
        if (method == QStringLiteral("workspace/configuration")) {
            QJsonArray result;

            QJsonObject o;
            o["suppressAnalyzerErrors"] = false;
            o["includePath"] = QJsonArray();
            o["implicitMutualInclusion"] = false;
            o["hoistEnumParentScope"] = false;
            o["explicitPropertyAccessor"] = true;
            o["allowUnicodeIdentifiers"] = true;
            o["supportsForEach"] = true;
            o["characterLiterals"] = false;
            o["supportsTypedEnumerations"] = true;
            o["supportsDigitSeparators"] = false;
            o["builtinStringType"] = "string";
            o["builtinArrayType"] = "array";
            QJsonObject fmt;
            fmt["maxBlankLines"] = 1;
            fmt["indentSpaces"] = 4;
            fmt["useTabIndent"] = false;
            o["formatter"] = fmt;
            QJsonObject trace;
            trace["server"] = "off";
            o["trace"] = trace;
            result.append(o);

            QJsonObject resp;
            resp["jsonrpc"] = "2.0";
            resp["id"] = id;
            resp["result"] = result;
            sendMessage(resp);
            return;
        }

        // Unknown server request: reply null result to avoid server waiting
        {
            QJsonObject resp;
            resp["jsonrpc"] = "2.0";
            resp["id"] = id;
            resp["result"] = QJsonValue();
            sendMessage(resp);
            return;
        }
    }

    // Notification (no id)
    if (msg.contains("method") && !msg.contains("id")) {
        QString method = msg["method"].toString();
        QJsonValue params = msg.value("params");

        if (method == QStringLiteral("textDocument/publishDiagnostics")) {
            if (params.isObject()) {
                QJsonObject o = params.toObject();
                QString uri = o.value("uri").toString();
                QJsonArray diags = o.value("diagnostics").toArray();
                auto total = diags.size();

                QList<LSP::Diagnostics> ds;
                for (qsizetype i = 0; i < total; ++i) {
                    auto item = diags.at(i);
                    LSP::Diagnostics d;
                    d.message = item["message"].toString();
                    d.range = readLSPDocRange(item.toObject());
                    d.severity =
                        LSP::DiagnosticSeverity(item["severity"].toInt());
                    ds.append(d);
                }
                Q_EMIT diagnosticsPublished(uri, ds);
            }
            return;
        }

        if (method == QStringLiteral("window/logMessage")) {
            if (params.isObject()) {
                QJsonObject o = params.toObject();
                int type = o.value("type").toInt();
                QString message = o.value("message").toString();
                qInfo() << "[logMessage]" << message;
                Q_EMIT logMessage(LSP::MessageType(type), message);
            }
            return;
        }

        // emit general notification
        Q_EMIT notificationReceived(method, params);
        return;
    }
}

LSP::Range AngelLsp::readLSPDocRange(const QJsonObject &items) {
    auto v = items["range"].toObject();
    LSP::Range ret;
    ret.start = readLSPDocLocation(v.value("start").toObject());
    ret.end = readLSPDocLocation(v.value("end").toObject());
    return ret;
}

LSP::Location AngelLsp::readLSPDocLocation(const QJsonObject &items) {
    LSP::Location ret;
    ret.line = items.value("line").toInt();
    ret.character = items.value("character").toInt();
    return ret;
}

QJsonObject AngelLsp::jsonLSPDocRange(const LSP::Range &range) {
    QJsonObject r;
    r["start"] = jsonLSPDocLocation(range.start);
    r["end"] = jsonLSPDocLocation(range.end);
    return r;
}

QJsonObject AngelLsp::jsonLSPDocLocation(const LSP::Location &loc) {
    QJsonObject r;
    r["line"] = loc.line;
    r["character"] = loc.character;
    return r;
}
