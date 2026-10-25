# 源码目录 / Source layout

保留按技术职责划分的顶层目录，在职责较多的层内再细分。新增文件先按下表归类，不将控件、脚本运行时或对话框继续放入通用服务目录。
Keep the top-level layers organized by technical responsibility, with subdirectories where a layer covers several areas. Classify new files using the table below rather than adding widgets, script runtime code, or dialogs to the service layer.

| 目录 / Directory | 职责 / Responsibility |
| --- | --- |
| `class/application` | 应用入口服务、日志、崩溃处理和更新 / Application services, logging, crash handling, and updates |
| `class/file` | 文件访问、最近文件、工作区和系统文件管理器集成 / File access, recent files, workspaces, and shell integration |
| `class/layout` | 窗口布局保存与恢复 / Window layout persistence |
| `class/plugin` | 宿主插件加载、注册和证书 / Host plugin loading, registration, and certificates |
| `class/settings` | 设置持久化、语言、主题和本地化元数据 / Settings persistence, language, themes, and localized metadata |
| `class/util` | 非控件通用辅助类 / General utilities independent of widgets |
| `control` | 通用控件、十六进制视图和导航控件 / General widgets, hex views, and navigation widgets |
| `control/common` | 图标引擎和事件过滤等控件辅助 / Widget support such as icon engines and event filters |
| `control/editor` | 代码编辑器、补全、缩进、提示、LSP 界面适配和编辑上下文 / Code editors, completion, indentation, tooltips, LSP UI adapters, and editor contexts |
| `control/console` | 脚本控制台、高亮和动画 / Script console, highlighting, and animation |
| `control/delegate` | Model-View 的显示和编辑委托 / Model-View display and editing delegates |
| `control/docking` | 停靠组件工厂和标签页扩展 / Dock component factories and tab extensions |
| `dialog` | 主窗口及业务对话框 / Main windows and feature dialogs |
| `dialog/common` | 通用文件、输入和消息对话框封装 / Common file, input, and message dialog wrappers |
| `luau` | Luau 绑定、调试器、变量检查及格式化 / Luau bindings, debugging, variable inspection, and formatting |
| `luau/runtime` | 脚本执行、调度、管理和运行配置 / Script execution, scheduling, management, and runtime settings |
| `luau/lsp` | 语言服务器、协议和语义分析 / Language server, protocol, and semantic analysis |
| `model` | Model-View 数据模型 / Model-View data models |
| `settings` | 设置页面及其 UI 文件 / Settings pages and their UI files |
| `widgetframe` | 无边框窗口辅助、标题栏和窗口按钮 / Frameless window support, title bars, and window buttons |

`class/settings` 管理设置数据，`settings` 提供设置界面；`luau/lsp` 处理语言服务，`control/editor` 将其接入编辑器。公开插件 SDK 仍位于仓库根目录的独立 `WingPlugin` 子模块，`class/plugin` 只包含宿主实现。
`class/settings` manages settings data, while `settings` provides its UI. `luau/lsp` implements language services, while `control/editor` integrates them with editors. The public plugin SDK remains in the separate root-level `WingPlugin` submodule; `class/plugin` contains host implementation only.

成对的头文件与实现、对应的 `.ui` 文件应放在一起。同目录引用使用文件名，跨目录引用使用相对 `src` 的完整路径。移动文件时同步更新根 CMake 源文件列表、资源、翻译位置和独立测试；不改变资源别名、翻译上下文、脚本 API 或类名。
Keep matching headers, implementations, and UI files together. Use filenames for includes within a directory and paths relative to `src` across directories. When moving files, update root CMake source lists, resources, translation locations, and standalone tests together; preserve resource aliases, translation contexts, script APIs, and class names.
