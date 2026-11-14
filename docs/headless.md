# 无头执行与常驻脚本服务 / Headless execution and persistent script service

```sh
WingHexExplorer2 --headless task.luau
WingHexExplorer2 --headless --live
WingHexExplorer2 --headless --live --address 127.0.0.1 --port 0
```

一次性模式必须指定一个脚本，执行完成后退出；live 不接受位置脚本参数。服务加载设置、插件及脚本绑定一次，通过独立 TCP socket 接收任务，不参与 GUI 单实例转发。脚本禁用时，在加载插件前返回 5，不修改设置。
One-shot mode requires one script and exits when it returns. Live mode accepts no positional scripts: it loads settings, plugins and bindings once and accepts tasks over a separate TCP socket. It does not participate in GUI single-instance forwarding. Disabled scripting exits with code 5 before loading plugins, without changing settings.

不加载编辑器、LSP、主窗口或 UI 脚本模块。所有 headless 模式中 `input()` 直接报错；服务不读取 stdin。脚本正常输出使用 stdout，警告和错误使用 stderr；多个任务的输出可交错，没有宿主添加的任务前缀。宿主日志和实际监听端点只写 stderr，JSON-RPC 只走管理 socket。旧 `host` 模块及 `host.exit()`、stdin 回调已移除。
Editors, LSP, main windows and UI script modules are not initialized. `input()` fails in every headless mode; the service never reads stdin. Script output goes to stdout, warnings and errors to stderr. Concurrent tasks may interleave output, without host-added task prefixes. Host diagnostics and the actual endpoint go only to stderr; JSON-RPC uses only the management socket. The former `host` module, `host.exit()` and stdin callbacks have been removed.

## 客户端示例 / Client examples

```sh
python examples/headless/rpc-client.py task.execute '{"taskId":"echo","path":"/absolute/path/socket-service.luau"}'
python examples/headless/rpc-client.py task.list
python examples/headless/rpc-client.py task.get '{"taskId":"echo"}'
python examples/headless/rpc-client.py service.subscribe '{"enabled":true}' --watch
python examples/headless/rpc-client.py task.stop '{"taskId":"echo"}'
python examples/headless/rpc-client.py task.remove '{"taskId":"echo"}'
python examples/headless/rpc-client.py service.shutdown
```

入口路径相对于服务启动目录解析；远程客户端提供的是服务机器上的路径，不会上传代码或修改服务工作目录。`socket-service.luau` 演示脚本自己的 echo 端口，`timer-task.luau` 演示驻留定时器；两者都由管理协议停止。
Entry paths resolve against the service's startup directory. Remote clients name files on the server; they do not upload code or change its working directory. `socket-service.luau` exposes its own echo port, while `timer-task.luau` runs a persistent timer. Both are stopped through the management protocol.

## JSON-RPC 协议 / JSON-RPC protocol

默认监听 `127.0.0.1:18888`，`--address` 接受 IP 地址，`--port 0` 由系统分配。`--address`、`--port`、`--cert` 和 `--key` 仅用于 `--headless --live`。每个 UTF-8 JSON 消息使用以下帧头，长度为字节数而非字符数：
The default listener is `127.0.0.1:18888`; `--address` takes an IP address and `--port 0` requests an OS-assigned port. `--address`, `--port`, `--cert` and `--key` require `--headless --live`. Each UTF-8 JSON message uses this header; length counts bytes, not characters:

```text
Content-Length: <bytes>\r\n\r\n<JSON body>
```

请求使用 JSON-RPC 2.0，参数为命名对象，支持批量请求；没有 `id` 的通知不产生响应。请求 `id` 与任务 `taskId` 无关。任务 ID 是非空字符串（最多 256 UTF-8 字节），在服务中唯一，直到记录被删除。
Requests use JSON-RPC 2.0 with named object parameters and support batches. Notifications without `id` receive no response. The request `id` is independent of `taskId`. Task IDs are nonempty strings of at most 256 UTF-8 bytes, unique within the service until their records are removed.

| 方法 / Method | 参数 / Parameters | 结果 / Result |
|---|---|---|
| `service.status` | `{}` | 端点、状态、任务数与容量 / Endpoint, state, counts and limits |
| `service.subscribe` | `{ "enabled": true/false }` | 当前任务快照；只影响此连接 / Current task snapshot; affects only this connection |
| `service.shutdown` | `{}` | `{ "accepted": true }`，随后清理退出 / Acknowledgment followed by cleanup and exit |
| `task.execute` | `{ "taskId": "id", "path": "entry.luau" }` | 接受任务，返回 queued 记录 / Accepts a task and returns its queued record |
| `task.list` | `{}` | 全部任务记录 / All task records |
| `task.get` | `{ "taskId": "id" }` | 指定任务记录 / Task record |
| `task.stop` | `{ "taskId": "id" }` | 停止该任务，返回当前记录 / Stops the task and returns its current record |
| `task.remove` | `{ "taskId": "id" }` | 删除 stopped/failed 记录 / Removes a stopped/failed record |

记录包含 `taskId`、规范化 `path`、`state`、UTC 的 `createdAt`/`updatedAt`/`finishedAt`，以及为空或含 `code`、`message` 的 `error`。未结束时 `finishedAt` 为空字符串。任务错误码为 `file-error`、`compilation-error`、`execution-error`、`callback-error` 或 `timeout`。订阅连接收到 `task.changed`（完整记录）和 `task.removed`（`taskId`）通知；客户端断开不会停止任务。
Records contain `taskId`, canonical `path`, `state`, UTC `createdAt`/`updatedAt`/`finishedAt`, and a null `error` or an object with `code` and `message`. `finishedAt` is an empty string until termination. Task error codes are `file-error`, `compilation-error`, `execution-error`, `callback-error` and `timeout`. Subscribers receive `task.changed` with a complete record and `task.removed` with `taskId`. Disconnecting a client never stops its tasks.

状态为 `queued → running → resident`。入口正常返回后继续驻留，包括没有活动资源的任务；停止经过 `stopping → stopped`，编译、入口、异步回调及超时错误进入 `failed`，不结束其他任务或服务。停止或失败后保留记录，显式删除后才可复用 ID。
States progress through `queued → running → resident`. A returned entry remains resident, even without active resources. Stopping progresses through `stopping → stopped`; compilation, entry, asynchronous callback and timeout errors produce `failed` without terminating other tasks or the service. Final records remain until explicitly removed, after which the ID may be reused.

标准 RPC 错误之外，`-32001` 未知任务、`-32002` 重复 ID、`-32003` 状态冲突、`-32004` 容量不足、`-32005` 文件无效、`-32006` 服务正在退出。文件错误在接受任务前返回；接受后的错误保存在任务记录。
Besides standard RPC errors: `-32001` means unknown task, `-32002` duplicate ID, `-32003` state conflict, `-32004` capacity reached, `-32005` invalid file and `-32006` service stopping. File validation errors reject acceptance; subsequent errors are stored in the task record.

上限为 64 个活动任务、1024 条记录、64 个连接、8 KiB 帧头、1 MiB 消息体及每连接 4 MiB 待发送数据。任务历史不会自动淘汰；超出连接缓冲限制会断开相应连接，不影响其他连接。
Limits are 64 active tasks, 1024 records, 64 connections, 8 KiB headers, 1 MiB message bodies and 4 MiB queued output per connection. History is not automatically evicted. Exceeding connection buffer limits disconnects that client without affecting others.

## TLS 与证书 / TLS and certificates

回环地址默认允许普通 TCP。非回环及通配地址必须提供 `--cert server.crt --key server.key` 并启用 TLS；回环也可使用这两个参数强制 TLS。服务端 PEM 证书和未加密私钥必须有效且匹配，使用 Qt TLS 后端支持的 RSA、EC 或 DSA 密钥。客户端必须验证服务端证书。
Loopback listeners allow plain TCP by default. Non-loopback and wildcard addresses require `--cert server.crt --key server.key` and TLS. Providing these options also enables TLS on loopback. The PEM server certificate and unencrypted private key must be valid and match, using RSA, EC or DSA keys supported by the Qt TLS backend. Clients must verify the server certificate.

TLS 客户端必须持有本地已注册证书对应的私钥。服务复用现有 `WingCert` 注册集合，核对客户端证书指纹、有效期及注册文件权限；仅被系统 CA 信任并不足以访问。既有 Ed25519 客户端证书可用于认证。证书目录为服务可执行文件旁的 `certs`，沿用插件证书注册规则，启动时加载；更新后重启服务。插件证书跳过设置不会绕过 RPC 认证。
TLS clients must hold the private key corresponding to a locally registered certificate. The service reuses `WingCert` registration, checking the client fingerprint, validity and registered-file permissions. System CA trust alone does not grant access. Existing Ed25519 client certificates can authenticate. The `certs` directory beside the executable follows existing plugin certificate registration rules and loads at startup; restart after changes. Plugin certificate bypass settings never bypass RPC authentication.

```sh
WingHexExplorer2 --headless --live --address 0.0.0.0 --cert server.crt --key server.key
python examples/headless/rpc-client.py service.status --address server.example --ca server-ca.crt --cert client.crt --key client.key
```

## 调度、托盘和退出 / Scheduling, tray and exit

插件实例和 VM 只初始化一次。任务使用独立 Luau 根线程、沙箱全局变量及文件模块缓存，Qt 对象和回调按上下文归属清理；插件原生状态仍然共享，并非进程隔离。主线程按约 10 ms 时间片执行入口、模块和异步回调，同一任务不会重入。超时按一次入口或回调的执行时间累计，不计驻留或调度等待；编译及同步原生插件调用不能被 Luau 调度器抢占。
Plugins and the VM initialize once. Each task has its own Luau root thread, sandbox globals and file-module cache, with context-owned Qt objects and callbacks. Native plugin state remains shared; this is not process isolation. Entries, modules and asynchronous callbacks use approximately 10 ms time slices on the main thread, without reentering a task. Timeouts accumulate execution time per entry or callback, excluding residency and scheduling waits. Compilation and synchronous native plugin calls cannot be preempted by the Luau scheduler.

有桌面配置时使用桌面 Qt 后端，系统托盘可用时显示菜单，提供任务状态、停止任务和强制清理退出。不创建宿主主窗口、编辑器或托盘辅助进程。没有桌面配置时使用 offscreen；显式 Qt 后端设置会被尊重，无效桌面配置仍可能导致 Qt 初始化失败。
With a configured desktop, the service uses a desktop Qt backend and shows a menu when the system tray is available, listing task states, task stopping and forced cleanup shutdown. It creates no host main window, editor or tray helper process. Without desktop configuration it uses offscreen. Explicit Qt backend settings are respected; invalid desktop configuration may still cause Qt initialization failure.

退出时中断所有任务，释放回调、对象、脚本句柄和线程，通知插件关闭，释放 VM，最后卸载插件。插件否决只记录警告。正常 RPC/托盘退出返回 0，Ctrl+C 返回 130，POSIX SIGTERM 返回 143。一次性成功为 0；退出码 2–9 保留插件配置、脚本初始化、内存不足、脚本禁用、参数、脚本文件、脚本执行及其他初始化错误语义。
Shutdown interrupts all tasks and releases callbacks, objects, script handles and threads, notifies plugins, releases the VM, and finally unloads plugins. Plugin vetoes only produce warnings. Normal RPC/tray shutdown returns 0, Ctrl+C returns 130 and POSIX SIGTERM returns 143. One-shot success returns 0; codes 2–9 retain their plugin configuration, script initialization, out-of-memory, scripting-disabled, arguments, script file, execution and other initialization meanings.
