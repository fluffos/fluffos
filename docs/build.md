---
layout: doc
title: Build
---

# Getting Started

[[toc]]

## Supported Environment

The best platform to build FluffOS is ubuntu 22.04+ (including WSL), and OSX latest, Windows on MSYS2/mingw64.

Compilers: FluffOS uses C++17 and C11, which requires at least GCC 7+ or LLVM clang 4+.

System Library Requirement (Must install):

1. ICU: FluffOS uses ICU for UTF-8 and transcoding support.
2. jemalloc: Release build use JEMALLOC by default, and is highly recommended in production.
3. OpenSSL (if PACAKGE_CRYPTO enabled)
4. PCRE (if PACKGAGE_PCRE enabled)
5. MysqlClient (if PACKAGE_DB enabled)

Bundled thirdparty library (no need to install):

1. Libevent 2.0+.
2. libtelnet: telnet protocol support
3. libwebsocket: websocket support.
4. ghc filesystem: polyfill for std::filesystem
5. backward-cpp: stacktrace
6. utf8_decoder_dfa: fast utf8 validation.
7. widecharwidth: wcwidth with unicode 11

## Ubuntu LTS

This is the best linux distro to build & run FluffOS, support for other distro is best effort only.

Build Steps

- Installing Dependencies
```shell
$ sudo apt update
$ sudo apt install build-essential bison libmysqlclient-dev libpcre3-dev libpq-dev \
libsqlite3-dev libssl-dev libz-dev libjemalloc-dev libicu-dev
```

- checkout git repo
```shell
$ git clone https://github.com/fluffos/fluffos.git
$ cd fluffos
$ git checkout master #(or an release tag)
```
- Upgrade your cmake
```shell
$ sudo pip install --upgrade cmake
```
- Build
```shell
$ mkdir build
$ cd build
$ cmake ..
$ make -j `nproc` install
```

- Find binary files and support files in ./bin/

## MacOS

Require OSX 10.15.

1. Install Homebrew, goto <https://brew.sh/> and follow instructions!
2. install libraries (checkout <https://github.com/fluffos/fluffos/blob/master/.github/workflows/ci-osx.yml> if you
 have issue)
```shell
$ brew install cmake pkg-config mysql pcre libgcrypt libevent openssl jemalloc icu4c
```
3.build same as under linux, you will need to pass two environment variables
```shell
$ mkdir build && cd build
$ OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/usr/local/opt/icu4c" cmake ..
$ make install
```

## Windows

Supported Environment: Windows 10 + MSYS2, the binary produced can run on Windows 7+.

checkout <https://github.com/fluffos/fluffos/blob/master/.github/workflows/ci-windows.yml> for most up-to-date commands.

First, you need to install MSYS2, from offical website here, pick X86_64 version if you are on 64 bit windows, i686 if you are on 32bit. (Note that it doesn’t matter what you compile into, FluffOS LPC VM is always 64bit! The only difference is that you can’t use more than 4G memory in 32bit.)

Next, you need to open msys2.exe and sync and update MSYS2 with pacman.

```shell
$ pacman -Syu
```

and after running this command you might need to close msys window and reopen, keep running that command until there is nothing to upgrade anymore.

Secondly, you want to install mingw64 devtoolchain, do this

```shell
$ pacman -S git mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
$ pacman -S mingw-w64-x86_64-zlib mingw-w64-x86_64-libevent mingw-w64-x86_64-pcre mingw-w64-x86_64-icu mingw-w64-x86_64-openssl
$ pacman -S bison
```

:::important
Once this is over, remember to close MSYS2 window, find MINGW64.exe in the same directory and open that instead. Very Important!
:::

Now, build things as usual!

```shell
$ mkdir build
$ cd build
$ cmake -G "MSYS Makefiles" -DPACKAGE_DB=OFF ..
$ make install
```
If you want PACKAGE_DB=ON , install additonal mysql package with

```shell
pacman -S mingw-w64-x86_64-libmariadbclient
```
Or if you want sqlite3

```shell
pacman -S mingw-w64-x86_64-sqlite3
```

## Packages

By default, driver have a default list of builtin packages to build.

If you want to turn off a package, run `cmake .. -DPACKAGE_XX=OFF -DPACAGE_YY=OFF`

## CPU compatibility

By default, driver built in release mode will optimize for running on current system CPU only. Copying driver to
another machine to run will generally not work!

if you need portable drivers, turn off MARCH_NATIVE as following.

```shell
$ cmake .. -DMARCH_NATIVE=OFF
```

## Static linking

you can pass -DSTATIC=ON to force driver to link static-ly for all libraries, this will only work in a specialized
 environment like alpine linux and Windows.

## Alpine Linux

Installing Dependencies

```shell
# Install all libs
$ apk add linux-headers gcc g++ clang-dev make cmake python2 bash \
    mariadb-dev mariadb-static postgresql-dev sqlite-dev sqlite-static\
    libevent-dev libevent-static libexecinfo-dev libexecinfo-static \
    openssl-dev openssl-libs-static zlib-dev zlib-static icu-dev icu-static \
    pcre-dev bison
```

Installing jemalloc

```shell
$ wget -O - https://github.com/jemalloc/jemalloc/releases/download/5.2.1/jemalloc-5.2.1.tar.bz2 | tar -xj && \
    cd jemalloc-5.2.1 && \
    ./configure --prefix=/usr && \
    make && \
    make install
```

Build Steps

```shell
$ git clone https://github.com/fluffos/fluffos.git
$ cd fluffos

$ mkdir build && cd build
$ cmake .. -DMARCH_NATIVE=OFF -DSTATIC=ON
$ make install
```

Check the result file to make sure it is a static file.

```shell
$ ldd bin/driver
    not a dynamic executable
```
