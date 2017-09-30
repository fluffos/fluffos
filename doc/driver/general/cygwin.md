FluffOS 3.0 fully support compiling under CYGWIN 64, thus you can run it on Windows.

Please use https://cygwin.com/install.html to download and install CYGWIN 64.

These packages must be installed:
wget zlib-devel make gcc-g++ libpcre-devel libiconv-devel libevent-devel bison autoconf automake

You can use offical CYGWIN graphical installer to select and install these packages. You can also use apt-cyg. (link)

NOTE: on 2017-09-28 , binutils 2.28 in CYGWIN can not successfully build the driver. You can work around this issue by
installing 2.25

NOTE: jemalloc will not work on CYGWIN, windows version is thus recommended for small servers only.
