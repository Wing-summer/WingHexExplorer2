# 主题与图标 / Themes and icons

ThemeManager 统一管理样式、调色板、标准图标及 SVG 配色。主题选择为
`System = 0`、`Dark = 1`、`Light = 2`，新配置默认跟随系统。切换立即生效，
无需重启；System 模式监听 `QStyleHints::colorSchemeChanged`，系统未提供颜色
方案时使用浅色。显式选择 Dark/Light 后不受系统变化影响。

ThemeManager owns styles, palettes, standard icons, and SVG colors. Theme choices
are `System = 0`, `Dark = 1`, and `Light = 2`; new settings follow the system.
Changes apply immediately. System mode listens to
`QStyleHints::colorSchemeChanged`, falling back to Light for an unknown scheme.
Explicit Dark/Light selections ignore system appearance changes.

设置直接使用 `skin.theme` 保存新枚举，不迁移旧编号；缺省值为 System。
`requestedTheme()` 返回所选模式；`currentTheme()` 和 `themeChanged` 使用实际
生效的 Dark/Light。插件 `currentAppTheme()` 保持原枚举及接口。

The `skin.theme` setting stores the new enum directly without legacy migration;
the default is System. `requestedTheme()` reports the selected policy;
`currentTheme()` and `themeChanged` report effective Dark/Light appearance. The
plugin `currentAppTheme()` API and enum remain unchanged.

## 标准图标与命名 / Standard icons and naming

`WingHex::StandardIcon` 使用 `Q_ENUM_NS`，数值仅可追加。ThemeManager 通过反射
取得枚举和文件名，不维护第二份目录表。文件名使用小写英文与连字符，如
`DeleteBookmark` 对应 `images/delete-bookmark.svg`、`OpenOther` 对应
`images/open-other.svg`。主题控件通过 QRC 别名放在同一命名空间的 `dark/`、
`light/` 目录，优先于通用图稿。未知 ID 返回空值。

`WingHex::StandardIcon` uses `Q_ENUM_NS`; append numeric IDs without reordering
existing values. ThemeManager reflects the enum rather than maintaining a second
catalog. Lowercase kebab-case filenames follow the enum: `DeleteBookmark` maps to
`images/delete-bookmark.svg` and `OpenOther` to `images/open-other.svg`. QRC aliases
place theme controls in the same namespace under `dark/` and `light/`, preferred
over common artwork. Unknown IDs return empty values.

```cpp
using Icon = ThemeManager::StandardIcon;
auto &theme = ThemeManager::instance();
button->setIcon(theme.standardIcon(Icon::OpenProject));
auto image = theme.standardPixmap(Icon::OpenProject);
```

`standardIcon()` 返回绘制时跟随主题的 QIcon。`standardPixmap()` 返回缓存的
200×200 位图；`standardIconData()` 返回当前 SVG 快照。缓存随实际主题切换清空。
已有位图及 QPicture 是快照，长期保存的调用方应在 `themeChanged` 后重新获取。
背景水印、状态图标、编辑器和控制台配色、调试标记及 Windows 标题栏已接入刷新。

`standardIcon()` returns a QIcon that resolves the theme when painted.
`standardPixmap()` returns a cached 200×200 bitmap; `standardIconData()` returns
an SVG snapshot. Effective theme changes invalidate caches. Stored QPixmaps and
QPictures remain snapshots: long-lived consumers must reacquire them after
`themeChanged`. Background watermarks, status icons, editor/console colors,
debugger marks, and Windows title bars subscribe to updates.

SDK 提供相同的 `standardIcon()`、`standardPixmap()`、`standardIconData()`，在
GUI 线程调用即可，不要硬编码宿主资源路径。补全图标等非标准资源使用宿主的
`resourceIcon()`，返回的图标同样跟随主题并共享 SVG 数据缓存。

The SDK offers `standardIcon()`, `standardPixmap()`, and `standardIconData()` on
the GUI thread, avoiding hardcoded host resource paths. Host resources outside
the standard catalog, such as completion icons, use `resourceIcon()`; returned
icons follow the theme and share the SVG data cache.

## 图稿与配色 / Artwork and colors

保留用户调整的路径和组合关系。颜色用 `data-theme-fill`、`data-theme-stroke`
标注角色：`foreground`、`muted`、`accent`、`gold`、`success`、`danger`、`purple`。
`auto` 保留原色相，只在与主题背景对比不足时调整亮度。白色内部留白和多色层次
按原图保留，应用标志、Qt 等品牌图稿不自动改色。资源路径本身返回原始图稿，
需要主题颜色时使用管理器接口。

Preserve user-edited paths and composition. Annotate `data-theme-fill` and
`data-theme-stroke` with `foreground`, `muted`, `accent`, `gold`, `success`,
`danger`, or `purple`. The `auto` role preserves hue and adjusts lightness only
when background contrast is insufficient. Retain internal white space and
multicolor layers. Application and Qt brand artwork keep their colors. Raw
resource paths contain original artwork; use manager APIs for themed colors.

## 验证 / Validation

独立测试见 [主题测试](../tests/theme/README.md)，覆盖枚举资源、状态、尺寸、DPI、
系统主题通知、缓存失效、SDK 调用及关键透明区域。测试和导出的 Qt 对照图不能
替代完整宿主、动态菜单、Ribbon 和平台原生控件的运行检查。

See [theme tests](../tests/theme/README.md) for enum resources, icon modes, sizes,
DPI, system notifications, cache invalidation, SDK calls, and transparent regions.
Standalone tests and exported Qt previews do not replace full-host checks of
dynamic menus, Ribbon controls, and platform-native widgets.
