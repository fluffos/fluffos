[![CI Status](https://github.com/fluffos/fluffos/workflows/CI%20on%20ubuntu/badge.svg)](https://github.com/fluffos/fluffos/actions)
[![CI Status](https://github.com/fluffos/fluffos/workflows/CI%20on%20OSX/badge.svg)](https://github.com/fluffos/fluffos/actions)
[![CI Status](https://github.com/fluffos/fluffos/workflows/CI%20on%20Windows/badge.svg)](https://github.com/fluffos/fluffos/actions)


[![Backers on Open Collective](https://opencollective.com/fluffos-579/backers/badge.svg)](#backers)
[![Sponsors on Open Collective](https://opencollective.com/fluffos-579/sponsors/badge.svg)](#sponsors)

Welcome
=======
FluffOS is an LPMUD driver, based on the last release of MudOS (v22.2b14), includes 10+ years of bug
 fixes and performance enhancement, with active support.

FluffOS supports all LPC based mud with very little code changes. if you are currently still using MudOS somehow
 it is definitely time to upgrade!

It is recommended to use the latest release on Github or the master branch, previous versions (v2017 in particular) are
not supported.

Build
-----
FluffOS's main platform is ubuntu 22.04 LTS, but also works on latest OSX and Windows (MSYS2/mingw64), WSL with
ubuntu 22.04 works fine too.

Build system: cmake (3.20)

Features
--------
LPC Language since FluffOS v2017
  - LPC string operations are UTF-8 EGS aware, range operator supports emoji and other unicode characters.
  - various new EFUNS for transparent input/output transcoding.

LPC Language since MudOS
  - Please read docs/archive/ChangeLog.* files for details

Driver Runtime
  - Jemalloc support
  - SHA512 crypt by default.
  - LPC Tracing
  - Mysql, Postgresql, SQLLite integration
  - Async IO operations
  - External program integration

Networking
  - TLS support
  - Websocket protocol support (with a minimal example for a webclient)

LPC Standard Library
  - see files under testsuite/std.

Misc
  - Testsuite for all EFUNS
  - Detailed memory accounting (through mud_status(1) EFUN)
  - Memory leak detection

V2017
-----
v2017 is the legacy version, with an autoconf based build system, it supports compiling on centOS/ubuntu
and under windows using CYGWIN. This release is no longer supported, it is kept only for historical interest now.

All previous MudOS and FluffOS releases are also kept in the code base as tags for historical reference.

Support
-------
Website / Documentation: <https://www.fluffos.info>
Discord Support: #fluffos on https://discord.gg/2a6qqG6Cdj
Forum: <https://forum.fluffos.info>
QQ support Group: 451819151 [![451819151](https://pub.idqqimg.com/wpa/images/group.png)](https://shang.qq.com/wpa/qunwpa?idkey=3fd552adb8ace1a8e3ae3a712e5d314c7caf49af8b87449473c595b7e1f1ddf9)

How to Build
------------
see <https://www.fluffos.info/build.html>

Bundled Third-party Dependencies
----------------------
- libwebsockets: <https://libwebsockets.org/>
- libevent: <https://libwebsockets.org/>
- backward-cpp <https://github.com/bombela/backward-cpp>
- crypt from musl: <https://www.musl-libc.org/>
- ghc::filesystem <https://github.com/gulrak/filesystem>
- nlohmann::json <https://github.com/nlohmann/json>
- scope_guard <https://github.com/Neargye/scope_guard>
- utfcpp <https://github.com/nemtrif/utfcpp>
- utf8_decoder <http://bjoern.hoehrmann.de/utf-8/decoder/dfa/>
- libtelnet, based on <https://github.com/seanmiddleditch/libtelnet> with local modifications

Non-bundled platform dependencies includes: libevent, ICU4C, OpenSSL, Zlib etc.

Projects Using FluffOS
----------------------
[Add Your Own](https://github.com/fluffos/fluffos/edit/master/README.md)

- Practically all Chinese
- [ThresholdRPG](https://wiki.thresholdrpg.com/)
-   [SWmud](http://www.swmud.org/)

Donations
---------
I would like to personal thank all the sponsors and contributors for showing their support.
All donations are 100% used towards purchasing tools, equipments and hosting cost for FluffOS development and website
 and forum hosting.

The list is in descending order by time donation received.

##### Received in 2019 Jan

- 逍遥山人, qq1102907881
- lostsnow
- 小瓶盖
- 星星 qq 55833173
- 胜华 gon***@126.com

#### Received in 2018 Nov

- felchoin@sjever
- 朝亮 wuc***@163.com
- 羽天邪
- 风清扬 832***@qq.com
- Mok say***@139.com
- 春龙 lon***@gmail.com
- 碎梦 lih***@163.com

## Contributors

This project exists thanks to all the people who contribute.
<a href="https://github.com/fluffos/fluffos/graphs/contributors"><img src="https://opencollective.com/fluffos-579/contributors.svg?width=890&button=false" /></a>

## Backers

Thank you to all our backers! 🙏 [[Become a backer](https://opencollective.com/fluffos-579#backer)]

<a href="https://opencollective.com/fluffos-579#backers" target="_blank"><img src="https://opencollective.com/fluffos-579/backers.svg?width=890"></a>

## Sponsors

Support this project by becoming a sponsor. Your logo will show up here with a link to your website. [[Become a sponsor](https://opencollective.com/fluffos-579#sponsor)]

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
