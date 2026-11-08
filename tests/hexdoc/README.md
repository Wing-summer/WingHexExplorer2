# hexdoc 验证 / hexdoc validation

先用正常 CMake 配置生成绑定；独立运行时测试使用真实 Luau VM、require 解析器、维护中的模块及原生禁止挂起辅助函数，其 reader/ctl 传输使用测试替身，不替代完整宿主验证。

Generate bindings through normal CMake configuration first. The isolated runtime suite uses the real Luau VM, require resolver, maintained module and native non-yielding helper. Its reader/ctl transport is a fixture, not full-host validation.

```sh
cmake -S tests/hexdoc -B build/hexdoc-tests -G Ninja \
  -DHEX_BINDINGS="$PWD/build/generated/WingLuauBind"
cmake --build build/hexdoc-tests --parallel 4
ctest --test-dir build/hexdoc-tests --output-on-failure

cmake -S tests/luaulsp -B build/hexdoc-lsp-tests -G Ninja \
  -DWING_GENERATED_BINDINGS_DIR="$PWD/build/generated/WingLuauBind"
cmake --build build/hexdoc-lsp-tests --parallel 4
ctest --test-dir build/hexdoc-lsp-tests --output-on-failure

PYTHONPATH=3rdparty/WingLuauQt/python python3 -m unittest discover \
  -s tests/hexdoc -p 'test_*.py'
PYTHONPATH=3rdparty/WingLuauQt/python python3 -m unittest discover \
  -s 3rdparty/WingLuauQt/tests/python -p test_native_caller.py
```

运行时覆盖单次切换、嵌套、冲突、作用域外调用、多返回值、异常、禁止挂起、释放与关闭、重复打开和上下文隔离。生成器测试检查映射与类型操作一致、默认参数重载、原始管理接口排除，以及私有辅助函数的 Qt 转换和原始 Lua C 回调注册。LSP 测试加载完整真实定义，通过普通 `@s/hexdoc` 路径验证对象补全、操作重载、泛型回调结果和错误参数／返回值诊断。宿主原生文件、设备、工作区和真实调度取消还需完整宿主探针验证。

Runtime coverage includes one switch, nesting, conflicts, scope enforcement, multiple results, errors, yield rejection, release versus close, reopening and context isolation. Generator tests check operation parity, default-argument overloads, management exclusions, and private helper Qt conversion and raw Lua C callback registration. LSP tests load the complete real declarations and use ordinary `@s/hexdoc` resolution to check object completion, operation overloads, generic callback results and invalid argument/return diagnostics. Native files, devices, workspaces and actual scheduler cancellation additionally require a full-host probe.

完整宿主构建后，可复用原生文档探针的链接器，运行真实文件／设备／工作区、句柄隔离及 ScriptMachine 完成／异常／取消／控制台重置验证：

After a full host build, reuse the native document probe linker for real file/device/workspace operations, handle isolation, and ScriptMachine completion/error/cancellation/console-reset coverage:

```sh
python3 tests/plugindocument/host_probe.py --build build \
  --source tests/hexdoc/host_probe.cpp
```

`hexdoc_api.luau` 同时生成运行时操作映射和导出的 `Document` 类型。`hexdoc.luau` 使用相对 `require` 引用它；运行时和 LSP 均通过 `WingLuauRequire` 解析模块，无需额外的全局 `.d.luau` 声明或路径映射。
`hexdoc_api.luau` generates both the runtime operation map and the exported `Document` type. `hexdoc.luau` imports it through a relative `require`; runtime and LSP use `WingLuauRequire` to resolve both modules, without an additional ambient `.d.luau` declaration or path map.

文档查询探针还覆盖 `reader.is_file_opened` 的上下文归属、两个文件列表的页面计数、工作区及设备工作区的 `wingpro` 路径，以及克隆页沿用原始页句柄。SDK 查询在稳定文档上验证后台读取；脚本对象的 `is_valid()` 覆盖释放和上下文结束后的失效状态。
The document query probe also checks handle ownership, per-page list counts, workspace/device-workspace `wingpro` paths and clones sharing their original page handles. SDK queries verify worker-thread reads on stable documents; script `is_valid()` covers release and context teardown.

宿主探针还验证撤销宏的嵌套配对、宏内切换及释放限制、重置撤销，以及上下文析构时补齐未结束的嵌套宏。
The host probe also checks nested undo macro pairing, switching and release guards during macros, reset rollback, and completion of unfinished nested macros during context destruction.
