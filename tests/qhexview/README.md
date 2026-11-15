# QHexView 回归测试 / QHexView regression tests

需要 CMake 3.20+、C++17 编译器和 Qt 6.6.2+ Widgets、Gui、Concurrent 和 Test；独立构建，CTest 使用 offscreen 平台，无需桌面会话。

Requires CMake 3.20+, a C++17 compiler, and Qt 6.6.2+ Widgets, Gui, Concurrent, and Test. Build independently; CTest uses the offscreen platform without a desktop session.

```sh
cmake -S tests/qhexview -B /tmp/wing-qhexview-tests -G Ninja
cmake --build /tmp/wing-qhexview-tests
ctest --test-dir /tmp/wing-qhexview-tests --output-on-failure

# Release 路径也必须检查 / Also check the Release path
cmake -S tests/qhexview -B /tmp/wing-qhexview-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/wing-qhexview-release
ctest --test-dir /tmp/wing-qhexview-release --output-on-failure
```

覆盖信号重绑定、共享光标、显示通知、标注渲染、多字节光标闪烁、选区、行宽、剪贴板和撤销。

Covers signal rebinding, shared cursors, display notifications, metadata rendering, multibyte cursor blinking, selections, line widths, clipboard operations, and undo.

十六进制数据单元格和数值面板的解码预览共享 `GlyphRenderer`，按实际字形覆盖范围选择字体，并通过 `QGlyphRun` 直接绘制独立字符，避免自动塑形和字体回退。不能将 `QTextLayout::setRawFont()` 与 `QTextLayout::draw()` 连用；Qt 将此组合标记为未定义行为。此路径仅用于逐字符显示二进制数据，不用于整段自然语言排版。

Hex data cells and decoded previews in the number inspector share `GlyphRenderer`, selecting fonts by actual glyph coverage and drawing isolated characters directly with `QGlyphRun`, without automatic shaping or font fallback. Do not combine `QTextLayout::setRawFont()` with `QTextLayout::draw()`; Qt marks that combination as undefined behavior. This path displays binary data one character at a time and does not lay out natural-language text runs.

测试显式启用 `qt.text.font.db.info`，覆盖多种文字、组合符号、补充平面字符、缺失字形、不可打印字符，以及字体库变化后的缓存失效；有字形时验证真实字符仍被绘制。数值面板测试同时检查列宽/行高测量、默认与选中状态的实际表格绘制，以及模型保留原始解码内容。

Tests explicitly enable `qt.text.font.db.info` and cover multiple scripts, combining marks, supplementary characters, missing glyphs, nonprintable characters, and cache invalidation after font database changes; available glyphs must remain visible. Number-inspector tests also exercise column/row sizing, actual table painting in normal and selected states, and preservation of the original decoded content in the model.
