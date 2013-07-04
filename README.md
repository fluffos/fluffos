FluffOS
=======

FluffOS is an collection of patches on top of last release of MudOS.

LPC runtime is compatible, if your mud runs on MudOSv22 +, it should run on FluffOS.

Current Release
---------------
The offical release is kept in the master branch, You can download a release archive through tags.

Current STABLE release: [2.27](https://github.com/fluffos/fluffos/archive/fluffos-2.27.tar.gz)

Current DEV branch: [next-3.0](https://github.com/fluffos/fluffos/tree/next-3.0), pull request should be sent to this branch.

Fetatures
---------
  * Backward compatible to MudOS. (Upgrade is easy!)
  * Lots of bug fixes. (including full CHECK_MEMORY enabled testsuite pass).
  * Compile on morden linux distros, 32bit & 64bit, CYGWIN support.
  * 64 bit LPC runtime, LPC int is always 64bit int, LPC float is C double.
  * MXP, GMCP, ZLIB, Websocket support (experimental). (optional) 
  * IPv6 support (optional)
  * ICONV support, code in UTF-8, dynamic transaltion on output (optional)
  * POSIX timer for better time precision. (optional)
  * More strict type checking (optional)
  * libevent integration, epoll backend.

To Build
--------
FLUFFOS 3.0 requires GCC 4.6+ or LLVM clang 2.9 + to compile, and uses c++11 features.

FLUFFOS 3.0 requires libevent 2.0+ to build and run.

Debian/Ubuntu:
```
$ sudo apt-get install libevent libevent-dev
```

CentOS:
  download and install libevent yourself through [libevent.org](http://libevent.org/).


Some efun packages also require addtional libraries, see options.h for detials.

To Build fluffos 3.0

```
$ cd src  
$ cp options.h local_options

<edit local_options to you need>

$ ./build.FluffOS
$ make
```

Copyright
---------
This game, LPmud, is copyright by Lars Pensj|, 1990, 1991.

Source code herein refers to the source code, and any executables
created from the same source code.

All rights reserved.  Permission is granted to extend and modify the
source code provided subject to the restriction that the source code may
not be used in any way whatsoever for monetary gain.

The name MudOS is copyright 1991-1992 by Erik Kay, Adam Beeman, Stephan Iannce
and John Garnett.  LPmud copyright restrictions still apply.

In addition, the entire package is copyright 1995 by Tim Hollebeek.

FluffOS is a range of patches to MudOS distributed as the patched source tree for convenience.
LPmud and MudOS copyright restrictions still apply.

ChangeLog
---------
See ChangeLog.fluffos-3.x file for details.

Contact
-------
  * http://www.lpmuds.net  [Board: Driver](http://lpmuds.net/smf/index.php?board=2.0)
  * Maintainer : sunyucong at gmail dot com
