> 如果是你中文用户，请[到此](README.md)

- [Contributors](#contributors)
- [Credit](#credit)
- [Contributing](#contributing)
- [Status](#status)
- [WingHexExplorer2](#winghexexplorer2)
  - [License](#license)
  - [Usage Statement](#usage-statement)
  - [For Packagers](#for-packagers)
  - [Issue](#issue)
- [Screenshot](#screenshot)
- [Donate](#donate)
- [Related Repository](#related-repository)
- [WIKI](#wiki)
- [Plugins](#plugins)

---

<h1 align="center"> WingSummer.WingHexExplorer2</h1>

<p align="center">
<img alt="WingHexExplorer2" src="images/icon.png" /> 
<p align="center">WingHexExplorer2</p>
</p>

<p align="center">
<img alt="Author" src="authorband.svg" />
<img alt="License" src="licenseband.svg" />
</p>

<p align="center">
<img src="https://github.com/Wing-summer/WingHexExplorer2/actions/workflows/clang-format-check.yml/badge.svg" />
<img src="https://github.com/Wing-summer/WingHexExplorer2/actions/workflows/cmake-format-check.yml/badge.svg" />
</p>

- Not easy to open source, please Star or [Donate](#donate)

## Contributors

Maintaining a useful tool is not something that can be done by one person. What is more important is that everyone maintains it together. The following are the people who have contributed to the code of this repository. Special thanks:

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

## Credit

Thanks to the sponsorship of the following people, I will try my best to continue to make a user-friendly, powerful and cross-platform PC-side hexadecimal editor (in chronological order):

| Sponsor |       Comment        |
| :-----: | :------------------: |
|   *欢   | a fellow from Deepin |

## Contributing

- If you want to participate in the development of this software code, please contact me in the pull request.
- This project supports donations. If you are willing, please go to this repository through WeChat or Alipay. If you don't have any of these, you can use Afadian that supports Paypal. The price of a bottle of water is enough to increase my enthusiasm for maintaining the project. Thank you for your support.
- If you want to submit code to fix or improve the program, please submit it in the pull request.
- Any comrades and sponsors who successfully participate in code bug fixes and improve program functions will be reflected in the ReadMe and attached documentation of this repository. If you are one of them, I can explain it according to your reasonable wishes.

\> **Joining us does not mean code maintenance, you can choose one or more of the following to participate**

- Code maintenance: implement new features or fix bugs, maintain and upgrade the code.
- Document editing: mainly interface documents and tutorials need to be written and edited, which is very important.
- Participate in discussions: mainly discuss the future development and direction of this project.
- Write plug-ins: work together to enhance the functionality of the software.

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for getting started with the contribution.

Make sure that you follow [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) while contributing and engaging in the discussions.

## Status

> For Qt 5, only version 5.15.2 or higher is supported, and for Qt 6, only version 6.6.2 or higher is supported. Other lower versions are not supported due to missing APIs of key libraries and some bugs of their own.

And also:

> Active compilation support for QT5 will be discontinued on 2025/05/26, and the related GitAction automated tests will be revoked at that time. If you need QT5 version for special purposes, please consult me ​​for commercial cooperation and paid technical support.

The repository code compiles with Qt 5.15.2 and 6.6.2 on the latest versions of Windows and Ubuntu: ![Status](https://github.com/Wing-summer/WingHexExplorer2/actions/workflows/qt-build-test.yml/badge.svg)

## WingHexExplorer2

This software is a hexadecimal editor written in QT and developed in C++, providing a powerful and free hexadecimal editor. The ultimate goal of this project is to create a powerful, lightweight, and cross-platform (only for PC) hex editor. Of course, I cannot achieve this goal by myself, which is one of the purposes of my open source. I only maintain the operating system platform I use, and only respond to the bugs that I can fix on the platform I use in my free time. More details will be seen in my [CNBlog](https://www.cnblogs.com/wingsummer/p/18286419) .

### License

This software complies with the `AGPL-3.0` agreement. Please do not use it for purposes other than this agreement. Selfishness knows no bound. I don't want to be involved in a lot of energy because of copyright issues.

If you want to use the code of this software for closed-source commercial code and want to lift the restriction of the `GPL` series that it must be open source, please consult me ​​in person to discuss commercial licensing matters.

For plugin development, the corresponding open source agreements are different. Only `src/plugin/iwingplugin.h`, `src/plugin/iwingpluginbase.h`, `src/plugin/iwingdevice.h` and `src/plugin/settingpage.h` in this repository comply with the `BSD 3-Clause` agreement to allow closed-source and commercial development. The code of `TestPlugin` in this repository (except the file `TranslationUtils.cmake` which complies with `BSD 3-Clause`) complies with the `MIT` agreement.

### Usage Statement

1. The source code of this software shall not be used for closed-source commercial purposes unless you open source it completely (as required by the GPL agreement). If you want to commercially close the code of the software warehouse, you must contact me to purchase a commercial license and sign a contract. Please contact me for the price. Please do not disturb me if you are not serious.
2. This software was written in my spare time. Please forgive me for not being able to fix bugs or provide technical support in time.
3. I am not a computer major, and there are inevitably bugs in the program I write. Welcome to submit PR.
4. Whenever you have questions, please read the Wiki in detail.

### For Packagers

First of all, I would like to express my sincere tanks for your enthusiastic help. As a free software, you can package and distribute it through legal channels without informing me under the condition of complying with the open source license and spirit. You can include your own information in the packaging information, but you must abide by the following agreements with me as the author of the software:

1. The software package must contain the files `LICENSE`, `authorband.svg`, `licenseband.svg`, `screenshot.png` and `README.md`, and they must be placed in the same directory as the main program. These files are in the root directory of the project, just copy them.
2. Unless it is stated in advance that the software is modified, the package must not contain the main program compiled from the modified source code.
3. The package must not contain content that infringes on the legitimate rights and interests of others, such as adding unauthorized paid content or computer viruses to it.
4. It is best to contain the checksum of the main program in the package, but it is not mandatory.
5. Regardless of the packaging format, its open source agreement and free attributes must be stated, and it is not allowed to hide and charge for it. Although I don't mind charging, the download server still needs money to maintain, but I hate charging by taking advantage of the information gap.
6. Regardless of the purpose, if you do not obtain the authorization of me, you may not modify any network links pointed to and the ABOUT contents in the program, such as sponsorship and content about the software.
7. You may not insert any plug-in containing commercial promotion in the installation program, and strictly abide by the content of the second article.

### Issue

If you have any suggestions, please be sure to read the following statement before submitting an issue to avoid wasting our precious time:

1. If you find that the software does not provide support for your language, you can add it through pull request. Note that the translated text in the code is just an identifier, it may not be the real text, please refer to my Chinese to confirm the translation.
2. For updates, major version plug-ins are incompatible. If there are any problems with the plug-in, please contact the developer (it is not ruled out that I will develop some plug-ins, so you can contact me directly).

All of the above, if you are a like-minded open source contributor, you are welcome to fork my repository for corresponding maintenance!

If a serious bug occurs, I may not respond in time, and I refuse any gossip from the moral high ground.

Without standing on the shoulders of giants, it will not have powerful and stable functions.**The source code of the open source basic projects used in this software that have not been modified by me all comply with the open source agreement of the original author; The parts that I have modified, unless otherwise specified, all comply with the agreement of this repository.**

## Screenshot

<p align="center">
<img alt="Screenshot" src="screenshot.png">
<p align="center">WingHexExplorer2</p>
</p>

## Donate

> If you are not Chinese, you can use [Afadian](https://afdian.com/a/wingsummer) to support me.

**<p align="center">Your every support will be a powerful driving force for the advancement of this project. Thank you very much for your support</p>**

<p align="center">

<img alt="Donate" src="Donate.jpg" />
<p align="center">Thanks for your support</p>

</p>

## Related Repository

The official repository: <https://github.com/Wing-summer/WingHexExplorer2> 。

Of course, there are other repositories as mirror for Chinese users (which will be updated later):

- Gitee：<https://gitee.com/wing-cloud/WingHexExplorer2>
- Gitlink：<https://www.gitlink.org.cn/wingsummer/WingHexExplorer2>
- GitCode：I don't consider it, because of "stealing code".

## WIKI

&emsp;&emsp;If you want to learn how to use WingHexEditor and how to develop plug-ins for the software, please go to this link: `Not available yet`. At the same time, you are also welcome to point out errors in the Wiki and contribute high-quality content.

## Plugins

- [WingAsm](https://github.com/Wing-summer/WingAsm) : A plugin that provides assembly and disassembly features. Author: **wingsummer**. License: **APGL-v3.0**.
