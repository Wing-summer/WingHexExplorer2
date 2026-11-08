# ADS 恢复基准 / ADS restoration benchmark

2026-11-06，Linux x86_64，Qt 6.12.0，GCC 16.2.1，Release / `-O3`，C++17，
`QT_QPA_PLATFORM=offscreen`。旧实现为 ADS 提交
`f581217a29074d13530524c8dccd716fac9cb220`，两者使用同一份 `benchmark.cpp`，
每组预热 3 次，再取 11 次中位数。完成全部编译和测试后顺序运行旧版、新版。
2026-11-06, Linux x86_64, Qt 6.12.0, GCC 16.2.1, Release / `-O3`, C++17,
`QT_QPA_PLATFORM=offscreen`. The old implementation is ADS revision
`f581217a29074d13530524c8dccd716fac9cb220`. Both use the same `benchmark.cpp`,
warming up three times before taking the median of 11 samples. Builds and tests
finished before running the old/new pair sequentially.

## 最终成对样本 / Final paired sample

总耗时包含显示、事件处理和延迟删除；单位 ms，字节数顺序为旧版 / 新版。
Total time includes showing, event processing and deferred deletion. Times are
milliseconds; byte sizes are old / new.

| 控件 / Widgets | 场景 / Case | 旧版 / Old ms | 新版 / New ms | 字节 / Bytes |
| --- | --- | ---: | ---: | ---: |
| 10 | 普通 / docked | 8.654 | 7.721 | 265 / 211 |
| 10 | 混合 / mixed | 9.759 | 7.690 | 265 / 211 |
| 50 | 普通 / docked | 47.081 | 34.857 | 443 / 882 |
| 50 | 混合 / mixed | 40.721 | 39.618 | 545 / 1062 |
| 200 | 普通 / docked | 394.244 | 180.855 | 1055 / 3479 |
| 200 | 混合 / mixed | 196.838 | 194.256 | 1256 / 4484 |

普通布局有改善；混合浮动/自动隐藏场景中，200 控件总体耗时接近持平。解码已经远小于
UI 恢复成本，不能把二进制解码速度作为总体加速的替代指标。由于未压缩，新数据可能
大于默认压缩的旧 XML。
Docked layouts improved; the 200-widget mixed floating/auto-hide fixture was
approximately unchanged overall. Decoding is already much smaller than UI
restoration cost, so faster decoding alone does not establish an overall speedup.
Uncompressed output can be larger than the old compressed XML.

## 波动与边界 / Variability and limits

开发期间的另外两组成对样本中，200 控件普通布局旧版中位数分别为 321.027、200.191 ms，
新版为 177.400、273.882 ms。可见当前共享运行环境存在明显波动，最终样本中的
394.244 → 180.855 ms 不能解释为稳定的倍数加速承诺。10、50 控件普通布局的改善
较一致；混合场景收益较小，应在真实桌面及应用布局上复测。
Two other development pairs measured 200-widget docked medians of 321.027 and
200.191 ms for old ADS, versus 177.400 and 273.882 ms for new ADS. This shared
runtime has substantial variability; the final 394.244 → 180.855 ms sample is
not a promise of a stable multiplicative speedup. Improvements in the 10- and
50-widget docked fixtures were more consistent. Mixed-case gains were small;
repeat measurements on the application's real layout and desktop.

新实现的 `restore_signal_span_ms` 不含预解码；旧实现该跨度包含预检和 XML 解码。
这两列不能直接比较为同一阶段；`decode_median_ms` 是独立的二进制解码测量。
The new `restore_signal_span_ms` excludes pre-decoding, while the old span includes
validation and XML decoding. These are different phases and must not be compared
as identical measurements. `decode_median_ms` independently measures binary decoding.

## 最终原始输出 / Final raw output

旧版 / Old:

```text
count=10 mixed=0 bytes=265 total_median_ms=8.654 restore_signal_span_ms=7.352
count=10 mixed=1 bytes=265 total_median_ms=9.759 restore_signal_span_ms=7.944
count=50 mixed=0 bytes=443 total_median_ms=47.081 restore_signal_span_ms=42.644
count=50 mixed=1 bytes=545 total_median_ms=40.721 restore_signal_span_ms=35.563
count=200 mixed=0 bytes=1055 total_median_ms=394.244 restore_signal_span_ms=349.747
count=200 mixed=1 bytes=1256 total_median_ms=196.838 restore_signal_span_ms=172.525
```

新版 / New:

```text
count=10 mixed=0 bytes=211 total_median_ms=7.721 restore_signal_span_ms=6.506 decode_median_ms=0.010
count=10 mixed=1 bytes=211 total_median_ms=7.690 restore_signal_span_ms=6.534 decode_median_ms=0.011
count=50 mixed=0 bytes=882 total_median_ms=34.857 restore_signal_span_ms=30.352 decode_median_ms=0.024
count=50 mixed=1 bytes=1062 total_median_ms=39.618 restore_signal_span_ms=33.913 decode_median_ms=0.027
count=200 mixed=0 bytes=3479 total_median_ms=180.855 restore_signal_span_ms=157.224 decode_median_ms=0.083
count=200 mixed=1 bytes=4484 total_median_ms=194.256 restore_signal_span_ms=163.963 decode_median_ms=0.090
```

## 验证状态 / Validation status

- C++17、C++20 独立套件均通过；补充父窗口禁用绘制及双进程第二次启动回归后，
  两种标准和 ASan/UBSan 再次通过。
  C++17 and C++20 suites passed again, along with ASan/UBSan, after adding
  ancestor-disabled painting and second-process startup regressions.
- LeakSanitizer 因运行环境的 ptrace 限制失败；仅禁用泄漏扫描后，内存地址和未定义行为
  检查通过。该结果不包含泄漏检查。
  LeakSanitizer failed because ptrace is active in this runtime. Disabling only
  leak scanning allowed address and undefined-behavior checks to pass. Leak
  detection is not covered by that result.
- `SimpleExample`、`CentralWidgetExample`、`AutoHideExample`、`ConfigFlagsExample`
  均已构建。三个受影响宿主实现文件通过针对性语法编译。
  The four named ADS examples built. The three affected host implementation
  files passed focused syntax compilation.
- 未执行完整宿主构建、Python SIP 绑定构建、Qt 6.6.2 实机验证或原生窗口管理器下的人工
  交互检查。offscreen 对部分浮动、父对象和激活操作报告平台不支持警告。
  Full-host builds, Python SIP builds, Qt 6.6.2 execution and native-window-manager
  manual checks were not performed. Offscreen reports unsupported-platform
  warnings for some floating, parenting and activation operations.
