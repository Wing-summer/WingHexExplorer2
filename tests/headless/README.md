# 无头与服务回归 / Headless and service regression

使用隔离设置及真实宿主对象，不读取用户设置。脚本服务测试需要本机 socket、Python 3 与 OpenSSL CLI，不要对普通用户进程执行测试。
Tests use isolated settings and real host objects, without reading user settings. Service tests require loopback sockets, Python 3 and the OpenSSL CLI. Do not run them against a regular user process.

```sh
cmake --build build
python tests/headless/build_process_probe.py --build build
python tests/headless/test_process.py --binary build/WingHexHeadlessProbe
python tests/headless/test_modes.py --binary build/WingHexHeadlessProbe --plugins
python tests/headless/test_service.py --binary build/WingHexHeadlessProbe --plugins
# 长时间执行与显式停止 / Long-running execution and explicit stopping
python tests/headless/test_service.py --binary build/WingHexHeadlessProbe --plugins --slow
```

覆盖一次性执行、禁止 stdin、移除 host 模块、信号、模式互斥与独立窗口；服务测试覆盖长度帧、批量请求、通知、Unicode、任务及模块隔离、CPU 循环及异步回调的公平调度、停止与 ID 复用、活动任务容量、脚本 socket 与管理 socket 隔离、资源释放、TLS 证书注册及插件只加载一次。证书测试在临时应用目录生成证书，不修改真实证书库。
Covers one-shot execution, forbidden stdin, removed host bindings, signals, exclusive modes and standalone windows. Service tests cover framing, batches, notifications, Unicode, task/module isolation, fairness for CPU loops and asynchronous callbacks, stopping and ID reuse, active capacity, separate script and management sockets, resource release, TLS certificate registration and one-time plugin initialization. Certificate tests generate certificates in a temporary application directory without changing the real trust store.

`--slow` 执行超过一分钟，确认旧版超时设置不再终止运行或驻留的任务，并验证显式停止仍然有效。托盘可用性及 Windows 标准流、原生桌面表现需要分别报告；离屏回归不能替代原生验证。
`--slow` runs for over a minute, checking that an obsolete timeout setting no longer terminates running or resident tasks and that explicit stopping still works. Report system-tray availability, Windows standard streams and native desktop behavior separately; offscreen regressions do not replace native validation.

可将已构建的 probe 传给 CTest：
An existing probe can also be registered with CTest:

```sh
cmake -S tests/headless -B build/headless-tests -DWINGHEX_SERVICE_BINARY="$PWD/build/WingHexHeadlessProbe"
ctest --test-dir build/headless-tests --output-on-failure
```

启动模式回归还覆盖后台 CPU 循环及模态输入期间关闭主窗口：确认退出后先完成环境清理，再继续关闭。
Startup-mode regression also closes the main window during a background CPU loop and modal input, verifying cleanup before confirmed shutdown completes.
