# 回归测试 / Regression tests

各套测试独立构建，直接使用仓库内的生产代码，不需要先构建完整应用。所有命令均从仓库根目录运行，构建产物应放在源码目录之外。
Each suite builds independently against production code in this repository, without building the full application first. Run the commands from the repository root and keep build artifacts outside the source tree.

## 测试入口 / Test suites

| 目录 / Directory | 覆盖范围 / Coverage | 运行入口 / Runner |
| --- | --- | --- |
| [qtableviewext](qtableviewext/README.md) | 表格搜索、异步排序及列宽 / Table search, async sorting, and column widths | CTest |
| [qhexview](qhexview/README.md) | 十六进制视图、选区、剪贴板与撤销 / Hex view, selections, clipboard, and undo | CTest |
| [luaulsp](luaulsp/README.md) | Luau 诊断、补全、类型与模块解析 / Luau diagnostics, completion, types, and module resolution | CTest |
| [luauindentation](luauindentation/README.md) | Luau 缩进、闭合对齐、编辑事务与控制台保护 / Luau indentation, closing alignment, edit transactions, and console protection | CTest |
| [i18n](i18n/README.md) | 语言加载、回退、插件翻译与 UI 尺寸 / Language loading, fallback, plugin translation, and UI sizing | Python |

## 环境与运行 / Environment and execution

需要 CMake 3.20+、Qt 6.6.2+ 和 C++ 编译器；Luau 语言服务测试需要 C++20；缩进及表格扩展测试默认使用 C++20，支持 C++17 回退；其余测试使用 C++17。各套所需的 Qt 模块见各自说明。使用非系统 Qt 时，通过 `CMAKE_PREFIX_PATH` 环境变量或 CMake 参数选择安装目录。依赖子模块须已初始化；测试配置不会自动下载依赖。
Requires CMake 3.20+, Qt 6.6.2+, and a C++ compiler. Luau language-server tests require C++20; indentation and table extension tests default to C++20 and support a C++17 fallback; the other suites use C++17. See each suite for its Qt modules. Select a non-system Qt installation using the `CMAKE_PREFIX_PATH` environment variable or CMake argument. Required submodules must already be initialized; test configuration does not download dependencies.

依次执行各套说明中的命令即可运行全部测试。QHexView 和 Luau 使用 CTest 注册测试；国际化探针由 Python 驱动，单独运行 CTest 不会执行国际化测试。无桌面环境时使用 Qt 的 offscreen 平台，需要安装该平台插件。
Run the commands in each suite guide to execute the complete test collection. QHexView and Luau register CTest tests; Python drives the internationalization probes, so CTest alone does not run them. Headless execution uses Qt's offscreen platform plugin, which must be installed.

## 维护与验证 / Maintenance and validation

新增测试时同步维护所属测试的构建入口、覆盖范围和运行说明。CMake 按本目录的 `.cmake-format.py` 排版，C++ 按根目录 `.clang-format` 排版。
When adding tests, update the suite's build entry, coverage, and execution instructions together. Format CMake with this directory's `.cmake-format.py` and C++ with the root `.clang-format`.

```sh
cmake-format --check tests/*/CMakeLists.txt
clang-format --dry-run --Werror tests/*/*.cpp tests/i18n/stubs/class/settings/settingmanager.h
git diff --check
```

这些独立测试不能替代完整应用构建及人工交互检查。国际化 UI 尺寸检查的限制见 [文案规范](../docs/i18n.md)。
These standalone tests do not replace a full application build or manual interaction checks. See the [UI text guide](../docs/i18n.md) for the limits of internationalization UI sizing checks.
