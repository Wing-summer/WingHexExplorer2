- [代码贡献者](#代码贡献者)
- [软件编译运行情况](#软件编译运行情况)
- [WingHexExplorer2](#winghexexplorer2)
  - [参与贡献](#参与贡献)
  - [协议](#协议)
  - [使用声明](#使用声明)
  - [issue 前必读](#issue-前必读)
  - [使用的开源项目](#使用的开源项目)
    - [QHexView](#qhexview)
    - [QHexEdit2](#qhexedit2)
    - [Qt-Advanced-Docking-System](#qt-advanced-docking-system)
    - [QCodeEditor](#qcodeeditor)
    - [QWingRibbon](#qwingribbon)
    - [QLogger](#qlogger)
    - [SingleApplication](#singleapplication)
    - [QPathEdit](#qpathedit)
    - [Qt-FramelessWindow](#qt-framelesswindow)
    - [AngelScript](#angelscript)
    - [主题](#主题)
- [效果图](#效果图)
- [捐助](#捐助)
- [有关仓库](#有关仓库)
- [WIKI](#wiki)
- [插件库](#插件库)
- [应用商店](#应用商店)

---

<h1 align="center"> WingSummer.WingHexExplorer2</h1>

<p align="center">
<img alt="PEHexExplorer" src="images/icon.png">
<p align="center">羽云十六进制浏览器</p>
</p>

<p align="center">
<img alt="作者" src="authorband.svg">
<img alt="协议" src="licenseband.svg">
</p>

- 开源不易，给个 Star 或者 [捐助](#捐助) 吧

## 代码贡献者

&emsp;&emsp;维护一个好用的工具并不是一个人能做到了，更重要的是大家共同维护，如下是对本仓库代码有贡献的同志，特此感谢：

> 吉位招租

## 软件编译运行情况

| 操作系统平台 |  QT5  |  QT6  |                         缺陷                         |
| :----------: | :---: | :---: | :--------------------------------------------------: |
|  Windows 11  |   ✔️   |   ✔️   |               缺乏对 Aero Snap 的支持                |
|  Deepin 20   |   ✔️   |   ❔   | 缺乏标题栏最大化时，拖动恢复窗体之后鼠标不会继续跟随 |
|    MacOS     |   ❌   |   ❌   |                 没有测试，未添加支持                 |

## WingHexExplorer2

> 该项目目前版本已经有基本的功能，但还不全，或许还有 Bug ，还请等我慢慢移植重构，不过是可以保证编译通过的，大家也可以克隆仓库编译看看啥情况。

&emsp;&emsp;本软件是基于 QT 编写的十六进制编辑器，采用 C++ 进行开发，提供强大而免费的十六进制编辑器。目前只有 010 Editor 具有强大的十六进制编辑功能，但它是商用的。关注我开发动态的应该知道我开发了在 Windows 上用 C# 开发的`WingSummer.WingCloudHexExplorer`，目的是方便专业人士修改分析 PE 文件，并可作为学习 PE 结构的重要辅助工具。还有我已经开发了只针对 Deepin 平台适配的`WingHexExplorer`，目前功能已基本完善了。除了在大文件搜索功能有问题，其他已经可以稳定使用了，我决定不再维护`WingHexExplorer`，我将移植重置这个项目。

&emsp;&emsp;为什么要开发这个项目？`WingHexExplorer2`和原来的版本有什么不同？首先，窗体界面是我自己基于开源项目进行修改，重新实现了自己的界面，保证在跨平台的 UI 一致性。虽然功能和系统原生相比还是有点缺陷，也只是算美中不足了。

&emsp;&emsp;该项目终极目标是实现一个功能强大、轻量级且跨平台（只针对 PC 平台）的十六进制编辑器。当然，实现这个目标仅仅靠我是不能够实现的，这也是我开源的目的之一。我只维护我使用的操作系统平台，只对我使用的平台出现的我能够修复的 Bug 在我有空闲的时间进行及时的响应。

&emsp;&emsp;如果你发现该项目的功能并没有满足你的需求。请看看`TODO.txt`里面有没有说明。如果没有，可以通过`issue`进行提出。我会评估提出的需求是共性的。如果是特别私有的需求，我将不会实现。如果你实在想加入，请自行实现插件，也欢迎来开源。提建议的时候不要天马行空，注意本软件只提供最基本的十六进制编辑和浏览服务，比如 010 Editor 里面的模版和脚本等服务，还需大家一起通过插件来实现！希望大家不要只提需求，更要提出建设性建议和解决方案，共同维护开源社区。具体详情将会在后面进行介绍。

### 参与贡献

- 如果您有想参与本软件代码开发递交，请在 pull request 联系我。
- 本项目支持捐助，如有意愿请到本仓库通过微信或者支付宝的方式进行，一瓶水的价钱足以提高我的维护该项目的热情，感谢大家的支持。
- 如果您想提交修复或者增进程序的代码，请在 pull request 递交。
- 任何成功参与代码 Bug 修复以及增进程序功能的同志和 Sponsor ，都会在本仓库 ReadMe 和附属说明文件中体现，您如果是其中之一，本人可以按照您合理的意愿来进行说明。

\> **加入我们并不意味着就是代码的维护，你可以选择下列一项或多项进行参与**

- 代码维护：实现新功能或修复 BUG ，对代码进行维护和升级。
- 文档编辑：主要是接口文档和教程需要撰写编辑，这很重要。
- 参与讨论：主要是讨论本项目未来发展和方向等。
- 编写插件：一起增强该软件的功能。

### 协议

&emsp;&emsp;本软件如果是开源版本将遵循`AGPL-3.0`协议，请勿用于该协议之外的用途。我的初衷是让 Linux 的生态更加完整，早日让祖国推动操作系统国产化，软件跨平台只是顺带。我不希望“吸血鬼”们利益归自己，脏活累活给开源，都那么理所当然，开源就是这么被败坏的。我不希望因为版权的事情牵扯了大量的精力。

&emsp;&emsp;如果你想将本软件的代码用于闭源的商业代码，想要解除`GPL`系列的必须开源的限制，请必须亲自咨询我，商讨商业授权相关事宜。

### 使用声明

1. 本软件源代码不得私自应用于闭源商业用途，除非你完整开源（GPL协议的要求）。如果要将软件仓库的代码商用闭源，必须找我购买商业授权签订合同，价格私聊，非诚勿扰。
2. 本软件是用我的业余时间编写，不能及时修复 Bug 或者提供技术支持，请见谅。
3. 本人非计算机专业，编写程序难免有 Bug ，欢迎提交 PR 。
4. 无论在什么时候有疑问，请详细阅读 Wiki 。

### issue 前必读

&emsp;&emsp;如果你有任何形式的建议，在提交 issue 之前，请一定要阅读下面的声明，以免浪费我们双方宝贵的时间：

1. 如果你发现该软件没有提供对你语言的支持，可以通过 pull request 方式来添加。注意，代码中的翻译文本仅仅是标识符，它可能不是真实的文本，请参考我的中文再确定译文。
2. 对于更新，大版本插件采用不兼容处理，如果插件有使用问题，请联系开发者（不排除我会开发一些插件，这样可以直接联系我）。

&emsp;&emsp;上面一切的一切，如果你是志同道合的开源贡献者，欢迎 fork 我的仓库进行相应的维护！

&emsp;&emsp;如果出现比较严重的 Bug ，本人也可能不会及时的响应，谢绝站在道德的制高点不嫌冷的指指点点。

### 使用的开源项目

&emsp;&emsp;不站在大佬的肩膀上，软件也不会具有强大而稳定的功能。**本软件使用的开源基础项目的源代码未经我修改的部分，全部遵守原作者的开源协议；我修改的部分，如没有特殊说明，全部遵守本仓库的协议。**

#### QHexView

&emsp;&emsp;本软件基于`QHexView`作为十六进制编辑器为基础进行开发，本人在改组件基础上添加新功能和进行代码的深度定制。如下是原仓库的必要说明，详情请点击 [此链接](https://github.com/Dax89/QHexView/tree/master)。

&emsp;&emsp;本人对改组件的改进如下：

1. 增加有关描述文件状态相关的功能：指示是否被修改、是否具有可写权限、是否锁定文件以防修改、是否可以增改字节数
2. 增加可隐藏地址栏和 ASCII 解码字符栏以及表头的功能
3. 实现超大文件（超出2GB）读写功能模块（原组件并未实现此功能）
4. 修改地址显示长度，以适应地址的使用习惯
5. 简化编码方式，删除一些冗余代码
6. 增加更多的信号，充分控制 QHexView 组件
7. 增加书签使用和管理功能
8. 修复滚动条有关内容完全仍能够显示但仍滚动内容的 Bug
9. 修复粘贴指针闪动位置保持不变问题，修改粘贴限制策略
10. 增加一些接口适应工作区支持需要
11. 修复标记背景色第一行有超出部分的 Bug
12. 修复复制中字节含有零字节会截断的 Bug
13. 撤销恢复支持光标跟随
14. 升级兼容 QT6

#### QHexEdit2

&emsp;&emsp;起初我打算使用`QHexEdit2`作为十六进制编辑器为基础进行开发，该组件虽然轻松打开超大文件，但是它的编辑功能能用是能用，但有很多大大小小的 Bug ，我还逐一修了修，但发现仅仅我的力量和时间是杯水车薪。然后我找到了`QHexView`，也就是上面所属的组件，但它有一个致命的缺陷，无法打开超大文件，被我放弃掉了，后来我尝试用了它，发现开发者在开发改组件是下了足够大的功夫的，编辑十分流畅。最近看到`QHexView`贡献者们想搞一个`QHexView 5.0`，对代码进行了重构，但并没有实现任何功能，差不多是个空空的框架，不过从接口看出更强大的易用性，这个是原组件所不具有的，这花费我比较多的时间来阅读源代码，并向外扩展接口以适应我的开发需求。

&emsp;&emsp;然后我想，既然`QHexEdit2`具有强大的打开文件的能力，而`QHexView`不具备，但它具有强大的编辑界面，于是乎，我移植`QHexEdit2`的打开超大文件的代码到`QHexView`当中，并做好了适配和功能增强。原仓库的链接： <https://github.com/Simsys/qhexedit2> ，原协议为`GPL`。

#### Qt-Advanced-Docking-System

&emsp;&emsp;QT Advanced Docking System 允许你使用完整的特色窗户对接系统创建可自定义的布局，类似于许多流行的集成开发环境（IDE）（例如Visual Studio）。该库功能十分强大，为了增强软件的易用性，就采用这个库。我略微修改了构建文件以方便直接嵌入到我的项目当中，也方便更新组件。

&emsp;&emsp;原仓库的链接： <https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System> ，原协议为`LGPL`。

#### QScintilla

&emsp;&emsp;一个文本编辑器，功能强大。该项目用来开发以及调试针对羽云编辑器的`AngelScript`脚本。我添加了`CMake`编译方式并且只保留核心部分，移除了自带的词法分析器，因为对我这个项目来说重点不是这个。官方链接： <https://riverbankcomputing.com/software/qscintilla/intro> ，协议细节请看官方文档。

#### QWingRibbon

&emsp;&emsp;虽然这个带`Wing`这个玩意，但并不是完全是我写的，这个是基于`Qt-Ribbon-Widget`魔改来的。之所以换名字是改动的有点多，打算自己维护一个自己的用来给该编辑器使用的。

&emsp;&emsp;原仓库的链接： <https://github.com/martijnkoopman/Qt-Ribbon-Widget> ，原协议为`LGPL`。

#### QLogger

&emsp;&emsp;一个日志框架，原仓库的链接： <https://github.com/sfadiga/QLogger> ，原协议为`MIT`。

#### SingleApplication

&emsp;&emsp;保留应用程序的主实例并终止每个后续实例。它可以（如果启用）生成第二（与主实例无关）实例，并且可以从第二实例向主实例发送数据。简而言之就是实现程序单例的库。原仓库的链接： <https://github.com/itay-grudev/SingleApplication> ，它的协议为`MIT`，多了一个额外限制，具体看其协议。

#### QPathEdit

&emsp;&emsp;以优化和简单的方式获取本地文件和文件夹路径的一个 QT 组件。原仓库的链接： <https://github.com/Skycoder42/QPathEdit> ，原协议为`MIT`。

#### Qt-FramelessWindow

&emsp;&emsp;这个无边框窗体是本项目界面的基础。我对其进行了一些修改，修复了窗体使用缩放的问题以及增加了标题栏的系统响应，以及最大化后窗体大小的处理优化和`QT6`的代码进一步兼容。然后做了一些封装之后，成为本软件的所有界面组成。

&emsp;&emsp;原仓库的链接： <https://github.com/winsoft666/Qt-FramelessWindow> ，原协议为`Apache v2.0`。

#### AngelScript

&emsp;&emsp;让 C++ 成为具有惊人功能的脚本语言！这个功能强大，且不需要外部依赖，直接编译进去就行，不像之前版本必须携带一个`Python`解释器和库，不太方便编译。

&emsp;&emsp;我只找到了镜像库： <https://github.com/IngwiePhoenix/AngelScript> ，原协议为`zlib license`，还是比较宽松的开源协议。

#### 主题

&emsp;&emsp;主题使用的是`BreezeStyleSheets`，这个主题是我修改的定制衍生版本，我的衍生仓库为：<https://github.com/Wing-summer/BreezeStyleSheets> ，原版仓库是这个：<https://github.com/Alexhuszagh/BreezeStyleSheets>。这个我修改不多，直接`MIT`就行。

## 效果图

<p align="center">
<img alt="效果图" src="screenshot.png">
<p align="center">羽云十六进制浏览器</p>
</p>

## 捐助

**<p align="center">您的每一份支持都将是本项目推进的强大动力，十分感谢您的支持</p>**

<p align="center">

<img alt="支付宝" src="支付宝捐助.jpg" height=50% width=50%>
<p align="center">感谢支持</p>

</p>

<p align="center">
<img alt="微信" src="微信捐助.png" height=50% width=50%>
<p align="center">感谢支持</p>

</p>

## 有关仓库

&emsp;&emsp;目前我认定的官方仓库 Github： <https://github.com/Wing-summer/WingHexExplorer2> 。

&emsp;&emsp;当然还有其他仓库作为国内用户镜像仓库（会滞后更新）：

- Gitee：<https://gitee.com/wing-cloud/WingHexExplorer2>
- Gitlink：<https://www.gitlink.org.cn/wingsummer/WingHexExplorer2>
- GitCode：不考虑，因为“窃码”，读书人的事叫偷吗？！

## WIKI

&emsp;&emsp;如果想学习羽云十六进制编辑器的使用以及如何开发该软件的插件，请到该链接：`还没有移植完捏` ，同时也欢迎大家指出 Wiki 的错误以及贡献优质内容。

## 插件库

> 敬请期待

## 应用商店

> 敬请期待
