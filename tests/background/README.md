# 后台脚本环境与输出面板 / Background environments and output panel

探针复用完整宿主的真实对象和依赖，使用临时配置及脚本，不加载主窗口或用户配置。先完成源码外宿主构建，再从仓库根目录运行：
The probe links real objects and dependencies from a completed out-of-source host build. It uses temporary settings and scripts without loading a main window or user settings. Run from the repository root:

```sh
python3 tests/background/run_host_probe.py --build-dir build
```

覆盖统一 16 环境上限、待启动和停止期间占位、常驻环境、同脚本独立全局与输出、单任务及全部终止、一次性完成与错误、chunk 执行、同脚本页面复用、16 页回收、旧消息隔离、运行中关闭限制及输入取消。headless RPC 的同脚本多实例、模块隔离、异步回调、公平调度及容量另见 `tests/headless/README.md`。
Coverage includes the shared 16-environment limit, pending/stopping reservations, resident environments, independent globals/output for the same script, individual/all-task cancellation, one-shot results/errors, chunk execution, page reuse, bounded page history, stale messages, close restrictions and input cancellation. See `tests/headless/README.md` for RPC instances, modules, callbacks, fairness and capacity.

默认使用 offscreen，可通过 `--platform wayland` 在原生桌面运行相同探针。设置 `WINGHEX_BACKGROUND_SCREENSHOT` 可保存面板截图；通过 `QT_SCALE_FACTOR` 验证不同缩放。自动化探针不替代原生菜单、主题及键盘交互的人工验收。
The default platform is offscreen; `--platform wayland` runs the same probe on a native desktop. Set `WINGHEX_BACKGROUND_SCREENSHOT` to save a panel capture and `QT_SCALE_FACTOR` to inspect scaling. Automated probes do not replace manual checks of native menus, themes and keyboard interaction.

可通过 `WINGHEX_BACKGROUND_TRANSLATION` 指定已生成的 Qt `.qm` 文件，以相同场景验证简繁中文。
Set `WINGHEX_BACKGROUND_TRANSLATION` to a generated Qt `.qm` catalog to run the same checks in Simplified or Traditional Chinese.
