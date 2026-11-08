# 工作区回归测试 / Workspace regression tests

直接构建生产编解码与迁移解析代码，并使用 Qt Test 验证，无需完整应用子模块。需要 CMake 3.20+、Qt 6.6.2+（Core、Gui、Widgets、Concurrent、Test）。默认 C++17，可通过 `CMAKE_CXX_STANDARD=20` 验证 C++20。
Tests build the production codec and legacy parser directly without the full application's submodules. Requires CMake 3.20+ and Qt 6.6.2+ (Core, Gui, Widgets, Concurrent, Test). C++17 is the default; use `CMAKE_CXX_STANDARD=20` to exercise C++20.

```sh
cmake -S tests/workspace -B /tmp/wing-workspace-tests -G Ninja -DWINGHEX_BUILD_WORKSPACE_MIGRATOR=ON
cmake --build /tmp/wing-workspace-tests
ctest --test-dir /tmp/wing-workspace-tests --output-on-failure
```

迁移工具默认不构建；开启选项后额外验证真实命令行的退出码、覆盖保护及输入输出身份检查。关闭选项仍运行格式、迁移解析及错误处理测试。
The migrator defaults to OFF. Enabling it adds real CLI checks for exit codes, overwrite protection, and input/output identity. With it OFF, format, legacy parsing, and failure-handling tests still run.

覆盖直接列表序列化、完整往返、64 位值、颜色透明度与无效状态、Unicode 与相对路径、设备 URL、插件数据保留、业务校验、每个字节位置的截断、非法列表数量、旧 JSON 迁移及目标保护。
Coverage includes direct list streaming, full round trips, 64-bit values, color alpha and invalid states, Unicode and relative paths, device URLs, plugin data preservation, validation, truncation at every byte boundary, invalid list counts, JSON migration, and destination protection.

这些独立测试不能替代完整宿主构建、编辑器另存交互及原生平台文件身份检查。格式和工具说明见 [工作区格式 / Workspace format](../../docs/workspace-format.md)。
These standalone tests do not replace full-host compilation, interactive editor Save As checks, or native-platform file identity checks. See [Workspace format](../../docs/workspace-format.md) for the format and tool documentation.
