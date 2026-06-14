# 安全策略 (Security Policy)

## 支持版本 (Supported Versions)

该项目仅支持 Qt6.6.2 及以上的版本，并且在作者维护开发期间会紧跟 Qt6 的最新更新，
只要 Qt 废弃了任意版本，该版本将不会受到支持，后续相关代码将会被进行清理。如果您确实需要，
请联系我进行付费技术支持。

This project only supports Qt 6.6.2 and above. While the author maintains 
and develops it, it will closely follow the latest Qt 6 updates. However, 
if any version of Qt is deprecated, this version will not be supported, and
the related code will be removed. If you require support, please contact me
for paid technical support.

## 正版验证

该仓库传统意义上没有官方正版的说法。由于发行版是通过 CI 方式构建自行生成的，
而且不同 Linux 发行版的差异也会造成一些麻烦，这需要自行编译才能基本解决。
就算这些问题都能够解决，签名需要私钥，由于个人开发，没有多余的人力财力维护这一套设施，
使用者是无法通过直接手段判断是否二进制产物来源于仓库拥有者。

Traditionally, there is no official, genuine version of this repository.
Furthermore, since the distributions are generated automatically via CI,
and differences between different Linux distributions can cause problems,
requiring manual compilation to resolve. Even if these issues are resolved,
signing requires a private key, and due to the individual development,
there isn't sufficient manpower or financial resources to maintain this system.
Users cannot directly determine whether the binary output originates from the repository owner.

## 报告漏洞 (Reporting a Vulnerability)

以下情况将不视为该软件的漏洞：

The following situations will not be considered vulnerabilities:

- 通过二进制修改或者注入的方式使程序崩溃等其他异常行为的
- Program crashes or other abnormal behaviors through binary modification or injection
- 通过将符合插件接口的恶意二进制放入加载目录并尝试修改程序配置导致被加载的
- Malicious binaries conforming to plugin interfaces to be loaded by placing them in the loading directory and attempting to modify program configurations
- 通过劫持技术或者动态链接库预加载方式的
- Crashes through hijacking techniques or dynamic link library preloading
- 因为插件崩溃和恶意行为导致软件崩溃的
- Software crashes due to plugin crashes and malicious behavior

以下情况可以视为漏洞：

The following situations can be considered vulnerabilities:

- 在无外部插件的情况下，文件无法保存或者使文件损坏
- Without external plugins, files cannot be saved or become corrupted.
- 执行脚本导致程序卡死或者崩溃的
- Script execution causes the program to freeze or crash.
- 在无外部插件且设置不被允许的情况下，脚本能够编辑和删除外部文件等权限越界的
- Without external plugins and when permissions are not allowed, scripts can edit and delete external files, exceeding permitted permissions.
- 在无外部插件的情况下，程序崩溃或者行为不符合正常工程学设计和预期的
- Without external plugins, the program crashes or behaves in ways that are not ergonomically designed or expected.

如果您发现了漏洞，你可以通过 issue 的方式进行，如果您提供修复补丁，欢迎 PR 。

If you discover a vulnerability, you can submit it as an issue. If you provide a patch, a pull request (PR) is welcome.

在没有特殊情况下，该软件将会在下一次月更周期发布。因为该软件的维护者和开发者只有仓库拥有者，
维护该软件完全靠作者为爱发电。所以请不要有让作者必须多少天就立即修复保证的苛责行为。
如果您确实需要保证和支持，请给予数目可观的赞助，否则请耐心等待或者自行修复后自己编译。

Barring unforeseen circumstances, this software will be released in the next monthly update cycle.
Because the only maintainer and developer of this software is the repository owner, 
maintenance is entirely the author's passion. Therefore, 
please refrain from demanding that the author fix the issue within a certain number of days.
If you genuinely require support and guarantees, please provide substantial donations;
otherwise, please wait patiently or fix the issue yourself and compile it yourself.


