# Filesystem / 文件系统

`filesystem` is a global module. Its `file`, `fileinfo` and `dir` constructors expose SDK-scanned QFile, QFileInfo and QDir APIs in snake_case. Qt aliases reuse the same registered types. Module selection, host hooks and exclusions live in `codegen/bindings.json`; registration, definitions and symbol pages are generated under the build directory.

`filesystem` 是全局模块，`file`、`fileinfo`、`dir` 构造器分别公开 SDK 扫描的 QFile、QFileInfo、QDir 接口，方法为 snake_case。Qt 别名复用同一类型注册。模块选择、宿主适配及排除项放在 `codegen/bindings.json`，注册、定义及符号页随 build 生成。

The host's `sdk_bindings.type_annotations` describes its native 64-bit integer representation. File sizes and write counts return Luau `integer`, preserving all 64 bits; they are distinct from ordinary `number` values. `tostring()` preserves their exact decimal representation.

宿主通过 `sdk_bindings.type_annotations` 声明原生 64 位整数表示。文件大小及写入计数返回 Luau `integer`，保留全部 64 位，与普通 `number` 不同；`tostring()` 保留精确的十进制表示。

```luau
local fs = filesystem
local info = fs.fileinfo("example.bin")
if info:exists() then
    local input = fs.file(info:file_path())
    if input:open(fs.file.OpenModeFlag.ReadOnly) then
        local bytes = input:read_all()
        print(#bytes)
        input:close()
    end
end
```

`scriptFileEnableOverwrite` controls writing, appending, truncating or resizing existing files. New files remain allowed; write opens use `NewOnly` when overwrite is disabled to prevent replacement races and symlink overwrites. Script-created QFile instances use a host-native virtual write sink so Qt stream/device consumers also respect the current setting. Read-only opens remain available.

`scriptFileEnableOverwrite` 控制已有文件的写入、追加、截断及改尺寸。仍允许创建新文件；关闭覆盖时，写打开使用 `NewOnly`，防止竞态及符号链接覆盖。脚本创建的 QFile 使用宿主原生虚拟写入检查，因此 Qt 流及设备使用者也遵守当前设置。只读打开保持可用。

`scriptFileSystemWrite` controls removal, recursive removal, renaming/moving, trash operations and filesystem metadata changes. Process-wide directory/search-path changes require this setting too. Creating directories and copying/linking to new destinations remain allowed, matching the previous host boundary. Raw descriptor opens, descriptor extraction, memory mapping and unsafe pointer/length write overloads are excluded.

`scriptFileSystemWrite` 控制删除、递归删除、重命名/移动、移入回收站及文件系统元数据修改。进程级当前目录及搜索路径修改也要求该设置。创建目录、复制或链接到新目标仍允许，沿用原宿主边界。排除原始描述符打开、提取描述符、内存映射及不安全的指针加长度写入重载。

File reads return binary Luau strings. Writes accept binary strings or `bytearray`, preserving embedded NUL bytes. C-string convenience write overloads are excluded to avoid truncation and ambiguous overload selection; convert `ustring` to UTF-8 explicitly before writing text.

文件读取返回二进制 Luau 字符串；写入接受二进制字符串或 `bytearray`，保留内嵌 NUL。排除 C 字符串便捷写入重载，避免截断及重载歧义；写入文本前应显式将 `ustring` 转换为 UTF-8。

The implementation is host-owned in `src/luau/runtime/scriptfilesystem.h/.cpp`, including its settings callbacks. WingLuauQt supplies only generic scanned-module and class-adapter generation; its filesystem overlay is an example, not a shipped host policy. Denied script operations raise errors; indirect Qt writes report failure through their native return/status APIs.

实现及设置回调归宿主所有，位于 `src/luau/runtime/scriptfilesystem.h/.cpp`。WingLuauQt 仅提供通用扫描模块及类适配生成能力，文件系统 overlay 只是示例，不内置宿主策略。拒绝的脚本操作抛出错误；间接 Qt 写入通过原生返回值或状态报告失败。

Permission classification is configured in `sdk_bindings.guard_selectors`. The generator resolves it once per scanned method and emits `ScriptFileSystem::Access` constants; native checks use an enum switch. Callback storage uses `Q_GLOBAL_STATIC`.

权限分类配置位于 `sdk_bindings.guard_selectors`，generator 对扫描到的每个方法解析一次并生成 `ScriptFileSystem::Access` 常量；原生检查使用枚举分支，回调存储使用 `Q_GLOBAL_STATIC`。
