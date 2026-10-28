# 表格扩展回归测试 / Table extension regression tests

直接编译生产组件与异步代理，需要 CMake 3.20+、Qt 6.6.2+ Widgets、Concurrent、Test。默认 C++20，支持 C++17。
Builds the production component and async proxy directly. Requires CMake 3.20+ and Qt 6.6.2+ Widgets, Concurrent, and Test. Uses C++20 by default and supports C++17.

```sh
cmake -S tests/qtableviewext -B /tmp/wing-table-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/wing-table-tests
ctest --test-dir /tmp/wing-table-tests --output-on-failure
cmake -S tests/qtableviewext -B /tmp/wing-table-tests17 -G Ninja -DWINGHEX_FORCE_CPP17=ON
cmake --build /tmp/wing-table-tests17
ctest --test-dir /tmp/wing-table-tests17 --output-on-failure
```

覆盖搜索选项、范围、纯文本角色、快捷键、焦点及隐藏行列；排序开关、表头点击、稳定性、整数边界、过期任务、源模型更新与销毁、索引及选择；列宽来源、上下限、权重分配、首次与持续更新、隐藏列、模型切换、手动拖动及重置。
Covers search options, ranges, plain-text roles, shortcuts, focus, hidden sections; sorting controls, header clicks, stability, integer limits, stale tasks, source updates and destruction, indexes and selections; width sources, bounds, weight allocation, initial and continuous updates, hidden columns, model switching, manual dragging, and reset.

性能用例对 1,000、10,000、100,000 行记录快照、后台排序、提交耗时及 GUI 心跳。心跳测试用于发现明显阻塞，不是跨机器耗时保证；单次模型访问、委托或正则调用仍可能超过批次预算。CTest 使用 offscreen 平台，不能替代完整应用内的停靠布局、原生菜单及实际跳转/复制/删除检查。
Performance cases log snapshot, worker, publication, and GUI-heartbeat timings for 1,000, 10,000, and 100,000 rows. The heartbeat check detects substantial blocking, not a cross-machine timing guarantee. Individual model, delegate, or regex calls may exceed a batch budget. CTest uses the offscreen platform and does not replace full-application checks of docking, native menus, and actual navigation/copy/delete actions.

接口与配置示例见 [组件说明](../../docs/qtableviewext.md)。
See the [component guide](../../docs/qtableviewext.md) for APIs and examples.

额外回归覆盖搜索结果隐藏列重新显示后的等分填满，以及图标按钮对齐、错误背景和浅深配色下的固定单行高度。
Additional regressions cover equal filling after a result column is revealed, icon alignment, error backgrounds, and fixed single-row height with light and dark palettes.

启动空模型回归覆盖无行时的等分填满、缩放、隐藏列，以及首次插入数据后仍执行内容测量。
Startup regressions cover equal filling without rows, resizing, hidden columns, and retained initial content measurement when the first row arrives.

布局回归还覆盖双滚动条、RTL、树的比例与手动调整，以及浅深主题表头的实际文字中心和高 DPI 图标墨迹边界。
Layout regressions also cover both scrollbars, RTL, tree ratios and manual resizing, rendered header text centering in both themes, and high-DPI icon ink bounds.

诊断委托额外验证连续长标识符的行宽、行高和缩窄后的重新布局。
The diagnostic delegate also verifies width, height, and relayout of long unbroken identifiers when the viewport narrows.

通用搜索栏及文本适配器测试覆盖留白、防抖、快捷键、选项、富文本保留和双滚动条布局；空表快速缩小测试检查整个过程没有临时正滚动范围。
Reusable search-bar and text-adapter tests cover padding, debounce, shortcuts, options, rich-text preservation, and both scrollbars. Rapid empty-view resizing checks that no transient positive scrollbar range appears.

符号树测试覆盖折叠及惰性子节点、名称范围、双向及循环查找、大小写及全词、无效正则、模型清空及销毁、焦点保留，以及浅深主题下与其他搜索栏等高和 RTL 双滚动条布局。
Symbol-tree tests cover collapsed and lazy children, name-only scope, bidirectional and wrapped navigation, case and whole-word options, invalid patterns, model clearing and destruction, focus retention, equal footer heights in both themes, and RTL layouts with both scrollbars.
