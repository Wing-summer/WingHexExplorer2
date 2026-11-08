# ADS 布局恢复 / ADS layout restoration

独立测试使用仓库中的 ADS 生产代码，需要 CMake 3.20+、Qt 6.6.2+ 的 Widgets、Test
模块和 C++17 编译器；不依赖完整宿主或脚本绑定。支持使用 `CMAKE_CXX_STANDARD=20`
验证 C++20，测试目标及 ADS 库均采用所选标准。
The standalone suite builds the repository's production ADS code with CMake 3.20+,
Qt 6.6.2+ Widgets and Test, and C++17. It does not require the host or script
bindings. Set `CMAKE_CXX_STANDARD=20` to compile both the suite and ADS as C++20.

```sh
cmake -S tests/ads -B /tmp/wing-ads-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/wing-ads-release
ctest --test-dir /tmp/wing-ads-release --output-on-failure
QT_QPA_PLATFORM=offscreen /tmp/wing-ads-release/ads_benchmark
```

## 覆盖 / Coverage

- 二进制往返、重复恢复、当前标签、关闭状态、区域属性、中央控件和焦点。
  Binary round trips, repeated restoration, selected tabs, closed state, area
  properties, central widgets and focus.
- 恢复时将原活动页切换为后台页，检查其内容保持隐藏；覆盖首次显示、已显示窗口及连续恢复，确认之后仍可正常切换页签。
  Restore a different selected tab while keeping the former page hidden. Cover startup, visible windows, repeated restoration, and subsequent tab switching.
- 停靠与自动隐藏互转、侧栏位置和尺寸、浮动窗口几何与多余窗口的延迟销毁。
  Docked/auto-hide transitions, sidebar locations and sizes, floating geometry
  and deferred deletion of surplus windows.
- 不处理事件就连续恢复，检查浮动注册表一致性及业务控件存活。
  Consecutive restores without event processing, checking floating registrations
  and dock widget survival.
- 排除临时编辑器、缺失控件、分割尺寸对应关系、空布局及 perspectives。
  Excluded transient editors, missing widgets, matching splitter sizes, empty
  layouts and perspectives.
- 旧 XML、压缩 XML、版本、截断、长度、枚举、重复名称、树深度及尾随数据检查；
  无效输入不修改现有布局；恢复期间禁止重入并保留隐藏和更新状态。
  Old XML, compressed XML, versions, truncation, lengths, enums, duplicate names,
  depth and trailing data; invalid input leaves the layout unchanged, restoration
  rejects reentry and preserves hidden/update states.

新增启动绘制回归覆盖父窗口禁用更新时的恢复：保留管理器自身的
`Qt::WA_ForceUpdatesDisabled`，避免把继承的 `updatesEnabled() == false` 写成永久
禁用绘制。Release 子进程探针连续启动两次，首次通过 QSettings 保存布局，第二次恢复，
并检查中央区和侧栏实际收到绘制事件；同时覆盖恢复前窗口已显示的情况。
Startup painting regressions cover restoration while ancestor updates are disabled.
Preserve the manager's own `Qt::WA_ForceUpdatesDisabled` flag, rather than turning
inherited `updatesEnabled() == false` into permanently disabled painting. A probe
runs twice in separate processes: the first saves through QSettings, the second
restores, and both verify actual central/side content paint events. An already
shown ancestor is covered as well.

宿主的 `EditorView`、`ScriptEditor` 通过 `isStateSaveEnabled() const` 返回 `false`，
测试中的派生控件覆盖同一扩展点。完整宿主构建及原生窗口管理器下的人工交互检查仍需
另行执行。offscreen 不支持完整的原生浮动、窗口层级及激活行为。
The host's `EditorView` and `ScriptEditor` return `false` from
`isStateSaveEnabled() const`; a test subclass exercises the same hook. Full-host
builds and manual interaction checks under a native window manager remain
separate. Offscreen does not fully support native floating, stacking or activation.

## 格式与迁移 / Format and migration

新布局为未压缩二进制，开头是 `ADSB`，接 16 位格式版本、32 位用户版本，再接名称及
容器树。整数使用固定宽度、小端字节序和 Qt 6 的 `QDataStream::Qt_6_0` 配置；名称使用
显式 32 位字节长度的 UTF-8，几何直接保存 Qt 6 的版本 3 几何数据。控件没有被序列化，
按对象名查找。读取器在分配前校验剩余字节数，树深度上限为 256，并在修改 UI 前完成
所有输入校验。旧 XML 不迁移，恢复失败时保留当前布局。
The uncompressed binary layout starts with `ADSB`, followed by a 16-bit format
version, a 32-bit user version, names and container trees. Integers have fixed
width and little-endian order, using `QDataStream::Qt_6_0` on Qt 6. Names are
UTF-8 with explicit 32-bit byte lengths; geometry is the original Qt 6 version-3
geometry blob. Widgets are resolved by object name, never serialized. The reader
bounds allocations by remaining bytes, limits tree depth to 256, and validates
all input before changing the UI. Old XML is not migrated; restore failure keeps
the current layout.

新数据通常大于默认压缩的旧 XML；本次以恢复耗时为目标，默认不压缩。
The new bytes may be larger than the old compressed XML. Restoration latency is
the priority, so the format is uncompressed.

## 性能测量 / Performance measurement

基准覆盖 10、50、200 个控件，每 8 个标签增加一个交替方向的分割区域。混合场景还包含
浮动和自动隐藏窗口；10 控件场景尚未达到创建浮动/自动隐藏项的间隔，因此两种布局相同。
每组预热 3 次，然后报告 11 次的中位数及保存字节数。
The benchmark uses 10, 50 and 200 widgets, adding an alternating splitter area
every eight tabs. Mixed cases also contain floating and auto-hide docks; the
10-widget fixture has not reached those intervals, so its two layouts match.
Each case warms up three times and reports the median of 11 samples and byte size.

`total_median_ms` 包括恢复、显示事件、事件处理和延迟删除；
`restore_signal_span_ms` 测量 `restoringState` 至 `stateRestored` 的跨度，新实现中为
布局应用（包括显示），旧实现中还包括预检和 XML 解码，二者不可作为同一阶段直接比较。
`decode_median_ms` 独立测量新格式的纯数据解码，不从总耗时中相减估算。
`total_median_ms` includes restoration, show events, event processing and deferred
deletion. `restore_signal_span_ms` measures `restoringState` through
`stateRestored`: layout application (including showing) in the new implementation,
but also validation/XML parsing in the old implementation. Those spans are not
the same phase. `decode_median_ms` independently measures new-format decoding,
without estimating it by subtraction from the total.

可用同一基准源码构建旧版 ADS。将旧版源码导出到临时目录，再配置
`ADS_SOURCE_DIR` 和 `ADS_LEGACY_STATE=ON`；旧版模式仅编译基准，不编译新格式测试。
运行前完成全部编译，按旧版、新版顺序成对运行，避免并行编译造成争用。
The same benchmark source can build against an exported older ADS revision using
`ADS_SOURCE_DIR` and `ADS_LEGACY_STATE=ON`; legacy mode builds only the benchmark.
Finish all compilation first, then run old/new pairs to avoid build contention.

```sh
cmake -S tests/ads -B /tmp/wing-ads-old -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DADS_SOURCE_DIR=/tmp/ads-old \
  -DADS_LEGACY_STATE=ON
cmake --build /tmp/wing-ads-old
QT_QPA_PLATFORM=offscreen /tmp/wing-ads-old/ads_benchmark
QT_QPA_PLATFORM=offscreen /tmp/wing-ads-release/ads_benchmark
```

性能结果见 [BENCHMARK.md](BENCHMARK.md)。
See [BENCHMARK.md](BENCHMARK.md) for measured results.
