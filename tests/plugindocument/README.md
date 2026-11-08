# 插件文档测试 / Plugin document tests

独立测试构建真实 SDK，覆盖 URL、三种打开方式、文档属性、关闭与释放句柄的区别、移动、外部关闭、插件销毁、线程限制及 `use()` 作用域。作用域测试验证只切换一次、嵌套、不同文档互斥、守卫移动、生命周期失效和作用域内后台只读。
The standalone suite builds the real SDK and covers URLs, the three opening methods, properties, close versus handle release, moves, external closure, plugin destruction, thread affinity and `use()` scopes. Scope tests cover a single switch, nesting, conflicting documents, guard moves, lifetime invalidation and worker reads under an active scope.

```sh
cmake -S tests/plugindocument -B build-plugin-document -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-plugin-document
ctest --test-dir build-plugin-document --output-on-failure
```

C++17 回退 / C++17 fallback:

```sh
cmake -S tests/plugindocument -B build-plugin-document17 -G Ninja -DWINGHEX_FORCE_CPP17=ON
cmake --build build-plugin-document17
ctest --test-dir build-plugin-document17 --output-on-failure
```

实际宿主探针复用当前源码的完整 Ninja 构建及其链接参数，用测试入口替换应用入口，保留真实 MainWindow、PluginSystem、EditorView 和资源。隔离配置和缓存到构建目录，不更改用户设置。要求构建时使用 `CMAKE_EXPORT_COMPILE_COMMANDS=ON`。
The real-host probe reuses a complete Ninja build of the current sources and its link arguments, replacing only the application entry point while retaining the real MainWindow, PluginSystem, EditorView and resources. It isolates settings and caches under the build directory. Configure with `CMAKE_EXPORT_COMPILE_COMMANDS=ON`.

```sh
python3 tests/plugindocument/host_probe.py --build build
```

测试不替代原生窗口、设备驱动及动态菜单的人工交互检查。
These tests do not replace manual inspection of native windows, device drivers or dynamic menus.


`legacy_slots.json` 固定增强前的 148 个 public slots 签名及返回类型；独立 CTest 检查源码声明，真实宿主探针同时检查 Qt 元对象中的旧接口，并实际调用旧 SDK 的读取入口。
`legacy_slots.json` freezes the 148 pre-extension public-slot signatures and return types. A standalone CTest checks source declarations; the real-host probe also checks old Qt metaobject entries and invokes the legacy SDK read path.

`closeFile(sender, handle)` 已按新接口要求移除，使用返回 `int` 的 `closeFile(sender, handle, force)`；仅释放句柄使用 `closeFileHandle(sender, handle)`。兼容性清单保留其余旧接口。
The old two-argument host close slot was intentionally replaced with the int-returning `closeFile(sender, handle, force)`. Use `closeFileHandle(sender, handle)` to release access only. The compatibility fixture retains the remaining legacy slots.

宿主句柄池优先复用已释放的 ID；`PluginFileContext` 的最后一份共享引用销毁后才归还 ID。句柄只在所属插件上下文中、有效持有期间使用。真实宿主探针覆盖延迟归还、池状态的共享生命周期、编号上限处复用以及跨插件隔离。
The host handle pool reuses released IDs first, returning an ID only when the last shared reference to its `PluginFileContext` is destroyed. Handles are valid only within their owning plugin context and lifetime. The real-host probe covers deferred recycling, shared pool-state lifetime, reuse at the ID limit, and isolation between plugins.
