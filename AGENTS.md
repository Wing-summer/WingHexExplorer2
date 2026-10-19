# 编码规范 / Coding Guidelines

## 范围 / Scope

- 项目规范在同一文件内提供中英双语，仅根目录 `README.md`、`README_en.md` 分开。贡献及提交要求见 `CONTRIBUTING.md`。
  Keep project guidelines bilingual in one file; only the root `README.md` and `README_en.md` use separate versions. Follow `CONTRIBUTING.md` for contributions and commits.
- 聚焦请求的改动，保留无关的暂存、未暂存和未跟踪工作，不得重置或批量格式化。
  Keep changes focused. Preserve unrelated staged, unstaged, and untracked work; do not reset or bulk-format it.
- 沿用 `src` 分层：`class` 服务、`control` 控件、`dialog` 对话框、`model` 模型、`settings` 设置、`luau` 集成。
  Follow the `src` layout: `class` services, `control` widgets, `dialog` dialogs, `model` models, `settings` pages, and `luau` integration.
- `WingPlugin` 是独立子模块及公开 SDK；编辑前检查其本地改动。尊重 `3rdparty` 边界，仅按任务需要修改依赖，遵循各自格式和许可，保留已有许可声明。
  `WingPlugin` is a separate submodule and public SDK; inspect its local changes before editing. Respect `3rdparty` boundaries, change dependencies only as required, follow their formatting and licenses, and preserve license headers.

## C++ 与格式 / C++ and formatting

- 项目自有 C++（包括 WingPlugin）的 `if`、`else`、`if constexpr`、所有 `for`、`while`、`do ... while` 主体及每个 `else if` 分支必须使用 `{}`，单语句和空主体也不例外。
  In project-owned C++, including WingPlugin, use `{}` for every `if`, `else`, `if constexpr`, all `for` loops, `while`, `do ... while`, and each `else if` branch, including single statements and empty bodies.
- 按适用的 `.clang-format` 格式化改动范围；根目录和 WingPlugin 均启用 `InsertBraces`。不为落实规则改写无关或生成代码。
  Format changed code with its applicable `.clang-format`; both root and WingPlugin enable `InsertBraces`. Do not rewrite unrelated or generated code to enforce these rules.
- 根配置使用四空格、无制表符、80 列、同行左花括号，以及 `QObject *object`、`const QString &text` 风格；换行和头文件排序交给格式化工具。
  The root style uses four spaces, no tabs, 80 columns, attached braces, and `QObject *object` / `const QString &text`. Let the formatter wrap lines and sort includes.
- 遵循相邻命名：类型 PascalCase，函数及局部变量 camelCase，文件名通常小写。保留类现有的 `_member` 或 `m_member` 约定及所在命名空间，不单纯为统一风格改名。
  Follow neighboring naming: PascalCase types, camelCase functions and locals, and generally lowercase filenames. Preserve existing member prefixes and namespaces; do not rename solely for consistency.
- 头文件保护宏遵循相邻文件，实现文件首先包含对应头文件。声明须自包含；前置声明足够时优先使用，否则包含完整类型，兼顾模板、内联代码和 Qt MOC。
  Match neighboring header guards and include the corresponding header first. Keep declarations self-contained; prefer sufficient forward declarations, otherwise include complete types for templates, inline code, and Qt MOC.
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
- 显示文案不改变内部名称、配置键或脚本 API。编辑 `.ui`、`.qrc` 源文件而非生成代码，并检查布局及无障碍影响。
  Display-text changes must preserve internal names, settings keys, and script APIs. Edit `.ui` and `.qrc` sources, not generated code, and check layout and accessibility.

## 构建与验证 / Build and validation

- 使用源码外构建，要求 CMake 3.20+、Qt 6.6.2+；依赖与选项以 `CMakeLists.txt`、`.github/workflows` 为准。复用合适的构建目录，发现本地工具路径，不提交机器专属绝对路径。
  Build out of source with CMake 3.20+ and Qt 6.6.2+. Use `CMakeLists.txt` and `.github/workflows` as the dependency and option references. Reuse suitable build directories and discover local tools without committing machine-specific paths.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

- 运行相关检查及 `git diff --check`；QHexView 测试见 `tests/qhexview/README.md`，翻译测试见 `docs/i18n.md`。UI 几何检查不能替代动态菜单、Ribbon 及原生控件的人工检查。
  Run relevant checks and `git diff --check`. See `tests/qhexview/README.md` for QHexView tests and `docs/i18n.md` for translation tests. Geometry checks do not replace manual inspection of dynamic menus, Ribbon controls, and native widgets.
- 修复警告原因，不抑制诊断掩盖缺陷。如实区分静态审查、针对性编译、独立测试与完整构建，说明依赖或环境阻碍。
  Fix warning causes rather than suppressing diagnostics to hide defects. Report static review, focused compilation, standalone tests, and full builds separately, including dependency or environment blockers.
