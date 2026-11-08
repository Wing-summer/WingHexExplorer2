# Luau 文档模块 / Luau document module

`require("@s/hexdoc")` 使用系统 `luaulib` 中的纯 Luau 模块，不绑定 C++ `WingHexDocument`。运行时操作映射和类型定义来自同一份脚本 API 扫描元数据，新增 reader/ctl 文档操作会同步进入模块。

`require("@s/hexdoc")` loads a pure Luau module from the system `luaulib`. It does not bind C++ `WingHexDocument`. The operation map and declarations share the scanned script API metadata, so new reader/ctl document operations are included together.

```luau
local hexdoc = require("@s/hexdoc")
local doc = hexdoc.new()
if doc:open_file("/path/to/file.bin") == hex.ErrFile.Success then
    doc:use(function(d)
        local bytes = d:read_bytes(0, 16)
        d:select(0, #bytes)
    end)
    doc:close_handle()
end
```

- `open_file(path)`、`open_ext(driver, path)`、`open_workspace(path)` 真正打开文档；同一插件或脚本执行上下文重复打开同一文档时复用句柄；释放或关闭该句柄会使共享它的对象失效。重新打开同一对象前释放其旧句柄，失败时保持未打开状态。
- `close(force?)` 真正关闭文档，默认 `false`；关闭失败保留句柄。`close_handle()` 仅释放访问句柄。正常操作返回整数错误码。
- `is_valid()` 查询句柄是否仍属于当前执行上下文且文档仍打开，无需 `use()`；执行上下文结束后返回 `false`。
- `document_info()` 查询当前句柄信息；`activate()` 激活编辑器。
- `use(callback, ...)` 最外层进入时切换一次插件目标，不改变 UI 激活，也不恢复原目标。同一对象可嵌套；同一执行上下文中不同对象不可重叠使用。
- 读写、光标、选择、查找、标记和标注操作必须在有效 `use()` 回调中调用。关闭、释放或重新打开会使原作用域失效。不要在回调中用原始 `ctl.switch_document()` 等接口改变目标。
- 回调支持多个返回值（包括其中的 `nil`），异常原样传播，禁止挂起。执行完成、异常、取消和控制台重置沿用已有上下文清理，仅释放句柄，不关闭编辑器。

- `open_file(path)`, `open_ext(driver, path)` and `open_workspace(path)` actually open documents. Repeated opens of the same document within one plugin or script execution context reuse its handle; releasing or closing that handle invalidates every object sharing it. Reopening an object releases its old handle first; a failed open leaves it unopened.
- `close(force?)` actually closes the document, defaulting to `false`, and retains the handle if closing fails. `close_handle()` only releases access. Normal operations return integer error codes.
- `is_valid()` checks that the context still owns a live document handle without requiring `use()`. It returns `false` after that execution context ends.
- `document_info()` queries the handle; `activate()` activates the editor.
- The outermost `use(callback, ...)` switches the script target once, without activating its UI or restoring the previous target. The same object may nest; different objects cannot overlap within one execution context.
- Reads, writes, cursor, selection, search, bookmark and metadata operations require a live `use()` callback. Closing, releasing or reopening invalidates the old scope. Do not change the target with raw APIs such as `ctl.switch_document()` inside a callback.
- Callbacks preserve multiple results, including `nil`, and propagate the original error. Yielding is prohibited. Completion, errors, cancellation and console reset use existing context cleanup to release handles without closing editors.

## 类型与生成 / Types and generation

`hexdoc.Document` 是模块导出的对象类型。当前 Luau 对包含泛型回调的递归对象存在推断限制；严格模式下，回调中使用同名操作的不同重载时，为回调参数添加类型即可获得完整补全、重载和返回值检查。

`hexdoc.Document` is the exported object type. The current Luau solver has an inference limitation for recursive objects containing generic callbacks. In strict mode, annotate the callback parameter when using different overloads of the same operation to retain full completion, overload and return checks.

```luau
--!strict
local hexdoc = require("@s/hexdoc")
local doc = hexdoc.new()
-- Open the document before entering use().
local first, missing, last = doc:use(function(d: hexdoc.Document, value: string): (string, nil, boolean)
    d:move_to(0)
    d:move_to(0, 1, 0, true)
    return value, nil, d:select(0, 8)
end, "value")
```

维护源文件为 `src/luau/modules/hexdoc.luau`。正常 CMake 配置会通过 `tools/generate_luau_bindings.py` 生成构建目录中的 `generated/hexdoc_api.luau`，其中同时包含操作映射和导出的 `Document` 类型。构建时将这两个 Luau 文件复制到程序目录 `luaulib`；运行时和 LSP 均通过现有 `WingLuauRequire` 解析普通模块及相对依赖，不增加全局定义文件或特殊路径映射。不要手改或提交生成文件。

The maintained source is `src/luau/modules/hexdoc.luau`. Normal CMake configuration runs `tools/generate_luau_bindings.py` to produce `generated/hexdoc_api.luau` under the build directory, containing both the operation map and the exported `Document` type. Building copies these two Luau files to the application's `luaulib`. Runtime and LSP use the existing `WingLuauRequire` resolver for ordinary modules and relative dependencies, without an additional global definition file or special path map. Do not edit or commit generated files.

对象树的“文档访问”分组通过宿主生成的符号目录展示 `hexdoc`，导入形式为 `require("@s/hexdoc")`。展开模块可以查看 `new` 和导出的 `Document` 类型，类型成员及重载按需加载。该目录只用于浏览 API，不改变模块路径解析。
The object tree lists `hexdoc` under Document access with `require("@s/hexdoc")`. Expand the module to browse `new` and the exported `Document` type; members and overloads load on demand. This catalog describes the API without changing module resolution.

宿主实现分为 `ScriptDocumentContext`（当前执行的句柄、目标和宏事务）与 `ScriptDocumentRegistry`（按 Luau 执行上下文查找及清理），声明和实现分别放在 `.h` / `.cpp` 中。SDK 回调转发及私有 `ctl` 辅助函数注册由 `codegen/forwarding.json` 驱动生成，参数和结果复用 WingLuauQt 的 Qt 转换。`_call_sync` 保留原始 Lua C 回调，维持多返回值和禁止挂起的语义。这些私有辅助函数不进入公开类型定义或符号目录。

The host implementation separates `ScriptDocumentContext` (execution handles, target and macro transactions) from `ScriptDocumentRegistry` (lookup and cleanup by Luau execution context), with declarations and implementations in `.h` / `.cpp` files. `codegen/forwarding.json` drives SDK callback forwarding and private `ctl` helper registration, reusing WingLuauQt's Qt argument and result conversion. `_call_sync` remains a raw Lua C callback to preserve multiple results and prohibit yielding. These private helpers do not enter public type definitions or the symbol catalog.

## 原始接口迁移 / Raw API migration

`ctl.open_file(url)` 现在真正打开文件、设备或 `wingpro` 工作区；`ctl.close_file(handle, force)` 真正关闭。旧的“释放访问”调用应迁移到 `ctl.close_file_handle(handle)`。`ctl.open_current()` / `ctl.close_current()` 保留兼容语义，后者仅释放当前句柄并恢复默认 UI 目标。

`ctl.open_file(url)` now actually opens a file, device or `wingpro` workspace; `ctl.close_file(handle, force)` actually closes it. Migrate old access-release calls to `ctl.close_file_handle(handle)`. `ctl.open_current()` / `ctl.close_current()` keep their compatibility semantics: the latter only releases the current handle and restores the default UI target.

## 打开文档查询 / Open document queries

`reader.is_file_opened(handle)` 只判断当前脚本执行上下文的有效句柄；`reader.opened_files()` 返回该上下文已打开的编辑页，`reader.all_opened_files()` 返回编辑器中全部已打开的编辑页。这三个只读接口不要求 `use()`，也不切换当前目标。

`reader.is_file_opened(handle)` checks a live handle owned by the current script execution context. `reader.opened_files()` lists pages opened by that context, while `reader.all_opened_files()` lists all open editor pages. These read-only queries require no `use()` and do not switch the target.

列表返回 QUrl 数组，不保证顺序，不按 URL 去重。句柄只绑定原始编辑页；克隆页操作沿用原始页句柄，`all_opened_files()` 遍历页面时排除克隆页。普通文件使用 `file`，设备使用 `wdrv`；工作区（包括设备工作区）使用工作区文件自身的 `wingpro` URL，不返回底层数据文件地址。

Lists contain QUrl values without guaranteed ordering or URL deduplication. Handles bind only original editor pages; clones use their original page's handle. `all_opened_files()` excludes clones when traversing pages. Ordinary files use `file`, devices use `wdrv`, and workspaces, including device workspaces, use the workspace file's own `wingpro` URL. Workspace entries do not expose the underlying data URL.
