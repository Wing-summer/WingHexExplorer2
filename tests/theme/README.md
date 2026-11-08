# 主题测试 / Theme tests

测试直接编译 ThemeManager、SDK 调用桥和实际应用资源，不依赖完整宿主或
Luau 生成器。覆盖标准图标目录、SVG 结构、深浅主题、四种图标状态、五种尺寸、
四档 DPI、已有 QIcon 的主题切换、主要前景色及按钮状态对比度、系统主题通知、
位图缓存失效及 SDK 图标/主题调用。

These tests compile ThemeManager, the SDK call bridge, and actual application
resources without the full host or Luau generator. They cover the standard icon
catalog, SVG structure, both themes, four icon modes, five sizes, four DPI ratios,
theme changes on existing QIcons, primary-color and button state contrast, system
appearance notifications, pixmap-cache invalidation, and SDK icon/theme calls.

```sh
cmake -S tests/theme -B build/theme-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/theme-tests
ctest --test-dir build/theme-tests --output-on-failure
```

C++17 回退在另一个构建目录添加 `-DWINGHEX_FORCE_CPP17=ON`。要求 Qt 6.6.2+ 的
Widgets、Svg、Xml 和 Test 模块；在较新 Qt 上通过不等于验证过 Qt 6.6.2。

For the C++17 fallback, use a separate build directory and add
`-DWINGHEX_FORCE_CPP17=ON`. Requires Qt 6.6.2+ Widgets, Svg, Xml, and Test modules.
Passing with a newer Qt does not establish validation on Qt 6.6.2.

## 预览 / Previews

指定 `WING_THEME_PREVIEWS` 可输出实际 Qt 渲染的深浅主题对照图。可选的
`WING_THEME_ORIGINALS` 指向旧 PNG 图标目录，只用于对照，不参与图稿生成。
每组从左至右为旧 PNG、浅色主题、深色主题。无旧 PNG 的既有 SVG 项留空。

Set `WING_THEME_PREVIEWS` to export light/dark comparison sheets rendered by Qt.
Optionally point `WING_THEME_ORIGINALS` at the old PNG directory for comparison;
these bitmaps are never used to generate artwork. Each group shows old PNG,
light theme, then dark theme. Existing SVG-only icons have an empty old-PNG cell.

```sh
QT_QPA_PLATFORM=offscreen WING_THEME_PREVIEWS="$PWD/build/theme-tests/previews" \
    build/theme-tests/wing_theme_tests
```

渲染检查不能替代实际菜单、Ribbon 和原生控件的人工检查。
Render checks do not replace manual inspection of menus, Ribbon, and native controls.

自动启动屏测试覆盖历史耗时过滤与中位数、快速启动不显示、延时显示、预计慢阶段前
提前显示、立即完成以及关闭后定时器不再显示窗口；这些测试在计时宏关闭时运行。
Automatic splash tests cover history filtering/medians, hidden fast starts,
deadline display, display before predicted slow phases, immediate completion,
and cancellation of pending display after close, with profiling compiled out.

## 启动优化验证 / Startup optimization checks

补充验证 SVG 栅格缓存的重复命中、多尺寸共存、淘汰后重绘、调色板变化，以及启动期
延迟应用 QSS 后的主题及缓存失效。`wing_theme_benchmark` 使用实际图标资源，预热后
测量 127 个图标以 16/24/32 尺寸重复绘制的中位数；局部收益不能替代完整启动测量。
Additional tests cover repeated raster hits, multiple sizes, eviction/repainting,
palette changes, and deferred startup QSS application with theme/cache invalidation.
`wing_theme_benchmark` measures warmed-up repeated rendering of 127 real icons at
16/24/32 pixels. Local gains do not replace full-host startup measurements.

```sh
QT_QPA_PLATFORM=offscreen build/theme-tests/wing_theme_benchmark
```

启动屏还会在全局 QSS 延后应用时提前取得当前主题样式，并仅应用于自身子树。
深浅主题测试比较全局 QSS 应用前后的逐像素截图，同时覆盖进度文案、主题切换和关闭
时的生命周期。可用 `WING_SPLASH_PREVIEWS` 输出测试截图：
The splash obtains the selected theme sheet before deferred global QSS application
and applies it only to its own subtree. Light/dark tests compare pixel-identical
renders before/after global application, and cover progress text, theme changes
and close-time lifetime. Export test renders with `WING_SPLASH_PREVIEWS`:

```sh
QT_QPA_PLATFORM=offscreen WING_SPLASH_PREVIEWS=build/splash-previews \
  build/theme-tests/wing_theme_tests splashBeforeGlobalStyleSheet
```

Qt 标准图标测试直接比较完整原始 QSS 与加载时提取默认值后的样式，覆盖当前 Qt 的全部标准角色（其中 54 项默认声明被提取）的
`QStyle::standardIcon()` / `standardPixmap()` 像素、深浅主题和 2 倍缩放。补充检查
Dock 专用透明图标、QDialogButtonBox、局部 QSS 优先级、注释/字符串中的伪规则、
重复默认值，以及未知属性、复杂值和缺失资源声明的保留。图标路径只维护在 QSS 中。
Qt standard-icon tests compare the complete original QSS with the extracted
runtime sheet: all standard roles in the current Qt version, including the 54
extracted defaults, through `standardIcon()` / `standardPixmap()`, both themes,
and 2x scaling. Additional checks cover Dock-specific transparent icons,
QDialogButtonBox, local QSS precedence, apparent rules in comments/strings,
repeated defaults, and preservation of unknown properties, complex values and
missing-resource declarations. Icon paths are maintained only in the QSS.

## 标准图标全量覆盖 / Complete standard-icon coverage

Qt 样式表可表达的默认图标继续由 QSS 决定；其余角色在 `theme/breeze.qrc` 使用
`/<light|dark>/standard-icons/SP_枚举名.svg` 别名绑定现有主题 SVG，不在 C++ 中维护
文件名清单。QSS 优先，枚举别名补充主题覆盖，原生样式处理未定义的自定义角色。
Qt-expressible defaults remain in QSS. Other roles use enum-named aliases under
`/<light|dark>/standard-icons/SP_EnumName.svg` in `theme/breeze.qrc`, pointing to
existing SVGs without a C++ filename catalog. QSS takes precedence, aliases
complete the theme, and undefined custom roles retain native-style behavior.

测试直接编译 TestPlugin 的真实 TestForm、ReaderTestForm、CtlTestForm 和 QCheckList，
使用最小 SDK 作者图片端点构造界面，不加载外部插件。逐项检查深浅主题的全部预定义
标准枚举：组合框条目数完整、QIcon 和 QPixmap 均非空；任何 `cbToastIcon` 警告均失败。
同时保留独立原生样式加完整原始 QSS 的对照，当前 Qt 6.12/Fusion 原本缺 13 项，
完整主题覆盖后缺失为零。
Tests compile the real TestPlugin forms and checklist, constructing TestForm
with a minimal SDK author-image endpoint and without loading external plugins.
Both themes check every predefined standard role: complete combo-box counts,
non-null QIcons/QPixmaps, and failure on any `cbToastIcon` warning. An independent
native-style/full-original-QSS reference remains: Qt 6.12/Fusion originally lacks
13 roles; complete theme coverage leaves none missing.
