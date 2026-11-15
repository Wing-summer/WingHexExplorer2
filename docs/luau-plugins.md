# Luau 插件 / Luau plugins

## 包和入口 / Packages and entry points

普通插件放在应用或用户数据目录的 `plugin/*.wingplg`，设备插件放在 `devdrv/*.wingdrv`。脚本包与原生插件使用相同扩展名；宿主读取文件开头的 ZIP `PK` 魔数分流：ZIP 走脚本包校验和对应 SDK 适配器，其余文件走原生 QPluginLoader。带 `PK` 魔数但损坏或不满足包协议的文件直接拒绝，不重试原生加载。插件须在插件设置中启用；证书检查、SDK 版本、PUID 和依赖规则与原生插件共用。

Ordinary packages use `plugin/*.wingplg`; device packages use `devdrv/*.wingdrv`, under the application or user data directory. Script and native plugins share these extensions. A leading ZIP `PK` signature selects script-package validation and the corresponding SDK adapter; other files use native QPluginLoader. A damaged or invalid package with a `PK` signature is rejected without retrying as a native plugin. Enable their IDs in plugin settings. Native and script plugins share certificate, SDK, identity and dependency checks.

根目录需要 `manifest.json` 和唯一一个入口：`main.lua`、`main.luau`、`main.luac` 或 `main.luauc`。前两种是源码，后两种是 **Luau** 字节码，不能装入标准 Lua 的 luac 输出。入口存在多个候选会拒绝加载；不靠内容猜测格式。

The root needs `manifest.json` and exactly one of the four entry names above. `.lua`/`.luau` contain source; `.luac`/`.luauc` contain **Luau**, not standard Lua, bytecode. Ambiguous entries are rejected. Format selection uses the extension, never content guessing.

```json
{
  "FormatVersion": 1,
  "Id": "MyPlugin",
  "SDK": 21,
  "Version": "1.0.0",
  "Author": "Your name",
  "License": "Your license",
  "Dependencies": [{"Id": "AnotherPlugin", "Version": "1.0.0"}]
}
```

元信息复用现有字段，不增加 Kind、IID 或二进制标记。包签名验证使用读入的同一份 ZIP 快照。资源路径不能越过包根目录，不允许链接或重复规范化路径。上限为 4096 项、单项 16 MiB、解压总量和压缩包各 64 MiB。

Metadata reuses existing fields, without Kind, IID or binary flags. Signatures verify the same ZIP snapshot used for loading. Paths must remain inside the package; links and duplicate normalized paths are rejected. Limits are 4,096 entries, 16 MiB per entry and 64 MiB each for the archive and total expanded contents.

## 对象和生命周期 / Objects and lifecycle

```luau
local P = plugin.Plugin:extend()
function P:plugin_name() return "My Plugin" end
function P:plugin_comment() return "Example" end
function P:init() return true end
function P:event_closing() return true end
function P:unload() end
return P:new()
```

设备插件使用 `plugin.DevicePlugin`。也可直接返回普通表或自行设置 metatable；宿主按 `self` 调用继承查找到的方法。`extend` 建立派生表，`new` 只建实例，初始化由宿主执行。名称和说明是必需方法。回调在入口加载后解析并固定，实例业务字段仍可修改。

Use `plugin.DevicePlugin` for device packages. Plain tables and custom metatables are also accepted. The host passes `self`, including for inherited methods. `new` creates an instance; only the host invokes initialization. Name and comment methods are required. Callback references are resolved after entry execution and then fixed; business fields remain mutable.

每插件独立 VM 和模块缓存，运行在 GUI 线程。初始化、关闭确认、UI 注册、绘制及设备回调不可 yield。`plugin.task.spawn(function() ... end)` 创建受管理任务，允许主动 yield，卸载时取消。无返回值事件排队，光标和选区事件合并待处理值，文件事件保持顺序。同步重入被拒绝；未捕获错误会停用插件；不设置脚本执行时限。

Each plugin owns a VM and module cache on the GUI thread. Synchronous callbacks cannot yield. `plugin.task.spawn` creates a managed task which may yield and is cancelled on unload. Void events are queued; pending cursor and selection events are coalesced, while file events retain their order. Synchronous reentry is rejected. Uncaught errors deactivate the plugin. Script execution has no time limit.

回调名称、签名及默认方法以维护的 [协议输入](../codegen/plugin_protocol.json) 为准，运行时查找表、默认基类、辅助函数注册和定义文件由该输入生成。

The maintained [protocol input](../codegen/plugin_protocol.json) supplies callback names, signatures and defaults. Lookup descriptors, base methods, helper registration and definitions are generated from it.

## 服务和 UI / Services and UI

`plugin.reader`、`plugin.ctl`、`plugin.log`、`plugin.toast`、`plugin.service` 复用 SDK 扫描出的服务签名，绑定所属插件 sender。设备插件不提供普通编辑器的 reader/ctl。`plugin.settings.get/set` 访问插件自己的设置。

Scanned SDK services bind the actual plugin sender. Device plugins do not expose ordinary editor reader/ctl services. Settings belong to the individual plugin.

```luau
plugin.service.export({
    add = function(self, left, right) return left + right end,
})
```

服务可在入口或初始化阶段导出，初始化结束后固定。现有 `service.invoke_service` / `plugin.service.invoke_service` 转发到脚本服务；参数与结果通过 Qt 值转换，不跨 VM 传递 LuaRef。表转换沿用 WingLuauQt 的深度、循环及容器检查，不支持的 userdata 明确失败。

Services may be exported during entry execution or initialization, and freeze afterwards. Existing service invocation routes dispatch to script services. Qt value conversion transfers arguments and results without sharing LuaRef objects; WingLuauQt conversion limits and cycle checks remain in force. Unsupported userdata fails explicitly.

UI 对象通过 WingLuauQt 创建。注册回调的描述结构如下；方法名和对象访问使用生成器的 snake_case 规则。

Create UI objects through WingLuauQt. Registration callbacks use these description shapes:

- `registered_ribbon_tools`: `{category, display_name, toolboxes = {{name, tools = {QToolButton, ...}}}}` 的列表 / a list of category/toolbox descriptions.
- `registered_dock_widgets`: `{id, name, widget, area}` 的列表 / a list of dock descriptions.
- `registered_hex_context_menu`: QMenu 或 nil / a QMenu or nil.
- `registered_setting_pages`: `{id, name, widget, restore, discard, contain_unsaved_changes, highlight_unsaved_change}` 的列表 / setting-page descriptions and callbacks.
- `registered_editor_view_widgets`: `{id, name, create}` 的列表；`create(self)` 返回 `{widget, load_state, save_state, has_unsaved_state, set_saved, toggled, on_closing, on_work_space_notify}` / factories returning a fresh view state and widget.

绘制回调得到作用域受限的 PaintContext 和借用控件，提供 draw_text、draw_line、fill_rect、set_pen；PaintContext 在回调结束后失效。不得复用同一 QWidget 作为多个编辑器视图的内容。

Painting receives a scoped PaintContext and borrowed widget. The paint facade expires after the callback. A view factory must create fresh content for each editor instance.

## 设备 IO / Device IO

**仅设备插件**有 `plugin.IODevice` 和 `plugin.io`。普通插件及普通脚本继续使用现有 WingLuauQt 文件系统路径，原有覆盖限制不变。

**Only device plugins** receive the dedicated IO type and factories. Ordinary plugins and scripts retain the existing filesystem restrictions.

```luau
local backend = plugin.io.open_file(path, 3) -- QIODevice::ReadWrite
local bytes = backend:read(256)
backend:seek(0)
backend:write(bytes)
backend:close()
```

包装层使用独立 QFile 后端，显式打开模式决定能否覆盖，不隐式添加截断模式。通用方法包括 read、read_all、write、seek、pos、size、at_end、is_open、is_readable、is_writable、is_sequential、open_mode、error_string、close。flush/resize 只用于支持它们的文件后端。

The facade uses an independent QFile backend. Explicit open modes control overwriting; truncation is not added implicitly. File-specific operations reject incompatible backends.

生成的 `_plugin_device_io`、`_plugin_device_types`、`_plugin_paint` 模块与其他绑定模块一样开放导入和类型补全，不根据模块名称判断权限。IO 工厂始终由宿主绑定到所属 VM 的真实上下文；普通脚本或普通插件调用时返回设备插件上下文不可用错误。设备专用 IO 和绘制包装不可自由构造，实例操作继续检查上下文、线程和有效期。

Generated `_plugin_device_io`, `_plugin_device_types` and `_plugin_paint` modules support imports and type completion like other binding modules, without name-based permission filtering. IO factories are bound by the host to their VM's actual context and reject ordinary scripts and plugins with an unavailable device-plugin context error. Device IO and painting wrappers cannot be freely constructed; operations continue to validate context, thread and lifetime.

`plugin.io.wrap(handle)` 只接受宿主授予当前设备插件的不透明句柄。底层销毁、授权撤销、插件停用或跨插件使用均不能继续访问；借用后端不改变宿主所有权，关闭能力单独授权。该包装不可作为普通 qt.QIODevice 或跨 VM 值传递。

Wrapping requires an opaque host grant for the current device plugin. Destruction, revocation and deactivation invalidate access. Borrowed devices retain host ownership and require an explicit close grant. The wrapper is not an ordinary qt.QIODevice and cannot cross VMs.

`on_open_file(self, path)` 返回设备表：read、size 必需，可写设备提供 write，随机访问设备提供 seek；close 可选。writable、sequential、keep_size、locked、overwrite 是设备状态字段。读返回字节数据；空字节表示 EOF，nil 与 error_string 表示读取失败；写返回数量或 -1。宿主校验长度并维护 QIODevice 状态。

The returned device table supplies read/size and capability-dependent write/seek methods, with an optional close callback. Empty bytes indicate EOF; nil plus error_string indicates read failure. Writes return a count or -1. The host validates sizes and maintains QIODevice state.

## 构建和打包 / Building and packaging

```sh
cmake --build /path/to/build --target WingLuauCompile
python3 tools/package_luau_plugin.py docs/examples/plugins/hello /tmp/LuauByteInspector.wingplg
python3 tools/package_luau_plugin.py docs/examples/plugins/device /tmp/LuauTailWindow.wingdrv \
  --compiler /path/to/build/wing-luau-compile
```

示例 `LuauByteInspector` 仅导出 `byte_summary` 字节统计服务；`LuauTailWindow` 将普通文件末尾最多 4096 字节映射成只读设备。两者使用独立 ID、名称和场景，可与原生 `TestPlugin`、`ShareMemoryDrv` 同时启用，不注册它们的 UI 或共享内存资源。同一示例的源码包和字节码包具有相同 ID，安装时二选一。

`LuauByteInspector` exports only the `byte_summary` statistics service. `LuauTailWindow` maps up to the last 4096 bytes of a regular file as a read-only device. Their distinct IDs, names and scenarios allow concurrent use with native `TestPlugin` and `ShareMemoryDrv`, without registering their UI or shared-memory resources. Install either the source or bytecode variant of each example; both variants share its ID.

工具按路径扩展名输出源码或编译后的 `.luauc`，保留 `.d.luau`，拒绝输出重名与链接，成功后才原子替换目标包。编译器与宿主使用同一 Luau 子模块版本。Luau 字节码不保证跨版本兼容，也不是任意损坏数据的验证格式；应保留可重新编译的源码。

The packager preserves declarations, rejects collisions and links, and publishes only a complete archive. Use the matching compiler built from the host checkout. Keep sources for recompilation; bytecode is neither a durable cross-version format nor a validator for arbitrary malformed data.

ScriptMachine 的 executeScript 按扩展名读取；executeChunk 接受原始字节、chunk 名称和显式格式；executeCode 仍只执行源码。字节码不作为源码发送给 LSP，源码调试请求会明确拒绝。加载插件时保持不同 VM；关闭控制台不会重置其他 VM 使用的进程级 UI 工厂。

File execution selects format by extension; executeChunk accepts explicit-format bytes, while executeCode remains source-only. Bytecode is not sent to source analysis, and source-debugging requests are rejected. Closing the console does not reset process-wide UI factories needed by plugin VMs.

`plugin.info` 是清单的只读快照；`plugin.resources.read/path` 读取包数据或返回只读虚拟路径。包内文件由内存中的 Qt 文件后端提供，不创建临时目录、占位文件或提取文件；虚拟路径可直接用于 QFile、QDir、图像、翻译和定义注册，包释放后失效。与 Qt 资源路径一样，它不是操作系统文件名；外部库应使用字节或 QIODevice 接口读取。`plugin.i18n.tr` 使用 `translations/<Id>_<locale>.qm`，上下文为清单 Id，不安装跨插件共享的翻译器。执行代码另有 1 MiB 上限。模块和配置通过统一的 QFile 路径读取；插件只为 require 设置包根目录边界，不另设内容读取回调。

`plugin.info` is a read-only manifest snapshot. Resource helpers read package bytes or return read-only virtual paths. An in-memory Qt file backend supplies package contents without temporary directories, placeholder files or extraction. Virtual paths work with QFile, QDir, images, translations and definition registration, and expire when the package is released. Like Qt resource paths, these are not operating-system filenames; external libraries should consume bytes or QIODevice input. Translation uses the plugin ID as context and its own translator, without global installation. Executable chunks additionally have a 1 MiB limit. Modules and configuration files share the QFile reader; plugins restrict require to the package root without providing a separate content-reader callback.

绘制回调的第三个参数是只读上下文快照，包含文档几何、光标／选区状态及颜色。它是值快照，不包含可保存后继续访问宿主的原生上下文指针。

The third painting argument is a read-only value snapshot of document geometry, cursor/selection state and colors, without a retained native context pointer.

依赖脚本插件停用时，关联脚本插件停止；关联原生插件的宿主事件及服务被禁用，但保留仍被编辑器引用的原生对象，需重启完成原生资源生命周期。此行为不等同原生插件热卸载。

Dependent script plugins stop with their dependency. Dependent native plugins lose host events/services, while native objects still referenced by editors remain until restart; this is not native hot unloading.

跨 VM 的 QObject、QWidget 和布局值使用 WingLuauQt 的 QObjectValue 载体保留 QPointer，在接收 VM 中建立非拥有包装，并叠加导出插件的有效期检查。导出方停用后，已有借用也不能继续访问对象；设备专用 IO 对象不在可传递范围内。

QObject, widget and layout values cross VMs through WingLuauQt's QObjectValue carrier. Receiving wrappers retain QPointer checks and an exporter-lifetime guard without adopting ownership. Device-plugin IO objects are excluded.

插件通过 `plugin.script.add_definition_file("definitions/api.d.luau")` 显式注册定义文件，入口或初始化阶段均可调用。宿主适配器在现有 `onRegisterScriptObj` 阶段使用同一个 `ScopeBuilder::addDefinitionFile()` 提交，直接使用包内文件的虚拟路径，重复注册只加载一次。初始化结束后不允许追加。定义仅描述插件暴露的脚本接口，不执行、不创建运行时函数；未注册的定义文件不会加载。LSP 不检索插件源码，也不为字节码模块自动查找声明。

Plugins explicitly register definitions using `plugin.script.add_definition_file("definitions/api.d.luau")` during entry execution or initialization. The host adapter submits them through the existing `ScopeBuilder::addDefinitionFile()` in `onRegisterScriptObj`, using virtual package paths and deduplicating repeated registration. Registration closes after initialization. Definitions describe exposed script interfaces without executing code or creating runtime functions. Unregistered definitions are not loaded. The LSP neither searches plugin source nor automatically locates declarations for bytecode modules.

插件发现、校验和依赖加载集中于 PluginLoader，PluginSystem 保留宿主注册、UI 安装和生命周期。脚本插件与原生插件共用定义文件注册与加载流程。WingCert 的文件与内存校验共用 QIODevice 入口；verifySignature 仅提供签名原语，信任与证书有效期由上层 verify 校验。

PluginLoader handles discovery, validation and dependency loading; PluginSystem retains host registration, UI installation and lifecycle. Script and native plugins share definition registration and loading. File and memory verification share WingCert's QIODevice entry point. verifySignature supplies cryptographic proof only; verify enforces trust and certificate validity.

Lua 调度器通过同一个执行接口恢复普通脚本和插件任务，执行状态由各自运行时维护，只发送完成通知。脚本文件读取、大小检查及格式识别复用 chunk 组件；Qt 服务参数适配集中在现有 ScriptServiceApi。PluginSystem 只保存一条按加载顺序排列的已加载插件列表，按接口类型查询并逆序卸载；停用集合单独记录仍保留对象的失效状态。依赖停用传播不依赖主窗口，UI 清理只处理界面登记。

The Lua scheduler resumes ordinary scripts and plugin tasks through one execution interface; each runtime owns its execution state, and the scheduler only emits completion. The chunk component handles script-file reads, size checks and format selection, while the existing ScriptServiceApi adapts Qt service arguments. PluginSystem keeps one ordered loaded-plugin list, filters it by interface type and unloads it in reverse. A separate inactive set records disabled objects retained by the host. Dependency deactivation works without a main window; UI cleanup only removes interface registrations.
