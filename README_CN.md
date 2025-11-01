[![CI 状态](https://github.com/fluffos/fluffos/actions/workflows/ci.yml/badge.svg)](https://github.com/fluffos/fluffos/actions)
[![CI 状态](https://github.com/fluffos/fluffos/actions/workflows/ci-osx.yml/badge.svg)](https://github.com/fluffos/fluffos/actions)
[![CI 状态](https://github.com/fluffos/fluffos/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/fluffos/fluffos/actions)

[![Open Collective 支持者](https://opencollective.com/fluffos-579/backers/badge.svg)](#支持者)
[![Open Collective 赞助商](https://opencollective.com/fluffos-579/sponsors/badge.svg)](#赞助商)

欢迎使用
=======

FluffOS 是一个 LPMUD 驱动程序，基于 MudOS 的最后一个版本 (v22.2b14)，包含了 10 多年的错误修复和性能增强，并提供持续的支持。

FluffOS 支持所有基于 LPC 的 MUD，代码改动非常小。如果您目前仍在使用 MudOS，现在绝对是升级的好时机！

建议使用 GitHub 上的最新版本或主分支，之前的版本（特别是 v2017）不再受支持。

构建
-----
FluffOS 的主要平台是 Ubuntu 22.04 LTS，但也支持最新的 OSX 和 Windows (MSYS2/mingw64)，使用 Ubuntu 22.04 的 WSL 也能正常工作。

构建系统：cmake (3.20)

特性
--------
### FluffOS v2017 以来的 LPC 语言特性
- LPC 字符串操作支持 UTF-8 EGS，范围操作符支持 emoji 和其他 Unicode 字符。
- 各种新的 EFUN 用于透明的输入/输出转码。

### 自 MudOS 以来的 LPC 语言特性
- 请阅读 docs/archive/ChangeLog.* 文件了解详情

### 驱动运行时特性
- Jemalloc 支持
- 默认使用 SHA512 加密
- LPC 追踪
- MySQL、PostgreSQL、SQLite 集成
- 异步 IO 操作
- 外部程序集成

### 网络特性
- TLS 支持
- WebSocket 协议支持（附带一个 web 客户端的最小示例）

### LPC 标准库
- 参见 testsuite/std 目录下的文件。

### 其他特性
- 所有 EFUN 的测试套件
- 详细的内存统计（通过 mud_status(1) EFUN）
- 内存泄漏检测

V2017 版本
-----
v2017 是旧版，使用 autoconf 构建系统，支持在 CentOS/Ubuntu 和 Windows（使用 CYGWIN）上编译。此版本不再受支持，仅出于历史兴趣保留。

所有之前的 MudOS 和 FluffOS 版本也以标签形式保留在代码库中作为历史参考。

支持
-------
- 网站/文档：https://www.fluffos.info
- Discord 支持：LPC 的 #fluffos 频道 https://discord.gg/2a6qqG6Cdj
- 论坛：https://forum.fluffos.info
- QQ 支持群：451819151 [![451819151](https://pub.idqqimg.com/wpa/images/group.png)](https://shang.qq.com/wpa/qunwpa?idkey=3fd552adb8ace1a8e3ae3a712e5d314c7caf49af8b87449473c595b7e1f1ddf9)

如何构建
------------
参见 <https://www.fluffos.info/build.html>

捆绑的第三方依赖
----------------------
- libwebsockets: <https://libwebsockets.org/>
- libevent: <https://libwebsockets.org/>
- backward-cpp: <https://github.com/bombela/backward-cpp>
- musl 的 crypt: <https://www.musl-libc.org/>
- ghc::filesystem: <https://github.com/gulrak/filesystem>
- nlohmann::json: <https://github.com/nlohmann/json>
- scope_guard: <https://github.com/Neargye/scope_guard>
- utfcpp: <https://github.com/nemtrif/utfcpp>
- utf8_decoder: <http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>
- libtelnet，基于 <https://github.com/seanmiddleditch/libtelnet> 并做了本地修改

非捆绑的平台依赖包括：libevent、ICU4C、OpenSSL、Zlib 等。

使用 FluffOS 的项目
----------------------
[添加您自己的项目](https://github.com/fluffos/fluffos/edit/master/README.md)

- 几乎所有中文 MUD
- [ThresholdRPG](https://wiki.thresholdrpg.com/)
- [SWmud](http://www.swmud.org/)
- [Merentha](https://www.merentha.com/)

捐赠
---------
我要特别感谢所有赞助商和贡献者给予的支持。
所有捐赠 100% 用于购买 FluffOS 开发和网站及论坛托管的工具、设备和托管费用。

列表按收到捐赠的时间降序排列。

### 2019 年 1 月收到

- 逍遥山人, qq1102907881
- lostsnow
- 小瓶盖
- 星星 qq 55833173
- 胜华 gon***@126.com

### 2018 年 11 月收到

- felchoin@sjever
- 朝亮 wuc***@163.com
- 羽天邪
- 风清扬 832***@qq.com
- Mok say***@139.com
- 春龙 lon***@gmail.com
- 碎梦 lih***@163.com

## 贡献者

这个项目得以存在要感谢所有贡献者。
<a href="https://github.com/fluffos/fluffos/graphs/contributors"><img src="https://opencollective.com/fluffos-579/contributors.svg?width=890&button=false" /></a>

## 支持者

感谢所有的支持者！🙏 [成为支持者](https://opencollective.com/fluffos-579#backer)

<a href="https://opencollective.com/fluffos-579#backers" target="_blank"><img src="https://opencollective.com/fluffos-579/backers.svg?width=890"></a>

## 赞助商

通过成为赞助商来支持这个项目。您的标志将显示在这里并链接到您的网站。[成为赞助商](https://opencollective.com/fluffos-579#sponsor)

<a href="https://opencollective.com/fluffos-579/sponsor/0/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/0/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/1/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/1/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/2/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/2/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/3/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/3/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/4/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/4/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/5/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/5/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/6/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/6/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/7/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/7/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/8/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/8/avatar.svg"></a>
<a href="https://opencollective.com/fluffos-579/sponsor/9/website" target="_blank"><img src="https://opencollective.com/fluffos-579/sponsor/9/avatar.svg"></a>