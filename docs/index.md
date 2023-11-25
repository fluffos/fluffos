---
layout: doc
title: FluffOS
---

# FluffOS

FluffOS is a game engine, commonly used for building Muds, see [LPMud](https://en.wikipedia.org/wiki/LPMud). 
Started as a collection of patches to the last release of MudOS, FluffOS has gone through major rewrites and bugfixes
to support new features and also maintain backward compatibility with existing mudlibs.

FluffOS codebase contains three parts:

* LPC Interpreter: Parse/Execute LPC based program.
* Network server: Manage client connections and expose to LPC program. Supports TELNET, TLS and websocket protocol.
* Glue Interface 
  * EFUN: predefined functions for LPC program to invoke C/C++ functions.
  * Apply: predefined callback functions for LPC to handle various Game events.   

## Current Release

[ALL RELEASES](https://github.com/fluffos/fluffos/releases)

* Current Release: (branch master), cmake build system, supports ubuntu, OSX, and Windows(MSYS2/mingw64).

## Features

- Build on Ubuntu, OSX, and Windows natively with cmake!
- UTF-8 native, with transparent input/output encoding support. (Emojis!)
- Websocket support!
- TLS support!
- SHA512 crypt by default.

## Features previously released

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

## Demo

    // TODO

## License & Copyright

> Copyright 2019-2023 Yucong Sun

> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
> documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
> rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
> persons to whom the Software is furnished to do so, subject to the following conditions:
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
> Software.
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
> WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
> COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
> OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Historical 

### License & Copyright

please see <https://github.com/fluffos/fluffos/blob/master/Copyright>

### ChangeLog

[ChangeLog.fluffos-3.x](https://github.com/fluffos/fluffos/blob/next-3.0/ChangeLog.fluffos-3.x) for details.
