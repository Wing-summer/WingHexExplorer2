# 通用脚本集成能力回归 / Shared script integration regression

本套直接测试生产代码，不构造主窗口。需要 Qt Widgets、Network、Test、OpenSSL 开发库和 OpenSSL 命令行工具；Python 启动器把测试程序及临时证书放入私有目录，不读取或修改应用的证书目录。

These tests exercise production code without constructing a main window. They require Qt Widgets, Network and Test, OpenSSL development libraries and the OpenSSL CLI. The Python launcher isolates the executable and certificates in a temporary directory, without touching the application's certificate directory.

```sh
cmake -S tests/scriptintegration -B build/script-integration-tests -G Ninja
cmake --build build/script-integration-tests
ctest --test-dir build/script-integration-tests --output-on-failure
```

覆盖空内容 MIME、文本与二进制区分、缺失文件和目录、签名原语、文件与 QBuffer 共用校验、部分读取、篡改、失效句柄及空消息签名。证书信任测试遵循平台的只读证书权限策略，不满足时会明确跳过；签名原语测试不受此限制。可通过 `-DCMAKE_CXX_STANDARD=20` 切换语言标准。

Coverage includes empty-content MIME classification, text/binary distinction, missing files and directories, signature primitives, shared file/QBuffer verification, partial reads, tampering, invalid devices and empty-message signatures. Trust-policy tests explicitly skip when the platform does not recognize fixture permissions as trusted; cryptographic tests still run. Select C++20 with `-DCMAKE_CXX_STANDARD=20`.
