---
layout: default
title: Welcome
---

FluffOS is the next generation of MudOS. It starts as an collection of patches on top of last release of MudOS, comes with many new features and bug fixes.

LPC runtime is mostly compatible, if your mud runs on MudOS v22+, it should be relatively easy to run on FluffOS. (contact us for help). The goal is to migrate everyone!

## Current Release

[LINK](https://github.com/fluffos/fluffos/releases)

## ChangeLog

See [ChangeLog.fluffos-3.x](https://github.com/fluffos/fluffos/blob/next-3.0/ChangeLog.fluffos-3.x) for details.

## Features

- Mostly backward-compatible to MudOS. (Upgrade should be easy!)
- Lots of bug fixes. (including full CHECK_MEMORY enabled testsuite pass).
- Compile on modern linux distros, 32bit & 64bit, CYGWIN support.
- 64 bit LPC runtime, LPC int is always 64bit int, LPC float is C double.
- MXP, GMCP, ZLIB, Websocket support (experimental). (optional)
- IPv6 support (optional)
- ICONV support, code in UTF-8, dynamic translation on output (optional)
- POSIX timer for better time precision. (optional)
- More strict type checking (optional)
- libevent integration, epoll backend.

## License

This game, LPmud, is copyright by Lars Pensj|, 1990, 1991.

Source code herein refers to the source code, and any executables
created from the same source code.

All rights reserved. Permission is granted to extend and modify the
source code provided subject to the restriction that the source code may
not be used in any way whatsoever for monetary gain.

The name MudOS is copyright 1991-1992 by Erik Kay, Adam Beeman, Stephan Iannce
and John Garnett. LPmud copyright restrictions still apply.

In addition, the entire package is copyright 1995 by Tim Hollebeek.

FluffOS is a range of patches to MudOS distributed as the patched source tree for convenience. LPmud and MudOS copyright restrictions still apply.
