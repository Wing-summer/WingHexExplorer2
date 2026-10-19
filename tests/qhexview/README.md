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
