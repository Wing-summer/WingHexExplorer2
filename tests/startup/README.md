# 启动计时测试 / Startup profiling tests

独立编译计时宏开启、关闭的两个 Qt Widgets 探针，需要 CMake 3.20+、Qt 6.6.2+、
C++17 和 Python 3.10+。无需完整主程序或依赖生成器。
Build two independent Qt Widgets probes with profiling compiled in/out. Requires
CMake 3.20+, Qt 6.6.2+, C++17 and Python 3.10+, without the host or binding generator.

```sh
cmake -S tests/startup -B build/startup-tests -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/startup-tests
ctest --test-dir build/startup-tests --output-on-failure
```

覆盖编译开关、默认启用行为、运行时静默、首帧后的受控退出，以及宏关闭时环境变量
不能启用计时或退出。Linux 上还验证测量脚本记录进程启动时间，并拒绝没有首帧标记的
未启用构建。计时开关默认关闭，主程序测量方法见[启动性能文档](../../docs/startup-performance.md)。
Covers compile-time gating, enabled-build defaults, runtime silencing, controlled
exit after first paint, and immunity to profiling/exit variables in disabled builds.
On Linux it also checks launch timing and rejection of uninstrumented binaries
without a ready marker. Profiling defaults off; see the
[startup performance guide](../../docs/startup-performance.md) for host measurements.

测量器还验证无启动屏时首次反馈时间回退为主窗口首帧时间。宿主测量使用独立运行
目录隔离单实例锁和套接字；主题测试另行覆盖普通构建中的自动启动屏行为。
The runner also verifies that feedback timing falls back to main readiness when
there is no splash. Host measurements isolate single-instance locks/sockets in
a private runtime directory; theme tests cover automatic splash behavior in
normal builds separately.

## 宿主设置入口 / Host settings entry points

已有完整 Ninja 宿主构建时，可复用生产对象文件
链接独立 UI 探针，不启动单实例 AppManager，也不访问用户配置：
With a complete Ninja host build, link
an independent UI probe against production objects, using isolated settings and
without the single-instance AppManager:

```sh
python3 tests/startup/run_ui_probe.py --build build/profile
```

验证启动时未创建设置对话框、首次打开选择正确页面、重复入口复用窗口；使用真实
模态事件循环。使用测试插件对象在注册窗口内调用生产注册函数，检查插件页面及 Ribbon
索引；不加载外部插件动态库。
Checks deferred creation, first-use page selection and reuse through the real modal
event loop. A fixture plugin object invokes the production registration function
during the registration window, checking plugin pages and Ribbon indices without
loading an external plugin library.
