---
layout: default
title: 编译 FluffOS
---

## Fluffos 编译说明

FluffOS 目前分 v2017 和 v2019 二个版本，主要区别是 v2017 版可以兼容支持旧MUD，只需对LIB微调即可，而 v2019 版要求LIB编码为 utf-8 ，支持websocket。

v2017 版目前支持在以下系统中编译：ubuntu 14.04+, raspbian, centos 7+, CYGWIN 64。

v2019 版目前支持在以下系统中编译：ubuntu 18.04+ (包括 WSL), raspbian, OSX, MSYS2/mingw64.

不管是 v2017 还是 v2019，编译配置文件都为 src 下面的 local_options 文件，请根据需要修改（非特别需求，推荐使用默认配置），如果需要驱动旧版MUD，请 `#undef SENSIBLE_MODIFIERS` 。

## 下载驱动源码

推荐使用 `git` 下载最新版的 fluffos，具体指令如下(如果不会使用 git 请先补充相关技能)

    git clone https://github.com/fluffos/fluffos.git
    cd fluffos
    # 如果要使用 v2017 版还需以下指令切换版本
    git checkout v2017

如果你访问github.com速度太慢，可以使用国内镜像地址下载项目：

    git clone https://gitee.com/mudren/fluffos.git

## Ubuntu & Raspberry 系统编译

Ubuntu 系统请执行以下指令安装编译所需的包，包括编译 v2017 和 v2019 所需的库。

    sudo apt install bison libjemalloc-dev zlib1g-dev libssl-dev libmariadb-dev libpcre3-dev libevent-dev libicu-dev libdw-dev binutils-dev gcc g++ autoconf automake cmake git -y

### v2017 编译

请在 `fluffos` 目录下使用以下指令：

    cd src
    ./build.FluffOS
    make install

v2017 编译完成后的驱动文件在 fluffos/bin 目录中，包括 `driver` 和 `portbind`。

### v2019 编译

请在 `fluffos` 目录下使用以下指令：

    rm -rf build && mkdir build && cd build
    cmake ..
    make -j4 install

注意，如果不需要某个包，请使用类似以下指令编译：

    cmake -DPACKAGE_DB=OFF ..

默认编译为动态编译，仅针对当前CPU优化，如果需要静态编译，请使用以下指令编译：

    cmake -DMARCH_NATIVE=OFF -DSTATIC=ON ..

v2019 编译完成后的驱动文件在 fluffos/build/bin 目录中，包括 `driver`、`lpcc`、`portbind`三个程序和 `include`、`std`、`www`三个目录，分别是驱动定义的头文件、驱动提供的sefun和 websocket 的 http dir 目录。

## CentOS 系统编译

经测试在CentOS 7下也可编译fluffos v2019，相对ubuntu来说，麻烦很多，因为cmake、g++、libevent-devel、libicu-devel版本过低，需要我们单独安装新版：

    # 安装pip
    yum -y install epel-release
    yum -y install python-pip
    # 安装cmake
    pip install --upgrade cmake
    # 安装devtoolset(g++)
    yum -y install centos-release-scl
    yum -y install devtoolset-8
    # 切换到gcc8版本
    scl enable devtoolset-8 bash
    # 安装其它组件
    yum -y install zlib-devel
    # yum install libevent-devel
    # yum install libicu-devel
    yum -y install bison
    yum -y install jemalloc-devel
    yum -y install openssl-devel
    yum -y install mariadb-devel
    yum -y install binutils-devel
    yum -y install git

下载源码编译安装libevent和libicu最新版：

    # 安装libevent
    $ wget https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
    $ tar -zxvf libevent-2.1.12-stable.tar.gz
    $ cd libevent-2.1.12-stable
    $ ./configure –prefix=/usr
    $ make -j4
    $ make install
    # 安装libicu
    $ wget https://github.com/unicode-org/icu/releases/download/release-69-1/icu4c-69_1-src.tgz
    $ tar -zxvf icu4c-69_1-src.tgz
    $ cd icu/source/
    $ ./configure -prefix=/usr
    $ make -j4
    $ make install
    # 更新动态链接库
    $ ldconfig

通过以上操作后，可和 ubuntu 系统下一样编译驱动。

## Windows 系统编译

### v2017

FluffOS v2017 如果要在 windows 系统下使用，可以在 cygwin 环境编译。如果对 cygwin 不了解，请先补充相关知识。

CYGWIN下载地址： http://www.cygwin.org/setup-x86_64.exe

需要安装以下包，推荐使用阿里云镜像下载包：http://mirrors.aliyun.com/cygwin/

* autoconf
* automake
* binutils
* bison
* cmake
* gcc-core
* gcc-g++
* git
* libcrypt-devel
* libevent-devel
* libiconv-devel
* libicu-devel
* libmariadb-devel
* libmysqlclient-devel
* libpcre-devel
* make
* python3
* zlib-devel

编译方式和 Ubuntu 系统下一样，在 `fluffos` 目录下执行以下指令：

    cd src
    ./build.FluffOS
    make install

编译完成后的驱动文件在 fluffos/bin 目录中，包括 `driver.exe` 和 `portbind.exe`。请注意，这个 `driver.exe`只能通过 cygwin 环境运行，如果要在其他主机下运行，请把需要的 dll 文件和 driver.exe 放到一起。具体需要哪些 dll 文件可以使用 `ldd driver` 指令查看。

### v2019

FluffOS v2019 如果要在 windows 系统下使用，可以在 MSYS2 环境编译。如果对 MSYS2 不了解，请先补充相关知识。

MSYS2 官方网站：https://www.msys2.org/ 下载安装后需运行 Mingw-w64 64 bit，更新系统并安装必须的包，国外镜像速度慢，最新版配置中包括了国内镜像，但非优先使用，可以先根据以下配置修改：

编辑 `/etc/pacman.d/mirrorlist.mingw32` ，把以下镜像放在文件开头：

    Server = http://mirror.bit.edu.cn/msys2/mingw/i686/
    Server = http://mirrors.ustc.edu.cn/msys2/mingw/i686/
    Server = https://mirrors.sjtug.sjtu.edu.cn/msys2/mingw/i686/
    Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/i686/

编辑 `/etc/pacman.d/mirrorlist.mingw64` ，把以下镜像放在文件开头：

    Server = http://mirror.bit.edu.cn/msys2/mingw/x86_64/
    Server = http://mirrors.ustc.edu.cn/msys2/mingw/x86_64/
    Server = https://mirrors.sjtug.sjtu.edu.cn/msys2/mingw/x86_64/
    Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/mingw/x86_64/

编辑 `/etc/pacman.d/mirrorlist.msys` ，把以下镜像放在文件开头：

    Server = http://mirror.bit.edu.cn/msys2/msys/$arch/
    Server = http://mirrors.ustc.edu.cn/msys2/msys/$arch/
    Server = https://mirrors.sjtug.sjtu.edu.cn/msys2/msys/$arch/
    Server = https://mirrors.tuna.tsinghua.edu.cn/msys2/msys/$arch/

然后执行 `pacman -Sy` 刷新源。

下载 fluffos 后，还需安装部分包，指令如下：

    pacman -Syu
    pacman -S git make mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
    pacman -S mingw-w64-x86_64-zlib mingw-w64-x86_64-libevent mingw-w64-x86_64-pcre mingw-w64-x86_64-icu
    pacman -S bison

编译方式和 Ubuntu 下 v2019 类似，在 `fluffos` 目录执行以下指令：

    rm -rf build && mkdir build && cd build
    cmake -G "MSYS Makefiles" -DPACKAGE_DB=OFF ..
    make install

## MACOS 系统编译

在最新的 macos 系统下编译只支持 FluffOS v2019，MAC 系统请执行以下指令安装编译所需的包：

    brew install cmake pkg-config mysql pcre libgcrypt libevent openssl jemalloc icu4c

编译方式和 Ubuntu 下 v2019 类似，在 `fluffos` 目录执行以下指令：

    rm -rf build && mkdir build && cd build
    OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/usr/local/opt/icu4c" cmake ..
    make -j4 install

## 友情提示

因为最新版驱动不定期更新，为了方便更新编译，可以在你的 fluffos 源码目录下新建一个脚本文件 `build.sh`，内容如下：

```bash
#!/bin/bash

git pull
rm -rf build
mkdir build && cd build
cmake .. && make -j4 install
```

然后使用 `chmod +x build.sh` 让脚本可执行，以后只需在源码目录中输入 `./build.sh` 即可自动下载更新并编译驱动。

如果你使用的是windows系统，不想自己每次编译新版，可在此下载最新版驱动：https://bbs.mud.ren/threads/4

如果你习惯使用docker，也可以使用fluffos驱动的docker镜像：https://hub.docker.com/r/fluffos/fluffos

## 编译演示

fluffos v2019 编译演示：

[![asciicast](https://asciinema.org/a/311570.svg)](https://asciinema.org/a/311570?autoplay=1&speed=3)

fluffos v2017 编译演示：

[![asciicast](https://asciinema.org/a/311567.svg)](https://asciinema.org/a/311567?autoplay=1&speed=2)


提示：演示视频可随时暂停，演示的指令也可直接复制。
