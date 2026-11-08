# 启动性能与计时 / Startup performance and profiling

2026-11-07：Linux x86_64、Qt 6.12.0、GCC 16.2.1、Release（O3 + LTO）、offscreen，
使用同一份配置副本、相同资源及依赖，预热一次后取 5 次中位数。
从启动子进程到首帧完成采样点，优化前 **650.1 ms**，优化后 **401.6 ms**，
减少约 **38%**。本轮未修改 `3rdparty/` Git 子模块，原有未提交修改保持不变。
2026-11-07: Linux x86_64, Qt 6.12.0, GCC 16.2.1, Release (O3 + LTO), offscreen,
using identical settings copies, resources and dependencies. Medians of five runs
after one warmup: **650.1 ms → 401.6 ms**, approximately **38% less time**
from child launch to the first-paint checkpoint. Third-party Git submodules were
not edited; their pre-existing uncommitted work was preserved.

## 改动及耗时 / Changes and timings

| 阶段 / Phase | 优化前 / Before ms | 优化后 / After ms |
| --- | ---: | ---: |
| `mainwindow.ribbon` | 33.7 | 9.3 |
| `mainwindow.docks` | 222.1 | 24.9 |
| `mainwindow.status_and_content` | 26.0 | 0.4 |
| `mainwindow.scripting` | 40.6 | 38.1 |
| `mainwindow.settings_pages` | 97.5 | 22.1 |
| `mainwindow.restore_layout` | 152.0 | 2.4 |
| `application.stylesheet` | —（原先分散在各阶段 / previously distributed） | 243.2 |
| 进程启动至首帧采样 / Launch to first-paint checkpoint | 650.1 | 401.6 |

- 初始化时先确定调色板与图标主题，面板树完成构建及恢复后统一应用全局 QSS，减少
  重设父对象时的重复样式计算。外部 UI 插件初始化前提前完成应用，保留插件读取及
  扩展应用样式表的时机。全局 QSS 的最终应用成本已计入总耗时。
  Resolve palette/icon appearance first, then apply global QSS after the initial
  dock tree has been built and restored, reducing repeated styling during
  reparenting. Flush before external UI plugin initialization so plugins can
  inspect and extend the application stylesheet. Final QSS application is
  included in the overall measurement.
- 启动屏通过 `ThemeManager::currentStyleSheet()` 提前取得选中主题的样式，并仅对其
  自身子树应用。边框、文字和进度条在主窗口构建前就采用最终样式，不提前启用全局
  QSS，也会跟随主题切换。
  The splash obtains the selected sheet through `ThemeManager::currentStyleSheet()`
  and styles only its own subtree. Its border, text and progress bar use their
  final styling before main-window construction, without enabling global QSS
  early, and follow theme changes.
- SVG 栅格图使用 Qt 自带的有界 `QPixmapCache`，区分尺寸、DPI、模式、状态、主题代次、
  调色板和样式。已有 QIcon 随主题切换、淘汰后重绘和禁用状态仍受测试覆盖。
  Cache SVG rasters in Qt's bounded `QPixmapCache`, keyed by size, DPI, mode,
  state, theme revision, palette and style. Tests cover existing icons across
  theme changes, eviction/repainting and disabled appearances.
- 启动期间不再为每个隐藏控件处理整个事件队列；启动屏仍在阶段之间处理进度。
  两个脚本设置编辑器预览及 LSP 设置页按需创建，设置入口和保存方式保持不变；首次
  打开脚本设置或脚本窗口会承担这部分构造成本。LSP 定义文档不再重复写入相同文本。
  Avoid pumping the entire queue for every hidden control during construction;
  the splash still pumps phase progress. Build the two script-settings editor
  previews and LSP settings on first use, preserving their entry points and
  persistence. Their construction cost moves to the first settings/script-window
  opening. LSP definition documents are no longer populated twice.

独立图标负载（127 个真实图标、三种尺寸、重复绘制）的中位数从约 304 ms 降至约 2 ms；
这只是局部负载，不等同于主程序的加速倍数。
An independent workload of 127 real icons at three repeated sizes improved from
roughly 304 ms to 2 ms. This is a local workload, not the host's speedup factor.

## 后续优化 / Follow-up optimization

- 标准图标路径保留在主题 QSS 中。加载时只提取通用 QWidget 规则中能够完整识别的
  标准图标默认值，交由 `ThemeProxyStyle` 按请求缓存，避免全树重复准备图标。
  控件/状态专用覆盖、未知属性、复杂值和缺失资源声明原样交给 Qt；C++ 仅维护 Qt
  属性名到标准角色的对应关系，不再维护资源文件名清单。
  Standard-icon paths remain in the theme QSS. Loading extracts only fully
  recognized defaults from unconditional QWidget rules for on-demand caching
  by `ThemeProxyStyle`, avoiding repeated preparation across the widget tree.
  Scoped/state overrides, unknown properties, complex values and missing-resource
  declarations stay with Qt. C++ maps Qt property names to roles, without a
  separate resource-filename catalog.
- 全量标准枚举由主题覆盖：QSS 保留 54 项默认值，其余 29 个角色使用 QRC 枚举别名，
  引用既有 SVG。`standardPixmap()` 仅渲染主题图标并保留原生兜底，不再维护递归状态。
  All standard roles are theme-backed: 54 QSS defaults plus 29 enum-named QRC
  aliases to existing SVGs. `standardPixmap()` renders theme icons with native
  fallback, without recursion state.
- 插件注册完成后固定设置页顺序和 Ribbon 入口，首次打开设置时才创建宿主设置窗口。
  插件返回的页面对象仍在原注册时机取得，交由主窗口持有，再转交设置栈；重复 ID
  检查、重启通知和窗口几何保存保留。插件页面构造本身不作延迟，首次打开宿主设置
  会承担延后的构建成本。
  Freeze settings order and Ribbon actions after plugin registration, then create
  the host settings window on first use. Plugin page objects are still obtained
  at their original registration point, retained by the main window and later
  adopted by the settings stack. Duplicate-ID checks, restart notifications and
  geometry persistence remain. Plugin page constructors are not deferred; first
  opening host settings pays their deferred host construction cost.
- ADS 插入及汇总最小尺寸时，每个控件只查询一次 `minimumSizeHint()`，同时用于宽高。
  初始区域批处理实验在独立基准中未显示稳定收益，已撤除：10/50 个控件略慢，200 个
  控件仅快约 1 ms；不新增批处理接口，也不改变标签激活和信号行为。
  ADS queries each widget's `minimumSizeHint()` once for both dimensions during
  insertion and aggregation. An initial-area batching experiment was removed:
  isolated measurements were slightly slower for 10/50 widgets and only about
  1 ms faster for 200. No batching API or tab-activation/signal changes remain.
- Ribbon 隐藏页跳过滚动指示器刷新，在显示、尺寸变化及滚动范围变化时更新。
  Hidden Ribbon pages skip indicator refresh; showing, resizing and scroll-range
  changes refresh the indicators.

本轮使用相同配置副本、预热 1 次后取 5 次中位数，完整 Release 构建结束后顺序测量。
以下为上述改动的组合收益，不能把各阶段变化单独归因于某一项修改。
This round uses the same settings copy and five runs after one warmup, measured
sequentially after the full Release build. These are combined results; individual
phase changes cannot be attributed to a single modification in isolation.

| 指标 / Metric | 本轮前 / Before ms | 本轮后 / After ms |
| --- | ---: | ---: |
| Dock 构建 / Dock construction | 24.58 | 22.30 |
| 设置页启动工作 / Settings startup work | 22.40 | 0.03 |
| 全局 QSS 应用 / Global QSS application | 245.79 | 134.11 |
| 启动到首次反馈 / Launch to feedback | 133.99 | 112.53 |
| 启动到主窗口首帧 / Launch to main readiness | 392.57 | 255.04 |

主窗口首帧总体减少约 **35.0%**。仍为 Linux/Qt 6.12/offscreen 结果，原生平台、
真实外部插件动态库及首次打开设置的交互延时另需人工验证。
Overall main readiness improves by approximately **35.0%**. These remain
Linux/Qt 6.12/offscreen results; native platforms, real external plugin libraries,
and the interaction delay when first opening settings require manual validation.

最终版本还验证了全新配置和启动打开脚本，各预热 1 次后测量 2 次：
The final build also checks fresh settings and opening a script at launch, with
two runs each after one warmup:

| 场景 / Scenario | 首次反馈 / Feedback ms | 主窗口首帧 / Main readiness ms |
| --- | ---: | ---: |
| 全新配置 / Fresh settings | 98.7 | 240.2 |
| 启动打开脚本 / Open script at launch | 107.6 | 486.4 |

QSS 单一来源修正后，再次完成完整 Release 构建，以同一配置各预热 1 次、测量 5 次：
主窗口首帧 **250.0 ms**、首次反馈 **106.8 ms**；启动打开脚本分别为
**501.6 ms / 110.1 ms**。完整原始 QSS 与优化后样式对照覆盖当前 Qt 全部
标准图标角色、控件专用规则、局部覆盖和 2 倍缩放；同时修复非正方形图标重复缩放。
After making QSS the sole path source, the complete Release build was measured
again with the same settings, five runs each after one warmup: main readiness
**250.0 ms**, feedback **106.8 ms**; opening a script at launch **501.6 ms / 110.1 ms**.
Original-versus-optimized QSS comparisons cover every standard icon role in the
current Qt, scoped rules, local overrides and 2x scaling. Non-square icon double
scaling was also corrected.

## 智能启动反馈 / Automatic startup feedback

默认以 **200 ms 内看到反馈**为目标。自动启动屏的兜底延时为 150 ms，为绘制留出
余量；在此之前完成启动则不显示。按应用版本、主题、脚本/插件开关及是否带参数，
分别记录最近 5 次成功启动耗时，用中位数预测下次启动；无记录时按 250 ms 估计。
进入全局 QSS 应用或外部插件初始化前，若预计会超过延时，提前显示并绘制启动屏，
避免 GUI 线程阻塞期间定时器无法触发。启动完成立即关闭，不设最短展示时间，
保留用户手动禁用启动屏的设置。此功能独立于计时宏，普通构建同样生效。
The default goal is visible feedback within **200 ms**. A 150 ms fallback delay
leaves room for painting; startups finishing earlier need no splash. The median
of the last five successful starts predicts duration, grouped by application
version, theme, script/plugin switches and presence of arguments. Unknown starts
use a conservative 250 ms estimate. Before global QSS or external plugin
initialization, predicted slow starts paint the splash ahead of GUI-thread
blocking. Completion closes it immediately, with no minimum exposure time. The
manual disable setting remains effective. This feature works independently of
the profiling macro in normal builds.

智能反馈初版的 Release/offscreen 实测（同一环境，预热后）：
Initial automatic-feedback Release/offscreen measurements, after warmup:

| 场景 / Scenario | 次数 / Runs | 首次反馈中位数 / Feedback median ms | 主窗口首帧中位数 / Main ready median ms |
| --- | ---: | ---: | ---: |
| 已有布局 / Saved layout | 5 | 132.7 | 460.1 |
| 全新配置 / Fresh settings | 2 | 126.6 | 416.4 |
| 启动打开脚本 / Open script at launch | 1 | 133.3 | 666.8 |

首次反馈计入进程创建和动态加载，与主窗口就绪分别统计。这轮增加的是启动期间的
反馈，不把启动屏出现当作启动完成。定时器和预测不能抢占任意同步阻塞；外部插件、
系统负载及窗口合成器仍需原生桌面检查，200 ms 是体验目标而非实时保证。
Feedback timing includes process creation and dynamic loading and is separate
from main-window readiness. Showing the splash does not mean startup has
finished. Timers and predictions cannot preempt arbitrary synchronous blocking;
external plugins, system load and the compositor require native-desktop checks.
The 200 ms target is an experience goal, not a real-time guarantee.

## 编译开关 / Build switch

`WINGHEX_ENABLE_STARTUP_PROFILE` 默认 **OFF**，仅为宿主目标定义同名宏。关闭时接口
为空实现，不编译计时实现，不安装首帧观察器，也不响应自动退出环境变量。
`WINGHEX_ENABLE_STARTUP_PROFILE` defaults to **OFF** and defines its macro only for
the host target. Disabled builds use empty interfaces, omit the profiling
implementation, install no first-paint observer and ignore the auto-exit variable.

```sh
cmake -S . -B build/profile -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DWINGHEX_ENABLE_STARTUP_PROFILE=ON
cmake --build build/profile --target WingHexExplorer2
```

开启后默认向 stderr 输出 `[startup] ` 开头的逐行 JSON，不经过 GUI 日志面板。
`WINGHEX_STARTUP_TRACE=0` 可在该构建中临时静默。正常运行不会自动退出。
Enabled builds emit line-delimited JSON to stderr with a `[startup] ` prefix,
bypassing the GUI log dock. `WINGHEX_STARTUP_TRACE=0` temporarily silences an
instrumented build. Normal runs do not auto-exit.

- `at_ms`：从 C++ `main()` 开始的时间，不含动态加载器和进入 main 前的静态初始化。
  Time since entering C++ `main()`, excluding the loader and pre-main initialization.
- `duration_ms`：所属计时器上一次检查点以来的耗时。子阶段包含在父阶段内，不能重复相加。
  Time since that phase timer's previous checkpoint. Nested phases are already
  included in parent phases and must not be added twice.
- `process.first_paint_ready`：主窗口首次绘制后返回事件循环的采样点，表示首次显示并交还
  GUI 事件循环；不声称所有后台任务已完成。
  A checkpoint after the first main-window paint returns to the GUI event loop;
  it does not claim that all background work is complete.
- `process.splash_first_paint`：启动屏首次绘制的采样点；不会触发自动退出。
  The first splash paint checkpoint; it does not trigger auto-exit.

## 隔离测量 / Isolated measurement

`tools/benchmark_startup.py` 当前仅支持 Linux，使用临时 XDG 配置、缓存和数据目录，
可复制已有 Qt 配置，但不会修改原件。同一轮的进程之间保留测试设置，以覆盖保存布局
后的第二次启动。脚本通过 `WINGHEX_STARTUP_EXIT_AFTER_READY=1` 在采样后正常关闭窗口。
宏关闭或没有收到首帧标记时会报错，不把提前退出误报为快速启动。
`tools/benchmark_startup.py` currently supports Linux only. It uses temporary XDG
configuration, cache and data directories, optionally copying existing Qt settings
without editing the original. Settings persist between runs within one invocation,
covering second launches with saved layouts. The runner sets
`WINGHEX_STARTUP_EXIT_AFTER_READY=1` to close normally after recording. Disabled
profiling or missing ready markers are errors, rather than false fast-start results.

同时隔离 `TMPDIR` 和 `XDG_RUNTIME_DIR` 中的单实例锁及本地套接字，避免把参数转发给
正在运行的用户实例。原生 Wayland 测量保留原合成器套接字的绝对路径。
Private `TMPDIR` and `XDG_RUNTIME_DIR` isolate single-instance locks and local
sockets, preventing argument forwarding to an existing user instance. Native
Wayland measurement preserves the compositor socket through its absolute path.

```sh
python tools/benchmark_startup.py build/profile/WingHexExplorer2 \
  --warmup 1 --runs 5 --output build/startup.json
# Add --config /path/to/settings.conf to test a copy of an existing layout.
# Add --file /path/to/example.luau to cover script-window startup as well.
```

输出包含原始阶段数据、各阶段中位数及父进程实际观察到的 `launch_to_ready_ms`；后者
包含进程创建、动态加载及进入 main 前的时间。冻结同一份配置副本，完成所有构建后，
顺序测量优化前后，避免并行编译造成争用。不要把 Qt 的 offscreen 结果作为原生桌面的
绝对承诺；外部插件、字体和平台窗口系统会影响结果，插件路径也可能提前应用 QSS。
Output includes raw phases, phase medians and parent-observed `launch_to_ready_ms`,
which includes process creation, dynamic loading and pre-main work. Freeze the same
settings copy, finish compilation, then measure old/new sequentially to avoid build
contention. Offscreen measurements are not absolute native-desktop guarantees;
external plugins, fonts and window systems affect results, and plugin loading may
flush QSS earlier.

`launch_to_feedback_ms` 记录启动屏或主窗口中先出现的一帧；没有启动屏时等于主窗口
首帧时间。输出同时提供两项指标各自的中位数。
`launch_to_feedback_ms` records the first observed splash or main-window frame;
without a splash it equals main readiness. Both metrics include their medians.

验证包括完整 Release 宿主构建、宏开/关及运行时开关测试、主题缓存和延迟样式测试、
Luau LSP 回归、全新配置的连续启动、启动时打开脚本文件。原生桌面人工检查和 Qt 6.6.2
执行验证尚未完成。该测试配置没有额外安装外部插件。
Validation covers the full Release host build, compile/runtime profiling gates,
theme cache and deferred styling tests, Luau LSP regressions, repeated launches
with fresh settings and opening a script during startup. Native-desktop manual
checks and Qt 6.6.2 execution remain unverified. The fixture has no additionally
installed external plugins.
