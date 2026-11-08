# 编码规范 / Coding Guidelines

## 范围 / Scope

- 项目规范在同一文件内提供中英双语，仅根目录 `README.md`、`README_en.md` 分开。贡献及提交要求见 `CONTRIBUTING.md`。
  Keep project guidelines bilingual in one file; only the root `README.md` and `README_en.md` use separate versions. Follow `CONTRIBUTING.md` for contributions and commits.
- 聚焦请求的改动，保留无关的暂存、未暂存和未跟踪工作。除明确的全仓风格基线任务外，不得重置或批量格式化；风格任务只处理项目自有 C++ 及非 submodule 集成 3rdparty。
  Keep changes focused. Preserve unrelated staged, unstaged, and untracked work. Do not reset or bulk-format except for an explicit repository-wide style baseline, and then only for project-owned C++ and the non-submodule integrated 3rdparty code.
- 沿用 `src` 分层：`class` 服务、`control` 控件、`dialog` 对话框、`model` 模型、`settings` 设置、`luau` 集成。
  Follow the `src` layout: `class` services, `control` widgets, `dialog` dialogs, `model` models, `settings` pages, and `luau` integration.
- `WingPlugin` 是独立子模块及公开 SDK；编辑前检查其本地改动。`3rdparty` 中非 submodule 的集成库（`QHexView`、`QConsoleWidget`、`QWingRibbon`、`qtsingleapplication`）按项目自有代码处理；其余 submodule 遵循各自格式和许可，保留已有许可声明。
  `WingPlugin` is a separate submodule and public SDK; inspect its local changes before editing. Non-submodule integrated libraries under `3rdparty` (`QHexView`, `QConsoleWidget`, `QWingRibbon`, and `qtsingleapplication`) are treated as project-owned code; the remaining submodules keep their own formatting, licenses, and license headers.

## C++ 与格式 / C++ and formatting

- 项目自有 C++（包括 WingPlugin）的 `if`、`else`、`if constexpr`、所有 `for`、`while`、`do ... while` 主体及每个 `else if` 分支必须使用 `{}`，单语句和空主体也不例外。
  In project-owned C++, including WingPlugin, use `{}` for every `if`, `else`, `if constexpr`, all `for` loops, `while`, `do ... while`, and each `else if` branch, including single statements and empty bodies.
- 按适用的 `.clang-format` 格式化项目自有代码；根目录使用 `InsertBraces`、`IncludeBlocks: Regroup`、`SeparateDefinitionBlocks: Always`，`WingPlugin` 使用自身配置。不为落实规则改写生成代码、git submodule 或复制的第三方测试头文件。
  Format project-owned code with its applicable `.clang-format`; the root config uses `InsertBraces`, `IncludeBlocks: Regroup`, and `SeparateDefinitionBlocks: Always`, while `WingPlugin` uses its own config. Do not rewrite generated code, git submodules, or copied third-party test headers to enforce these rules.
- 根配置使用四空格、无制表符、80 列、同行左花括号，以及 `QObject *object`、`const QString &text` 风格；换行、包含分组和函数间空行交给格式化工具。
  The root style uses four spaces, no tabs, 80 columns, attached braces, and `QObject *object` / `const QString &text`. Let the formatter wrap lines, group includes, and separate definitions.
- 类型（类、结构体、枚举、命名空间、类型别名）使用 PascalCase，缩略词按单词处理（如 `Xml`）；项目类型不添加 `Q` 前缀。函数、方法和局部变量使用 camelCase；getter 使用 `foo()`/`isFoo()`/`hasFoo()`，setter 使用 `setFoo()`。文件名保持小写。
  Types (classes, structs, enums, namespaces, type aliases) use PascalCase with acronyms treated as words (e.g. `Xml`); do not add a `Q` prefix to project types. Functions, methods, and locals use camelCase; getters use `foo()`/`isFoo()`/`hasFoo()`, and setters use `setFoo()`. Keep filenames lowercase.
- 私有和受保护的非静态数据成员使用 `m_` 前缀加 camelCase；公开聚合数据成员使用无前缀 camelCase；不使用前导或尾随下划线。常量与枚举值使用 PascalCase，宏使用 SCREAMING_SNAKE_CASE。Qt pimpl 指针 `d_ptr`、`q_ptr` 及 Designer 界面的 `ui` 指针保持 Qt 写法。
  Private and protected non-static data members use the `m_` prefix with camelCase; public aggregate data members use camelCase without a prefix; do not use leading or trailing underscores. Constants and enumerators use PascalCase, and macros use SCREAMING_SNAKE_CASE. Keep the Qt pimpl pointers `d_ptr` and `q_ptr` and the Designer `ui` pointer in their Qt form.
- 例外：公开插件 SDK、脚本 API、`Q_PROPERTY`/`Q_ENUM` 注册名、设置键、资源别名、翻译上下文、Qt 自动连接槽（`on_<object>_<signal>`）、Qt 重写、继承的第三方成员，以及以字符串为契约的枚举值不因风格改名；新增代码遵循上述规则。
  Exceptions: public plugin SDK, script API, `Q_PROPERTY`/`Q_ENUM` registered names, settings keys, resource aliases, translation contexts, Qt auto-connect slots (`on_<object>_<signal>`), Qt overrides, inherited third-party members, and string-keyed enum values are not renamed for style; new code follows the rules above.
- 信号统一使用 `sig` 前缀并采用 `sigOn<语义事件>` 形式，例如 `sigOnTextChanged`；名称须表达触发时机与对象，避免 `log`、`triggered`、`finished` 等泛化名。重命名时同步 `Q_PROPERTY NOTIFY`、连接、`QSignalSpy` 和 `.ui` 连接。
  Signals use the `sig` prefix and the `sigOn<SemanticEvent>` form, for example `sigOnTextChanged`; names must convey the trigger and object, avoiding generic names such as `log`, `triggered`, and `finished`. Update `Q_PROPERTY NOTIFY`, connections, `QSignalSpy`, and `.ui` connections together.
- 头文件保护宏遵循相邻文件，新增头文件使用 `FILENAME_H` 与 `#endif // FILENAME_H`。实现文件首先包含对应头文件。声明须自包含；前置声明足够时优先使用，否则包含完整类型，兼顾模板、内联代码和 Qt MOC。
  Match neighboring header guards; new headers use `FILENAME_H` and `#endif // FILENAME_H`. Include the corresponding header first in implementation files. Keep declarations self-contained; prefer sufficient forward declarations, otherwise include complete types for templates, inline code, and Qt MOC.
- 头文件按固定组排序：对应头文件 → `ui_*.h` → C++ 标准库 → C/POSIX/Windows 系统头 → Qt（含 `<private/...>`）→ Lua C API → Luau → LuaBridge3 → 其他第三方库 → 项目头文件。含 UI 的实现文件最前面两行必须是 `#include "foo.h"` 与 `#include "ui_foo.h"`，两者之间无空行；之后才允许空行和其他分组。组间一个空行，组内字母排序；条件包含留在所属组末尾。使用 LuaBridge3 时必须先包含 Luau 头文件，禁止把 LuaBridge3 排到 Luau 前面。由 clang-format 执行。
  Sort includes in fixed groups: corresponding header → `ui_*.h` → C++ standard library → C/POSIX/Windows system headers → Qt (including `<private/...>`) → Lua C API → Luau → LuaBridge3 → other third-party libraries → project headers. Files that use a generated UI header must start with `#include "foo.h"` followed immediately by `#include "ui_foo.h"` with no blank line between them; only then may a blank line and other groups follow. Separate groups with one blank line and sort alphabetically within a group; keep conditional includes at the end of their group. When using LuaBridge3, include the Luau headers first; never put LuaBridge3 before Luau. Let clang-format enforce this.
- 每个实现文件的函数定义之间保留一个空行；构造函数、析构函数、运算符、命名空间级函数及类内联定义同样适用；不保留连续空行。由 `SeparateDefinitionBlocks: Always` 执行。
  Keep exactly one blank line between function definitions in implementation files, including constructors, destructors, operators, namespace-level functions, and in-class inline definitions; do not keep consecutive blank lines. `SeparateDefinitionBlocks: Always` enforces this.
- 根目录 `.clang-tidy` 持续检查私有/受保护数据成员及局部变量、参数命名；类型、函数和信号命名仍由本规范与代码评审约束。`d_ptr`、`q_ptr`、`ui` 已豁免。
  The root `.clang-tidy` continuously checks private/protected data-member and local/parameter naming; type, function, and signal naming remain governed by this document and code review. `d_ptr`, `q_ptr`, and `ui` are exempt.
- 使用 `nullptr`、const 正确性、`override` 和适当的 `explicit`。兼容 CMake 选择的 C++20 及 `WINGHEX_FORCE_CPP17` 回退路径。
  Use `nullptr`, const correctness, `override`, and appropriate `explicit` constructors. Support CMake's C++20 selection and the `WINGHEX_FORCE_CPP17` fallback.

## Qt 与接口 / Qt and interfaces

- Qt 与插件边界沿用 Qt 类型；固定文本使用 `QStringLiteral`，资源使用现有辅助函数。区分字节与文本，显式转换编码。
  Use Qt types at Qt and plugin boundaries, `QStringLiteral` for fixed strings, and existing resource helpers. Distinguish bytes from text and convert encodings explicitly.
- 遵循 QObject 父对象所有权和控件生命周期，避免重复所有权；其他资源沿用既有智能指针或 RAII。
  Follow QObject parent ownership and widget lifetimes without duplicate ownership; use existing smart-pointer or RAII patterns for other resources.
- 优先类型安全的信号槽；lambda 提供 QObject 上下文并确保捕获对象有效。沿用类的信号槽声明风格，UI 操作留在 GUI 线程，后台工作使用已有异步机制。
  Prefer typed connections with QObject contexts and valid lambda captures. Match the class's signal/slot style, keep UI work on the GUI thread, and use existing asynchronous mechanisms.
- 模型重置及结构变更须配对 begin/end 通知。复用现有管理器、设置、日志和生命周期钩子，避免重复基础设施。
  Pair begin/end notifications for model resets and structural changes. Reuse existing managers, settings, logging, and lifecycle hooks rather than duplicating infrastructure.
- 除任务明确要求外，保持公开 SDK 签名、插件 ID、所有权和 Qt 元对象契约；变更时同步调用方及示例。宿主细节留在应用内。
  Preserve public SDK signatures, plugin IDs, ownership, and Qt meta-object contracts unless explicitly required; update callers and examples together. Keep host details in the application.
- 脚本集成沿用 Luau 绑定及插件注册路径，保留注册顺序、沙箱、错误传播和 userdata 所有权。编辑器定制使用扩展点，不改生成或上游高亮数据。
  Extend existing Luau bindings and plugin registration while preserving order, sandboxing, error propagation, and userdata ownership. Customize editors through extension points, not generated or upstream highlighting data.

## 文案 / UI text

- 遵循[文案与翻译规范](docs/i18n.md)。使用自然英文原文及适当翻译上下文，维护简繁中文目录，不重新引入英文语言包。
  Follow the [UI text guide](docs/i18n.md). Use natural English source text and appropriate translation contexts, maintain the Chinese catalogs, and do not reintroduce an English package.
- 短 tooltip、按钮和标签避免多余的句末句号；信息框、文本框和长说明可保留结尾标点。按语义保留疑问或强调的 `?`、`!`（含中文形式），以及表示进行中或后续操作的 `...`。
  Avoid unnecessary final periods in short tooltips, buttons, and labels. Message boxes, text boxes, and extended explanations may retain punctuation. Preserve meaningful question/exclamation marks, including Chinese forms, and `...` for progress or further actions.
- 显示文案不改变内部名称、配置键或脚本 API。编辑 `.ui`、`.qrc` 源文件而非生成代码，并检查布局及无障碍影响。
  Display-text changes must preserve internal names, settings keys, and script APIs. Edit `.ui` and `.qrc` sources, not generated code, and check layout and accessibility.

## 构建与验证 / Build and validation

- 使用源码外构建，要求 CMake 3.20+、Qt 6.6.2+；依赖与选项以 `CMakeLists.txt`、`.github/workflows` 为准。复用合适的构建目录，发现本地工具路径，不提交机器专属绝对路径。
  Build out of source with CMake 3.20+ and Qt 6.6.2+. Use `CMakeLists.txt` and `.github/workflows` as the dependency and option references. Reuse suitable build directories and discover local tools without committing machine-specific paths.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

```sh
# 格式检查（与 CI 相同的 clang-format 22）
find src TestPlugin TestDepPlugin ShareMemoryDrv tools tests \
     3rdparty/QHexView 3rdparty/QConsoleWidget \
     3rdparty/QWingRibbon 3rdparty/qtsingleapplication \
  -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' \
             -o -name '*.cc' -o -name '*.c' \) -print0 |
  xargs -0 clang-format --dry-run -Werror

# 命名检查（根 .clang-tidy；需要 compile_commands.json）
run-clang-tidy -p build -config-file .clang-tidy \
  '.*/(src|TestPlugin|TestDepPlugin|ShareMemoryDrv|tools|3rdparty/(QHexView|QConsoleWidget|QWingRibbon|qtsingleapplication))/'
```

若 clang-tidy 因 Qt 的 GCC 专属参数 `-mno-direct-extern-access` 无法解析，使用 clangd，或从编译数据库中移除该参数后运行同一 `.clang-tidy`。
If clang-tidy cannot parse Qt headers because of Qt's GCC-only `-mno-direct-extern-access` flag, use clangd or remove that flag from the compilation database before running the same `.clang-tidy`.

- 使用 clang-format 22（与 CI 一致）检查项目自有 C++；提交前用 22 与 23 对拍，输出必须一致。头文件分组、UI 头顺序和函数间空行由根 `.clang-format` 执行。
  Check project-owned C++ with clang-format 22 (matching CI). Before submitting, compare 22 and 23 outputs and require identical results. The root `.clang-format` enforces include grouping, UI-header order, and blank lines between definitions.
- 根目录 `.clang-tidy` 持续检查成员/局部命名；类型、函数和信号命名仍由本规范与代码评审约束。`d_ptr`、`q_ptr`、`ui` 不重命名。
  The root `.clang-tidy` continuously checks member/local naming; type, function, and signal naming remain governed by this document and code review. Do not rename `d_ptr`, `q_ptr`, or `ui`.
- 运行相关检查及 `git diff --check`；QHexView 测试见 `tests/qhexview/README.md`，翻译测试见 `docs/i18n.md`。UI 几何检查不能替代动态菜单、Ribbon 及原生控件的人工检查。
  Run relevant checks and `git diff --check`. See `tests/qhexview/README.md` for QHexView tests and `docs/i18n.md` for translation tests. Geometry checks do not replace manual inspection of dynamic menus, Ribbon controls, and native widgets.
- 修复警告原因，不抑制诊断掩盖缺陷。如实区分静态审查、针对性编译、独立测试与完整构建，说明依赖或环境阻碍。
  Fix warning causes rather than suppressing diagnostics to hide defects. Report static review, focused compilation, standalone tests, and full builds separately, including dependency or environment blockers.
