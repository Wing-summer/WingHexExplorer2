# 国际化回归测试 / Internationalization regression tests

需要 Python 3.9+、CMake 3.20+、C++17 编译器及 Qt 6.6.2+ 的 Core、Gui、Widgets、UiTools 和 LinguistTools。语言加载器使用 Qt 私有 ZIP 接口，因此还需安装匹配版本的 Qt Core 私有开发头文件。
Requires Python 3.9+, CMake 3.20+, a C++17 compiler, and Qt 6.6.2+ with Core, Gui, Widgets, UiTools, and LinguistTools. The language loader uses Qt's private ZIP interface, so matching Qt Core private development headers are also required.

在仓库根目录运行：
Run from the repository root:

```sh
python3 tools/check_translations.py
python3 tests/i18n/run.py --build-dir /tmp/wing-i18n-tests
```

`run.py` 自动配置、构建两个探针，并通过生成的 `qt-tools.json` 查找同一 Qt 安装中的 `lrelease` 和 `rcc`。可用 `--lrelease /path/to/lrelease` 覆盖翻译编译工具。该脚本直接运行探针，不通过 CTest；测试资源在临时目录创建并自动清理，构建目录保留供复用。
`run.py` configures and builds both probes, then discovers `lrelease` and `rcc` from the same Qt installation through the generated `qt-tools.json`. Override the translation compiler with `--lrelease /path/to/lrelease`. The script runs probes directly rather than through CTest. Test resources are created in temporary directories and automatically cleaned up; the build directory remains available for reuse.

使用默认单配置生成器，探针位于构建目录根部；多配置生成器的配置子目录目前不受运行脚本支持。
Use a single-configuration generator, which places probes at the build directory root. Configuration subdirectories from multi-configuration generators are currently unsupported by the runner.

## 覆盖范围 / Coverage

- 语言选择与回退：显式设置、系统语言、不支持的语言、缺失或损坏的语言包及旧英文语言包。
  Language selection and fallback: explicit settings, system locales, unsupported languages, missing or damaged packages, and stale English packages.
- 实际插件翻译资源加载，以及本地化脚本元数据的英文回退；仅用桩替换持久化语言设置，生产加载器保持不变。
  Real plugin translation resource loading and English fallback for localized script metadata. Only the persisted language setting is stubbed; the production loader remains unchanged.
- 英文、简体中文、繁体中文在 100%、150%、200% 缩放下的表单尺寸，遍历可见按钮、标签及选项卡。
  Form sizing in English, Simplified Chinese, and Traditional Chinese at 100%, 150%, and 200% scaling, inspecting visible buttons, labels, and tabs.

动态菜单、Ribbon 和原生控件仍需完整应用中的人工检查。维护翻译时遵循 [文案与翻译规范](../../docs/i18n.md)。
Dynamic menus, Ribbon controls, and native widgets still require manual checks in the full application. Follow the [UI text and translation guide](../../docs/i18n.md) when maintaining translations.
