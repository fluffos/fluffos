FluffOS
=======

FluffOS is an collection of patches on top of last release of MudOS.

LPC runtime is compatible, if your mud runs on MudOSv22 +, it should run on FluffOS.

Release
-------
The offical release is kept in the master branch, You can download a release archive through tags.

Current release: [2.26.1](https://github.com/fluffos/fluffos/archive/fluffos-2.26.1.tar.gz)

Current next branch: next-2.27, pull request should be send to that branch.

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

To Build
--------
```
cd src
cp options.h local_options
./build.FluffOS
make
```

Contact
-------
  http://www.lpmuds.net
