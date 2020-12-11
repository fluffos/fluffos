---
layout: default
title: Build
---

## Supported Environment

v2019 supports building on ubuntu 18.04+ (including WSL), and OSX latest, Windows on MSYS2/mingw64.

Compilers: FluffOS v2019 uses C++17 and C11, which requires at least GCC 7+ or LLVM clang 4+.

System Library Requirement (Must install):
1. Libevent 2.0+.
1. ICU: FluffOS uses ICU for UTF-8 and transcoding support.
1. jemalloc: Release build use JEMALLOC by default, and is highly recommended in production.
1. OpenSSL (if PACAKGE_CRYPTO enabled)
1. PCRE (if PACKGAGE_PCRE enabled)
1. MysqlClient (if PACKAGE_DB enabled)

Bundled thirdparty library (no need to install):
1. libtelnet: telnet protocol support
1. libwebsocket: websocket support.
1. ghc filesystem: polyfill for std::filesystem
1. backward-cpp: stacktrace
1. utf8_decoder_dfa: fast utf8 validation.
1. widecharwidth: wcwidth with unicode 11

## Ubuntu 18.04 LTS

This is the best linux distro to build & run FluffOS, support for other distro is best effort only.

Installing Dependencies

```bash
# Install all libs
$ sudo apt update
$ sudo apt install build-essential bison libevent-dev libmysqlclient-dev libpcre3-dev libpq-dev \
libsqlite3-dev libssl-dev libz-dev libjemalloc-dev libicu-dev
```

Build Steps

- checkout git repo
```shell
$ git clone https://github.com/fluffos/fluffos.git
$ cd fluffos
$ git checkout v2019 #(or an v2019 release tag)
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
$ make -j4 install
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

3. build same as under linux, you will need to pass two environment variables
```shell
$ make build && cd build
$ OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/usr/local/opt/icu4c" cmake ..
$ make install
```

## Windows

Supported Environment: Windows 10 + MSYS2, the binary produced can run on Windows 7+.

see <https://forum.fluffos.info/t/compiling-fluffos-v2019-under-osx-windows-msys2-mingw64/601>

checkout <https://github.com/fluffos/fluffos/blob/master/.github/workflows/ci-windows.yml> if you have issue.

## Packages

By default driver have an default list of builtin packages to build.

If you want to turn off an package, run `cmake .. -DPACKAGE_XX=OFF -DPACAGE_YY=OFF`

## CPU compatibility

By default driver built in release mode will optimize for running on current system CPU only. Copying driver to
another machine to run will generally not work!

if you need portable drivers, turn off MARCH_NATIVE as following.

```shell
$ cmake .. -DMARCH_NATIVE=OFF
```

## Static linking

you can pass -DSTATIC=ON to force driver to link staticly for all libraries, this will only work in a specialized
 environment like alpine linux and Windows. Check the result file to make sure it is an static file.

```shell
$ ldd bin/driver
    not a dynamic executable
```
