---
layout: default
title: Welcome
---

FluffOS is the next generation of MudOS. It starts as an collection of patches on top of last release of MudOS, comes with many new features and bug fixes.

LPC runtime is mostly compatible, if your mud runs on MudOS v22+, it should be relatively easy to run on FluffOS. (contact us for help). The goal is to migrate everyone!

## Current Release

[ALL RELEASES](https://github.com/fluffos/fluffos/releases)

In 2019, there will be 2 release track maintained:

v2017: no build system upgrade, no new features, critical bugfix only.
 
v2019: cmake build system, JS interop support and rolling updates.

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

see https://github.com/fluffos/fluffos/blob/master/Copyright
