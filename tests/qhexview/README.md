# QHexView 回归测试 / QHexView regression tests

需要 CMake 3.20+、C++17 编译器和 Qt 6.6.2+ Widgets、Gui、Concurrent 和 Test；独立构建，CTest 使用 offscreen 平台，无需桌面会话。

Requires CMake 3.20+, a C++17 compiler, and Qt 6.6.2+ Widgets, Gui, Concurrent, and Test. Build independently; CTest uses the offscreen platform without a desktop session.

```sh
cmake -S tests/qhexview -B /tmp/wing-qhexview-tests -G Ninja
cmake --build /tmp/wing-qhexview-tests
ctest --test-dir /tmp/wing-qhexview-tests --output-on-failure
```

覆盖信号重绑定、共享光标、显示通知、标注渲染、多字节光标闪烁、选区、行宽、剪贴板和撤销。

Covers signal rebinding, shared cursors, display notifications, metadata rendering, multibyte cursor blinking, selections, line widths, clipboard operations, and undo.

Unicode 数据单元格按实际字形覆盖范围选择字体，使用独立字符布局，避免 Qt 因缺少特定 OpenType 表而反复回退。测试覆盖多种文字、补充平面字符、缺失字形、不可打印字符，以及字体库变化后的缓存失效；有字形时验证真实字符仍被绘制。此路径仅用于逐字符显示二进制数据，不用于整段自然语言排版。

Unicode data cells select fonts by actual glyph coverage and use isolated-character layouts, avoiding repeated Qt fallback caused by missing script-specific OpenType tables. Tests cover multiple scripts, supplementary characters, missing glyphs, nonprintable characters, and cache invalidation after font database changes; available glyphs must remain visible. This path displays binary data one character at a time and does not lay out natural-language text runs.
