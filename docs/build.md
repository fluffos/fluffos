---
layout: default
title: Build
---

## Environment

It is highly recommend to use the latest ubuntu LTS version (current 18.04 LTS), to build either version of the driver.

v2017 currently supports ubuntu 14.04+, centos 7+, CYGWIN 64. BSD & OSX still has issues.

v2019 will support ubuntu 16.04+, CYGWIN, Win10+WSL, native windows using VS2017, and OSX/BSD.

Compilers: FluffOS uses C++11, which requires at least GCC 4.6+ or LLVM clang 3.0+.

Library: libevent 2.0+, additional libraries depends on the package selection. 

jemalloc: use JEMALLOC is highly recommended in production. otherwise you may run into memory issue.

## BUILD (v2019)

Ubuntu 16.04+

This is the best platform to build & run FluffOS, support for other platform
is best effort only.

    # Install all libs
    $ sudo apt update
    $ sudo apt install build-essential bison libevent-dev libjemalloc-dev \
    libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev libssl-dev libz-dev libgtest-dev

To Build fluffOS v2019 (Cargo + CMake)

    # 1. checkout git repo
    $ git clone https://github.com/fluffos/fluffos.git
    $ cd fluffos
    $ git checkout v2019 (or any specfic release tag)

    # 2. Upgrade your cmake
    $ sudo pip install --upgrade cmake

    # 3. build
    $ cargo build -vvv

    # 4. find the built binary in target/debug/, with fluffos & libdriver.so two files.

Packages

    # By default driver have an default list of builtin packages to build.
    # If you want to turn off an package, modify src/packages/XXX/CMakeLists.txt. and change ON to OFF

Advanced Build features (v2019) (broken)

    # By default driver will link dynamic libraries and optimize for running on current CPU only.
    # if you wish to cross compile for other machines, turn off MARCH_NATIVE and turn
    # on STATIC.

    $ cmake -DMARCH_NATIVE=OFF -DSTATIC=ON ..

    # Check the result file to make sure it is an static file

    $ ldd src/driver
    not a dynamic executable

## BUILD (v2017)

To Build fluffOS v2017 (Autoconf & in tree build)

    # 0. You need to install autoconf & automake!
    $ sudo apt install autoconf automake
   
    # 1. checkout git repo
    $ git clone https://github.com/fluffos/fluffos.git
    $ cd fluffos
    $ git checkout v2017 (or any specfic release tag)

    # 2. modify local_options file as need.
    $ cd src
    <edit local_options to you need>

    # 3. Build!
    $ ./build.FluffOS
    $ make

    # 4. find the built binary in src/driver and src/portbind

## CentOS

The problem with CentOS is that most of the package is too old for FluffOS,
you must upgrade them manually first.

Move to ubuntu or run an ubuntu based docker images is highly recommended.

    # Install GCC 4.8 (or the latest one you like!)

    $ wget http://ftp.gnu.org/gnu/gcc/gcc-4.8.0/gcc-4.8.0.tar.bz2
    $ tar -jxvf  gcc-4.8.0.tar.bz2
    $ cd gcc-4.8.0
    $ ./contrib/download_prerequisites

    # Build GCC
    $ cd ..
    $ mkdir gcc-build-4.8.0
    $ cd gcc-build-4.8.0
    $ ../gcc-4.8.0/configure --enable-checking=release --enable-languages=c,c++ --disable-multilib
    $ make -j16 # (Will take an hour)
    $ make install

    # Verify now gcc is installed correctly.
    # "gcc -v" should output "gcc (GCC) 4.8.0"

    # NOTE:Manually install new version of libstdc++
    # you have to copy libstdc++.so.6 in your gcc4.8 build directory to /usr/lib64
    # override the existing file.

    # Install libevent2.0 manually

    $ wget https://github.com/downloads/libevent/libevent/libevent-2.0.21-stable.tar.gz
    $ tar zxvf libevent-2.0.21-stable.tar.gz
    $ cd libevent-2.0.21-stable
    $ ./configure –prefix=/usr
    $ make
    $ make install
 
## CYGWIN32/CYGWIN64 (v2017 & V2019)

FluffOS v2017 is fully functional under CYGWIN32 and CYGWIN64.

    # 1. Get CYGWIN setup file on http://www.cygwin.org , preferably x64 one.

    # 2. Make sure to install following packages, using apt-cyg
         (https://github.com/transcode-open/apt-cyg) is highly recommended. 

      - autoconf
      - automake
      - binutils (make sure to choose 2.28-3, there are still crashing bugs for latest version)
      - bison
      - gcc-core
      - gcc-g++
      - git
      - libcrypt-devel
      - libevent-devel
      - libiconv-devel
      - libpcre-devel
      - zlib-devel
      - libmysqlclient-devel

    # CYGWIN setup should take care of other dependencies for you

    # 3. Build FluffOS as if we are under normal linux!
