# Luau 语言服务器回归测试 / Luau language server regression tests

独立编译项目实际使用的语言服务器、模块解析器和当前 Luau 子模块，不依赖主应用的插件及崩溃追踪库。
Builds the actual language server, module resolver, and current Luau submodule independently of the application's plugins and crash-tracing libraries.

```sh
cmake -S tests/luaulsp -B /tmp/wing-luau-lsp-build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/wing-luau-lsp-build -j 2
ctest --test-dir /tmp/wing-luau-lsp-build --output-on-failure
```

测试使用 Qt 离屏平台；需要 Qt 6.6.2 或更高版本及 C++20 编译器。
Tests use Qt's offscreen platform and require Qt 6.6.2 or newer and a C++20 compiler.

## 覆盖范围 / Coverage

- `features`：诊断后悬停、签名帮助、定义及类型定义跳转、语义标记、成员补全、文档更新和关闭。
  Hover, signature help, definition and type-definition navigation, semantic tokens, member completion, document updates, and closing after diagnostics.
- `types`：用户自定义类型函数及默认 nonstrict 模式下的成员推断。
  User-defined type functions and member inference in the default nonstrict mode.
- `overloads`：自定义定义文件、符号树、重载选择、不完整调用和带接收者的方法调用。
  Custom definition files, the symbol tree, overload selection, incomplete calls, and method receivers.
- `modules`：含空格路径下的相对 `require`、跨模块跳转及依赖修改后的补全刷新。
  Relative `require` in paths containing spaces, cross-module navigation, and completion refresh after dependency edits.

## 新求解器迁移依据 / New solver migration rationale

新求解器采用局部类型推断，将类型推断与诊断检查分开，使语言功能与诊断能够共享同一份类型图，并支持用户自定义类型函数。迁移不意味着 strict 模式下的诊断结果与旧求解器完全一致，也不代表已证明所有项目的性能都会提高。
The new solver uses local type inference, separating inference from diagnostic checking so language features and diagnostics can share one type graph. It also supports user-defined type functions. Migration does not imply identical strict-mode diagnostics or a demonstrated performance improvement for every project.

当前适配在构造前端时选择 `SolverMode::New`，统一读取普通模块及全局类型缓存，为打开的文档保留类型图，并使用新求解器的子类型规则匹配签名重载。无需再次注册补全专用的全局定义。
The adapter selects `SolverMode::New` when constructing the frontend, uses the regular module and global-type caches, retains type graphs for open documents, and matches signature overloads using the new solver's subtyping rules. Separate autocomplete globals no longer need registration.

参考资料 / References:

- [Luau: Local Type Inference](https://rfcs.luau.org/local-type-inference.html)
- [Luau: User-Defined Type Functions](https://rfcs.luau.org/user-defined-type-functions.html)
- 本地实现 / Local implementation: `3rdparty/luau/Analysis/src/Frontend.cpp`, `Autocomplete.cpp`, `AutocompleteCore.cpp`, `FragmentAutocomplete.cpp`.
