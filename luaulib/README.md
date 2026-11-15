# Luau 标准库 / Luau standard library

使用 `require("@s/name")` 显式加载系统库；库源码随应用构建及安装部署，无需联网。
宿主能力通过 `require("@b/winghex")` 提供只读的 `env`、`StandardIcon` 和标准图标接口，并生成模块类型声明。
The read-only `@b/winghex` module exposes host data and standard-icon operations, with generated module declarations.

Load system modules explicitly with `require("@s/name")`. Builds and installs deploy
sources, generated `.d.luau` declarations and licenses without network access.

| 模块 / Module | 功能 / Purpose |
| --- | --- |
| `env` | 深度只读的宿主 UI 语言、应用和运行时 Qt 版本 / Deeply read-only UI locale, application and runtime Qt versions |
| `i18n` | 每脚本独立词典和 `tr` / Independent dictionaries and `tr` |
| `icons` | `StandardIcon` 枚举及当前主题图像 / StandardIcon IDs and current-theme images |
| `binary` | 固定布局编解码、cffi 布局适配 / Fixed layouts and cffi adaptation |
| `analysis` | 文档作用域、事务写回及标注 / Document scopes, transactional writes and annotations |
| `binaryreader` | PE/ELF 有界随机访问解析计划 / Bounded random-access parser plans |
| `bytes`, `base64`, `lz4` | 字节转换、规范 Base64、LZ4 block / Byte conversion, canonical Base64, LZ4 blocks |
| `mcp` | MIT 许可的 MCP 2026-07-28 核心及 HTTP 服务 / MIT MCP 2026-07-28 core and HTTP server |

## 部署与声明 / Deployment and declarations

默认目录为可执行程序旁的 `luaulib`。`WINGHEX_LUAULIB_DEPLOY_DIR` 可指定构建部署目录，
相对值以构建目录为基准；同一宿主运行时和 LSP 使用该目录。安装包保持平台原有布局。
Default deployment is `luaulib` beside the executable. The CMake cache option
`WINGHEX_LUAULIB_DEPLOY_DIR` overrides build deployment and the host/LSP root;
relative values are based on the build directory. Installs retain the platform layout.

`api.json` 是公开签名、类型及文档的来源；`tools/luaulib_codegen.py` 确定性生成 `.d.luau`
声明及轻量的 `*_api.luau` 类型模块。库源码通过普通 `require` 显式导出这些类型，与
`hexdoc` 的方式一致。LSP 与运行时使用相同的源码路径，不增加相邻声明文件的优先规则。
声明不作为全局类型目录加载。部署清单只删除本工具写入且内容未被修改的过期文件。
`api.json` owns public signatures, types and documentation. The generator emits
`.d.luau` declarations and lightweight `*_api.luau` type modules. Library sources
explicitly export those types through ordinary requires, following hexdoc. LSP and
runtime use the same source paths, with no sidecar precedence policy or ambient
global declarations. Deployment removes only obsolete, unmodified files it owns.

## 翻译与图标 / Translation and icons

```luau
local env = require("@s/env")
local simplified = { ["Hello %s"] = "你好 %s" }
local traditional = { ["Hello %s"] = "您好 %s" }
local tr = require("@s/i18n").create({
    zh = simplified, zh_Hant = traditional,
    zh_TW = traditional, zh_HK = traditional, zh_MO = traditional,
})
print(tr("Hello %s", env.app.name))
local icons = require("@s/icons")
toast.show(icons.pixmap(icons.Id.CopyHex), tr("Hello %s", "Wing"))
```

语言在 VM 初始化时取宿主实际 UI 语言快照；可为 `create` 传入显式语言。
语言标签统一大小写和 `_`/`-`，按完整标签逐级移除末尾子标签。例如 `sr-Latn-RS`
依次查找 `sr-Latn-RS`、`sr-Latn`、`sr`；每个词条独立回退，缺词返回原文。
地区与文字系统之间没有内置映射；需要的别名由词典显式引用同一张翻译表。
参数使用 `string.format`，无参数的文本保持原样。重复的规范化语言键及非字符串译文会报错。
图标由宿主主题服务获取，不拼接资源路径；无 GUI 时抛出明确错误，不初始化 UI。
The locale snapshots the actual host UI language at VM initialization. Pass an
explicit locale to `create` to override it. Tags ignore case and normalize underscores
to hyphens. Each message tries the full tag, then successively removes the final
subtag (`sr-Latn-RS`, `sr-Latn`, `sr`) before returning its source. There are no built-in
territory/script mappings; catalogs explicitly share dictionaries for aliases.
Formatting uses `string.format`; text without arguments stays unchanged. Duplicate
normalized locale keys and non-string translations are rejected. Icons use the host
theme service. GUI-less calls fail explicitly.

## 结构与事务 / Structures and transactions

```luau
local binary = require("@s/binary")
local analysis = require("@s/analysis")
local schema = binary.register("header", { endian = "little", size = 16, fields = {
    { name = "magic", type = "bytes", size = 4 },
    { name = "flags", type = "u32", offset = 8 },
}})
analysis.current(function(document)
    local parsed = analysis.read(document, schema, 0)
    analysis.annotate(document, parsed.fields, { offset = parsed.offset })
    -- Explicit write; the document must permit editing.
    analysis.write(document, parsed, { flags = 1 })
end)
```

偏移从零开始，数组从一开始。类型支持 `u8/i8/u16/i16/u32/i32/u64/i64/f32/f64`、
定长 `bytes/string` 和已注册的嵌套结构；字段 `count` 表示数组。union 必须显式提供
`union=true` 和 `member`。`register_cffi(name, ffi, ctype, definition)` 接收调用方的独立
`require("@b/cffi").new()` 实例，保留其声明与隔离；检查 sizeof、alignof、offsetof。
它不解析 C 声明、不推断字段类型、不解引用指针。磁盘布局应使用显式偏移与字节序。
Offsets are zero based; arrays are one based. Primitive types, fixed bytes/strings,
arrays (`count`) and registered nested layouts are supported. Unions require
`union=true` and an explicit `member`. `register_cffi(name, ffi, ctype, definition)`
uses the caller's isolated cffi instance and validates size, alignment and offsets.
It does not parse C declarations, infer field types or follow pointers. Use explicit
layouts for disk formats rather than assuming native ABI alignment.

解析结果包含 `values/fields/original/offset/size/schema`；字段范围相对于结构起点。
64 位值使用宿主整数 userdata。默认限制为 64 MiB、64 层和 200000 次字段操作。
写回仅原位覆盖：先验证全部新值，再比较原始字节，冲突时拒绝写入；未提供的字段及
padding 保留。失败通过异常报告；文档写入和标注使用撤销组，失败回滚。调用方必须在
`document:use()` 中操作，不在已有撤销组内开启独立事务。
Results contain values, relative field spans, original bytes, offset, size and schema.
64-bit values retain host integer userdata. Defaults bound parsing to 64 MiB,
64 nesting levels and 200000 field operations. Writes validate before mutation,
reject changed originals and preserve omitted fields/padding. Errors raise exceptions.
Writes/annotations use undo groups and roll back on failure. Operate inside
`document:use()` and do not start independent transactions inside an existing undo group.

`analysis.current` 只释放访问句柄，不关闭编辑器；标注按宽范围优先顺序应用。字段标注可提供
`foreground/background/bookmark`。示例见 `examples/structure-edit.luau` 与 PE/ELF 示例。
`analysis.current` releases access without closing the editor. Annotation plans apply
broad ranges first and accept foreground/background colors and optional bookmarks.
See the structure-edit and PE/ELF examples.

## MCP

`require("@s/mcp")` 为 MIT 许可的 ash 适配，源仓库及提交见 `vendor/manifest.json`。
MCP 本身和适配代码使用 MIT；其他库保留各自许可证。
The MCP adaptation and its wrapper are MIT licensed; upstream provenance is pinned
in `vendor/manifest.json`. Other modules retain their respective licenses.

```luau
local mcp = require("@s/mcp")
local server = mcp.new({ name = "binary-tools", version = "1.0" })
server:tool({ name = "echo", inputSchema = { type = "object" } }, function(args)
    return { content = {{ type = "text", text = args.text or "" }} }
end)
local transport = server:serve_http({ port = 0 })
println(transport.port)
-- server:stop() explicitly closes the listener; task teardown also releases resources.
```

`dispatch(request)` 可脱离网络测试，支持 discover、tools、resources、prompts、middleware 和 ping。
使用 MCP 2026-07-28 的无状态单 JSON 响应；无初始化握手、占位 SSE、stdin 服务或 AI UI。
JSON 由宿主已注册的 `json` 完成，脚本适配仅保留 null 和空数组/对象的表示。
HTTP 默认 `127.0.0.1`、`/mcp`、1 MiB 请求、16 KiB 头、32 个连接、15 秒读取期限。
当前只允许 loopback；Origin 仅允许显式 `allowedOrigins`，缺省拒绝携带 Origin 的请求。
要求 Content-Length；不支持 chunked 请求体。按协议校验版本和镜像头；`x-mcp-header`
只支持 properties 路径上的字符串、整数和布尔字段。默认禁用结果缓存。
`dispatch` is independently testable. The stateless HTTP transport supports the
2026-07-28 JSON-response mode, discover, tools, resources, prompts, middleware and ping.
JSON uses the registered host module; the adapter only preserves null and container
representations. Defaults: loopback, `/mcp`, 1 MiB bodies, 16 KiB headers, 32 connections
and a 15-second read deadline. Origins require an explicit allowlist. Content-Length
is required; chunked bodies are unsupported. Version, standard and annotated parameter
headers are validated. Results disable caching. No stdin or placeholder SSE is installed.

官方 TypeScript client 2.4 使用 `versionNegotiation: {mode: {pin: "2026-07-28"}}`；
其默认 legacy 模式不适用于本服务。所有功能须由脚本显式注册和启动，不自动开放编辑器工具。
For the official TypeScript 2.4 client, select the pinned modern version; its default
legacy handshake is not supported. Scripts explicitly register tools and start services;
the library never automatically exposes editor operations.

## 第三方适配 / Vendor changes

- Base64：保留上游 buffer 实现，外层验证填充、字符及规范编码 / Retains upstream buffer implementation with strict validation.
- bytes：重定向 Base64 依赖，外层拒绝奇数位十六进制 / Redirects its dependency and rejects odd-length hex.
- LZ4：修复子范围、解压长度传递、初始缓冲上限和非法零距离匹配 / Fixes ranges, propagated limits, initial allocation and zero-distance matches.
- MCP：修正导入，移除 Lune/stdio/占位 HTTP，使用注册 JSON，并适配现代协议 / Replaces runtime dependencies, imports and legacy transport with registered JSON and modern HTTP.

LZ4 仅处理 block，不处理 frame；默认解压输出上限 64 MiB，可显式调整。
LZ4 handles blocks, not frames, with a configurable 64 MiB default output limit.
