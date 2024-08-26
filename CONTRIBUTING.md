## 如何参与该项目 / How to CONTRIBUTING

目前该项目由寂静的羽夏主导且是唯一开发者，我首先表达对你想要参与开源贡献的行为表示感谢。

Currently the project is led by wingsummer and is the only developer. I would like to first express my gratitude for your willingness to participate in open source contributions.

如果你是短期参与贡献或者不想和作者联系，你可以自己 fork 该仓库，并修改完毕代码之后保证与我的最新仓库仅仅提前一次 commit，然后向该仓库提 Pull requests，我会亲自审核并及时做出反应。

If you are contributing for a short period of time or do not want to contact the author, you can fork the repository yourself, modify the code, and make sure to commit only one commit ahead of my latest repository, then submit a Pull request to the repository. I will personally review it and respond promptly.

如果你想要长期和作者（我）一起开发项目，并且你和我在现实世界很熟悉的话，你可以向我索要仓库权限，我可以给你放开。

If you want to develop the project with the author (me) for a long time, and you and I are familiar with each other in the real world, you can ask me for repository permissions and I can give them to you.

## 项目结构 / Project Structure

该仓库之后将会保持`main`和`release`两个分支，第一个分支是不稳定分支，我会根据我的空余时间会持续更新软件。后者是稳定发行版分支，到了我预定冻结功能和发行的时候，我会将`main`的修改合并到`release`分支。

The repository will maintain two branches, `main` and `release`. The first branch is an unstable branch, and I will continue to update the software according to my spare time. The latter is a stable release branch. When I plan to freeze the function and release it, I will merge the changes of `main` into the `release` branch.

项目目录结构详细分类如下：

The project directory structure is classified in detail as follows:

- `3rdparty`：使用的第三方库会放到这里面。/ The third-party libraries used will be placed here.
- `images`: 程序使用的图片资源。/ Image resources used by the program.
- `lang`: 程序的语言本地化文件，里面文件夹以国际标准化代码来指示一种语言支持。/ The language localization files of the program, the folders inside indicate a language support with international standardized codes.
- `mkinstaller`: 打包发行版工具，里面的文件夹以生成的文件为名表示支持的安装包类型。/ A distribution packaging tool. The folders in it are named after the generated files to indicate the supported installation package types.
- `src`: 软件源码。/ Software source code.
  - `class`: 所有的通用功能代码类。/ All common function code classes.
  - `control`: 控件相关代码。/ Control related code.
  - `dialog`: 界面类。/ Dialog class.
  - `model`: `Model-View`中使用的数据模型。/ The data model used in `Model-View`.
  - `plugin`: 插件相关的类。/ Plugin related classes.
  - `scriptaddon`: 为`AngelScript`提供额外功能的绑定类。/ Binding class that provides additional functionality to `AngelScript`.
  - `settings`: 设置相关代码。/ Codes for APP Settings.
  - `widgetframe`: 和无边框界面标题栏相关的代码。/ Code related to the borderless dialog title bar.
- `theme`: 软件主题文件，里面每个文件夹以主题名命名（必须英文）。/ Software theme files, each folder in it is named with the theme name (must be in English).

## 注意事项 / Cautions

您提供的代码将会遵守`AGPL-3.0`协议，第三方库将会遵守作者的开源协议。开源虽无国界，但您提供的代码来源请一定确保可靠性，不违反法律法规，不包含政治相关内容，不包含侵犯其他人或公司权益的代码。否则开源就是个笑话。

The code you provide will comply with the `AGPL-3.0` agreement, and the third-party library will comply with the author's open source agreement. Although open source has no borders, please ensure that the source of the code you provide is reliable, does not violate laws and regulations, does not contain political content, and does not contain code that infringes on the rights of other people or companies. Otherwise, open source is a joke.

最后，再次感谢您对开源的支持和帮助。

Finally, thank you again for your support and help for open source.
