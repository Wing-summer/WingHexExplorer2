# QHexView 回归测试 / QHexView regression tests

独立于主应用构建并运行：

Build and run independently of the main application:

```sh
cmake -S tests/qhexview -B /tmp/wing-qhexview-tests -G Ninja
cmake --build /tmp/wing-qhexview-tests
ctest --test-dir /tmp/wing-qhexview-tests --output-on-failure
```

Requires Qt 6 Widgets, Gui, Concurrent, and Test. CTest selects the offscreen
platform, so the tests also run without a desktop session. Coverage includes
signal rebinding, shared cursors, display notifications, metadata rendering,
multibyte cursor blinking, selection ranges, line widths, clipboard operations,
and undo behavior.

需要 Qt 6 的 Widgets、Gui、Concurrent 和 Test 模块。CTest 使用 offscreen 平台，因此测试也可以在无桌面会话时运行。覆盖信号重绑定、共享光标、显示通知、标注渲染、多字节光标闪烁、选区范围、行宽、剪贴板操作和撤销行为。
