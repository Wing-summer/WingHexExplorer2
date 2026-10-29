# Luau modules / Luau 模块

The generator owns registration, definitions and the browse-only symbol catalog. `codegen/` contains policy only. Outputs belong to the selected CMake build directory under `generated/WingLuauBind/`.

注册、定义和浏览符号目录由 generator 共同产生。`codegen/` 仅保存配置，产物位于所选 CMake build 目录的 `generated/WingLuauBind/`。

## Public entry points / 公共入口

| Entry / 入口 | Access / 访问 | Purpose / 用途 |
| --- | --- | --- |
| reader | global / 全局 | Document state, reads, search, selections, metadata and bookmarks / 文档状态、读取、查找、选择、元数据、书签 |
| ctl | global / 全局 | Editing, macros, writes and document control / 编辑、宏、写入、文档管理 |
| log | global / 全局 | trace, debug, info, warn, error |
| hex | global / 全局 | HexPosition, MetadataInfo, ErrFile, SelectionMode |
| ustring | global / 全局 | QString, explicit UTF-8 conversion / QString、显式 UTF-8 转换 |
| bytearray | global / 全局 | QByteArray |
| variant | global / 全局 | QVariant |
| color | global / 全局 | QColor |
| url | global / 全局 | QUrl |
| char | global / 全局 | QChar |
| regex | global / 全局 | Regular expressions and matches / 正则表达式和匹配结果 |
| json | global / 全局 | QJsonDocument and related types / JSON 文档及相关类型 |
| xml | global / 全局 | QXmlStreamReader, QXmlStreamWriter |
| cbor | global / 全局 | QCborValue and related types / CBOR 值及相关类型 |
| env | global / 全局 | Four readonly functions listed below / 下述四个只读函数 |
| crypto | global / 全局 | QCryptographicHash |
| filesystem | global / 全局 | file / fileinfo / dir: QFile, QFileInfo, QDir |
| service | global / 全局 | Service discovery and calls / 服务查询及调用 |
| toast | global / 全局 | show |
| network | require("@network") or require("@b/network") | TCP, UDP, TLS and local sockets / TCP、UDP、TLS 及本地 socket；见 [网络接口](luau-network.md) |
| qt | require("@qt") or require("@b/qt") | Qt classes, dialogs, clipboard, widgets, layouts and UI loading / Qt 类、对话框、剪贴板、控件、布局、UI 加载 |
| cffi | require("@cffi") or require("@b/cffi") | Independent FFI instances / 独立 FFI 实例 |

`@b/` explicitly selects a builtin. Shorthand `@name` follows the require system's package resolution rules. Module policy defaults to global; `modules.<name>.require: true` makes that module require-only without renaming it.

`@b/` 明确选择内置模块，`@name` 遵循 require 的包解析规则。模块默认全局公开；配置 `modules.<name>.require: true` 后必须 require，模块名称不变。

`WingLuauRequire` resolves paths, builtin names and configuration aliases using its native require resolver. Analysis replaces only the final load operation with a destination query; it never executes the target module. The language server maps a resolved builtin name to its generated definition entry and does not interpret require syntax or alias priority.

路径、内置名称和配置别名统一由 `WingLuauRequire` 的原生 require 解析器处理。分析仅将最终加载替换为目标查询，不执行模块。语言服务器只把解析后的内置名称映射到生成定义入口，不解析 require 语法或别名优先级。

The language server keeps one `WingLuauRequire::ResolutionSession` instead of opening a VM for each import. Queries clear native result caches and navigation state so configuration/filesystem changes are observed. Symbol browsing lives in `luausymbols.cpp`; generated pages carry individual overload signatures, displayed as separate rows with stable selection IDs.

语言服务器复用一个解析会话，避免每次导入创建 VM。查询清理原生结果缓存和导航状态，保证配置及文件变化可见。符号浏览位于 `luausymbols.cpp`；索引生成独立重载签名，对象树逐条显示，并使用稳定 ID 保留选择。

Symbol sources and categories use host enums. Opaque stable IDs are used only for identity, cache keys and parent relationships; navigation uses explicit paths and module-root metadata, not ID prefixes.

符号来源和分类使用宿主枚举。稳定 ID 仅用于身份、缓存键及父子关联；导航使用独立路径和模块根元数据，不解析 ID 前缀。

Native plugin registrations provide a `QList<ScriptSymbol>` value snapshot with enum kinds, without JSON conversion. LSP initialization builds parent-to-children indexes in linear time and caches module roots. JSON remains the on-disk format for generated catalog pages and is decoded only when reading those pages.

原生插件注册提供带枚举类型的 `QList<ScriptSymbol>` 值快照，不经过 JSON 中转。LSP 初始化建立父子索引并缓存模块根目录。JSON 仅作为生成目录页的磁盘格式，在读取页面时解码。

`input()` remains an `ustring`. Its `__tostring` registration is generated from the configured SDK conversion method; host output inspection uses the metamethod through a protected call. Handwritten host integration headers use `.h`; generated framework headers remain in the build directory.

`input()` 保持返回 ustring。`__tostring` 由 generator 根据配置选择的 SDK 转换方法生成，宿主输出检查器通过受保护调用使用它。手写宿主集成头文件使用 `.h`，框架生成头文件继续位于 build 目录。

The runtime also supplies the Luau standard libraries (`coroutine`, `table`, `os`, `string`, `math`, `debug`, `utf8`, `bit32`, `buffer`, `vector`, and `integer` when enabled), base functions, `require` and `proxyrequire`. Host globals are `print/println/warnprint/warnprintln/infoprint/infoprintln/errprint/errprintln`, `input`, `stringify` and `assert_x`. Host debugging extends `debug` with `set_break` and `elapsed_time` while retaining native `debug.traceback`. Plugins/devices contribute namespaces under their actual PUID; these are marked as plugin extensions.

运行时另外提供 Luau 标准库、基础函数、require/proxyrequire，以及上述宿主输出和辅助函数。`debug` 保留原生能力并增加宿主调试方法。插件和设备按实际 PUID 动态公开命名空间，目录标记为插件扩展。

`env` exports `read_environment_var(name, default?)`, `is_environment_var_set(name)`, `is_environment_var_empty(name)` and `read_environment_var_int(name)`. Text arguments accept string or ustring; text results are ustring. Integer reads return `(number, boolean)`. There is no environment object or mutation API.

`env` 只公开以上四个读取函数；文本参数接受 string/ustring，文本结果为 ustring，整数读取返回数值及成功标记，不公开环境对象和写操作。

## Main migration matrix / main 迁移矩阵

This matrix follows `main:src/class/scriptmachine.cpp` (host globals, `registerEngineAddon`, clipboard, assertions, debugging and exception routines). SDK reader/ctl/log forwarding remains selected from the actual public SDK headers by generator policy; the generated catalog lists the exact current methods.

本表对照 main 的宿主注册和 addon 注册入口。reader/ctl/log 继续从实际 SDK 公共头扫描并选择，精确方法清单以生成目录为准。

| Main capability / main 能力 | Current handling / 当前处理 | Status / 状态 |
| --- | --- | --- |
| reader, ctl, log, hex | Same global module boundaries; generated SDK forwarding / 保持全局模块边界，生成 SDK 转发 | Implemented / 已实现 |
| Eight print functions / 八个输出函数 | Same host behavior; snake_case definitions / 保留宿主输出行为 | Implemented / 已实现 |
| printf, warnprintf, infoprintf, errprintf | Luau interpolated strings / Luau 插值字符串 | Excluded by policy / 按策略排除 |
| exec | No process execution API; QProcess excluded / 不公开进程执行 | Excluded by policy / 按策略排除 |
| input, stringify, assert_x | Host implementation, generated registration and definitions / 宿主实现，生成注册和定义 | Added in this change / 本轮补齐 |
| debug.setBreak, elapsedTime | debug.set_break, debug.elapsed_time | Added in this change / 本轮补齐 |
| assert, throw, exception state, stack traces | assert/error/pcall/xpcall/debug.traceback | Luau native replacement / Luau 原生替换 |
| msgbox, inputbox, filedlg, colordlg | qt.QMessageBox, qt.QInputDialog, qt.QFileDialog, qt.QColorDialog | Qt replacement / Qt 替换 |
| clipboard | Static qt.QClipboard; MIME and QImage / 静态 QClipboard、MIME、图像 | Qt replacement / Qt 替换 |
| RegisterQString, QStringUtils, QStringRegExSupport | ustring and scanned Qt APIs / ustring 及 Qt 扫描接口 | Qt replacement / Qt 替换 |
| RegisterScriptRegex | regex | Qt replacement / Qt 替换 |
| RegisterColor, QJson, Url, Crypto, Env | color, json, url, crypto, readonly env / 只读 env | Qt replacement / Qt 替换 |
| RegisterOptBox | Qt dialog classes / Qt 对话框类 | Qt replacement / Qt 替换 |
| RegisterScriptArray, Dictionary, Grid | Luau tables; containers convert at boundaries / Luau table 和边界容器转换 | Luau native replacement / Luau 原生替换 |
| RegisterScriptAny, Handle, WeakRef | Luau values, userdata and GC / Luau 值、userdata、GC | Engine-specific replacement / 引擎专属替换 |
| RegisterScriptMath | math | Luau native replacement / Luau 原生替换 |
| RegisterScriptMathComplex | Not automatically exposed / 不自动公开 | Excluded by policy / 按策略排除 |
| RegisterScriptDateTime | qt.QDate/QTime/QDateTime | Qt replacement / Qt 替换 |
| RegisterScriptFile, FileSystem | filesystem.file / fileinfo / dir | Scanned Qt replacement / Qt 扫描替换 |
| ASReflection | Not exposed / 不公开 | Excluded by policy / 按策略排除 |
| AngelScript engine/context/type-ID routines / 引擎、上下文、type ID | No AngelScript emulation / 不模拟 AngelScript | Engine-specific / 引擎专属 |

Old dialog module names are not compatibility aliases. `qt.QDialog` and `qt.QMainWindow` retain their script names while the host factory supplies Frameless implementations. Construction and `.ui` loading use the same factory/finalizer contract. Host classes and settings remain outside WingLuauQt.

不保留旧对话框模块空壳。QDialog/QMainWindow 的脚本名称不变，构造和 UI 加载通过同一工厂和完成回调创建宿主 Frameless 类型；宿主类及设置留在应用中。

Script-only host subclasses track content assembly privately. Ordinary Frameless windows retain their existing implementation and manual assembly contract. UI loading preserves root geometry and moves UI-owned dialog children into the content area; main-window menu access uses host hooks to retain the Frameless title bar.

脚本专用宿主子类私有记录装配状态；普通 Frameless 窗口保持原实现和手动装配契约。UI 加载保留根对象几何，并把对话框的 UI 子控件移入内容区；主窗口菜单访问通过宿主钩子保留无边框标题栏。

## Loading and generated reports / 加载与生成报告

Each VM has registration states and completion counters. Public dependency types register once; Qt reuses those identities. Qt-only registration executes on first require in a private writable environment; repeated require returns the cached module. A failed loader stays failed and cannot partially register again. cffi retains its static-library `registerBuiltinValue` path.

每个 VM 独立维护注册状态和完成计数；公共依赖只注册一次，Qt 复用类型身份。Qt 专属注册延迟到首次 require，在私有可写环境中执行。失败 loader 不再重试部分注册。cffi 保持静态库及原有值注册路径。

Definitions use dependency-first strongly connected components. Native userdata and callable class tables retain nominal identities; overloads are emitted as native declarations rather than large normalized intersections. Only public components preload. Qt/cffi private components are ordinary required definition modules; their declaration syntax is enabled only in the configured builtin definition directory.

定义按依赖顺序划分强连通分量，原生对象及可调用类保留具名身份，重载使用原生声明，避免大型交集正规化。公共依赖预加载，Qt/cffi 私有分片按 require 分析；声明语法仅对配置的内置定义目录启用。

Generated files in the build directory:

构建目录中的报告：

- `manifest.json`: module entry paths, preload order, component dependencies and sizes / 模块路径、预加载顺序、依赖和体积。
- `module_api_report.json` and `definitions/*_symbols*.json`: full current API directory and direct-member pages / 当前完整 API 目录及直接成员页。
- `definition_report.json`: SCC dependencies and per-component byte sizes / 分片依赖和体积。
- `exclusion_report.json`: configured exclusions and discovery rejection reasons / 排除策略及扫描拒绝原因。
- `lua_c_api_report.json`: fixed cffi factory/instance/callback exports, fields and contract coverage / cffi 固定导出、字段及契约覆盖。

QMdiArea, QMdiSubWindow, QWizardPage, QProcessEnvironment and QProcess remain excluded from discovery, dependencies, generated APIs and UI factories. Unsupported signatures are omitted, never converted to `any` to expose an excluded pointer.

上述排除类型不会通过依赖、方法或 UI 工厂重新公开，无法安全表达的签名被跳过，不通过 any 泄露指针。
