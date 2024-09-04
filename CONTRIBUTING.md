## 如何参与该项目 / How to CONTRIBUTE

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

### 递交注释格式 / Commit Message Format

每条提交消息都由**标题（header）**、**正文（body）**和**页脚（footer）**组成。标题具有特殊格式，其中包括**类型（type）**、**范围（scope）**和**主题（subject）**：

Each commit message consists of a **header**, a **body** and a **footer**.  The header has a special format that includes a **type**, a **scope** and a **subject**:

```bash
<type>(<scope>): <subject>
<BLANK LINE>
<body>
<BLANK LINE>
<footer>
```

**标题（header）** 是强制性的，而 **范围（scope）** 是可选的。

The **header** is mandatory and the **scope** of the header is optional.

提交消息的任何一行都不能超过 100 个字符！这使得消息在 GitHub 以及各种 git 工具中更容易阅读。

Any line of the commit message cannot be longer 100 characters! This allows the message to be easier
to read on GitHub as well as in various git tools.

如果 **正文（body）** 存在， **空行（BLANK LINE）** 是必须的。

If **body** is present, **BLANK LINE** is required.

如果有的话，页脚（footer）应包含 [将拉取请求链接到议题](https://help.github.com/articles/closing-issues-via-commit-messages/)。

The footer should contain a [closing reference to an issue](https://help.github.com/articles/closing-issues-via-commit-messages/) if any.

举个例子：

Samples:

```bash
docs(changelog): update changelog to beta.5
```

```bash
fix(release): need to depend on latest qt5

The version in our package.json gets copied to the one we publish, and users need the latest of these.
```

### 回退分支 / Revert

如果提交撤销了之前的提交，则它应该以`revert:`开头，后跟撤销的提交的标题。在正文中，它应该写着：`这将撤销提交 <hash>`，其中哈希是被撤销的提交的 SHA。

If the commit reverts a previous commit, it should begin with `revert:`, followed by the header of the reverted commit. In the body it should say: `This reverts commit <hash>`, where the hash is the SHA of the commit being reverted.

### 类型 / Type

必须是下列之一：

Must be one of the following:

- **build**: 影响构建系统或外部依赖项的更改 / Changes that affect the build system or external dependencies
- **ci**: 更改 CI 配置文件和脚本 / Changes to our CI configuration files and scripts
- **docs**: 仅文档更改
- **feat**: 新功能 / A new feature
- **fix**: 缺陷修复 / A bug fix
- **perf**: 修改代码得到性能提升 / A code change that improves performance
- **refactor**: 代码更改既没有修复错误也没有添加功能 / A code change that neither fixes a bug nor adds a feature
- **style**: 不影响代码含义的更改（空格、格式、缺少分号等） / Changes that do not affect the meaning of the code (white-space, formatting, missing semi-colons, etc)
- **test**: 添加缺失的测试或更正现有的测试 / Adding missing tests or correcting existing tests

### 范围 / Scope

范围是可选的，它通常会更加详细的指示了影响的范围，可以是分支名，也可以是版本号（比如 v1.0.0），也可以是如下几种类型：

The scope is optional. It usually indicates the scope of impact in more detail. It can be a branch name, a version number(e.g. v1.0.0), or the following types:

- **changelog**: 用于更新 CHANGELOG.md 中的发布说明 / used for updating the release notes in CHANGELOG.md
- **readme**: 用于更新 README 中的内容 / used for updating the contents of README
- **license**: 用于更新 LICENSE 中的内容 / used for updating the contents of LICENSE

### 主题 / Subject

主题包含对变更的简洁描述。

The subject contains a succinct description of the change.

### 正文 / Body

正文应该包括改变的目的并将其与之前的行为进行对比。如果在一次提交中有多种类型的修改内容，也可以按照标题的形式写到 **正文（body）** 里面。

The body should include the motivation for the change and contrast this with previous behavior. If there are multiple types of changes in a commit, they can also be written in the **body** in the form of titles.

### 脚注 / Footer

页脚应包含有关**重大变更**的任何信息，也是引用此提交**关闭**的 GitHub 问题的地方。

The footer should contain any information about **Breaking Changes** and is also the place to reference GitHub issues that this commit **Closes**.

**重大变更** 应以单词 `BREAKING CHANGE:` 开头，中间有一个空格或两个换行符。提交消息的其余部分将用于此目的。

**Breaking Changes** should start with the word `BREAKING CHANGE:` with a space or two newlines. The rest of the commit message is then used for this.

## 注意事项 / Cautions

您提供的代码将会遵守`AGPL-3.0`协议，第三方库将会遵守作者的开源协议。开源虽无国界，但您提供的代码来源请一定确保可靠性，不违反法律法规，不包含政治相关内容，不包含侵犯其他人或公司合法权益的代码。否则开源就是个笑话。**当你向仓库提交任何形式的代码，默认表示同意以上声明，且自愿承担因为自己的代码提交的一切责任。**

The code you provide will comply with the `AGPL-3.0` agreement, and the third-party library will comply with the author's open source agreement. Although open source has no borders, please ensure that the source of the code you provide is reliable, does not violate laws and regulations, does not contain political content, and does not contain code that infringes on the legal rights of other people or companies. Otherwise, open source is a joke. **When you submit any form of code to the repository, it means that you agree to the above statement and voluntarily assume all responsibilities for your code submission.**

所有的 PR 合并都需要通过 CI 检查，请注意你的**代码格式化规范**和代码提交说明是否遵循**最基础的约定式提交标准**。

All PR merges need to pass CI checks. Please pay attention to whether your **code formatting specifications** and code submission instructions follow the **most basic conventional submission standards**.

最后，再次感谢您对开源的支持和帮助。

Finally, thank you again for your support and help for open source.
