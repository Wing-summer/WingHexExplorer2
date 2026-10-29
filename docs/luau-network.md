# Network sockets / 网络 socket

Import `local network = require("@network")` or `require("@b/network")`. There is no global `network`. `network.tcp_socket` and `require("@qt").QTcpSocket` are the same class table; either load order is supported. Importing network does not require importing Qt, and does not register widgets. Methods use snake_case; enum names retain Qt spelling.

使用 `require("@network")` 或 `require("@b/network")` 导入，没有全局 `network`。简短别名和 `qt` 下的 Qt 类名复用同一类型及所有权；加载网络模块不要求导入 Qt，也不会注册控件。方法采用 snake_case，枚举沿用 Qt 名称。

| network | Qt |
|---|---|
| abstract_socket | QAbstractSocket (enums and interface, no constructor / 枚举及接口，不可构造) |
| tcp_socket / tcp_server | QTcpSocket / QTcpServer |
| udp_socket / datagram | QUdpSocket / QNetworkDatagram |
| local_socket / local_server | QLocalSocket / QLocalServer |
| ssl_socket / ssl_server | QSslSocket / QSslServer |
| host_address / host_info | QHostAddress / QHostInfo |
| interface / address_entry / proxy | QNetworkInterface / QNetworkAddressEntry / QNetworkProxy |
| ssl_configuration / ssl_certificate / ssl_key / ssl_error / ssl_cipher | Corresponding QSsl classes / 对应 QSsl 类型 |
| ssl | QSsl enum namespace / 枚举命名空间 |

Additional required QtNetwork value types are discovered from signatures and exported with snake_case aliases after removing the leading `Q`. Shared types such as ustring and bytearray retain their existing names. HTTP and WebSocket are not selected by this configuration.

签名依赖的其他 QtNetwork 值类型由扫描器补齐，去掉 `Q` 前缀后转为 snake_case。ustring、bytearray 等基础类型继续复用。当前配置不选择 HTTP 和 WebSocket。

## I/O and events / 读写与事件

`write()` accepts a Lua string or bytearray, including embedded zero bytes; `read_all()` returns a binary Lua string. Byte counts use the existing 64-bit integer bindings. Use `receive_datagram()` for UDP payload, sender/receiver addresses and ports. Raw writable buffer-pointer overloads are omitted. Synchronous `wait_for_*` methods retain Qt semantics and defaults: they block their owning thread. Prefer signals in the application UI thread.

`write()` 接受含零字节的 Lua string 或 bytearray，`read_all()` 返回二进制 Lua string；字节计数沿用现有 64 位整数绑定。UDP 使用 `receive_datagram()` 获取数据及地址、端口。无法安全表达的原始可写缓冲区重载不公开。同步 `wait_for_*` 保留 Qt 默认参数和行为，会阻塞所属线程；应用 GUI 线程宜使用信号。

`socket:connect("state_changed", function(state) ... end)` receives the scanned parameters. Native Qt names and full signatures, such as `bytesWritten(qint64)`, remain accepted. Ambiguous overload names require a full signature. Generated socket signals are events, not callable methods. Value arguments are copied at emission; QObject arguments use guarded references. Unsupported temporary pointer arguments are recorded in the generated exclusion report.

信号回调保留扫描得到的参数；兼容 Qt 原始名称和完整签名，重载不明确时必须指定完整签名。信号不作为普通方法公开。值参数在发射时复制，QObject 参数使用生命周期保护引用；不支持的临时指针参数进入生成排除报告。

Queued readiness notifications can outlive the available data: drain `has_pending_datagrams()` / `has_pending_connections()` before reading or accepting.

排队通知可能在数据已被处理后才到达；读取数据报或接受连接前，使用 `has_pending_datagrams()` / `has_pending_connections()` 检查并消费队列。

Delivery is queued on the owning event loop and runs on a separate protected Lua coroutine, preserving a suspended script’s stack. Callbacks cannot yield; keep a top-level script coroutine running with `coroutine.yield()` while waiting for events. Returning, stopping, or closing that script context cancels queued callbacks, disconnects its connections and releases its unparented network objects. Child objects follow Qt ownership; returned accepted connections are borrowed from their server and become invalid after server destruction. Context cleanup does not stop another script context. Native socket descriptors retain Qt ownership rules when explicitly adopted. Explicit local-server endpoint removal obeys the existing filesystem-removal setting; closing an owned server still cleans its own endpoint.

回调排队交付，在独立的受保护 Lua 协程中执行，不改变挂起脚本的栈；不允许在回调内 yield。等待网络事件期间，应让顶层脚本通过 `coroutine.yield()` 保持运行。脚本返回、中止或关闭上下文时，取消未执行的回调、断开所属连接并释放无父对象的网络对象。子对象沿用 Qt 所有权，服务器返回的连接为借用对象，服务器销毁后不可继续使用。清理不会停止其他脚本上下文。显式接收原生 socket 句柄时，沿用 Qt 的句柄所有权规则。显式删除本地服务端点遵守既有文件系统删除设置；关闭所属服务器仍会清理自己的端点。

## TLS / TLS

Start from `ssl_configuration.default_configuration()` to retain Qt's default cipher and protocol settings; the plain constructor represents a null configuration. Use per-socket/server SSL configuration for certificates, keys and trusted CAs. Default peer verification is unchanged. Observe `pending_connection_available` for accepted TLS connections; Qt's private emission tag is not a callback argument. `ssl_server:next_pending_connection()` returns an SSL socket with `is_encrypted()` and the other SSL operations. Inspect `ssl_errors` arguments for verification failures. Availability of a runtime TLS backend can be checked with `ssl_socket.supports_ssl()`; a missing backend is distinct from a binding-generation failure. No system trust-store changes are needed by the tests.

使用 `ssl_configuration.default_configuration()` 保留 Qt 的默认密码套件和协议；普通构造器返回空配置。通过每个 socket/server 的 SSL 配置设置证书、密钥和受信任 CA，不改变 Qt 默认验证行为。TLS 服务端使用 `pending_connection_available` 事件接收已完成握手的连接，Qt 私有发射标记不作为回调参数。接受连接后可直接调用 `is_encrypted()` 等 TLS 方法。验证失败可检查 `ssl_errors` 参数。`ssl_socket.supports_ssl()` 可检查运行时后端；缺少后端与生成失败分开报告。测试不修改系统信任库。

## Examples and reports / 示例与报告

See [TCP client](examples/network/tcp-client.luau), [TCP echo server](examples/network/tcp-server.luau), [UDP](examples/network/udp.luau), [TLS client](examples/network/tls-client.luau), [TLS server](examples/network/tls-server.luau), and [local socket](examples/network/local.luau). Change endpoints and certificate paths for your tool. Stop the server script through the existing scripting controls.

示例覆盖客户端、监听、UDP、TLS 和本地通信；按工具需求修改端点及证书路径，使用现有脚本控制停止监听脚本。

The host policy is in `codegen/bindings.json`. Outputs remain under `build/Debug/generated/WingLuauBind`: `module_api_report.json`, `module_dependencies.json`, `exclusion_report.json`, `definition_report.json`, and the generated definitions/symbol catalog describe the actual SDK surface. The independent network test links the real generated archive and uses loopback endpoints and test-only PEM fixtures. A missing TLS backend is explicitly skipped; TCP/UDP/local socket tests still run.

宿主策略在 codegen 配置中，生成的 API、排除及定义依赖报告跟随 build 目录。独立网络测试链接真实生成库，使用回环端点及仅供测试的 PEM 文件；缺少 TLS 后端时明确跳过 TLS 执行，其他协议仍继续验证。

## Validation / 验证

On Linux with Qt 6.12, the full IDE Debug build passed. The registration/network tests passed 2/2 (both module orders, paused coroutine callbacks, TCP/UDP/local sockets, real TLS success and certificate rejection); LSP passed 5/5 and generator tests passed 16/16. Ruff, strict Pyright and diff checks passed. The real host probe exercised Scripting/Background scheduling, abort and normal-exit cleanup, and lazy browser expansion. Windows/macOS runtime execution remains unverified.

Linux、Qt 6.12 环境下，IDE Debug 完整构建通过；注册和网络测试 2/2、LSP 5/5、generator 16/16 通过。TLS 成功握手与证书拒绝均实际执行；Ruff、严格 Pyright 和差异检查通过。真实宿主探针验证两种调度模式、中止、正常退出清理及对象树按需展开。尚未在 Windows/macOS 实际运行。

```sh
ctest --test-dir build/Debug/registration-smoke --output-on-failure
ctest --test-dir build/Debug/lsp-restructure --output-on-failure
python tests/luaubindings/build_host_probe.py --build build/Debug
QT_QPA_PLATFORM=offscreen build/Debug/WingLuauHostInputProbe --test-plugin --network
```
