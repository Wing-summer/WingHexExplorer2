# 脚本示例 / Script examples

本目录的示例使用 [BSD-3-Clause](LICENSE)。先构建宿主以部署 `luaulib`；标准库通过
`require("@s/...")` 加载。脚本函数名使用 `snake_case`，公开宿主 API 保持原契约。
These examples use [BSD-3-Clause](LICENSE). Build the host first to deploy its
`luaulib`; examples load standard modules through `require("@s/...")`. Script
functions use `snake_case`, preserving existing host API contracts.

| 脚本 / Script | 用途 / Purpose |
| --- | --- |
| `copyascarray.luau` | 将所有选区按顺序复制为 C/C++ 字节数组 / Copy all selections as a C/C++ byte array |
| `file-structure/peparser.luau` | 分析并标注 PE/COFF / Analyze and annotate PE/COFF |
| `file-structure/elfparser.luau` | 分析并标注 ELF / Analyze and annotate ELF |
| `structure-edit.luau` | 固定布局解析、原位写回、标注与事务回滚 / Fixed-layout parsing, writes, annotations and rollback |

## 运行 / Running

打开目标文档，在脚本编辑器中运行相应入口。复制脚本使用 `@s/i18n`、`@s/icons` 和
`@s/analysis`，可单独复制到其他位置，不依赖示例目录中的本地模块。无内容时保留剪贴板；
提示在释放文档访问之后显示，编辑器保持打开。图标通过枚举获取，不拼接资源路径。
Open the target document and run its entry script in the script editor. The copy
tool uses the deployed i18n, icons and analysis libraries and can be copied elsewhere
without local example modules. Empty selections leave the clipboard unchanged;
notifications appear after document access is released, leaving the editor open.
Icons come from the standard enum rather than resource-path construction.

PE/ELF 示例使用 `@s/binaryreader`、`@s/analysis` 与本地的格式解析器和词典；请保留该组
目录结构，详见 [文件结构分析](file-structure/README.md)。
PE/ELF examples combine the standard binary reader and analysis libraries with local
format parsers and dictionaries. Keep that group together; see [file structure analysis](file-structure/README.md).

`structure-edit.luau` 要求可编辑的八字节文件：前四字节为 `HEAD`，后四字节为小端 u32。
它设置 flags 的最低位并标注字段，再演示失败事务自动回滚；成功写入及标注可在宿主中撤销。
The structure-edit example requires an editable eight-byte fixture: `HEAD` followed
by a little-endian u32. It sets the low flags bit, annotates fields and demonstrates
rollback of a failed transaction. Successful writes and annotations can be undone
through the host.

## 语言 / Language

入口的 `language = nil` 默认使用 `@s/env.locale`，即宿主实际 UI 语言，而非系统区域设置。
可显式设置 `"en-US"`、`"zh-CN"`、`"zh-TW"` 等标签。词典使用语言键与具名翻译表；
`@s/i18n` 统一大小写和分隔符，逐级移除标签末尾子项，每个词条独立查找，最终返回英文原文。
Entry scripts use `language = nil` to follow `@s/env.locale`, the actual host UI
language. Set a locale tag explicitly to override it. Catalogs map locale keys to
named dictionaries. The standard i18n library normalizes case/separators and removes
trailing subtags, falling back per message to the English source.

示例显式声明 `zh` 简体词典以及 `zh-Hant`、`zh-TW`、`zh-HK`、`zh-MO` 的繁体词典别名；
标准库没有针对中文或其他语言的内置推断。新增语言只需补充词典数据。
The examples explicitly declare a Simplified Chinese `zh` dictionary and Traditional
Chinese aliases for `zh-Hant`, `zh-TW`, `zh-HK` and `zh-MO`. The library contains no
Chinese-specific or other language inference. Adding languages requires catalog data.

toast、撤销组名称和已知解析错误随语言变化；PE/ELF 字段名、文件内名称、生成的 C 代码及
未知底层诊断保持原文。文件结构组的 `lib/i18n.luau` 只适配本组词典及诊断格式。
Toasts, undo titles and known parser diagnostics are translated. Format field names,
file-provided names, generated C code and unknown native diagnostics retain their
original text. The local file-structure i18n adapter owns its dictionaries and diagnostic patterns.

`headless/` 的任务及 JSON-RPC 客户端沿用现有网络和定时器接口，见 [无头模式文档](../docs/headless.md)。
The headless tasks/client use the existing network and timer interfaces; see [headless mode](../docs/headless.md).
回归方法见 [示例测试](../tests/examples/README.md)。
See [example tests](../tests/examples/README.md) for validation.
