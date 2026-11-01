# 主题测试 / Theme tests

测试直接编译 ThemeManager、SDK 调用桥和实际应用资源，不依赖完整宿主或
Luau 生成器。覆盖标准图标目录、SVG 结构、深浅主题、四种图标状态、五种尺寸、
四档 DPI、已有 QIcon 的主题切换、主要前景色对比度、系统主题通知、位图缓存失效及 SDK 图标/主题调用。

These tests compile ThemeManager, the SDK call bridge, and actual application
resources without the full host or Luau generator. They cover the standard icon
catalog, SVG structure, both themes, four icon modes, five sizes, four DPI ratios,
theme changes on existing QIcons, primary-color contrast, system appearance
notifications, pixmap-cache invalidation, and SDK icon/theme calls.

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
