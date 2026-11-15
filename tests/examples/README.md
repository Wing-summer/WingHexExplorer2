# Luau 示例回归 / Luau example regression

使用 Luau CLI 执行生产解析模块。Python 仅构造二进制样本及临时启动器，不依赖 Python PE/ELF 库。
The Luau CLI runs the production parser modules. Python only creates binary fixtures and temporary launchers, without PE/ELF packages.

```sh
python3 tests/examples/run.py --luau /path/to/luau
# 可选：真实文件在 CLI 中回归，合成样本另在宿主 VM 中回归
# Optional: real files in the CLI, synthetic fixtures also in the host VM
python3 tests/examples/run.py --luau /path/to/luau \
  --elf /bin/ls --pe /path/to/application.exe \
  --host build/WingHexHeadlessProbe
```

默认覆盖 PE32/PE32+；ELF32/ELF64、双字节序、扩展编号、高位虚拟地址及 NOBITS；导入、延迟导入、序号导入、导出、资源、重定位、证书、调试、TLS、LoadConfig、绑定导入及 CLR；符号、重定位、动态字符串、NOTE。损坏样本覆盖截断、目录溢出、错误字节序、错误字符串索引、零长记录及资源循环。检查所有输出范围在文件内，且整个文件均有标注。
Defaults cover PE32/PE32+; ELF32/ELF64, both byte orders, extended numbering, high virtual addresses and NOBITS; imports/delayed imports/ordinals, exports, resources, relocations, certificates, debug, TLS, LoadConfig, bound imports and CLR; ELF symbols, relocations, dynamic strings and notes. Malformed cases cover truncation, directory overflow, invalid byte order/string indices, zero-length records and resource cycles. Every annotation must lie inside the file, and all file bytes must be covered.

宿主边界替身执行独立 C 数组复制脚本（禁止加载本地模块）和文件结构分析组的文档包装器，检查多选区、空选区、剪贴板文本、预检查、异常清理、撤销组结束和失败回滚；检查成功、无内容及失败的 toast 文案与图标，并确认提示发生在清理之后。`--host` 使用隔离设置在已有宿主的真实 VM/require 路径执行合成测试（避免把大二进制作为脚本文本而超过宿主 1 MiB 脚本限制）。这些检查不替代真实 GUI 中的配色、剪贴板和撤销验收。
Host-boundary fixtures execute the standalone copy script (rejecting local module imports) and the file analysis document wrapper, checking multiple/empty selections, exact clipboard text, preflight, cleanup, undo completion and failure rollback, plus toast messages/icons for success, empty selections and failures after cleanup. `--host` uses isolated settings and the real host VM/require path for synthetic tests (large binary literals would exceed the host's 1 MiB script limit). These checks do not replace native GUI acceptance of colors, clipboard and undo.

多语言回归覆盖英文、简繁中文、词典声明的 `zh-Hans` / `zh-Hant` 和港台澳标签、未知语言回退、显式语言覆盖、带动态参数的错误翻译、源位置前缀处理和未知底层错误保留。对三语言及回退语言运行复制/分析入口边界，检查 toast 图标、清理顺序、文案及 C 输出不随语言改变。
Localization regression covers English, both Chinese scripts, explicit catalog script/territory aliases, unsupported-language fallback, explicit overrides, parameterized diagnostics, source-location prefixes and preservation of unknown native errors. Copy/analysis host-boundary cases verify toast icons, cleanup ordering, localized messages and unchanged C output.

结构编辑入口同样使用实际 `@s/binary` 和 `@s/analysis`，验证 HEAD 示例文件的字段写回、metadata 标注及失败事务回滚。
The structure-edit entry also runs with the actual binary and analysis modules,
checking field writes, metadata annotations and rollback on the HEAD fixture.
