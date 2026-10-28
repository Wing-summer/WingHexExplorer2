# 表格扩展 / Table view extensions

`QTableViewExt` 保持 `QTableView` 接口；搜索和异步排序默认关闭。仅支持平面表格模型。
`QTableViewExt` retains the `QTableView` interface. Search and asynchronous sorting are disabled by default. Only flat table models are supported.

## 通用搜索栏 / Reusable search bar

`SearchBar` 独立管理输入、选项、防抖、统一图标和错误状态；宿主连接 `searchRequested(bool backwards)` 完成查找，使用 `setState()` 反馈结果，并通过 `handleKeyEvent()` 转发宿主内的快捷键。`optionsMenu()` 可添加宿主特有选项，例如表格的列范围。外边距为左右 8、上下 6 个逻辑像素，输入框另有文字留白。
`SearchBar` owns input, options, debounce, consistent icons, and error states. Hosts handle `searchRequested(bool backwards)`, report results with `setState()`, and forward local shortcuts through `handleKeyEvent()`. `optionsMenu()` accepts host-specific options such as table columns. Outer margins are 8 horizontal and 6 vertical logical pixels, with additional text padding inside the input.

文本视图可直接使用 `SearchableTextBrowser` 并调用 `setSearchEnabled(true)`。解码字符串已接入此组件；搜索文档中的纯文本，保留原有格式，支持前后查找及相同选项。搜索栏随视口和滚动条重新布局，关闭后释放预留空间。
Text views can use `SearchableTextBrowser` with `setSearchEnabled(true)`. Decoded text uses this adapter to search document text while preserving formatting, with navigation and the same options. The footer follows viewport and scrollbar geometry and releases its reserved space when closed.

## 符号树 / Symbol tree

`ObjTreeWidget` 复用 `SearchableTreeWidget`，使用同一搜索栏及快捷键，仅匹配符号名称，跳过分类标题和声明列。搜索按当前排序深度优先遍历，分批读取尚未展开的子符号；命中后展开父节点并滚动定位，不过滤项目。条件、模型或内容变化会取消旧扫描；树与搜索栏之间切换焦点保留选择。
`ObjTreeWidget` reuses `SearchableTreeWidget` with the same search bar and shortcuts. Only symbol names match; category titles and declaration text are excluded. Search follows the current sorted depth-first order and loads collapsed children in batches. Matches reveal their ancestors and scroll into view without filtering items. Query, model, or content changes cancel pending scans; moving focus between the tree and search bar retains selection.

`SearchBar::Query` 保存文本、选项和编译后的正则，三个适配器共用其匹配语义。表格中搜索配置、扫描进度、搜索控件及排序状态分别归组；符号缓存和刷新恢复状态独立保存，避免无关生命周期混杂。
`SearchBar::Query` holds text, options, and a compiled pattern shared by the three adapters. Table search configuration, scan progress, search controls, and sorting state have separate groups. Symbol caches and refresh restoration state are kept separately so their lifetimes remain clear.

## 搜索与排序 / Search and sorting

```cpp
view->setSearchEnabled(true);
view->setSearchableColumns({0, 2});
view->setColumnSearchRole(2, PlainTextRole);
view->setAsyncSortingEnabled(true);
view->setColumnSortable(2, false);
view->setColumnSortRole(0, NumericValueRole);
view->setModel(source);
```

未配置搜索列时允许所有列；显式空列表表示无搜索列，`resetSearchableColumns()` 恢复全部列。用户可以在允许范围内选择。搜索使用显示角色，可为富文本配置纯文本角色；不会修改模型内容。
All columns are searchable until configured. An explicit empty list allows none; `resetSearchableColumns()` restores all columns. Users can select within the allowed range. Search uses the display role unless a column-specific plain-text role is configured; it does not modify model data.

Ctrl+F 打开搜索栏，Enter/F3 向后查找，Shift+Enter/Shift+F3 向前查找，Esc 关闭。按当前显示行列顺序定位，跳过隐藏行列。支持大小写、全词、正则和循环查找。
Ctrl+F opens search, Enter/F3 finds the next match, Shift+Enter/Shift+F3 finds the previous match, and Esc closes search. Navigation follows displayed row and column order, skipping hidden sections. Options include case sensitivity, whole words, regular expressions, and wrapping.

搜索栏保持单行，图标按钮采用统一尺寸和垂直居中。未匹配或正则错误时输入框显示红色底色及警告图标，详细原因保留在工具提示和无障碍描述中。
The search bar stays on one row with equally sized, vertically centered icon buttons. Missing matches and invalid patterns use a red input background and warning icon; tooltips and accessibility descriptions retain the full explanation.

启用异步排序后，各列默认可排序，使用 `setColumnSortable()` 逐列关闭。用组件的 `sortByColumn()` 或点击表头请求排序，不需要调用 Qt 的 `setSortingEnabled()`。禁用当前排序列会恢复源行序。相同键保持源行序，整数按数值排序；格式化偏移应提供原始数值角色。
After asynchronous sorting is enabled, all columns are sortable unless disabled with `setColumnSortable()`. Request sorting through the component's `sortByColumn()` or header clicks; do not additionally enable Qt's `setSortingEnabled()`. Disabling the active column restores source order. Equal keys retain source order; integers sort numerically, so formatted offsets should provide a raw numeric role.

`setModel()` 始终接收源模型；启用排序时 `model()` 返回显示代理。Qt 索引信号及选择模型携带显示索引，访问源数据前必须使用 `mapToSource()`；反向使用 `mapFromSource()`，源模型由 `sourceModel()` 获取。代理不改变源数据顺序，支持编辑转发。
`setModel()` always accepts the source model; `model()` returns the display proxy while sorting is enabled. Qt index signals and selections use display indexes. Use `mapToSource()` before accessing source records, `mapFromSource()` for the reverse direction, and `sourceModel()` to access the source model. Sorting does not reorder source data; edits are forwarded.

排序在 GUI 线程分批读取快照，后台仅比较快照；过期结果丢弃。搜索与内容宽度测量也分批执行。约 4ms 是批次预算，不能中断单次昂贵的模型、委托或正则调用；模型应正确通知数据及结构变化。
Sorting collects keys in GUI-thread batches and compares only snapshots in the worker; stale results are discarded. Search and content measurement also use batches. The approximately 4ms budget cannot interrupt an individual expensive model, delegate, or regex call. Models must correctly notify data and structural changes.

## 列宽策略 / Column width policies

```cpp
QTableViewExt::ColumnWidthPolicy content;
content.source = QTableViewExt::WidthSource::Contents;
content.maximumPixels = 360;
content.maximumRatio = 0.3;
view->setColumnWidthPolicy(0, content);

QTableViewExt::ColumnWidthPolicy weighted;
weighted.source = QTableViewExt::WidthSource::Weight;
weighted.update = QTableViewExt::WidthUpdate::Initial;
weighted.weight = 1;
view->setColumnWidthPolicy(1, weighted);
view->setColumnWidthPolicy(2, weighted);
```

尺寸来源为 `Specified`、`Contents` 或 `Weight`。指定尺寸通过 `unit` 和 `value` 表示逻辑像素或视口比例；权重由 `weight` 指定，必须为有限正值。比例范围为 0–1。无效配置返回 `false`，保留原策略。
The width source is `Specified`, `Contents`, or `Weight`. Specified widths use `unit` and `value` for logical pixels or a viewport ratio. `weight` must be finite and positive. Ratios range from 0 to 1. Invalid configuration returns `false` without changing the policy.

所有来源都支持可选的 `minimumPixels`、`maximumPixels`、`minimumRatio`、`maximumRatio`。取最强下限和最强上限；冲突时下限优先。表头最小值同样有效。权重列共享其他可见列占用后的剩余空间；达到上下限后重新分配，空间不足允许水平滚动，上限全部达到后允许留白。
All sources support optional `minimumPixels`, `maximumPixels`, `minimumRatio`, and `maximumRatio`. The strongest minimum and maximum apply; the minimum wins conflicts. The header minimum also applies. Weighted columns share the space left after other visible columns, redistributing around constraints. Insufficient space permits horizontal scrolling; capped columns can leave unused space.

`Initial` 在每个源模型首次可见且有数据时执行，隐藏列延迟执行；模型重置和切换回来不重复执行。`Continuous` 随布局变化更新，内容策略在数据、字体、样式或委托尺寸变化后重新测量。内容策略最多均匀采样 1,000 行，并包含表头和委托尺寸提示。
`Initial` runs once per source model when visible and populated; hidden columns wait until shown. Model resets and switching back do not repeat it. `Continuous` follows layout changes; content policies remeasure after data, font, style, or delegate size changes. Content measurement includes the header and delegate size hints for at most 1,000 evenly sampled rows.

手动拖动暂停该列自动计算。`resetColumnWidths(column)` 恢复策略，默认参数重置全部配置列。状态按源模型保留，不跨程序启动保存。`columnWidthPolicy()` 返回可选配置，`clearColumnWidthPolicy()` 移除策略。配置权重列或最后一列时关闭表头的全局 `stretchLastSection`，由策略负责分配空间；其他未配置列的独立 resize mode 保持。
Manual dragging suspends automatic calculation for that column. `resetColumnWidths(column)` restores its policy; the default argument resets all configured columns. State is retained per source model without persistence across launches. `columnWidthPolicy()` returns an optional policy; `clearColumnWidthPolicy()` removes it. Configuring weights or the last column disables the header's global `stretchLastSection` so the policies can allocate space; other unconfigured columns retain their individual resize modes.

相同权重加 `Initial` 表示首次等宽，之后可独立拖动；改为 `Continuous` 则持续分配剩余空间，直到用户手动覆盖。未配置列宽策略的组件保留 Qt 的原有列宽行为。
Equal weights with `Initial` provide equal initial widths followed by independent resizing. `Continuous` keeps allocating remaining space until manually overridden. Components without width policies retain Qt's existing width behavior.

搜索结果的最后两列使用持续等权重策略，在显示/隐藏解码列及调整停靠面板大小后重新平分剩余空间；手动拖动仍优先。
The last two search-result columns use continuous equal weights, sharing the remaining width when the decoded column is shown or hidden or the dock is resized. Manual overrides still take precedence.

启动空表在 Qt 计算滚动范围前同步执行持续权重/比例布局，避免快速缩小时短暂出现滚动条；内容列先使用受约束的表头宽度估计值，不标记首次内容测量完成，有数据后仍测量一次。
Empty tables settle continuous weight and ratio layout before Qt calculates scrollbar ranges, preventing transient scrollbars during rapid resizing. Content columns use bounded header-width estimates without consuming their initial content measurement; they still measure once data arrives.

## 默认布局 / Default layouts

搜索栏在视口底部预留区域内布局，避开两个滚动条及行表头，支持从右到左布局。图标采用统一网格和线宽；浅深主题的表头使用对称留白，避免排序箭头使文字偏移。
The search bar uses reserved space below the viewport, avoiding both scrollbars and the row header in either layout direction. Icons share one grid and stroke weight; both themes use symmetric header gutters to keep labels centered with sort arrows.

| 视图 / View | 默认空间分配 / Default allocation |
| --- | --- |
| 搜索结果 / Search results | 行、列、偏移内容宽度上限分别为视口 12%、10%、22%；余量由内容列等分 / Line, column, and offset content widths capped at 12%, 10%, and 22%; content columns share the remainder |
| 书签 / Bookmarks | 偏移 25%、注释 75% / Offset 25%, comment 75% |
| 标注 / Annotations | 起止位置各 20%、颜色各 10%、注释 40% / Begin and end 20% each, colors 10% each, comment 40% |
| 脚本变量、监视及符号树 / Script variable, watch, and symbol trees | 名称 40%、值或声明 60% / Name 40%, value or declaration 60% |
| 脚本调用栈 / Script stack | 行号按内容、上限 15%；文件与声明按 40:60 分配余量 / Line capped at 15%; file and declaration share the rest 40:60 |

比例受基于字体的最小可读宽度约束，空间不足时允许水平滚动。树视图复用列宽控制器并保留手动调整，符号刷新不再强制覆盖名称列。诊断列表使用纯文本换行委托，随视口宽度更新行高；长连续标识符也会折行，不依赖 Qt 默认单词换行。
Ratios respect font-based readable minimums and permit horizontal scrolling when necessary. Tree views reuse the width controller and preserve manual changes; symbol refresh no longer overrides the name column. Diagnostics use a plain-text wrapping delegate with viewport-based row heights; long unbroken identifiers wrap as well.
