---
layout: default
title: Welcome
---

FluffOS is the next generation of MudOS. It starts as an collection of patches on top of last release of MudOS, comes with many new features and bug fixes.

LPC runtime is mostly compatible, if your mud runs on MudOS v22+, it should be relatively easy to run on FluffOS. (contact us for help). The goal is to migrate everyone!

## Current Release

[ALL RELEASES](https://github.com/fluffos/fluffos/releases)

v2017: Current release (branch v2017), support ubuntu 16.04, centos 7, cygwin, using autoconf build system, critical bugfix only.
 
v2019: Currently in development (branch master), cmake build system, will support ubuntu 18.04, OSX 10.15, and MSVC 2017.

## ChangeLog

[ChangeLog.fluffos-3.x](https://github.com/fluffos/fluffos/blob/next-3.0/ChangeLog.fluffos-3.x) for details.

## Features

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

for old code please see https://github.com/fluffos/fluffos/blob/master/Copyright

For all code wrote by Yucong Sun:

Copyright 2019 Yucong Sun

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
