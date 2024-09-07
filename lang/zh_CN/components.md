 
不站在大佬的肩膀上，软件也不会具有强大而稳定的功能。**本软件使用的开源基础项目的源代码未经我修改的部分，全部遵守原作者的开源协议；我修改的部分，如没有特殊说明，全部遵守本仓库的协议。**

#### QHexView

本软件基于`QHexView`作为十六进制编辑器为基础进行开发，本人在改组件基础上添加新功能和进行代码的深度定制。如下是原仓库的必要说明，详情请点击 [此链接](https://github.com/Dax89/QHexView/tree/master)。

#### QHexEdit2

起初我打算使用`QHexEdit2`作为十六进制编辑器为基础进行开发，该组件虽然轻松打开超大文件，但是它的编辑功能能用是能用，但有很多大大小小的 Bug ，我还逐一修了修，但发现仅仅我的力量和时间是杯水车薪。然后我找到了`QHexView`，也就是上面所属的组件，但它有一个致命的缺陷，无法打开超大文件，被我放弃掉了，后来我尝试用了它，发现开发者在开发改组件是下了足够大的功夫的，编辑十分流畅。最近看到`QHexView`贡献者们想搞一个`QHexView 5.0`，对代码进行了重构，但并没有实现任何功能，差不多是个空空的框架，不过从接口看出更强大的易用性，这个是原组件所不具有的，这花费我比较多的时间来阅读源代码，并向外扩展接口以适应我的开发需求。

然后我想，既然`QHexEdit2`具有强大的打开文件的能力，而`QHexView`不具备，但它具有强大的编辑界面，于是乎，我移植`QHexEdit2`的打开超大文件的代码到`QHexView`当中，并做好了适配和功能增强。原仓库的链接： <https://github.com/Simsys/qhexedit2> ，原协议为`GPL`。

#### Qt-Advanced-Docking-System

QT Advanced Docking System 允许你使用完整的特色窗户对接系统创建可自定义的布局，类似于许多流行的集成开发环境（IDE）（例如Visual Studio）。该库功能十分强大，为了增强软件的易用性，就采用这个库。我略微修改了构建文件以方便直接嵌入到我的项目当中，也方便更新组件。

原仓库的链接： <https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System> ，原协议为`LGPL`。

#### QCodeEditor2

一个文本编辑器，功能强大。该项目用来开发以及调试针对羽云编辑器的`AngelScript`脚本。该组件来源于`edyuk`开源项目，采用`GPL`协议，不过这玩意在`Github`上也没有，只有在 [SourceForge](https://sourceforge.net/projects/edyuk) 有它的身影。我对其进行了一些定制和小`Bug`的修复。

#### QCodeModel2

该组件来源于`edyuk`这一个项目，用于代码解析。本项目的代码分析和代码智能提示也是基于该项目中的`C++`分析器相关代码修改而来，来源同`QCodeEditor2`。

#### QWingRibbon

虽然这个带`Wing`这个玩意，但并不是完全是我写的，这个是基于`Qt-Ribbon-Widget`魔改来的。之所以换名字是改动的有点多，打算自己维护一个自己的用来给该编辑器使用的。

原仓库的链接： <https://github.com/martijnkoopman/Qt-Ribbon-Widget> ，原协议为`LGPL`。

#### SingleApplication

保留应用程序的主实例并终止每个后续实例。它可以（如果启用）生成第二（与主实例无关）实例，并且可以从第二实例向主实例发送数据。简而言之就是实现程序单例的库。原仓库的链接： <https://github.com/itay-grudev/SingleApplication> ，它的协议为`MIT`，多了一个额外限制，具体看其协议。

#### QPathEdit

以优化和简单的方式获取本地文件和文件夹路径的一个 QT 组件。原仓库的链接： <https://github.com/Skycoder42/QPathEdit> ，原协议为`MIT`。

#### QWindowKit

该组件是用户界面的基础，用于在不同操作系统平台提供较为统一的用户界面体验。原仓库的链接： <https://github.com/stdware/qwindowkit> ，原协议为`Apache v2.0`。

#### AngelScript

让 C++ 成为具有惊人功能的脚本语言！这个功能强大，且不需要外部依赖，直接编译进去就行，不像之前版本必须携带一个`Python`解释器和库，不太方便编译。

镜像库： <https://github.com/codecat/angelscript-mirror> ，原协议为`zlib license`，还是比较宽松的开源协议。

#### QConsoleWidget

一个轻量的控制台组件，协议为`MIT`，仓库链接：<https://github.com/gapost/qconsolewidget> 。

#### QColorPicker

一个颜色选择组件，协议为`MIT`，仓库链接：<https://github.com/arsdever/qcolorpicker> 。

#### QtJsonModel

QJsonModel 是一个基于 QAbstractItemModel 的 Qt6/C++17 的 JSON 树模型类。在该项目用于插件和脚本给予宿主进行树形可视化的基础支持组件。我对其增加了 Qt5 的支持并且写了一个合适的 CMake 文件。原协议为`MIT`，修改不多遵守这个就行了。仓库链接：<https://github.com/dridk/QJsonmodel> 。
