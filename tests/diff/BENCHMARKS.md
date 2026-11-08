# 本次实现基准 / Implementation benchmark

Release 独立引擎，Linux x86-64、Qt 6.12、GCC 16，默认 64 MiB 工作预算、100,000 条详细差异。使用实际写入的文件，内容为重复的 1 MiB 伪随机块。计时不含生成时间，未隔离文件系统缓存及其他开发任务；不能代表所有文件或比较窗口的完整 RSS。
Standalone Release engine on Linux x86-64, Qt 6.12 and GCC 16, with the default 64 MiB budget and 100,000 detailed differences. Inputs are physically written files with repeated 1 MiB pseudorandom blocks. Timings exclude generation; filesystem caches and concurrent development work were not isolated. These results do not represent every file or the full comparison window's RSS.

| 每侧规模 / Size | 场景 / Workload | 耗时 / Time | 差异区间 / Intervals | 粗略 / Coarse | 进程峰值 RSS / Peak RSS |
| --- | --- | ---: | ---: | ---: | ---: |
| 1 GiB | 相同 / Identical | 0.32 s | 0 | 0 | 18.7 MiB |
| 1 GiB | 稀疏修改 / Sparse edits | 0.35 s | 16 | 0 | 20.8 MiB |
| 1 GiB | 插入 / Insert | 0.36 s | 1 | 0 | 23.6 MiB |
| 1 GiB | 删除 / Delete | 0.28 s | 1 | 0 | 24.6 MiB |
| 1 GiB | 密集修改 / Dense edits | 2.18 s | 100,508 | 507 | 27.2 MiB |
| 1 GiB | 全不同 / Unrelated | 0.99 s | 1 | 1 | 27.0 MiB |
| 16 GiB | 相同 / Identical | 8.98 s | 0 | 0 | 18.6 MiB |
| 16 GiB | 稀疏修改 / Sparse edits | 9.30 s | 256 | 0 | 20.7 MiB |
| 16 GiB | 插入 / Insert | 10.74 s | 1 | 0 | 25.7 MiB |
| 16 GiB | 删除 / Delete | 11.51 s | 1 | 0 | 25.9 MiB |
| 16 GiB | 密集修改 / Dense edits | 25.15 s | 108,187 | 8,186 | 27.5 MiB |
| 16 GiB | 全不同 / Unrelated | 22.16 s | 1 | 1 | 26.2 MiB |

16 GiB 全不同数据的全局搜索增加预算后，耗时从 65.77 s 降至 22.16 s，首批结果从 65.77 s 提前至 8.39 s。该场景以粗略替换表示未细化内容；更早降级会牺牲细节，不保证全局最短编辑脚本。快速／精细模式分别在新数据扫描 64／256 MiB 仍无全局锚点时转入汇总；汇总连续未命中后降低重同步频率。
Adding a global search budget reduced the 16 GiB unrelated workload from 65.77 s to 22.16 s, with the first result moving from 65.77 s to 8.39 s. This workload uses coarse replacements. Earlier fallback sacrifices detail and does not guarantee a globally minimal script. Fast/precise modes switch to summary after scanning 64/256 MiB of new data without a global anchor; repeated misses reduce resynchronization frequency.

插入场景为新内容预留空间，最大一侧恰好 16 GiB。删除场景反向复用该数据对。1 GiB 全不同用例优化后约 0.99 s，首批结果约 0.33 s。
The insert workload reserves room so the larger input is exactly 16 GiB. The delete workload reuses the pair in reverse. The refined 1 GiB unrelated workload took about 0.99 s, with a first result at about 0.33 s.

运行方法见 [README](README.md)。本次还独立检查了 C++17/20、共享文档与线程读取、真实宿主离屏 UI、主题和翻译；这些检查不替代原生无边框窗口的桌面交互检查。
See [README](README.md) for reproduction. Separate checks cover C++17/20, shared documents and threaded reads, real-host offscreen UI, themes and translations. They do not replace native frameless-window desktop interaction checks.

## 渲染与列表重构回归 / Rendering and list refactor regression (2026-11-08)

保持比较算法与预算，Release 独立引擎重新检查 1 GiB 代表场景。耗时不含生成，文件系统缓存未隔离。
The comparison algorithm and budgets are unchanged. Representative 1 GiB cases were rerun with the standalone Release engine. Generation is excluded and filesystem caches were not isolated.

| 场景 / Workload | 耗时 / Time | 读取 / Read | Seek | 差异／粗略 / Intervals/coarse | 峰值 RSS / Peak RSS |
| --- | ---: | ---: | ---: | ---: | ---: |
| 相同 / Identical | 0.287 s | 2,147,483,648 B | 1,024 | 0 / 0 | 20,100 KiB |
| 密集修改 / Dense | 2.081 s | 1,616,622,464 B | 771 | 100,508 / 507 | 27,700 KiB |
| 全不同 / Unrelated | 0.812 s | 3,290,431,488 B | 1,569 | 1 / 1 | 27,828 KiB |

真实宿主离屏探针使用连续列表，预先分配结果存储后测量附加控件的进程 RSS 增量。100 万行约增加 2.2 MiB，显示、末行定位及追加约 9–10 ms；后台扫描偏移文本至末行约 0.39 s。2 倍缩放下该增量约 7.9 MiB。RSS 增量受分配器和绘制缓存影响，并非精确的 QObject 大小统计；预览仍仅按可见行读取。探针同时验证选中行在追加后保留。
The real-host offscreen probe measures process RSS growth after result storage has already been allocated. A continuous million-row table adds about 2.2 MiB, with show/end-row navigation/append taking about 9–10 ms; a background offset-text search to the last row takes about 0.39 s. At 2x scale the growth is about 7.9 MiB. Allocators and painting caches affect RSS deltas, which are not exact QObject accounting. Previews still read only visible rows. Selection preservation across append is also checked.

Diff 的 C++17/20 测试，以及 QHexView、设置、主题、翻译与真实宿主离屏检查通过。宿主新增组件的 C++17 语法检查通过；MainWindow 的 C++17 检查被现有 `src/luau/lsp/lsp.h` 中 C++20 默认比较运算符阻挡。本轮未重测 16 GiB，也未进行原生桌面人工交互检查。
Diff tests passed in C++17/20; QHexView, settings, theme, translation and real-host offscreen checks also passed. New host components passed C++17 syntax checks; MainWindow's C++17 check is blocked by existing C++20 defaulted comparison operators in `src/luau/lsp/lsp.h`. This refactor did not rerun the 16 GiB matrix or native desktop manual interaction checks.

## 32／64 GiB 容量实测 / Capacity benchmark (2026-11-08)

Linux x86-64、Btrfs/NVMe、Qt 6.12、GCC 16.2.1，Release C++20 独立引擎，单工作线程、快速模式、64 MiB 工作预算、100,000 详细区间设置。CLI 将单侧上限显式设为 64 GiB，分别测试应用默认允许规模 32 GiB 和最大允许规模 64 GiB。
Standalone Release C++20 engine on Linux x86-64, Btrfs/NVMe, Qt 6.12 and GCC 16.2.1, using one worker, fast mode, a 64 MiB working budget and a configured detail limit of 100,000. The CLI explicitly raises the per-side limit to 64 GiB and tests both the application's default allowed size (32 GiB) and maximum allowed size (64 GiB).

每项重新实际写入两个非稀疏文件，输入由种子 131 生成的 1 MiB 伪随机块重复构成；稀疏修改每 64 MiB 改一个字节，插入／删除长度为 128 KiB。插入为新数据预留空间，较大一侧恰好达到表中规模。计时只包含比较，不含生成和清理。每项运行一次，未清空或隔离系统缓存，不能将本表归类为严格冷缓存或热缓存测试。
Each case writes two real, non-sparse files using repeated 1 MiB pseudorandom blocks from seed 131. Sparse cases change one byte every 64 MiB; insert/delete cases use 128 KiB. Insert cases reserve space so the larger side exactly reaches the stated size. Timing includes comparison only, excluding generation and cleanup. Each case ran once; caches were neither cleared nor isolated, so these are not controlled cold- or warm-cache measurements.

| 规模 / Size | 场景 / Workload | 耗时 / Time | 首批 / First result | 读取 / Read | 区间（粗略） / Intervals (coarse) | 结果分配 / Result allocation | 峰值 RSS / Peak RSS |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 32 GiB | 相同 / Identical | 15.357 s | 0.009 s | 64.000 GiB | 0 (0) | 0.041 MiB | 19.57 MiB |
| 32 GiB | 稀疏 / Sparse | 15.047 s | 0.006 s | 64.000 GiB | 512 (0) | 0.082 MiB | 20.61 MiB |
| 32 GiB | 插入 / Insert | 20.106 s | 0.009 s | 80.003 GiB | 1 (0) | 0.041 MiB | 26.24 MiB |
| 32 GiB | 删除 / Delete | 20.223 s | 0.007 s | 80.000 GiB | 1 (0) | 0.041 MiB | 26.12 MiB |
| 32 GiB | 密集 / Dense | 45.114 s | 0.007 s | 48.004 GiB | 116,379 (16,378) | 9.360 MiB | 27.85 MiB |
| 32 GiB | 全不同 / Unrelated | 29.206 s | 9.066 s | 96.064 GiB | 1 (1) | 0.041 MiB | 26.26 MiB |
| 64 GiB | 相同 / Identical | 31.434 s | 0.008 s | 128.000 GiB | 0 (0) | 0.041 MiB | 19.59 MiB |
| 64 GiB | 稀疏 / Sparse | 31.889 s | 0.016 s | 128.000 GiB | 1,024 (0) | 0.123 MiB | 20.66 MiB |
| 64 GiB | 插入 / Insert | 39.299 s | 0.009 s | 160.003 GiB | 1 (0) | 0.041 MiB | 26.03 MiB |
| 64 GiB | 删除 / Delete | 40.376 s | 0.007 s | 160.000 GiB | 1 (0) | 0.041 MiB | 26.54 MiB |
| 64 GiB | 密集 / Dense | 77.271 s | 0.008 s | 127.996 GiB | 121,667 (21,667) | 9.771 MiB | 28.22 MiB |
| 64 GiB | 全不同 / Unrelated | 60.974 s | 18.193 s | 192.064 GiB | 1 (1) | 0.041 MiB | 26.53 MiB |

所有场景完成且错误枚举为 None。相同、稀疏、插入和删除均无粗略区间；密集差异和全不同场景按当前预算降级。粗略 Replace 可能包含相等字节，区间数不代表准确编辑次数。当前详细上限边界可能多产生 1 条详细区间：32 GiB 实测 100,001，64 GiB 实测 100,000。本轮据实记录，未调整算法。
All cases completed with error None. Identical, sparse, insert and delete cases produced no coarse intervals; dense and unrelated cases degraded under the current budgets. Coarse Replace ranges may include equal bytes, so interval counts are not exact edit counts. The current detail-limit boundary may produce one extra detailed interval: 100,001 at 32 GiB and 100,000 at 64 GiB. This is recorded without changing the algorithm.

RSS 包含 Qt、分配器及输入生成的进程内存，不包含内核文件缓存，也不代表完整 GUI 的内存。结果分配列仅计分段结果存储。原始 JSON 同时保留实际左右长度、读取字节、寻址次数、首批时间和状态码：[capacity-32-64.json](capacity-32-64.json)。
RSS includes Qt, allocator overhead and in-process input generation, excludes the kernel file cache, and does not represent the full GUI. Result allocation counts only compact result storage. [Raw JSON](capacity-32-64.json) also retains actual input lengths, bytes read, seek counts, first-result times and status codes.

测试二进制摘要 / Measured executable SHA-256: `1e72cdafec99155728af7bf0954256c01d286afdd756ac82ad69da62cc2711f8`
