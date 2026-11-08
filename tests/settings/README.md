# 配置键测试 / Settings key tests

独立验证 `src/settings/settings.h` 的配置键重载及读写宏，需要 CMake 3.20+、
Qt 6.6.2+ Widgets/Test 和 C++17；不需要完整宿主或依赖子模块。
Standalone checks for settings-key overloads and read/write macros in
`src/settings/settings.h`. Requires CMake 3.20+, Qt 6.6.2+ Widgets/Test and
C++17, without the full host or submodules.

```sh
cmake -S tests/settings -B build/settings-tests -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/settings-tests
ctest --test-dir build/settings-tests --output-on-failure
```

另用构建目录并传入 `-DCMAKE_CXX_STANDARD=20` 验证 C++20。
Use another build directory with `-DCMAKE_CXX_STANDARD=20` to check C++20.

覆盖普通/const/临时 QString、QString 指针、真实 Q_GLOBAL_STATIC 和智能指针键，
以及不支持类型的重载排除。Q_GLOBAL_STATIC 保留其原有解引用限定，不强行转换成
const 包装器。INI 往返覆盖动态 Splash 历史键的 QVariantList、静态键、默认值和
类型化读取宏；全部写入临时文件，不访问用户配置或注册表。
Covers mutable/const/temporary QString values, QString pointers, real
Q_GLOBAL_STATIC keys, smart pointers and rejection of unsupported types.
Q_GLOBAL_STATIC keeps its native dereference qualifiers rather than forcing
const wrappers. INI round trips cover QVariantList splash history under a
dynamic key, static keys, defaults and typed read macros. All writes use temporary
files, leaving user settings and the registry untouched.
