# Luau 缩进回归测试 / Luau indentation regression tests

需要 CMake 3.20+、Qt 6.6.2+ Widgets、PrintSupport 和 Test，以及已初始化的 WingCodeEdit、syntax-highlighting 和 Luau 子模块。独立构建，直接链接仓库中的缩进策略及控件，不构建完整应用，也不下载依赖。
Requires CMake 3.20+, Qt 6.6.2+ Widgets, PrintSupport, and Test, and initialized WingCodeEdit, syntax-highlighting, and Luau submodules. This standalone suite links the production indentation policy and widgets without building the full application or downloading dependencies.

从仓库根目录运行；CTest 自动使用 offscreen 平台，无需桌面会话。
Run from the repository root; CTest selects the offscreen platform without a desktop session.

```sh
cmake -S tests/luauindentation -B /tmp/wing-luau-indentation-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/wing-luau-indentation-tests --target wing_luau_indentation_tests
ctest --test-dir /tmp/wing-luau-indentation-tests --output-on-failure
```

默认使用 C++20；加入 `-DWINGHEX_FORCE_CPP17=ON` 可验证 C++17 回退路径。
The default is C++20; add `-DWINGHEX_FORCE_CPP17=ON` to verify the C++17 fallback.

## 覆盖范围 / Coverage

| CTest 测试 / CTest test | 验证内容 / Checks |
| --- | --- |
| `wing_luau_indent_newline` | 块、分支、括号、续行、普通行、空行、Tab 与空格设置；换行及撤销/重做 / Blocks, branches, delimiters, continuation lines, ordinary and blank lines, indentation settings, newline insertion, and undo/redo |
| `wing_luau_indent_closing` | `end`、`else`、`elseif`、`until` 及闭合括号对齐；拒绝修改注释、长字符串、标识符、无匹配结构及表达式 `if` 分支 / Alignment of closing keywords and delimiters; declining comments, long strings, identifiers, unmatched structures, and expression-form `if` branches |
| `wing_luau_indent_editor` | 真实按键、选区替换、标识符内部输入、补全后的显式重缩进、自动缩进开关、只读保护及策略回退 / Real key events, selection replacement, typing inside identifiers, explicit reindentation after completion edits, auto-indent and read-only controls, and policy fallback |
| `wing_luau_indent_console` | 提示符及历史输出保护、传入策略的源码隔离 / Prompt and output protection and isolation of the source passed to the policy |

同时覆盖 Luau 语句与表达式形式的 `if`、嵌套表达式、注释、普通及长字符串、插值字符串和 Unicode 文本，防止把字符串中的关键字误判为代码块。
The suite also covers statement-form and expression-form `if`, nested expressions, comments, quoted and long strings, interpolated strings, and Unicode text so literal keywords do not become false block boundaries.

控制台测试使用真实 `QConsoleWidget` 与 Luau 策略，只有将累计命令和可编辑输入交给策略的宿主适配器是测试夹具；不实例化完整 `ScriptingConsole`、脚本运行时或 LSP。补全测试验证编辑事务与 `reindentCurrentLine()` 的接入约定，不操作补全弹窗。
The console test uses the production `QConsoleWidget` and Luau policy; only the host adapter that supplies accumulated commands and editable input is a fixture. It does not instantiate the full `ScriptingConsole`, script runtime, or LSP. The completion test verifies the edit-block and `reindentCurrentLine()` contract rather than operating the completion popup.

## 定位失败 / Diagnosing failures

Qt Test 输出具体的数据行名称，也可以单独执行一个用例。
Qt Test reports individual data-row names; a single case can also be run directly.

```sh
QT_QPA_PLATFORM=offscreen /tmp/wing-luau-indentation-tests/wing_luau_indentation_tests typedClosing:nested-end
```

这些独立测试不能替代完整应用构建及人工交互检查。
These standalone tests do not replace a full application build or manual interaction checks.
