---
layout: doc
title: Build
---

# Getting Started

[[toc]]

## Supported Environment

The best platform to build FluffOS is ubuntu 22.04+ (including WSL), macOS latest (both Intel and Apple Silicon), and Windows on MSYS2/MINGW64.

**Compilers**: FluffOS uses C++17 and C11, which requires at least GCC 7+ or LLVM clang 4+.

**Tested Configurations** (validated by CI):
- **Ubuntu 22.04**: GCC and Clang with Debug/RelWithDebInfo builds
- **macOS 14 (Apple Silicon)**: Clang with Debug/RelWithDebInfo builds
- **Windows (MSYS2/MINGW64)**: GCC with Debug/RelWithDebInfo builds
- **Alpine Linux 3.18**: Static builds for Docker containers

**System Library Requirements** (Must install):

1. **ICU**: FluffOS uses ICU for UTF-8 and transcoding support.
2. **jemalloc**: Release build use JEMALLOC by default, and is highly recommended in production.
3. **OpenSSL** (if PACKAGE_CRYPTO enabled) - Note: typically disabled on Windows
4. **PCRE** (if PACKAGE_PCRE enabled)
5. **MysqlClient** (if PACKAGE_DB enabled with MySQL support)
6. **SQLite3** (if PACKAGE_DB_SQLITE enabled)
7. **PostgreSQL** (if PACKAGE_DB enabled with PostgreSQL support)
8. **GoogleTest** (for running unit tests)

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

### Installing Dependencies

```shell
$ sudo apt update
$ sudo apt install -y build-essential autoconf automake bison expect \
  libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev \
  libssl-dev libtool libz-dev telnet libjemalloc-dev libicu-dev \
  libgtest-dev
```

For **sanitizer builds** (memory debugging), also install:
```shell
$ sudo apt install -y libdw-dev libbz2-dev
```

### Checkout Git Repo

```shell
$ git clone https://github.com/fluffos/fluffos.git
$ cd fluffos
$ git checkout master  # or a release tag like v2019
```

### Ensure CMake 3.22+

FluffOS requires CMake 3.22 or higher. Ubuntu 22.04+ includes this by default. If needed:
```shell
$ sudo apt install cmake  # or
$ sudo pip install --upgrade cmake
```

### Build (Standard)

```shell
$ mkdir build && cd build
$ cmake ..
$ make -j $(nproc) install
```

Binary files and support files will be in `./bin/`

### Build Types

**Debug Build** (for development):
```shell
$ cmake -DCMAKE_BUILD_TYPE=Debug -DPACKAGE_DB_SQLITE=2 ..
$ make -j $(nproc) install
```

**Release Build with Info** (recommended for testing):
```shell
$ cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPACKAGE_DB_SQLITE=2 ..
$ make -j $(nproc) install
```

**Sanitizer Build** (for memory debugging):
```shell
$ export CC=clang CXX=clang++
$ cmake -DCMAKE_BUILD_TYPE=Debug -DPACKAGE_DB_SQLITE=2 -DENABLE_SANITIZER=ON ..
$ make -j $(nproc) install
```

### Running Tests

After building, run the test suite:
```shell
# Unit tests
$ cd build
$ make test
# or with verbose output
$ CTEST_OUTPUT_ON_FAILURE=1 make test

# LPC testsuite
$ cd testsuite
$ ../build/bin/driver etc/config.test -ftest
```

## MacOS

Requires macOS 10.15+. Tested on macOS 14 (both Intel and Apple Silicon).

### Install Homebrew

If not already installed, goto <https://brew.sh/> and follow instructions!

### Install Dependencies

```shell
$ brew install cmake pkg-config mysql pcre libgcrypt openssl jemalloc icu4c \
  sqlite3 googletest
```

Set `HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=1` to speed up brew installs if needed.

### Build

You need to pass environment variables for OpenSSL and ICU locations:

**For Apple Silicon (M1/M2/M3)**:
```shell
$ mkdir build && cd build
$ OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/opt/homebrew/opt/icu4c" \
  cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPACKAGE_DB_SQLITE=2 ..
$ make -j $(sysctl -n hw.ncpu) install
```

**For Intel Macs**:
```shell
$ mkdir build && cd build
$ OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/usr/local/opt/icu4c" \
  cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPACKAGE_DB_SQLITE=2 ..
$ make -j $(sysctl -n hw.ncpu) install
```

### Running Tests

```shell
# Unit tests
$ cd build
$ CTEST_OUTPUT_ON_FAILURE=1 make test

# LPC testsuite
$ cd testsuite
$ ../build/bin/driver etc/config.test -ftest
```

**Note**: If you encounter issues, check the latest CI configuration at:
<https://github.com/fluffos/fluffos/blob/master/.github/workflows/ci-osx.yml>

## Windows

**Supported Environment**: Windows 10+ with MSYS2/MINGW64. Binary produced can run on Windows 7+.

**Note**: FluffOS LPC VM is always 64-bit! You can't use more than 4GB memory in 32-bit builds.

For the most up-to-date commands, check: <https://github.com/fluffos/fluffos/blob/master/.github/workflows/ci-windows.yml>

### Install MSYS2

1. Download MSYS2 from the official website: <https://www.msys2.org/>
2. Pick the X86_64 version for 64-bit Windows

### Update MSYS2

Open **msys2.exe** and sync/update MSYS2:

```shell
$ pacman -Syu
```

You may need to close and reopen the MSYS2 window. Keep running the update command until there's nothing left to upgrade.

### Install Build Dependencies

```shell
$ pacman --noconfirm -S --needed \
  git mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-zlib mingw-w64-x86_64-pcre \
  mingw-w64-x86_64-icu mingw-w64-x86_64-sqlite3 \
  mingw-w64-x86_64-jemalloc mingw-w64-x86_64-gtest \
  bison make
```

**Optional**: For MySQL/MariaDB support:
```shell
$ pacman -S mingw-w64-x86_64-libmariadbclient
```

:::important
**CRITICAL**: After installation, close the MSYS2 window and open **MINGW64.exe** instead! All build commands must run in the MINGW64 shell.
:::

### Build

In the **MINGW64** shell:

```shell
$ git clone https://github.com/fluffos/fluffos.git
$ cd fluffos
$ mkdir build && cd build
$ cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug \
  -DMARCH_NATIVE=OFF -DPACKAGE_CRYPTO=OFF \
  -DPACKAGE_DB_MYSQL="" -DPACKAGE_DB_SQLITE=1 ..
$ make -j $(nproc) install
```

**Notes**:
- `-DMARCH_NATIVE=OFF`: Ensures portability across different CPUs
- `-DPACKAGE_CRYPTO=OFF`: Crypto package is typically disabled on Windows
- `-DPACKAGE_DB_SQLITE=1`: Enable SQLite (set to 2 for version 2)
- `-DPACKAGE_DB_MYSQL=""`: Disable MySQL (or set to enable)

### Running Tests

```shell
# Unit tests
$ cd build
$ CTEST_OUTPUT_ON_FAILURE=1 make test

# LPC testsuite
$ cd testsuite
$ ../build/bin/driver etc/config.test -ftest
```

## Build Configuration Options

### Packages

By default, the driver has a default list of builtin packages to build.

To disable specific packages:
```shell
$ cmake .. -DPACKAGE_CRYPTO=OFF -DPACKAGE_DB=OFF
```

Common package options:
- `PACKAGE_DB_SQLITE=1` or `=2`: Enable SQLite support (version 1 or 2)
- `PACKAGE_DB_MYSQL=""`: Disable MySQL (or set to enable)
- `PACKAGE_CRYPTO=OFF`: Disable crypto package (typical on Windows)
- `PACKAGE_PCRE=OFF`: Disable PCRE package

### Build Types

FluffOS supports standard CMake build types:

- **Debug**: No optimization, full debug symbols (`-DCMAKE_BUILD_TYPE=Debug`)
- **Release**: Full optimization, no debug symbols (`-DCMAKE_BUILD_TYPE=Release`)
- **RelWithDebInfo**: Optimized with debug symbols (`-DCMAKE_BUILD_TYPE=RelWithDebInfo`) - **recommended for testing**

### CPU Compatibility

**By default**, driver built in release mode will optimize for the current system CPU only using `-march=native`. Copying the driver to another machine with a different CPU may not work!

If you need portable drivers, turn off `MARCH_NATIVE`:

```shell
$ cmake .. -DMARCH_NATIVE=OFF
```

This is **automatically disabled** in CI for Windows, Docker, and when building for distribution.

### Static Linking

You can force static linking for all libraries:

```shell
$ cmake .. -DSTATIC=ON
```

**Note**: This only works in specialized environments like Alpine Linux and Windows/MSYS2.

### Sanitizer Builds

Enable AddressSanitizer for memory debugging:

```shell
$ cmake .. -DENABLE_SANITIZER=ON
```

**Requirements**:
- Use Clang compiler (recommended)
- Install additional dependencies on Ubuntu: `libdw-dev libbz2-dev`
- Build in Debug or RelWithDebInfo mode

## Testing

### Unit Tests

FluffOS includes C++ unit tests using GoogleTest:

```shell
$ cd build
$ make test
# or with verbose output
$ CTEST_OUTPUT_ON_FAILURE=1 make test
```

### LPC Testsuite

The LPC testsuite tests the driver with actual LPC code:

```shell
$ cd testsuite
$ ../build/bin/driver etc/config.test -ftest
```

### Continuous Integration

FluffOS uses GitHub Actions for automated testing on every push and pull request:

- **Ubuntu CI**: Tests with GCC and Clang on ubuntu-22.04
- **macOS CI**: Tests on macOS 14 (Apple Silicon)
- **Windows CI**: Tests with MSYS2/MINGW64
- **Sanitizer CI**: Memory safety checks with Clang + AddressSanitizer
- **CodeQL**: Security vulnerability scanning
- **Coverity Scan**: Static analysis (weekly)

All CI workflows run both unit tests and the LPC testsuite.

See `.github/workflows/` for the exact CI configuration.

## Alpine Linux & Docker

Alpine Linux is used for building static binaries suitable for Docker containers. The Docker image is based on Alpine 3.18.

### Installing Dependencies

```shell
# Install all libs
$ apk add --no-cache linux-headers gcc g++ clang-dev make cmake bash \
    mariadb-dev mariadb-static postgresql-dev sqlite-dev sqlite-static \
    openssl-dev openssl-libs-static zlib-dev zlib-static icu-dev icu-static \
    pcre-dev bison git musl-dev libelf-static elfutils-dev \
    zstd-static bzip2-static xz-static
```

### Installing jemalloc

Jemalloc must be installed manually on Alpine:

```shell
$ wget -O - https://github.com/jemalloc/jemalloc/releases/download/5.3.0/jemalloc-5.3.0.tar.bz2 | tar -xj
$ cd jemalloc-5.3.0
$ ./configure --prefix=/usr
$ make && make install
```

### Build Steps

```shell
$ git clone https://github.com/fluffos/fluffos.git
$ cd fluffos
$ mkdir build && cd build
$ cmake .. -DMARCH_NATIVE=OFF -DSTATIC=ON
$ make install
```

### Verify Static Binary

Check that the result is a static executable:

```shell
$ ldd bin/driver
    not a dynamic executable
```

### Using Docker

FluffOS provides an official Docker image built automatically on every push to master:

**Pull and run the official image:**
```shell
$ docker pull ghcr.io/fluffos/fluffos:master
$ docker run -it ghcr.io/fluffos/fluffos:master /path/to/config.cfg
```

**Build your own Docker image:**
```shell
$ docker build -t fluffos:local .
$ docker run -it fluffos:local /path/to/config.cfg
```

The Dockerfile uses a multi-stage build:
1. **Builder stage**: Compiles FluffOS statically on Alpine 3.18
2. **Runtime stage**: Creates minimal image with just the binary

See the `Dockerfile` in the repository root for details.
