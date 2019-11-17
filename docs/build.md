---
layout: default
title: Build
---

## Environment

v2019 will support ubuntu 18.04+ (including Win10+WSL), and OSX/BSD, native windows build with MSVC 2017 is under development.

Compilers: FluffOS v2019 uses C++17, which requires at least GCC 7+ or LLVM clang 4+.

Library: libevent 2.0+, additional libraries depends on the package selection. 

jemalloc: use JEMALLOC is highly recommended in production. otherwise you may run into memory issue.

## BUILD (v2019)

Ubuntu 18.04

This is the best platform to build & run FluffOS, support for other platform
is best effort only.

    # Install all libs
    $ sudo apt update
    $ sudo apt install build-essential bison libevent-dev libjemalloc-dev \
    libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev libssl-dev libz-dev

To Build fluffOS v2019 (CMake)

    # 1. checkout git repo
    $ git clone https://github.com/fluffos/fluffos.git
    $ cd fluffos
    $ git checkout master #(or v2019 branch or any specfic release tag)

    # 2. Upgrade your cmake
    $ sudo pip install --upgrade cmake

    # 3. build
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make install

    # 4. find the two built binary in bin/driver and bin/portbind

Packages

    # By default driver have an default list of builtin packages to build.
    # If you want to turn off an package, run cmake .. -DPACKAGE_XX=OFF -DPACAGE_YY=OFF

Advanced Build features (v2019)

    # By default driver will link dynamic libraries and optimize for running on current CPU only.
    # if you wish to cross compile for other machines, turn off MARCH_NATIVE and turn
    # on STATIC.

    $ cmake .. -DMARCH_NATIVE=OFF -DSTATIC=ON

    # Check the result file to make sure it is an static file

    $ ldd bin/driver
    not a dynamic executable
    
## OSX

    # 1. Install Homebrew, goto https://brew.sh/ and follow instructions!
    
    # 2. install libraries (checkout .github/workflow/ci-osx.yml)
    $ brew install cmake pkg-config mysql pcre libgcrypt libevent openssl jemalloc
    
    # 3. build same as under linux
    
## Windows
    # currently under development
