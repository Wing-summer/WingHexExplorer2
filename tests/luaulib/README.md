# 标准库验证 / Standard library validation

```sh
python3 -m unittest discover -s tests/luaulib -p 'test_*.py'
python3 tests/luaulib/run.py --luau luau
python3 tests/examples/run.py --luau luau
cmake --build build
python3 tests/headless/build_process_probe.py --build build
python3 tests/luaulib/integration.py --binary build/WingHexHeadlessProbe
```

真实宿主测试需要 loopback socket 权限，使用临时设置；不要改用普通用户配置的宿主。
The native integration suite needs loopback socket access and uses an isolated-settings
process probe. Do not substitute a normal host that loads user settings.

可选官方客户端互通验证 / Optional official-client interoperability:

```sh
npm install --prefix /tmp/wing-mcp-sdk --ignore-scripts @modelcontextprotocol/client@2.4.0
python3 tests/luaulib/integration.py --binary build/WingHexHeadlessProbe \
    --sdk-prefix /tmp/wing-mcp-sdk
```

```sh
cmake --build build/hexdoc-lsp-tests
QT_QPA_PLATFORM=offscreen \
WING_BINDINGS_DIR="$PWD/build/generated/WingLuauBind" \
WING_STDLIB_DIR="$PWD/build/luaulib" \
    build/hexdoc-lsp-tests/wing_luau_lsp_tests stdlib
ctest --test-dir build/hexdoc-lsp-tests --output-on-failure
python3 tests/plugindocument/host_probe.py --build build \
    --source tests/hexdoc/host_probe.cpp
```

独立脚本测试覆盖编码、边界、布局、翻译；宿主测试覆盖只读环境、整数、cffi、注册 JSON、
真实 MCP HTTP、多客户端和任务清理。宿主文档探针覆盖真实写回、冲突、回滚和标注及 GUI
图标接口。LSP 验证模块声明、枚举补全和错误类型诊断。离屏验证不替代原生桌面与跨平台验收。
The script suite covers codecs, bounds, layouts and translation. Host tests exercise
read-only environment data, wide integers, cffi, registered JSON, real MCP HTTP,
multiple clients and task cleanup. The document probe checks real writes, conflicts,
rollback, annotations and GUI icon APIs. LSP checks declarations, enum completion
and invalid types. Offscreen tests do not replace native or cross-platform acceptance.
