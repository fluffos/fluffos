---
layout: default
title: Build
---

## Requirements

Compiler: GCC 4.6+ or LLVM clang 3.0+

Library: libevent 2.0+, addtional libraries depends on the package selection.

## General Building Steps

To Build fluffos 3.0

    # 1. heckout git repo
    $ git clone https://github.com/fluffos/fluffos.git
    $ cd fluffos
    $ git checkout master

    # 2. create your own local_options file

    $ cd src
    $ cp local_options.testrelease local_options

    <edit local_options to you need>

    # 3. Build!
    $ ./build.FluffOS
    $ make
    $ make install

    # 4. Find the built binary "driver" and "portbind" in bin/ directory

## Ubuntu/Debian x86_64

This is the best platform to run fluffos currently, support for other platform
is best effort only.

    # recommended version: 12.04 LTS+

    # 1. Install compile environment
    $ sudo apt-get install build-essential gcc g++

    # 2. Install libevent 2.0
    $ sudo apt-get install libevent-dev

    # 3. Install other libraries for PACAKGE_*
    $ sudo apt-get install libmysqlclient-dev libsqlite3-dev \
      libpq-dev libz-dev libssl-dev libpcre3-dev

## CentOS X86_64

gcc/libevent on CentOS is too old for FluffOS, you must upgrade them manually.

    # Install GCC 4.8

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

## CYGWIN32/CYGWIN64

FluffOS 3.0alpha8 is fully functional under CYGWIN32 and CYGWIN64 now!

WARNING: FluffOS currently still require your mudlib files have unix line-ending.

    # 1. Get CYGWIN setup file on http://www.cygwin.org

    # 2. Install following packages:
    #
    # gcc-core   4.8.2-1
    # gcc-g++    4.8.2-1
    # libstdc++6 4.8.2-1
    # make       4.0-2
    # m4         1.4.17-1
    # git
    # vim (recommended)
    #
    # CYGWIN setup should take care of other dependencies for you

    # 3. Install libevent2.0 manually

    $ wget https://github.com/downloads/libevent/libevent/libevent-2.0.21-stable.tar.gz
    $ tar zxvf libevent-2.0.21-stable.tar.gz
    $ cd libevent-2.0.21-stable
    $ ./configure
    $ make
    $ make install

    # 4. Build FluffOS as under normal linux

    # 5. To convert all your mudlib files into unix line-ending:
    $ cd <your mudlib dir>
    $ find . | xargs dos2unix

## FreeBSD

Currently not supported.

## Customization

TODO: list important local_options directive here.
