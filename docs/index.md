---
layout: default
title: FluffOS Official website
---

FluffOS is the next generation of MudOS. It starts as an collection of patches on top of last release of MudOS. It has gone through major rewrites and bugfixes and maintained backward compatibility with existing mudlibs.

I'm always eager to help people migrate to FluffOS, if you have an working mud that you want to migrate (hack, even if you just have bunch of files laying around, send it over nonetheless!), send to me and I will make it run on FluffOS! Feel free to contact me for help. The goal is to migrate everyone!

## Current Release

[ALL RELEASES](https://github.com/fluffos/fluffos/releases)

* v2019: Current Release (branch master), cmake build system, supports ubuntu 18.04, OSX 10.15, and MSYS2/mingw64

* v2017: historical release (branch v2017), bugfix only, support ubuntu 16.04, centos 7, cygwin, using autoconf build
 system, no more support.

## ChangeLog

[ChangeLog.fluffos-3.x](https://github.com/fluffos/fluffos/blob/next-3.0/ChangeLog.fluffos-3.x) for details.

## v2019 Features
- Builds on Ubuntu, OSX, and Windows natively with cmake!
- UTF-8 native, with transparent input/output encoding support.
- Native Websocket support!
- SHA512 crypt by default.

## Demo

    // TODO

## v2017 Features

- Mostly backward-compatible to MudOS. (Upgrade should be easy!)
- Lots of bug fixes. (including full CHECK_MEMORY enabled testsuite pass).
- Compile on modern linux distros, 32bit & 64bit, CYGWIN support.
- 64 bit LPC runtime, LPC int is always 64bit int, LPC float is C double.
- MXP, GMCP, ZLIB.
- IPv6 support (optional)
- ICONV support, code in UTF-8, dynamic translation on output (optional)
- POSIX timer for better time precision. (optional)
- More strict type checking (optional)
- libevent integration, epoll backend.

## License & Copyright

> Copyright 2019-2020 Yucong Sun

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Historical License

please see <https://github.com/fluffos/fluffos/blob/master/Copyright>
