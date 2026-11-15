# 文件结构分析 / File structure analysis

本组脚本及 `lib/` 共享模块均使用 [BSD-3-Clause](../LICENSE)。
These scripts and their shared `lib/` modules use [BSD-3-Clause](../LICENSE).

在宿主中打开目标文件，再运行 `peparser.luau` 或 `elfparser.luau`。复制此组时保留整个文件夹，入口通过相对路径加载 `lib/`。成功使用调色板 toast，失败使用错误 toast；提示发生在文档访问释放和撤销组清理之后。
Open a target document in the host, then run `peparser.luau` or `elfparser.luau`. Keep the whole folder together when copying this group: entries load `lib/` through relative paths. Completion uses a palette toast; failures use an error toast after document access and undo cleanup.

语言设置见 [示例语言说明](../README.md#语言--language)：两个入口的 `language = nil` 默认跟随宿主 UI 语言，也可显式设置语言标签。`lib/i18n.luau` 使用 `@s/i18n`，只保存本组词典、显式地区别名及解析错误格式，不再自行判断简繁中文。
See [example language settings](../README.md#语言--language): `language = nil` follows the host UI language, or set an explicit locale tag. The local i18n adapter uses `@s/i18n` and contains only these tools' dictionaries, explicit aliases and parser diagnostic patterns; it does not infer Chinese variants.

## 结构着色 / Structure coloring

每个标准头字段有独立颜色及完整路径批注；节、段、目录、数据载荷、填充和文件尾附加数据覆盖整个文件。先画大区域，再画细字段。固定深色前景保证浅色背景的对比度。脚本保留已有标注，重复运行会追加标注；每次运行可整体撤销，也可保存到工作区。
Each standard header field has a color and a path comment. Sections, segments, directories, payloads, padding and overlays cover the whole file. Broad regions are painted before fine fields, with a dark foreground for contrast. Existing annotations are retained; rerunning appends annotations. Each run is one undo group, and annotations can be saved in a workspace.

- **PE**：DOS 头、stub/Rich 区域、签名、COFF 头、32/64 位可选头及全部数据目录、节头与原始数据；COFF 符号/辅助记录/字符串、节重定位和行号；导入/延迟导入描述符、名称、INT/IAT、按序号导入；导出、转发名称；递归资源目录、UTF-16 名称、数据记录与载荷；基址重定位；证书头、载荷与对齐；调试目录及载荷；TLS 目录、模板、索引和回调；AMD64 异常函数记录；按声明长度解析的 LoadConfig 字段；绑定导入；CLR 头及子目录区域。
  **PE**: DOS/stub/Rich regions, signature, COFF and optional headers, all data directories, section headers/data; COFF symbols/auxiliary records/strings, section relocations and line records; normal/delayed imports, names, INT/IAT and ordinals; exports/forwarders; recursive resources and UTF-16 names; base relocations; certificates and padding; debug entries/payloads; TLS/template/index/callbacks; AMD64 runtime function records; size-versioned LoadConfig fields; bound imports; CLR header/subdirectory regions.
- **ELF**：标识及完整文件头、程序头、节头、名称表、文件节/段；扩展数量和索引；符号及名称、REL/RELA、动态条目及字符串；NOTE、解释器、SysV/GNU hash；初始化/终止指针数组、GROUP、扩展符号节索引、GNU 版本定义/依赖/符号版本；压缩节头和压缩载荷。`SHT_NOBITS` 和段的内存尾部不占文件字节，不产生虚假范围。
  **ELF**: identification/header, program/section headers and names, file-backed sections/segments, extended counts/indices; symbols/names, REL/RELA, dynamic entries/strings; notes, interpreter, SysV/GNU hashes; init/fini arrays, GROUP, extended symbol section indices, GNU version records; compression headers/payloads. `SHT_NOBITS` and segment memory tails do not create fictitious file ranges.

机器码、DWARF、展开码、CLR 元数据表、证书内容、资源载荷以及厂商扩展作为带名称的区域着色，不对其内部语言或平台专用编码作语义解码。着色不是可执行文件有效性或安全性鉴定。
Machine code, DWARF, unwind opcodes, CLR metadata tables, certificate contents, resource payloads and vendor extensions are named colored regions; their internal languages/platform encodings are not semantically decoded. Coloring is not executable validation or a security assessment.

解析按需读取，不把整个文件载入内存。所有标注先完成边界检查，再开启撤销组；解析失败不改标注，应用失败回滚该组。检查截断、RVA 文件映射、表尺寸、字符串终止、循环资源目录等；限制为 200,000 条标注、2,000,000 次小块读取、32 层资源目录和 64 KiB 单个字符串。超限明确报错。64 位地址字段保留原始字节，地址差按字节精确计算；实际文件偏移/数量超过 Luau 精确整数范围时拒绝处理。
Parsing uses random-access reads. It validates the annotation plan before starting the undo group; parse failures leave annotations unchanged, and application failures roll back that group. Checks cover truncation, file-backed RVAs, table sizes, string termination and resource cycles. Limits are 200,000 annotations, 2,000,000 small reads, 32 resource levels and 64 KiB per string; exceeding a limit reports an error. Address fields retain their raw bytes and address differences are computed exactly; file offsets/counts outside Luau's exact integer range are rejected.

格式依据 / Format references: [Microsoft PE/COFF](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format), [System V ELF ABI](https://www.sco.com/developers/gabi/latest/contents.html).


[返回示例目录 / Back to examples](../README.md)
