> If you are not a Chinese user, please click [this](README_en.md)

- [代码贡献者](#代码贡献者)
- [赞助鸣谢](#赞助鸣谢)
- [参与贡献](#参与贡献)
- [软件编译运行情况](#软件编译运行情况)
- [WingHexExplorer2](#winghexexplorer2)
  - [协议](#协议)
  - [使用声明](#使用声明)
  - [打包者必读](#打包者必读)
  - [issue 前必读](#issue-前必读)
- [效果图](#效果图)
- [捐助](#捐助)
- [有关仓库](#有关仓库)
- [WIKI](#wiki)
- [插件库](#插件库)

---

<h1 align="center"> WingSummer.WingHexExplorer2</h1>

<p align="center">
<img alt="WingHexExplorer2" src="images/icon.png" />
<p align="center">羽云十六进制浏览器</p>
</p>

<p align="center">
<img alt="作者" src="authorband.svg" />
<img alt="协议" src="licenseband.svg" />
<a href="https://deepwiki.com/Wing-summer/WingHexExplorer2"><img src="https://deepwiki.com/badge.svg" alt="Ask DeepWiki"></a>
</p>

<p align="center">
<img src="https://github.com/Wing-summer/WingHexExplorer2/actions/workflows/clang-format-check.yml/badge.svg" />
<img src="https://github.com/Wing-summer/WingHexExplorer2/actions/workflows/cmake-format-check.yml/badge.svg" />
</p>

- 开源不易，给个 Star 或者 [捐助](#捐助) 吧

## 代码贡献者

&emsp;&emsp;维护一个好用的工具并不是一个人能做到了，更重要的是大家共同维护，如下是对本仓库代码有贡献的同志，特此感谢：

<!-- readme: collaborators,contributors -start -->
<table>
	<tbody>
		<tr>
            <td align="center">
                <a href="https://github.com/Wing-summer">
                    <img src="https://avatars.githubusercontent.com/u/59953528?v=4" width="100;" alt="Wing-summer"/>
                    <br />
                    <sub><b>wingsummer</b></sub>
                </a>
            </td>
            <td align="center">
                <a href="https://github.com/oPengLuo">
                    <img src="https://avatars.githubusercontent.com/u/41234604?v=4" width="100;" alt="oPengLuo"/>
                    <br />
                    <sub><b>大大的力量</b></sub>
                </a>
            </td>
		</tr>
	<tbody>
</table>
<!-- readme: collaborators,contributors -end -->

## 赞助鸣谢

&emsp;&emsp;感谢以下同志的赞助，我会尽量持续做成一个好用且功能强大的且跨平台的 PC 端的十六进制编辑器（时间先后顺序）：

| 赞助者 |          备注          |
| :----: | :--------------------: |
|  *欢   | 来自 Deepin 的一位道友 |

## 参与贡献

- 如果您有想参与本软件代码开发递交，请在 pull request 联系我。
- 本项目支持捐助，如有意愿请到本仓库通过微信或者支付宝的方式进行，一瓶水的价钱足以提高我的维护该项目的热情，感谢大家的支持。
- 如果您想提交修复或者增进程序的代码，请在 pull request 递交。
- 任何成功参与代码 Bug 修复以及增进程序功能的同志和 Sponsor ，都会在本仓库 ReadMe 和附属说明文件中体现，您如果是其中之一，本人可以按照您合理的意愿来进行说明。

\> **加入我们并不意味着就是代码的维护，你可以选择下列一项或多项进行参与**

- 代码维护：实现新功能或修复 BUG ，对代码进行维护和升级。
- 文档编辑：主要是接口文档和教程需要撰写编辑，这很重要。
- 参与讨论：主要是讨论本项目未来发展和方向等。
- 编写插件：一起增强该软件的功能。

&emsp;&emsp;在参与贡献前，请仔细阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。

&emsp;&emsp;确保在贡献和参与讨论时遵循 [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)。

## 软件编译运行情况

> 对于 Qt 5 版本，建议 5.15.2 或更高版本，Qt 6 的话 6.6.2 或更高的版本，其他低版本由于关键库的 API 缺失和自身的一些 Bug，所以请不要使用。

另外：

> QT5 版本主动编译支持已停止，我不会再支持和修复相关问题，但接受 PR 。如果因特殊需要 QT5 版本，请咨询我商业合作和付费技术支持。

&emsp;&emsp;该仓库代码使用 Qt 6.6.2 在最新版 Windows 和 Ubuntu 编译情况： ![Status](https://github.com/Wing-summer/WingHexExplorer2/actions/workflows/qt-build-test.yml/badge.svg)

## WingHexExplorer2

&emsp;&emsp;本软件是基于 QT 编写的十六进制编辑器，采用 C++ 进行开发，提供强大而免费的十六进制编辑器。该项目终极目标是实现一个功能强大、轻量级且跨平台（只针对 PC 平台）的十六进制编辑器。当然，实现这个目标仅仅靠我是不能够实现的，这也是我开源的目的之一。我只维护我使用的操作系统平台，只对我使用的平台出现的我能够修复的 Bug 在我有空闲的时间进行及时的响应。具体细节请看我的博客园的 [相关说明](https://www.cnblogs.com/wingsummer/p/18286419) 。

### 协议

&emsp;&emsp;本软件遵循`AGPL-3.0`协议，请勿用于该协议之外的用途。我不希望因为版权的事情牵扯了大量的精力。

&emsp;&emsp;如果你想将本软件的代码用于闭源的商业代码，想要解除`GPL`系列的必须开源的限制，请必须亲自咨询我，商讨商业授权相关事宜。

&emsp;&emsp;对于插件开发相关的，对应的开源协议就不一样了。只针对本仓库下的`WingPlugin`的代码遵守`BSD 3-Clause`协议，以允许闭源商业开发。对于本仓库下的`TestPlugin`的代码（除`TranslationUtils.cmake`这一个文件遵守`BSD 3-Clause`）遵守`MIT`协议。

### 使用声明

1. 本软件源代码不得私自应用于闭源商业用途，除非你完整开源（GPL协议的要求）。如果要将软件仓库的代码商用闭源，必须找我购买商业授权签订合同，价格私聊，非诚勿扰。
2. 本软件是用我的业余时间编写，不能及时修复 Bug 或者提供技术支持，请见谅。
3. 无论在什么时候有疑问，请详细阅读 Wiki 。

### 打包者必读

&emsp;&emsp;首先，我对你的热心帮助表示真挚的感谢。作为一个自由软件，你可以在遵守开源协议和精神的条件下，无需告知我就可以打包并通过合法渠道进行分发。你可以在打包信息里包含自己的信息，但是，你必须遵守以下作为该软件的作者我的约定：

1. 软件包必须包含`LICENSE`、`authorband.svg`、`licenseband.svg`、 `screenshot.png`和`README.md`这几个文件，且都放到和主程序一致的目录下。这些文件都在项目的根目录下，直接复制就行了。
2. 除非事前声明该软件已经被二次修改，打包的二进制安装包中不得含有经修改后源码编译的主程序。
3. 打包后的二进制安装包不得含有侵犯他人合法权益的内容，比如向其加入未经授权的付费内容或计算机病毒等。
4. 包内最好含有主程序的校验和，但不强求。
5. 无论是哪种打包形式，都一定要声明其开源协议和自由属性，不得隐藏而借机收费。虽然收费我并不反感，下载服务器还是需要资金维护的，但借着信息差骗人收费，这是我极其深恶痛绝的。
6. 无论以任何为目的，如未获得我作者的授权，不得修改任意程序内指向的网络链接和软件关于信息，比如赞助和关于软件部分的内容等。
7. 不得在安装程序内插入任何含有商业推广的插件。

### issue 前必读

&emsp;&emsp;如果你有任何形式的建议，在提交 issue 之前，请一定要阅读下面的声明，以免浪费我们双方宝贵的时间：

1. 如果你发现该软件没有提供对你语言的支持，可以通过 pull request 方式来添加。注意，代码中的翻译文本仅仅是标识符，它可能不是真实的文本，请参考我的中文再确定译文。
2. 对于更新，大版本插件采用不兼容处理，如果插件有使用问题，请联系开发者（不排除我会开发一些插件，这样可以直接联系我）。

&emsp;&emsp;上面一切的一切，如果你是志同道合的开源贡献者，欢迎 fork 我的仓库进行相应的维护！

&emsp;&emsp;如果出现比较严重的 Bug ，本人也可能不会及时的响应，谢绝站在道德的制高点不嫌冷的指指点点。

&emsp;&emsp;不站在大佬的肩膀上，软件也不会具有强大而稳定的功能。**本软件使用的开源基础项目的源代码未经我修改的部分，全部遵守原作者的开源协议；我修改的部分，如没有特殊说明，全部遵守本仓库的协议。**

## 效果图

<p align="center">
<img alt="效果图" src="screenshot.png">
<p align="center">羽云十六进制浏览器</p>
</p>

## 捐助

> If you are not Chinese, you can use [Afadian](https://afdian.com/a/wingsummer) to support me.

**<p align="center">您的每一份支持都将是本项目推进的强大动力，十分感谢您的支持</p>**

<p align="center">

<img alt="捐助" src="Donate.jpg" />
<p align="center">感谢支持</p>
</p>

## 有关仓库

&emsp;&emsp;目前我认定的官方仓库 Github： <https://github.com/Wing-summer/WingHexExplorer2> 。

&emsp;&emsp;当然还有其他仓库作为国内用户镜像仓库（会滞后更新）：

- Gitee：<https://gitee.com/wing-cloud/WingHexExplorer2>
- Gitlink：<https://www.gitlink.org.cn/wingsummer/WingHexExplorer2>
- GitCode：不考虑，因为“窃码”，读书人的事叫偷吗？！

## WIKI

&emsp;&emsp;如果想学习羽云十六进制编辑器的使用以及如何开发该软件的插件，请到 [该链接](https://wing-summer.github.io/WingHexExplorer2/docs/zh/credits.html) ，同时也欢迎大家指出 Wiki 的错误以及贡献优质内容。

## 插件库

- [WingAsm](https://github.com/Wing-summer/WingAsm) : 一个提供汇编和反汇编的插件，作者 **寂静的羽夏** ，协议 **APGL-v3.0** 。
