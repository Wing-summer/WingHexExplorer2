# 工作区格式与迁移 / Workspace format and migration

## 二进制格式 / Binary format

`.wingpro` 工作区不再接受 JSON。文件开头为原始七字节 `WINGPRO`（无 NUL），随后按 BigEndian 保存 `quint16(1)`。所有 Qt 类型固定使用 `QDataStream::Qt_6_0`，不随编译时 Qt 版本变化。
`.wingpro` workspaces no longer accept JSON. Each file starts with the seven raw bytes `WINGPRO` (without a NUL), followed by a BigEndian `quint16(1)`. All Qt values use `QDataStream::Qt_6_0`, independently of the Qt version used to build the application.

| 顺序 / Order | 类型 / Type | 内容 / Content |
| --- | --- | --- |
| 1 | `QString` | 相对本地路径或完整 `wdrv:` URL / Relative local path or complete `wdrv:` URL |
| 2 | `WorkSpaceInfo` | `quint64 base`、`QByteArray checkSum`，按顺序直接写入 / Directly streamed in that order |
| 3 | `QList<BookMark>` | 每项为 `quint64 pos`、`QString comment` / Each item streams these fields in order |
| 4 | `QList<MetaData>` | 每项为 `quint64 begin`、`quint64 end`、`QColor foreground`、`QColor background`、`QString comment` / Each item streams these fields in order |
| 5 | `QVariantMap` | `plugindata`，插件 ID 到原始字节 / Plugin IDs mapped to raw bytes |

集合直接使用 Qt 的容器流操作符，包含元素数量。条目类型只定义字段的流操作符，不注册 QVariant 自定义类型，不转换为逐条 QVariantMap。颜色保留 alpha 和无效状态，运行时 `adjflag` 不落盘。
Lists use Qt's native container stream operators, including their element counts. Item types define field stream operators without custom QVariant registration or per-item QVariant maps. Colors retain alpha and invalid states; the runtime `adjflag` is not persisted.

工作区的源是实际编辑的普通文件或设备，不能是 `wingpro:` URL；`wingpro:` 是外部打开工作区的入口，不允许工作区嵌套。读取本地相对路径时，以工作区目录为基准并清理路径。
A workspace references the actual local file or device being edited. It cannot reference a `wingpro:` URL: that scheme is an external workspace-opening entry point, not a nested source. Relative local paths resolve against the workspace directory and are normalized.

## 插件数据 / Plugin data

文件末尾直接保存 `plugindata`，类型为 `QVariantMap`，键为非空插件 ID，值为原始 `QByteArray`，不进行 Base64 编码。没有插件数据时保存空 map，不设置通用扩展区，也不再嵌套一层 `plugindata` 键。
The final field is directly `plugindata`, a `QVariantMap` mapping nonempty plugin IDs to raw `QByteArray` values without Base64 encoding. An empty map represents no plugin data. There is no general extension field or nested `plugindata` key.

通过 `WorkSpaceManager::pluginData()` 读取；`setPluginData(data, id, bytes)` 更新单个插件并保留其他插件。接收整个 map 的重载替换插件集合，使现有插件数据清理操作也能持久化。编辑器复用已有 `_pluginData`，不增加另一份缓存状态。
Use `WorkSpaceManager::pluginData()` to read it. `setPluginData(data, id, bytes)` updates one plugin while retaining the others. The overload accepting a complete map replaces the plugin collection, allowing existing plugin-data cleanup to persist. The editor reuses its existing `_pluginData` without an additional cache.

普通保存、另存和补写校验值保留插件数据；切换其他文档时清除关联状态。旧 JSON 中未知字段不迁移。修改固定字段布局必须提升格式版本。
Plugin data survives saving, Save As, and checksum backfill. Associated state is cleared when switching documents. Unknown legacy JSON fields are not migrated. Fixed-layout changes require a format-version increment.

损坏结构、未知版本、截断、尾随数据、非法地址或区间、重复书签、非法插件字段和超出 QHexView 数量或注释长度限制的数据会被拒绝，不进行部分恢复。列表数量在容器分配前检查。完整读取并校验后才提交业务对象；写入使用 `QSaveFile` 原子替换。
Malformed structures, unknown versions, truncation, trailing data, invalid addresses or ranges, duplicate bookmarks, invalid plugin fields, and QHexView count or comment limits are rejected without partial recovery. List counts are checked before container allocation. Business data is committed only after complete validation; writes use atomic `QSaveFile` replacement.

## 构建迁移工具 / Building the migrator

主工程的 CMake 选项默认关闭，不会创建或安装工具目标。显式开启后可仅构建工具目标：
The main CMake option defaults to OFF, without creating or installing a tool target. Enable it explicitly to build the tool:

```sh
cmake -S . -B /tmp/winghex-build -G Ninja -DWINGHEX_BUILD_WORKSPACE_MIGRATOR=ON
cmake --build /tmp/winghex-build --target wingpro-migrate
```

也可独立构建，只需要 Qt 6.6.2+（Core、Gui、Widgets、Concurrent）、CMake 3.20+ 和 C++17 编译器，无需完整宿主的其他子模块：
Alternatively, build independently with Qt 6.6.2+ (Core, Gui, Widgets, Concurrent), CMake 3.20+, and a C++17 compiler, without the full host's other submodules:

```sh
cmake -S tools/wingpro-migrate -B /tmp/wingpro-migrate-build -G Ninja -DWINGHEX_BUILD_WORKSPACE_MIGRATOR=ON
cmake --build /tmp/wingpro-migrate-build --target wingpro-migrate
cmake --install /tmp/wingpro-migrate-build --prefix /tmp/wingpro-migrate-install
```

## 迁移用法 / Migration usage

```sh
/tmp/wingpro-migrate-build/wingpro-migrate old.wingpro converted.wingpro
/tmp/wingpro-migrate-build/wingpro-migrate old.wingpro converted.wingpro --force
```

输入始终保留；输入输出不得指向同一文件，包括符号链接和硬链接。默认拒绝覆盖输出，`--force` 允许原子替换其他目标。成功退出码为 0，解析或写入失败为 1，位置参数数量及目标冲突为 2。
The input is retained. Input and output must be different files, including symbolic and hard links. Existing outputs are rejected unless `--force` allows atomic replacement of a different target. Exit codes are 0 for success, 1 for parsing or writing failure, and 2 for positional-argument count or destination conflicts.

工具严格检查数字、颜色、Hex、Base64、条目数量和注释长度，遇到损坏会报告字段位置并停止，不跳过或截断条目。合法可选字段缺省使用默认值；忽略未知顶层 JSON 字段。旧插件数组转换为插件 map，恢复其原始字节。
The tool validates numbers, colors, Hex, Base64, item counts, and comment lengths. Invalid fields are reported and conversion stops without skipping or truncating entries. Missing optional fields receive defaults; unknown top-level JSON fields are ignored. Legacy plugin arrays become plugin maps containing their decoded bytes.

源路径先按输入目录解析，再按输出目录重新计算；源文件或设备无需在线，不重新计算校验值。旧 JSON 已丢失的颜色透明度等信息无法恢复。
Source paths resolve against the input directory and are rewritten relative to the output directory. Source files and devices need not be available, and fingerprints are not recomputed. Information already lost in legacy JSON, such as color alpha omitted by the old writer, cannot be recovered.

测试入口见 [工作区测试 / Workspace tests](../tests/workspace/README.md)。
See [Workspace tests](../tests/workspace/README.md) for validation commands.
