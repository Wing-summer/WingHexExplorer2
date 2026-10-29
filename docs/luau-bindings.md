# WingLuauQt host configuration / 宿主生成配置

The `codegen/` directory contains only JSON configuration. Framework Python generators produce value bindings, module registration and definitions. Native operations live in `3rdparty/WingLuauQt/support/bindings/`, the host driver in `tools/generate_luau_bindings.py`, and tests in `tests/luaubindings/`. WingLuauQt is the generic framework submodule. Generated WingLuauBind files are disposable build output.

codegen 目录仅维护 JSON 配置；绑定注册及定义由框架 Python 生成，原生操作位于 3rdparty/WingLuauQt/support/bindings，驱动位于 tools，测试位于 tests/luaubindings。通用框架作为 `3rdparty/WingLuauQt` 子模块引入，WingLuauBind 是可删除、可重新生成的构建产物，不再作为第二份维护源码。

## Configuration files / 配置文件

| File / 文件 | Purpose / 用途 |
| --- | --- |
| `generation.json` | Driver, policies, dependencies, registration includes and build-relative output / 驱动、策略、依赖、注册 include 及构建目录内的产物路径 |
| `bindings.json` | Qt baseline/discovery, module ownership, extensions and callback signatures/native symbols / 类型、接口及回调策略 |
| `forwarding.json` | SDK callback routing, defaults, receiver expressions and UI/service assignments / 宿主转发规则及原生适配表达式 |
| `framework_bindings` in `bindings.json` | Select generated framework binding groups / 选择框架自动生成的绑定分组 |
| `wingluauqt_integrate` + `codegen/generation.json` | Automatic configure-time generation and static-library integration / 配置阶段自动生成及静态库接入 |

All native input paths resolve against the host root. `generation.json` selects policy files relative to its own directory and an output path relative to the current CMake build directory. Change its `output` to relocate generated sources; do not edit generated CMake files.

原生输入相对于宿主根目录；generation.json 中 bindings/forwarding 相对于该文件目录，output 相对于当前 CMake 构建目录，相对的 WINGHEX_BINDINGS_OUTPUT_DIR 也按该目录解析。修改 output 即可改变生成位置，也可使用 CMake 变量 `WINGHEX_BINDINGS_OUTPUT_DIR` 临时覆盖。

## Build as a submodule / 子模块构建

Use CMake 3.24+ and Python 3.12+. CMake reuses an environment with the required tsujikiri version or prepares a build-local environment through WingLuauQt. After the SDK target exists, the host calls the framework’s `wingluauqt_integrate` with `codegen/generation.json`; generation runs before the static library is added. No manual generation step is required.

使用 CMake 3.24+、Python 3.12+；框架优先复用依赖完整的 Python 环境，否则在构建目录准备虚拟环境。可通过 Python3_EXECUTABLE 指定已有环境，通过 WINGLUAUQT_BOOTSTRAP_PYTHON=OFF 禁止自动安装。

```sh
# Host root; optionally pass -DPython3_EXECUTABLE=<prepared interpreter>.
cmake -S . -B build -G Ninja
cmake --build build --parallel 3
```

Python and tsujikiri are generation dependencies. Qt, Luau and LuaBridge3 reuse host dependencies; use the same Qt SDK for discovery and compilation. For a temporary external framework checkout, override `WINGLUAUQT_SOURCE_DIR` with a path relative to the host root.

默认生成到 `${CMAKE_BINARY_DIR}/generated/WingLuauBind`，例如 `cmake -B /tmp/winghex-build` 对应 `/tmp/winghex-build/generated/WingLuauBind`，不固定写入源码树的 build。框架位置由 WINGLUAUQT_SOURCE_DIR 配置，默认 3rdparty/WingLuauQt；不要将本机绝对路径写入配置。已有生成工程也能独立编译，此时不需要 Python。

For manual regeneration or a temporary destination, use an interpreter with tsujikiri installed and expose the submodule's Python package:

手动生成时使用已安装 tsujikiri 的解释器，并通过 PYTHONPATH 指向子模块包；不要求子模块内存在 .venv。驱动必须显式指定 --build-dir 或 --out，不默认在源码目录生成。

```sh
PYTHONPATH=3rdparty/WingLuauQt/python python3 tools/generate_luau_bindings.py --build-dir build
PYTHONPATH=3rdparty/WingLuauQt/python python3 tools/generate_luau_bindings.py --out build/temporary-bindings
```

## Changing ScriptMachine registration / 修改注册

Each function in `bindings.json` → `callback_sets` supplies its Luau name, parameter/return signature and `native` symbol, for example `&ScriptMachine::print`. That single descriptor generates the C++ callback contract, definition and `callback_initializers.inc`. ScriptMachine includes it through `SCRIPTMACHINE_LUAU_CALLBACKS`; do not maintain another function-pointer list.

修改 print 等接口时在同一处修改 name、parameters/returns 和 native。原生函数实现仍由 ScriptMachine 维护，注册和定义会同步再生成。若修改 SDK 转发，先修改 native_apis 的选择规则，再在 forwarding.json 添加必要的接收者、参数转换或默认返回值；不用定位并修改 tools/generate_luau_bindings.py 中的函数分支。

The registration include macros are configurable: `SCRIPTMACHINE_LUAU_BINDING`, `SCRIPTMACHINE_LUAU_CALLBACKS` and `PLUGINSYSTEM_LUAU_BINDING`.

三个 include 宏分别控制 ScriptMachine 集成头、自动回调初始化片段及 PluginSystem 转发片段，宿主源码不依赖生成文件的具体名字。

The generated binding pipeline is mandatory. ScriptMachine initialization/cleanup, definition loading and PluginSystem forwarding use it directly; there is no legacy registration fallback or `WINGHEX_LUAU_BINDINGS` switch. Superseded AngelScript addon sources and clipboard/file-permission stubs have been removed.

生成绑定链路始终启用；ScriptMachine 初始化、清理、定义加载和 PluginSystem 转发直接使用它，不再保留旧注册回退或条件开关。已移除被替代的 AngelScript addon 源码及剪贴板、文件权限空壳。

## Modules and type files / 模块与类型文件

Globals are reader, ctl, log, hex and ScriptMachine output functions. hex contains HexPosition, MetadataInfo, ErrFile and SelectionMode. QString is ustring; QColor is color. Containers become tables with no List/Map aliases. Qt dialog names remain QMessageBox, QInputDialog, QFileDialog and QColorDialog; methods follow snake_case.

Native value userdata uses `qtluau::QtValueRef<QString>` and `qtluau::QtValueRef<QByteArray>`; the Luau names remain `ustring` and `bytearray`. Use the framework's conversion API for ordinary strings and containers, with explicit UTF-8 at host text boundaries.

原生值 userdata 使用上述 QtValueRef 模板，Luau 名称仍为 ustring 和 bytearray；普通字符串及容器复用框架转换接口，宿主文本边界明确使用 UTF-8。

公共值类型还有 bytearray、url、char、variant、regex。Qt 采用可配置的 6.6.2 API 基准；用较新 SDK 扫描旧基准时需要匹配的 QDoc 索引，仍应使用基准 SDK 进行最终兼容验证。

Named utilities json/xml/cbor/env/crypto/toast/service are globals by default; their optional require mode uses canonical builtin paths such as `require("@b/json")`. Select Qt binding groups with `framework_bindings`. Python scans SDK declarations to generate methods, constructors, conversions, dependency classes and matching definitions. Concrete document and service callbacks remain in host forwarding configuration.

剪贴板位于 `qt.QClipboard`，其扫描得到的实例方法通过 `QGuiApplication::clipboard()` 作为静态接口调用，例如 `qt.QClipboard.set_image(image)` 和 `qt.QClipboard.image()`；不提供独立的 clipboard 模块。`QImage` 等签名依赖由生成器递归发现。该实例提供者策略同样适用于其他 Qt 单例。

Clipboard is exposed as static `qt.QClipboard` functions through the configured `QGuiApplication::clipboard()` instance provider. Signature dependencies such as `QImage` are discovered recursively; no separate clipboard module is registered.

运行时通过 `registerBuiltinValue` 注册模块值。`definitionFiles()` 只返回公共定义，直接交给 LSP 初始化；`moduleFiles()` 返回精简模块类型入口，通过 LuauLanguageServer::InitializationOptions::builtinModuleFiles 交给语言服务器实例。运行时仅通过 registerBuiltinValue 注册值，不修改 WingLuauRequire 的文件解析路径，也不维护全局模块路径表。

CMake copies the shared definition and all module type entries beside the executable under luaudef. Change `WingLuauBind_DEFINITIONS_DIR` to choose another directory. Definitions are not Qt resources; the runtime resolves deployed paths relative to the executable directory.

Installation also deploys these files with their generated names through the `LuauDefinitions` component. On macOS they reside beside the executable inside the application bundle.

安装时通过 LuauDefinitions 组件部署相同名称的定义文件；macOS 放在应用包内的可执行文件旁。

## Add native classes / 增加原生类

Add an extensions rule with a header glob, explicit classes and public/access policy. For host-owned implementation/MOC, retain reference mode and compile_moc=false. Restricted members require meta-object or friend access. A custom dialog can expose QWidget as its base; UI roots are controlled by the host resolver.

新增类可将 headers 设为整个目录的 glob，classes 维护暴露策略。private/protected 不绕过 C++ 权限；公共默认 public。容器属性赋值使用 table 快照，宿主保留 QObject 所有权和线程规则。详见框架的 docs/extensions.md。

## Validation / 验证

```sh
cmake -S tests/luaubindings -B build/bindings-tests -G Ninja
cmake --build build/bindings-tests --parallel 3
ctest --test-dir build/bindings-tests --output-on-failure
```

The tests generate a fresh project through the same CMake function, compile the real host resolver/LSP, check the shared definition and module file paths, and execute all utility modules. This does not replace a complete host build or a build with the baseline Qt SDK.

The generated host library omits execution and require helpers by default. Tests explicitly compile the framework execution support and use the real `WingLuauRequire`, registering module values under canonical `@b/` paths just like the application.

生成库默认不导出执行及 require 助手；测试显式编译框架执行支持，并使用真实宿主 WingLuauRequire 和 @b/ 模块路径，与应用保持一致。

独立测试与完整宿主构建分别验证、分别报告；静态库测试中的文档回调不等同于所有 GUI 文档操作已覆盖。

## Module exposure / 模块暴露

Modules keep their names in both modes. They are globals by default. Set `modules.<name>.require` to `true` to hide that global and import it through the host builtin require path. This project selects:

模块名称在两种模式下保持一致，默认直接公开为全局；将 `modules.<name>.require` 设为 `true` 后，只能通过宿主 builtin require 路径导入。本项目配置：

```json
"modules": { "qt": { "require": true } }
```

```luau
local qt = require("@b/qt")
qt.QClipboard.set_text("example")
local doc, error = json.from_json('{}')
```

`moduleFiles()` lists only require modules; the host registers these values through `WingLuauRequire::registerBuiltinValue`. Public globals and module type entries follow the same exposure policy. The existing cffi registration remains host-owned.

`moduleFiles()` 仅列出 require 模块，宿主通过 `WingLuauRequire::registerBuiltinValue` 注册模块值；全局声明与模块类型入口遵循同一暴露配置。既有 cffi 注册由宿主维护。

For fast registration regression checks, see `tests/luaubindings/smoke/README.md`. It reuses built dependencies and does not download or build the application.

快速注册自测见 `tests/luaubindings/smoke/README.md`，复用已构建依赖，不下载依赖或编译应用本体。

The module directory, main migration matrix, readonly env contract and generated reports are documented in [Luau modules](luau-modules.md).

完整模块目录、main 迁移矩阵、只读 env 契约及生成报告见 [Luau 模块](luau-modules.md)。


`generation.json` owns driver/output paths, source dependency patterns, native dependency roots and registration include settings. The framework CMake function implements the shared generation/link/copy/install steps. The former host-only CMake integration file has been removed.

`generation.json` 保存驱动与产物路径、源码依赖模式、原生依赖目录和注册 include 设置；框架通用函数负责生成、链接、复制和安装，已移除宿主专用 CMake 集成文件。

Exports serialize writers for the same output directory and generate in a private staging directory. Successful exports publish individual files atomically, keep unchanged timestamps and remove stale generated assets. Generation failures leave compiler inputs intact; host forwarding is included in the same staging step.

同一输出目录的生成任务互斥执行，并在隔离目录完成生成。成功后逐文件原子发布，保留未变文件时间戳并清理过期产物；生成失败不会破坏编译输入，宿主转发也在同一暂存步骤完成。

`scriptserviceapi.h/.cpp` is a native host service adapter, not generated binding code. The generator scans its real declaration and derives the callback contract, definitions and forwarding from configuration. Its implementation stays in the host; the native signature is not duplicated in configuration.

`scriptserviceapi.h/.cpp` 是宿主原生服务适配，不是生成绑定。generator 扫描真实声明，并按配置生成回调契约、定义和转发；原生行为保留在宿主，不在配置重复维护签名。

The generated symbol directory also records public type presentation names for all modules, including shared host values, callable namespaces and cffi types. Browse pages use those names at generation time. LSP hover, completion details and signature help use an optional Luau type-rendering callback, selected by the named type's identity and definition origin. This does not rename bindings, mutate frozen types or rewrite rendered text; unrelated plugin/user types and string singleton types retain their original names.

生成的符号目录同时记录所有模块的公开类型显示名，包括宿主共享值、可调用命名空间及 cffi 类型。浏览页在生成时使用这些名称；LSP 的 hover、补全详情和参数提示通过 Luau 可选类型渲染回调，依据命名类型及定义来源选择显示名。不重命名绑定、不修改冻结类型图，也不替换渲染后的文本；插件及用户的无关类型和字符串单例类型保持原样。
