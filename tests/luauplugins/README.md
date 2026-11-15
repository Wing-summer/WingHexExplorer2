# Luau 插件验证 / Luau plugin validation

独立测试覆盖 chunk 栈契约、源码和字节码、require、包与设备 IO。使用源码外构建：

Standalone tests exercise chunk stack contracts, source/bytecode, require, packages and device IO:

```sh
cmake -S tests/luauplugins -B /tmp/wing-plugin-tests -G Ninja
cmake --build /tmp/wing-plugin-tests --target tst_luauplugins WingLuauCompile
ctest --test-dir /tmp/wing-plugin-tests --output-on-failure
```

通过 `-DCMAKE_CXX_STANDARD=20` 或 `17` 分别验证语言版本。宿主集成探针复用已完成的应用构建，不下载依赖：

Select C++20 or C++17 using CMAKE_CXX_STANDARD. The integration probe reuses a completed host build:

```sh
python3 tests/luaubindings/build_host_probe.py --build /path/to/build \
  --source tests/luauplugins/host_runtime.cpp --output WingLuauPluginProbe
QT_QPA_PLATFORM=offscreen /path/to/build/WingLuauPluginProbe
```

探针使用隔离的配置目录，检查源码与字节码插件、表继承、双 VM 隔离、任务与控件回调、事件排队、导出服务、QObject 借用失效、显式定义文件注册与补全、设备 IO 以及 ScriptMachine 四种扩展名、内存字节码、交互式源码与失败清理。真实窗口中的 Ribbon、原生菜单、停靠面板和绘制仍须人工检查。

The probe isolates configuration and checks source/binary plugins, inheritance, VM isolation, task-created widget callbacks, event delivery, services, QObject borrow expiry, explicit definition registration and completion, device IO and ScriptMachine file/memory execution, interactive source, and failed-load cleanup. Real-window Ribbon, native menu, dock and paint behavior still needs manual inspection.

示例包另有运行验证：将文档中的两个示例各打包为源码和字节码变体，输出到同一目录，文件名分别为 `LuauByteInspector[-bytecode].wingplg`、`LuauTailWindow[-bytecode].wingdrv`；运行探针时设置 `WING_PLUGIN_EXAMPLES_DIR=/path/to/packages`。这会验证真实示例的含 NUL 字节统计、尾部窗口定位、EOF、空文件和原文件保持不变。两个示例的 ID 与原生测试插件不同；打包测试同时检查 ID 唯一性。同一示例的两种变体仅依次加载。

To run the actual examples, package source and bytecode variants in one directory as `LuauByteInspector[-bytecode].wingplg` and `LuauTailWindow[-bytecode].wingdrv`, then set `WING_PLUGIN_EXAMPLES_DIR=/path/to/packages` when running the probe. It checks NUL-aware statistics, tail-window offsets, EOF, empty files and unchanged input bytes. Packaging tests check that example IDs differ from the native test plugins. Variants of the same example are loaded sequentially.

只读虚拟文件测试覆盖 QFile 定位和含 NUL 读取、QDir 列举与过滤、Unicode 路径、图像加载、禁止修改及挂载释放。宿主探针同时验证不落盘的源码／字节码 require、模块缓存、资源路径、翻译以及原有定义注册流程。探针使用应用构建所需的同版本 lrelease 生成隔离翻译夹具。

Read-only virtual-file tests cover QFile seeking and NUL-aware reads, QDir enumeration and filtering, Unicode paths, image loading, rejected mutations and mount disposal. The host probe checks source/bytecode require and caching, resource paths, translations and existing definition registration without extracting package contents. It uses the matching lrelease required by the application build to compile an isolated translation fixture.

宿主探针验证普通脚本、普通插件和设备插件均能导入插件绑定模块；普通上下文调用 IO 工厂被拒绝，设备上下文可正常读写，包装对象不允许自由构造。无头进程测试也覆盖模块可导入且能力按上下文拒绝的行为。

The host probe imports plugin binding modules from ordinary scripts, ordinary plugins and device plugins. IO factories reject ordinary contexts, device contexts can read and write, and wrappers cannot be freely constructed. The headless process regression also checks successful imports and context-based capability rejection.

require 读取边界测试同时覆盖物理与虚拟目录：包内模块及配置别名正常加载，内置模块不受文件边界影响，父目录配置、包外别名、同名前缀兄弟目录及 Unix 符号链接不能绕过边界。

Require boundary tests cover physical and virtual directories: in-root modules and configuration aliases load normally, builtins remain available, and parent configurations, outside aliases, sibling directories with matching prefixes and Unix symlinks cannot bypass the boundary.

后缀、候选文件和有界读取由共享 chunk 组件提供；测试覆盖大小写后缀、类型声明排除、源码／字节码原始字节、空文件和读取上限。require 配置与脚本读取复用同一读取函数，配置仍拒绝 NUL。

The shared chunk component owns suffixes, candidate names and bounded reads. Tests cover case-insensitive extensions, declaration exclusion, source/bytecode bytes, empty files and size limits. Require configurations use the same reader while still rejecting NUL bytes.
