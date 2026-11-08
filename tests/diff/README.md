# 二进制比较回归测试 / Binary diff regression tests

独立测试直接使用生产引擎、分段结果、模型、QHexView 和读取会话，不需要构建完整宿主。
Standalone tests exercise the production engine, bounded result store, model, QHexView and read sessions without the full host.

```sh
cmake -S tests/diff -B /tmp/wing-diff-tests -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/wing-diff-tests
ctest --test-dir /tmp/wing-diff-tests --output-on-failure
```

使用 `-DWINGHEX_FORCE_CPP17=ON` 验证 C++17 回退。要求 Qt Widgets、Concurrent、Test。
Use `-DWINGHEX_FORCE_CPP17=ON` for the C++17 fallback. Requires Qt Widgets, Concurrent and Test.

覆盖增删改、随机修改、跨缓存重同步、取消、连续百万行列表、64 GiB 偏移、详细／内存上限、共享文档只读隔离、未保存修改读取和设备线程取消。
Coverage includes edits, random edits, cross-cache resynchronization, cancellation, a continuous million-row list, 64 GiB offsets, detail/memory limits, shared-document read-only isolation, unsaved reads and owner-thread cancellation.

## 宿主探针 / Host probe

先以 Ninja 和 `CMAKE_EXPORT_COMPILE_COMMANDS=ON` 配置并构建主项目，然后运行：
Configure and build the application with Ninja and `CMAKE_EXPORT_COMPILE_COMMANDS=ON`, then run:

```sh
python3 tests/diff/run_host_probe.py --build-dir build
```

探针链接真实宿主对象，使用隔离的临时设置，验证 `instances()`、clone 去重、共享文档、只读、连续滚动、应用设置页与按对象定位、布局切换、表格自适应宽度、底部搜索栏、源修改后的自动重比、非内容操作保留结果、进度条显隐和运行中关闭。生成 `/tmp/wing-diff-ui-light.png` 和 `/tmp/wing-diff-ui-dark.png`。
The probe links real host objects, isolates its settings, and checks `instances()`, clone deduplication, document sharing, read-only behavior, continuous scrolling, application settings and page-object selection, orientation switching, adaptive column widths, bottom search placement, automatic recomparison after byte changes, result preservation for presentation-only operations, progress visibility and closing during a run. It writes `/tmp/wing-diff-ui-light.png` and `/tmp/wing-diff-ui-dark.png`.

离屏探针不替代原生无边框窗口和实际桌面交互检查。
The offscreen probe does not replace native frameless-window and desktop interaction checks.

## 性能基准 / Benchmarks

```sh
python3 tests/diff/run_benchmarks.py \
  --executable /tmp/wing-diff-tests/diff_benchmark \
  --directory build/diff-benchmark --output build/diff-benchmarks.json
```

默认覆盖 32／64 GiB 相同、稀疏修改、插入、删除、密集修改和全不同文件。实际写入文件；每次最多占用约 128 GiB 临时磁盘，退出清理。`--sizes 1024` 缩小测试，`--resume` 跳过已有结果。插入场景为新数据预留空间，保证两侧均不超过上限。
Defaults cover 32/64 GiB identical, sparse, insert, delete, dense and unrelated inputs. Files are actually written, with about 128 GiB peak temporary disk usage, and removed on exit. Use `--sizes 1024` for smaller runs and `--resume` to skip existing results. Insert workloads reserve space so both inputs stay within the limit.

内容由重复的 1 MiB 伪随机块构成，不能代表所有真实文件；不控制文件系统缓存，也不清空系统缓存。计时不包含输入生成。报告包括耗时、读取字节、seek 次数、结果内存、区间数及进程最大 RSS；RSS 包含 Qt 和输入生成，不是引擎额外内存。
Content uses repeated 1 MiB pseudorandom blocks and is not representative of every file. Filesystem cache state is uncontrolled; system caches are not cleared. Timings exclude generation. Reports include time, bytes read, seeks, result allocation, intervals and peak process RSS; RSS includes Qt and generation, not just incremental engine memory.

## 实现约束 / Implementation constraints

默认 64 MiB 工作预算、100,000 个详细差异、单侧默认 32 GiB 上限（设置可提高至 64 GiB）。达到详细或内存限制后继续输出有上限的粗略区间。没有完整内容快照或结果临时文件。粗略替换可以包含相等字节；结果不保证全局最短编辑脚本。
Defaults are a 64 MiB working budget, 100,000 detailed differences and a default 32 GiB per-side limit (configurable to 64 GiB). Detail/memory exhaustion switches to bounded coarse results. No full-content snapshots or temporary result files are created. Coarse replacements may contain equal bytes; the result is not guaranteed globally minimal.

普通文件后台使用独立句柄和隐式共享修改块；不能独立读取的设备通过 GUI 所属线程的有界请求读取。映射或保留的共享修改内容超过预算时也使用该适配路径，避免无界映射副本及编辑时的大额写时复制。慢设备的单次同步读取仍可能阻塞其所属线程。已检测到的来源变化使结果失效；不承诺任意外部写入具有原子一致性。
Regular files use independent worker handles plus implicitly shared edited pieces. Other devices use bounded requests to their owning GUI thread. Mappings or retained edited payloads that exceed budget also use that path, avoiding unbounded mapping copies and large copy-on-write allocations during editing. A single slow synchronous device read may still block its owner. Detected source changes invalidate results; arbitrary external writes are not atomically versioned.

全局搜索在快速／精细模式下分别扫描最多 64／256 MiB 新数据；超过后转为粗略汇总。连续汇总窗口未命中锚点时降低搜索频率。预算耗尽在状态栏明确提示。
Global search scans at most 64/256 MiB of new data in fast/precise modes before switching to coarse summaries. Repeated summary-window misses reduce search frequency. The status bar identifies search-budget exhaustion.

`diff_benchmark ... --reuse` 可直接使用指定目录已有的 `left.bin` 和 `right.bin`（删除场景交换两者），避免重复生成大文件。已有文件的实际长度会写入报告。测试者负责这些输入的内容与清理。
`diff_benchmark ... --reuse` uses existing `left.bin` and `right.bin` in the supplied directory (swapped for delete), avoiding repeated large-file generation. Actual input lengths are included in the report. The tester owns input preparation and cleanup.

本次结果见 [BENCHMARKS.md](BENCHMARKS.md)。
See [BENCHMARKS.md](BENCHMARKS.md) for this run's results.

渲染扩展统一使用 `QHexView::setRenderer()` 和受保护的 `applyLineFormats()`；选择覆盖与对比度修正由基类完成。读取接口返回对象与 `QHexReadError`，引擎返回 `WingDiff::Error`。诊断分类为 `wing.qhexview.read` 和 `wing.diff`；预算降级使用调试日志，正常取消不输出错误。
Rendering extensions use `QHexView::setRenderer()` and protected `applyLineFormats()`. The base class finalizes selection and contrast. Read interfaces return an object and `QHexReadError`; the engine returns `WingDiff::Error`. Diagnostic categories are `wing.qhexview.read` and `wing.diff`; budget fallback uses debug logging and normal cancellation emits no error.

首次开始比较后，默认在字节内容变动停止 500 ms 后自动重比；可通过“自动比较”关闭。取消当前任务同时暂停自动比较，已提交的部分结果仍保留。基址、书签、标注显示和未改字节的保存保留有效结果；关闭来源或检测到外部文件变化会明确提示失效，外部变化须重新加载或手动比较。比较状态在 Ribbon 下方持续显示，进度条仅在任务运行时显示。
After the first comparison, byte changes trigger automatic recomparison once edits have settled for 500 ms; Auto compare can disable this. Cancellation also pauses automatic comparison and retains committed partial results. Base-address changes, bookmarks, annotation visibility and saves without byte changes preserve valid results. Closing a source or detecting external file changes explicitly invalidates results; external changes require reload or manual comparison. The comparison state remains visible below the Ribbon, while the progress bar appears only during a running task.
