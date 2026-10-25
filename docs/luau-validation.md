# Luau integration validation / Luau 集成验证

Validated on 2026-10-25 with the existing Linux Debug build, Qt 6.12 and offscreen widgets. No dependencies were downloaded for these probes. Generated outputs remain under `build/Debug/generated/WingLuauBind`; only configuration remains in `codegen/`.

2026-10-25 使用现有 Linux Debug 构建、Qt 6.12 和 offscreen 控件验证。探针未下载依赖；生成产物保留在 build 目录，`codegen/` 仅含配置。

## Commands and results / 命令及结果

```sh
cmake --build build/Debug --parallel 4
cmake -S tests/luaubindings/smoke -B build/Debug/registration-smoke
cmake --build build/Debug/registration-smoke --parallel 3
ctest --test-dir build/Debug/registration-smoke --output-on-failure --timeout 45
python tests/luaubindings/build_host_probe.py --build build/Debug
QT_QPA_PLATFORM=offscreen build/Debug/WingLuauHostInputProbe
cmake --build build/Debug/lsp-restructure --parallel 3
ctest --test-dir build/Debug/lsp-restructure --output-on-failure --timeout 60
PYTHONPATH=3rdparty/WingLuauQt/python \
  build/Debug/.wingluauqt-venv/bin/python tests/luaubindings/test_definition_graph.py
```

- The complete IDE application build passes. The independent runtime test passes across three VMs, and all five LSP groups pass.
- The real-host probe passes whole-line Unicode/whitespace/empty input, input cancellation and window closure, coroutine input, helper errors, both native-titlebar settings, direct construction and UI loading, content/menu/status/toolbars, geometry and absolute-position children. Ordinary Frameless classes have no changes.
- Runtime probes check idempotent eager registration and require, cached loader failures, shared type identity after forced GC, snake_case, excluded surfaces, readonly environment/MIME, image/text clipboard, dialog overloads and cancellation, cffi isolation/memory/callbacks and a real test-library call.
- Source-driven cffi contracts cover one factory function, 21 instance functions, two callback functions and four fields. Missing and unused contract sets are empty; the linked static library's exports match the report.
- Generator project regression, SCC/symbol tests, Ruff and strict Pyright pass. The project regression checks repeated generation and loads its generated definitions with the standalone definition checker. Root/framework/cffi whitespace checks pass.
- Definition deployment removes an obsolete generated file while preserving an unrelated definition in the same directory.

完整应用构建、三个 VM 的独立运行时测试、五组 LSP 测试和真实宿主探针均通过。宿主探针覆盖输入、协程及两种标题栏设置下的窗口装配；运行时探针覆盖重复注册、GC、模块替换及真实 cffi 静态库。生成器回归、静态检查和过期产物清理检查通过。

## Measurements / 测量

These are local Debug samples, not release benchmarks. The eager comparison uses the same current binding library with all modules registered immediately; it is not a historical binary comparison.

以下为本机 Debug 抽样，不代表 Release 性能。全量立即注册对照使用同一份当前绑定库，不是历史版本二进制比较。

| Operation / 操作 | Result / 结果 |
| --- | --- |
| All modules registered immediately / 全量立即注册对照 | 13.82 ms |
| Public-only VM startup, three runs / 仅公共模块启动，三次 | 7.39–7.61 ms |
| Deferred first Qt registration, three runs / 首次 Qt 延迟注册，三次 | 8.95–9.94 ms |
| Public definition initialization / 公共定义初始化 | 305 ms |
| Catalog roots plus Qt direct branch / 目录根及 Qt 直接分支 | 2 ms |
| Strict Qt and cffi script analysis / Qt、cffi 严格脚本分析 | 1230 ms |
| Actual object-tree creation / 实际对象树创建 | 19.76 ms |
| Actual Qt and QDialog tree expansion / Qt、QDialog 实际展开 | 2.65 ms |

Startup avoids Qt-only registration; its cost moves to first require. Browser expansion reads generated direct-member pages and leaves the VM's Qt registration untouched. There is no retained comparable historical LSP/tree timing, so no numerical speedup is claimed for those paths.

启动不执行 Qt 专属注册，成本移至首次 require；对象树只读取直接成员索引页，不触发 VM 注册。没有保留可比的历史 LSP/对象树测量，因此不声称这两项的数值提升。

Definitions contain 172 SCC type components and 182 definition files in total: 449,047 bytes. Public preload uses 83 files and 139,602 bytes; 309,445 bytes remain outside startup preload. The largest SCC is 164,906 bytes. The catalog root is 8,966 bytes; member pages load separately. Exact per-file sizes and dependency edges are generated in `manifest.json` and `definition_report.json`.

定义包含 172 个类型强连通分片、共 182 个定义文件；公共预加载约占总字节的 31%。递归依赖保留同片，精确体积和依赖边见生成报告。目录根约 9 KB，成员页单独加载。

Follow-up checks also pass `print(input())`/`tostring()` for Unicode, whitespace and empty input in all three modes, Simplified/Traditional Chinese browser categories, separate QMessageBox overload rows, and overload selection after refresh. The reusable native require session resolves the same destination after repeated queries and a prior failure. A local 100-query Debug sample measured 11,461 microseconds with fresh VMs versus 2,578 microseconds with a reused VM; this measures resolver queries only.

后续检查另覆盖三种模式下输入的打印和 tostring、简繁中文分类、QMessageBox 重载逐条显示及刷新后选择保留。复用原生 require 会话在重复查询和失败后仍正确定位。100 次查询本机 Debug 抽样为新建 VM 11,461 微秒、复用 VM 2,578 微秒；该数据仅测量解析查询。

## Coverage boundaries / 验证边界

Offscreen checks validate object behavior, ownership and signal-driven input; they do not replace visual/native-platform window testing. Windows/macOS window behavior was not exercised. The host input probe uses console providers in all execution modes; it does not interact with the application's background modal input provider. Debugger break and application-clock elapsed time still require an active application debugger/scheduler session for end-to-end validation.

offscreen 检查验证对象行为、所有权和信号输入，不能替代原生平台视觉检查；未运行 Windows/macOS。输入探针在各执行模式使用控制台提供者，未交互验证应用的后台模态输入提供者；断点和执行计时的端到端验证需要真实调试器及调度会话。

The [module inventory and main migration matrix](luau-modules.md) describe the interface boundary. Full API, exclusion, cffi coverage and definition dependency reports are generated with each build.

[模块清单及 main 迁移矩阵](luau-modules.md) 描述接口边界；完整 API、排除、cffi 覆盖及定义依赖报告随构建生成。
