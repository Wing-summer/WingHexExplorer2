# 独立启动模式 / Independent startup modes

```sh
WingHexExplorer2 [files...]
WingHexExplorer2 --diff [left.bin [right.bin]]
WingHexExplorer2 --script [first.luau second.luau ...]
WingHexExplorer2 --headless task.luau
WingHexExplorer2 --headless --live
```

不指定模式进入完整编辑器，保留单实例文件转发。`--diff`、`--script`、`--headless` 互斥，各自启动独立进程，不向已有窗口转发。`--live` 仅用于 `--headless`，不接受脚本参数，通过 socket JSON-RPC 接收任务。参数错误返回 6；`--help` 和 `--version` 输出说明并返回 0。以 `-` 开头的文件名放在 `--` 后面。路径相对于当前工作目录。

Without a mode option, the full editor retains its single-instance file forwarding. `--diff`, `--script` and `--headless` are mutually exclusive and each starts an independent process. `--live` requires `--headless`, takes no script argument, and accepts tasks through socket JSON-RPC. Invalid arguments return 6. `--help` and `--version` print information and return 0. Put filenames beginning with `-` after `--`. Relative paths use the working directory.

## 文件比较 / File comparison

`--diff` 只打开比较窗口，不加载插件、脚本引擎或 LSP。零个文件打开空窗口；一个文件填左侧，两个文件依次填左右侧。通过每侧的“打开”按钮加载或替换文件，然后手动开始比较；“关闭”按钮清空该侧。打开和关闭按钮仅出现在 `--diff` 独立模式。

`--diff` opens only the comparison window, without plugins, the script engine or LSP. Supply no files for an empty window, one for the left side, or two for both sides. Each side has an Open button for loading or replacing its file and a Close button for clearing that side. These buttons appear only in standalone `--diff` mode. Start comparison manually.

新文件通过共享文档加载函数只读打开，直接使用比较窗口已有的两个视图；不创建隐藏编辑器或停靠窗口。完整编辑器中的比较窗口仍可选择已打开的文档并共享未保存修改。打开失败保留原源；关闭比较窗口不会关闭其他窗口的文档。比较沿用现有大文件读取会话和内存预算。

New files use the shared document loader in read-only mode and bind directly to the two existing comparison views. No hidden editors or dock windows are created. Comparisons inside the full editor can still share open documents, including unsaved changes. A failed open preserves the current source. Closing comparison does not close another window's documents. Existing large-file read sessions and memory budgets remain in use.

## 脚本编辑 / Script editing

`--script` 只打开脚本编辑器，接受多个文件，不自动执行。加载编辑器、LSP 和已启用插件提供的脚本服务，不创建十六进制主窗口或安装插件主窗口界面。禁用脚本时，加载插件之前弹窗报错并返回 5，不修改设置。关闭时保留运行中脚本和未保存文件的确认流程。

`--script` opens only the script editor, accepts multiple files and does not execute them automatically. It loads the editor, LSP and script services from enabled plugins, without creating the hex editor main window or installing plugin main-window UI. If scripting is disabled, it shows an error before loading plugins and returns 5 without changing settings. Closing retains confirmation for running scripts and unsaved files.

无头脚本与服务接口详见[无头模式说明](headless.md)。

See [headless execution and services](headless.md) for the windowless script runtime and service interfaces.

## 实现边界 / Implementation boundaries

环境配置始终先于解析和应用对象创建。启动配置只安装一次；Qt 命令行解析与各模式运行入口分离。脚本运行环境集中管理插件、VM 注册及清理顺序；引擎通过中止回调接收退出请求。终端分行逻辑使用公共 Qt 实现，Windows/POSIX 的读取、输出与信号处理放在 CMake 选择的独立后端，业务代码不访问原生句柄。

Environment configuration always precedes parsing and application creation. Startup configuration is installed once. Qt command-line parsing is separate from each mode's runner. The script environment manages plugin and VM registration and cleanup order; the engine receives exit requests through an interruption callback. Terminal line framing uses common Qt code. Native input, output and signals live in Windows/POSIX backends selected by CMake, without exposing native handles to business code.
