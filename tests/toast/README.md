# Toast 回归测试 / Toast regression tests

独立编译生产 Toast 控件，不依赖宿主或插件 SDK。需要 Qt 6.6.2+ Widgets、Test 和 CMake 3.20+。
Builds the production Toast independently of the host and plugin SDK. Requires Qt 6.6.2+ Widgets and Test, and CMake 3.20+.

```sh
cmake -S tests/toast -B /tmp/wing-toast-build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/wing-toast-build
ctest --test-dir /tmp/wing-toast-build --output-on-failure
```

默认 CTest 使用 offscreen；添加 `-DWINGHEX_FORCE_CPP17=ON` 可验证 C++17 回退。
CTest uses offscreen by default. Add `-DWINGHEX_FORCE_CPP17=ON` to verify the C++17 fallback.

在可访问当前桌面 socket 的环境中直接运行可验证真实 Wayland；此时会短暂显示测试窗口。
Run directly with access to the desktop socket to test native Wayland; test windows appear briefly.

```sh
QT_QPA_PLATFORM=wayland /tmp/wing-toast-build/wing_toast_tests
```

覆盖子控件浮层、焦点保持、嵌套父控件、窗口及控件输入、缩放及位置保持、小尺寸裁剪、样式与像素字体、图标更新、多行及自动折行、父控件隐藏与销毁、连续替换、悬停暂停、圆角背景及淡出像素。
Covers the child overlay, focus preservation, nested parents, window and widget input, resizing and position preservation, small bounds, styles and pixel fonts, icon updates, multiline text and wrapping, parent hiding and destruction, replacement, hover pauses, rounded corner backgrounds, and fade pixels.

Toast 在父控件的内容区域内显示，沿用全局仅一个 Toast 的行为，不需要顶层窗口定位、原生窗口透明度或焦点激活。此独立测试不能替代宿主内多屏、缩放和模态对话框的人工交互检查。
Toast appears within the parent's content area and preserves the single global Toast behavior. It needs no top-level positioning, native window opacity, or focus activation. These standalone tests do not replace manual host checks with multiple screens, scaling, and modal dialogs.

默认圆角半径为 8 个逻辑像素，使用抗锯齿路径绘制，边角显示父控件背景；不使用原生窗口遮罩。
The default corner radius is 8 logical pixels. An antialiased path draws the background and reveals the parent at the corners, without a native window mask.

消息保留显式换行（`\n`、`\r\n`、`\r`）及空行；长文本优先按单词折行，长单词也可拆行。文本布局在尺寸或字体变化时更新，图标位于首行左侧。Toast 高度限制在父控件内容区域内，超出该区域的文字裁剪显示。
Messages preserve explicit line breaks (`\n`, `\r\n`, `\r`) and blank lines. Long text wraps at word boundaries, splitting long words when needed. The text layout updates with size or font changes; the icon sits to the left of the first line. Toast height is bounded by the parent content area, and overflowing text is clipped.
