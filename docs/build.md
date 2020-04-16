---
layout: default
title: Build
---

## Environment

v2019 supports building on ubuntu 18.04+ (including WSL), and OSX latest, Windows on MSYS2/mingw64.

Compilers: FluffOS v2019 uses C++17 and C11, which requires at least GCC 7+ or LLVM clang 4+.

System Library Requirement:
1. Libevent 2.0+.
1. ICU: FluffOS uses ICU for UTF-8 and transcoding support.
1. jemalloc: Release build use JEMALLOC by default, and is highly recommended in production.
1. OpenSSL (if PACAKGE_CRYPTO enabled)
1. PCRE (if PACKGAGE_PCRE enabled)
1. MysqlClient (if PACKAGE_DB enabled)

Bundled thirdparty library:
1. libtelnet: telnet protocol support
1. libwebsocket: websocket support.
1. ghc filesystem: polyfill for std::filesystem
1. backward-cpp: stacktrace
1. utf8_decoder_dfa: fast utf8 validation.
1. widecharwidth: wcwidth with unicode 11

## BUILD (v2019)

Ubuntu 18.04

This is the best platform to build & run FluffOS, support for other platform is best effort only.

    # Install all libs
    $ sudo apt update
    $ sudo apt install build-essential bison \
    libevent-dev libmysqlclient-dev libpcre3-dev libpq-dev \
    libsqlite3-dev libssl-dev libz-dev libjemalloc-dev

Build

1. checkout git repo
    ```
    $ git clone https://github.com/fluffos/fluffos.git
    $ cd fluffos
    $ git checkout v2019 #(or an v2019 release tag)
    ```
2. Upgrade your cmake
    ```
    $ sudo pip install --upgrade cmake
    ```
3. build
    ```
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make -j4 install
    ```

4. find the built binary and support files in ./bin/

## Packages

By default driver have an default list of builtin packages to build.

If you want to turn off an package, run `cmake .. -DPACKAGE_XX=OFF -DPACAGE_YY=OFF`

## Advanced Build features

1. By default driver in release mode will optimize for running on current CPU only.
if you wish to compile and copy the driver to somewhere else to run, turn off MARCH_NATIVE.

    ```
    $ cmake .. -DMARCH_NATIVE=OFF
    ```

2. Static linking: you can pass -DSTATIC=ON to force driver to link staticly for all libraries, this will only work
     in a specialized environment like alpine linux and Windows. Check the result file to make sure it is an static
      file.

    ```
    $ ldd bin/driver
        not a dynamic executable
    ```

## OSX

1. Install Homebrew, goto https://brew.sh/ and follow instructions!

2. install libraries (checkout https://github.com/fluffos/fluffos/blob/master/.github/workflows/ci-osx.yml if you
 have issue)
    ```
    $ brew install cmake pkg-config mysql pcre libgcrypt libevent openssl jemalloc icu4c
    ```

3. build same as under linux, you will need to pass two environment variables
    ```
    $ make build && cd build
    $ OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/usr/local/opt/icu4c" cmake ..
    $ make install
    ```

## Windows
see https://forum.fluffos.info/t/compiling-fluffos-v2019-under-osx-windows-msys2-mingw64/601
