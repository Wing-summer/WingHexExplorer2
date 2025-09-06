// MIT Licensed
// see https://github.com/Paril/angelscript-ui-debugger

#include "as_debugger_dap.h"
#include "as_debugger.h"
#include <charconv>
#include <dap/session.h>
#include <filesystem>

class asIDBDAPClient {
public:
    asIDBDebugger *dbg;
    std::atomic_bool configuration_complete = false;
    std::atomic_bool terminate = false;
    std::unique_ptr<dap::Session> session;

    asIDBDAPClient(asIDBDebugger *dbg,
                   const std::shared_ptr<dap::ReaderWriter> &socket)
        : dbg(dbg), session(dap::Session::create()) {
        session->setOnInvalidData(dap::kClose);
        session->bind(socket);

        session->registerHandler([&](const dap::InitializeRequest &request) {
            dap::InitializeResponse response;
            response.supportsClipboardContext = true;
            response.supportsConfigurationDoneRequest = true;
            response.supportsDelayedStackTraceLoading = true;
            response.supportsEvaluateForHovers = true;
            response.supportsFunctionBreakpoints = true;
            response.supportsBreakpointLocationsRequest = true;
            response.supportsLoadedSourcesRequest = true;
            response.supportsReadMemoryRequest = true;
            response.supportsExceptionInfoRequest = true;
            return response;
        });

        session->registerHandler([&](const dap::DisconnectRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler(
            [&](const dap::SetBreakpointsRequest &request) {
                return this->HandleRequest(request);
            });
        session->registerHandler(
            [&](const dap::SetFunctionBreakpointsRequest &request) {
                return this->HandleRequest(request);
            });
        session->registerHandler(
            [&](const dap::ConfigurationDoneRequest &request) {
                return this->HandleRequest(request);
            });
        session->registerHandler([&](const dap::ThreadsRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::StackTraceRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::ScopesRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::VariablesRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::PauseRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::ContinueRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::StepOutRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::StepInRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::NextRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::EvaluateRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler([&](const dap::AttachRequest &request) {
            return this->HandleRequest(request);
        });
        session->registerHandler(
            [&](const dap::BreakpointLocationsRequest &request) {
                return this->HandleRequest(request);
            });
        session->registerHandler(
            [&](const dap::LoadedSourcesRequest &response) {
                return this->HandleRequest(response);
            });
        session->registerHandler([&](const dap::SourceRequest &response) {
            return this->HandleRequest(response);
        });
        session->registerHandler([&](const dap::ReadMemoryRequest &response) {
            return this->HandleRequest(response);
        });
        session->registerHandler(
            [&](const dap::ExceptionInfoRequest &response) {
                return this->HandleRequest(response);
            });

        session->registerSentHandler(
            [&](const dap::ResponseOrError<dap::InitializeResponse> &response) {
                OnResponseSent(response);
            });
        session->registerSentHandler(
            [&](const dap::ResponseOrError<dap::ConfigurationDoneResponse>
                    &response) { OnResponseSent(response); });
    }

    dap::ReadMemoryResponse
    HandleRequest(const dap::ReadMemoryRequest &request) {
        dap::ReadMemoryResponse response{};

        uintptr_t ptr;
        std::from_chars(request.memoryReference.c_str(),
                        request.memoryReference.c_str() +
                            request.memoryReference.size(),
                        ptr);

        asIDBVariable *var = reinterpret_cast<asIDBVariable *>(ptr);

        uint8_t *data = var->address.ResolveAs<uint8_t>();
        // FIXME: engine?
        size_t size = var->address.GetSize(dbg->cache->ctx->GetEngine());
        uint8_t *data_end = data + size;

        int64_t offset =
            request.offset.has_value() ? (int64_t)request.offset.value() : 0;
        int64_t count = request.count;

        data += offset;

        uint8_t *end = std::min(data_end, data + count);

        if (!count || !(end - data) || data >= end)
            return response;

        std::string buf((intptr_t)(end - data), '\0');
        memcpy(buf.data(), data, buf.size());
        response.address = fmt::format("{}", reinterpret_cast<uintptr_t>(data));
        response.data = base64::to_base64(buf);

        return response;
    }

    dap::ExceptionInfoResponse
    HandleRequest(const dap::ExceptionInfoRequest &request) {
        dap::ExceptionInfoResponse response{};

        response.breakMode = "always";
        response.description = dbg->cache->ctx->GetExceptionString();
        dap::ExceptionDetails details{};
        details.message = dbg->cache->ctx->GetExceptionString();
        response.details = details;
        response.exceptionId = "exception";

        return response;
    }

    dap::AttachResponse HandleRequest(const dap::AttachRequest &request) {
        return dap::AttachResponse{};
    }

    dap::LoadedSourcesResponse
    HandleRequest(const dap::LoadedSourcesRequest &request) {
        dap::LoadedSourcesResponse response{};

        for (auto &source : dbg->workspace->sections) {
            auto &src = response.sources.emplace_back();
            src.name = source.section;
            src.path = dbg->workspace->SectionToPath(source.section);
            if (dbg->workspace->SectionIsVirtual(source.section))
                src.sourceReference = source.ref;
        }

        return response;
    }

    dap::ResponseOrError<dap::SourceResponse>
    HandleRequest(const dap::SourceRequest &request) {
        dap::SourceResponse response{};

        auto source_it = dbg->workspace->sections.find(
            std::string_view(request.source->name.value()));

        if (source_it == dbg->workspace->sections.end())
            return dap::Error("can't find source");

        response.content =
            dbg->workspace->SectionSource(request.source->name.value());

        return response;
    }

    dap::BreakpointLocationsResponse
    HandleRequest(const dap::BreakpointLocationsRequest &request) {
        dap::BreakpointLocationsResponse response{};

        if (request.source.name.has_value()) {
            std::scoped_lock lock(dbg->mutex);
            if (auto linecols = dbg->workspace->potential_breakpoints.find(
                    request.source.name.value());
                linecols != dbg->workspace->potential_breakpoints.end()) {
                // FIXME: STL equal_range maybe?
                for (auto &lc : linecols->second) {
                    if (lc.line < request.line)
                        continue;
                    else if (request.endLine.has_value()
                                 ? (lc.line > request.endLine.value())
                                 : (lc.line > request.line))
                        continue;
                    else if (request.column.has_value() &&
                             lc.col < request.column.value())
                        continue;
                    else if (request.endColumn.has_value() &&
                             lc.col > request.endColumn.value())
                        continue;

                    dap::BreakpointLocation bp =
                        response.breakpoints.emplace_back();

                    bp.line = lc.line;
                    bp.column = lc.col;

                    response.breakpoints.push_back(bp);
                }
            }
        }

        return response;
    }

    dap::DisconnectResponse
    HandleRequest(const dap::DisconnectRequest &request) {
        terminate = true;
        return dap::DisconnectResponse{};
    }

    dap::SetBreakpointsResponse
    HandleRequest(const dap::SetBreakpointsRequest &request) {
        dap::SetBreakpointsResponse response;

        if (request.source.path) {
            auto rel =
                dbg->workspace->PathToSection(request.source.path.value());
            auto pathstr = std::filesystem::path(rel).generic_string();

            std::scoped_lock lock(dbg->mutex);

            dbg->workspace->AddSection(pathstr);

            if (auto it = dbg->breakpoints.find(pathstr);
                it != dbg->breakpoints.end())
                dbg->breakpoints.erase(it);

            if (request.breakpoints && !request.breakpoints->empty()) {
                auto &pathstrptr = *dbg->workspace->sections.find(pathstr);
                auto &positions =
                    dbg->workspace->potential_breakpoints[pathstrptr.section];
                auto it = dbg->breakpoints.find(pathstrptr.section);

                for (auto &bp : *request.breakpoints) {
                    asIDBLineCol closest{-1, -1};

                    // FIXME: there's probably some STL methods that can
                    // speed up this lookup.
                    for (auto &pos : positions) {
                        if (pos.line != bp.line)
                            continue;
                        else if (bp.column.has_value() &&
                                 pos.col < bp.column.value())
                            continue;

                        closest.line = pos.line;
                        closest.col = (closest.col == -1)
                                          ? pos.col
                                          : std::min(closest.col, pos.col);
                    }

                    auto &placed_bp = response.breakpoints.emplace_back();

                    if (closest.line == -1) {
                        placed_bp.verified = false;
                        placed_bp.reason = "failed";
                        placed_bp.message =
                            "No suspend instruction can be found";
                        continue;
                    }

                    placed_bp.line = closest.line;
                    placed_bp.column = closest.col;
                    placed_bp.verified = true;

                    if (it == dbg->breakpoints.end())
                        it = dbg->breakpoints
                                 .insert({pathstrptr.section,
                                          asIDBSectionBreakpoints{}})
                                 .first;

                    it->second.push_back({(int)closest.line, (int)closest.col});
                }
            }
        }

        return response;
    }

    dap::SetFunctionBreakpointsResponse
    HandleRequest(const dap::SetFunctionBreakpointsRequest &request) {
        std::scoped_lock lock(dbg->mutex);
        dap::SetFunctionBreakpointsResponse response{};

        dbg->function_breakpoints.clear();

        for (auto &bp : request.breakpoints) {
            dbg->function_breakpoints.insert(bp.name);

            dap::Breakpoint bp0{};
            // TODO: validate name
            // TODO: link line/col
            bp0.verified = true;
            response.breakpoints.emplace_back(std::move(bp0));
        }
        return response;
    }

    dap::ConfigurationDoneResponse
    HandleRequest(const dap::ConfigurationDoneRequest &request) {
        return {};
    }

    dap::ThreadsResponse HandleRequest(const dap::ThreadsRequest &request) {
        dap::ThreadsResponse response{};
        response.threads.push_back({1, "Main"});
        return response;
    }

    dap::StackTraceResponse
    HandleRequest(const dap::StackTraceRequest &request) {
        std::scoped_lock lock(dbg->mutex);
        dap::StackTraceResponse response{};

        if (!dbg->cache || !dbg->cache->ctx) {
            response.totalFrames = 1;
            auto &frame = response.stackFrames.emplace_back();
            frame.id = -1;
            frame.presentationHint = "label";
            frame.name = "system func";
            return response;
        }

        dbg->cache->CacheCallstack();

        int64_t start = request.startFrame.has_value()
                            ? (int64_t)(*request.startFrame)
                            : (int64_t)0;
        int64_t levels =
            (request.levels.has_value() && request.levels.value() > 0)
                ? (int64_t)(*request.levels)
                : (int64_t)9999;

        response.totalFrames = dbg->cache->call_stack.size();

        for (asUINT i = 0;
             i < levels && (size_t)(i + start) < dbg->cache->call_stack.size();
             i++) {
            auto &frame = response.stackFrames.emplace_back();
            auto &stack = dbg->cache->call_stack[i + start];
            frame.id = stack.id;
            if (stack.scope.offset != SCOPE_SYSTEM) {
                frame.line = stack.row;
                frame.column = stack.column;
                dap::Source src;
                src.path = dbg->workspace->SectionToPath(stack.section);
                src.name = std::string(stack.section);
                frame.source = std::move(src);
            } else
                frame.presentationHint = "label";
            frame.name = stack.declaration;
        }

        return response;
    }

    dap::ResponseOrError<dap::ScopesResponse>
    HandleRequest(const dap::ScopesRequest &request) {
        std::scoped_lock lock(dbg->mutex);

        dap::ScopesResponse response{};

        if (!dbg->cache)
            return response;

        bool found = false;

        dbg->cache->CacheGlobals();

        for (auto &stack : dbg->cache->call_stack) {
            if (stack.id != request.frameId)
                continue;

            if (!stack.scope.locals->namedProps.empty() ||
                !stack.scope.locals->indexedProps.empty()) {
                auto &scope = response.scopes.emplace_back();
                scope.name = "Locals";
                scope.presentationHint = "locals";
                scope.namedVariables = stack.scope.locals->namedProps.size();
                scope.indexedVariables =
                    stack.scope.locals->indexedProps.size();
                scope.variablesReference =
                    stack.scope.locals->expandRefId.value();
            }
            if (!stack.scope.parameters->namedProps.empty() ||
                !stack.scope.parameters->indexedProps.empty()) {
                auto &scope = response.scopes.emplace_back();
                scope.name = "Parameters";
                scope.presentationHint = "parameters";
                scope.namedVariables =
                    stack.scope.parameters->namedProps.size();
                scope.indexedVariables =
                    stack.scope.locals->indexedProps.size();
                scope.variablesReference =
                    stack.scope.parameters->expandRefId.value();
            }
            if (!stack.scope.registers->namedProps.empty() ||
                !stack.scope.registers->indexedProps.empty()) {
                auto &scope = response.scopes.emplace_back();
                scope.name = "Registers";
                scope.presentationHint = "registers";
                scope.namedVariables = stack.scope.registers->namedProps.size();
                scope.indexedVariables =
                    stack.scope.locals->indexedProps.size();
                scope.variablesReference =
                    stack.scope.registers->expandRefId.value();
            }
            found = true;
            break;
        }

        if (!found)
            return dap::Error{"invalid stack ID"};

        if (!dbg->cache->globals->namedProps.empty() ||
            !dbg->cache->globals->indexedProps.empty()) {
            auto &scope = response.scopes.emplace_back();
            scope.name = "Globals";
            scope.presentationHint = "globals";
            scope.namedVariables = dbg->cache->globals->namedProps.size();
            scope.indexedVariables = dbg->cache->globals->indexedProps.size();
            scope.variablesReference = dbg->cache->globals->expandRefId.value();
            scope.expensive = true;
        }

        return response;
    }

    dap::ResponseOrError<dap::VariablesResponse>
    HandleRequest(const dap::VariablesRequest &request) {
        std::scoped_lock lock(dbg->mutex);
        auto varit = dbg->cache->variable_refs.find(request.variablesReference);

        if (varit == dbg->cache->variable_refs.end())
            return dap::Error("invalid variablesReference");

        auto varContainer = varit->second.lock();
        dap::VariablesResponse response{};

        varContainer->Evaluate();
        varContainer->Expand();

        auto emplace_var = [&](const asIDBVariable::Ptr &local) {
            auto &var = response.variables.emplace_back();
            local->Evaluate();
            var.name = local->identifier.Combine();
            var.type = dap::string(local->typeName);
            if (local->getter) {
                dap::VariablePresentationHint hint{};
                hint.lazy = true;
                var.presentationHint = hint;
                var.value = "(...)";
            } else {
                var.value =
                    local->value.empty() ? local->typeName : local->value;
            }
            var.namedVariables = local->namedProps.size();
            var.indexedVariables = local->indexedProps.size();
            var.variablesReference = local->expandRefId.value_or(0);
            var.memoryReference = fmt::format(
                "{}", reinterpret_cast<uintptr_t>(local->ptr.lock().get()));
        };

        if (!request.filter.has_value() || request.filter.value() == "named")
            for (auto &var : varContainer->namedProps)
                emplace_var(var);

        if (!request.filter.has_value() ||
            request.filter.value() == "indexed") {
            int64_t start =
                request.start.has_value() ? (int64_t)request.start.value() : 0;
            int64_t count = request.count.has_value()
                                ? (int64_t)request.count.value()
                                : varContainer->indexedProps.size();

            for (int64_t i = start; i < count; i++)
                emplace_var(varContainer->indexedProps[i]);
        }

        return response;
    }

    dap::PauseResponse HandleRequest(const dap::PauseRequest &request) {
        dbg->SetAction(asIDBAction::Pause);
        return {};
    }

    dap::ContinueResponse HandleRequest(const dap::ContinueRequest &request) {
        dbg->SetAction(asIDBAction::Continue);
        return {};
    }

    dap::StepOutResponse HandleRequest(const dap::StepOutRequest &request) {
        dbg->SetAction(asIDBAction::StepOut);
        return {};
    }

    dap::StepInResponse HandleRequest(const dap::StepInRequest &request) {
        dbg->SetAction(asIDBAction::StepInto);
        return {};
    }

    dap::NextResponse HandleRequest(const dap::NextRequest &request) {
        dbg->SetAction(asIDBAction::StepOver);
        return {};
    }

    dap::ResponseOrError<dap::EvaluateResponse>
    HandleRequest(const dap::EvaluateRequest &request) {
        std::scoped_lock lock(dbg->mutex);
        dap::EvaluateResponse response{};

        if (!dbg->cache)
            return response;

        std::optional<int> stack_index = std::nullopt;

        if (request.frameId.has_value()) {
            for (size_t i = 0; i < dbg->cache->call_stack.size(); i++)
                if (request.frameId.value() == dbg->cache->call_stack[i].id) {
                    stack_index = int(i);
                    break;
                }
        }

        auto result =
            dbg->cache->ResolveExpression(request.expression, stack_index);

        if (!result.has_value())
            return dap::Error{result.error().data()};

        auto var = result.value().lock();

        var->Evaluate();

        response.type = dap::string(var->typeName);
        response.result = var->value.empty() ? var->typeName : var->value;
        response.variablesReference = var->expandRefId.value_or(0);

        return response;
    }

    void OnResponseSent(
        const dap::ResponseOrError<dap::InitializeResponse> &response) {
        {
            std::scoped_lock lock(dbg->mutex);
            dbg->breakpoints.clear();
        }
        session->send(dap::InitializedEvent());
    }

    void OnResponseSent(
        const dap::ResponseOrError<dap::ConfigurationDoneResponse> &response) {
        configuration_complete = true;
    }
};

asIDBDAPServer::asIDBDAPServer(int port, asIDBDebugger *debugger)
    : port(port), dbg(debugger) {}

/*virtual*/ asIDBDAPServer::~asIDBDAPServer() { StopServer(); }

void asIDBDAPServer::ClientConnected(
    const std::shared_ptr<dap::ReaderWriter> &socket) {
    client = std::make_unique<asIDBDAPClient>(dbg, socket);
}

void asIDBDAPServer::ClientError(const char *msg) {}

void asIDBDAPServer::StartServer() {
    server = dap::net::Server::create();

    server->start(
        port,
        [&](const std::shared_ptr<dap::ReaderWriter> &socket) {
            this->ClientConnected(socket);
        },
        [&](const char *msg) { this->ClientError(msg); });
}

void asIDBDAPServer::StopServer() {
    if (client)
        client.reset();
    server.reset();
}

void asIDBDAPServer::Tick() {
    if (client && client->terminate)
        client.reset();
}

void asIDBDAPServer::SendEventToClient(const dap::TypeInfo *typeinfo,
                                       const void *event) {
    if (client)
        client->session->send(typeinfo, event);
}
