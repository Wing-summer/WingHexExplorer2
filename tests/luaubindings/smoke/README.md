# Registration smoke test / 注册自测

This small target consumes an existing generated binding library and built Luau/SDK dependencies. It never downloads dependencies or builds the application. Fix binding behavior in the generator, regenerate the library, then rebuild this target to relink and rerun the test.

此小程序复用现有生成库及已构建的 Luau、SDK，不下载依赖，也不编译完整应用。绑定问题应在 generator 中修复；重新生成并构建绑定库后，本目标只需重新链接即可反复测试。

```sh
cmake --build build/Debug --target WingLuauBind --parallel 3
cmake -S tests/luaubindings/smoke -B build/Debug/registration-smoke -G Ninja \
  -DHOST_BUILD_DIR="$PWD/build/Debug"
cmake --build build/Debug/registration-smoke --parallel 2
ctest --test-dir build/Debug/registration-smoke --output-on-failure
```

The test creates three independent VMs. Public modules register eagerly; Qt registers once through the real builtin loader after sandboxing. Registry counters reject duplicate registration. The real static cffi library is compared against the generated export report. Tests also exercise Qt enums, snake_case, dialogs, readonly binary MIME, QImage clipboard data, Frameless UI loading, FFI memory and callbacks.

测试创建三个独立 VM：全局模块立即注册，沙箱启用后 Qt 通过真实 builtin loader 只加载一次，并检查注册计数。真实 cffi 静态库的导出与生成报告逐项比较；另验证枚举、snake_case、对话框、只读二进制 MIME、QImage、Frameless UI、FFI 内存和回调。

Static dialog tests open and automatically close real message boxes, checking omitted/explicit parents, default buttons and combined QFlags values.

静态对话框测试打开并自动关闭真实消息框，检查省略或显式传入父对象、默认按钮以及 QFlags 按钮组合。

The real-host probe reuses the application's existing object files and linker command. It checks whole-line Unicode input, cancellation, coroutines, script-only Frameless construction/UI loading, and lazy object-tree expansion with the actual host settings and window library.

真实宿主探针复用应用对象文件及链接命令，验证整行 Unicode 输入、取消、协程，以及实际宿主设置和窗口库下的脚本 Frameless 构造、UI 加载和对象树按需展开。

```sh
cmake --build build/Debug --parallel 4
python tests/luaubindings/build_host_probe.py --build build/Debug
QT_QPA_PLATFORM=offscreen build/Debug/WingLuauHostInputProbe
```

To exercise the real TestPlugin, enable its build and pass `--test-plugin`. This checks resource-definition deduplication, completion, type checking, definition navigation, overloaded functions, object properties and script exceptions. Registration after initialization must fail.

验证真实 TestPlugin 时，启用插件构建并传入 `--test-plugin`，检查资源定义去重、补全、类型检查、定义定位、函数重载、对象属性及脚本异常，并确认初始化完成后无法追加定义。

```sh
cmake -S . -B build/Debug -DWINGHEX_BUILD_TEST_PLUGIN=ON
cmake --build build/Debug --parallel 4
python tests/luaubindings/build_host_probe.py --build build/Debug
QT_QPA_PLATFORM=offscreen build/Debug/WingLuauHostInputProbe --test-plugin
```

See [validation results](../../../docs/luau-validation.md) for commands, measurements and coverage boundaries.

命令、测量结果及验证边界见[验证记录](../../../docs/luau-validation.md)。

For a short generator iteration, select one SDK class. The target scans that class using the production generator and compiles its registration before linking the existing library. Run the ordinary target above after rebuilding the production library to validate the complete generated output.

需要快速迭代 generator 时，可选择单个 SDK 类。目标通过正式 generator 扫描该类，编译其注册代码，再链接现有绑定库。正式库重新构建后，应运行上面的常规目标，验证完整生成产物。

```sh
cmake -S tests/luaubindings/smoke -B build/Debug/dialog-smoke -G Ninja \
  -DHOST_BUILD_DIR="$PWD/build/Debug" -DSDK_PROBE_CLASS=QMessageBox
cmake --build build/Debug/dialog-smoke --parallel 2
ctest --test-dir build/Debug/dialog-smoke --output-on-failure --timeout 30
```

Pass `--network` to the host probe to run loopback UDP through the real Scripting/Background scheduler. The independent `network` CTest additionally covers TCP, UDP, local sockets, TLS, suspended coroutine callback delivery and context cleanup; these tests require permission to create local sockets.

宿主探针传入 `--network` 可通过真实 Scripting/Background 调度器验证回环 UDP。独立 network CTest 另覆盖 TCP、UDP、本地 socket、TLS、暂停协程回调及上下文清理；测试环境需要允许创建本地 socket。
