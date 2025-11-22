# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

FluffOS is an LPMUD driver based on the last release of MudOS (v22.2b14) with 10+ years of bug fixes and performance enhancements. It supports all LPC-based MUDs with minimal code changes and includes modern features like UTF-8 support, TLS, WebSocket protocol, async IO, and database integration.

## Architecture

### Core Components

**Driver Layer** (`src/`)

- `main.cc` - Entry point for the driver executable
- `backend.cc` - Main game loop and event handling
- `comm.cc` - Network communication layer
- `user.cc` - User/session management
- `symbol.cc` - Symbol table management
- `ofile.cc` - Object file handling

**VM Layer** (`src/vm/`)

- `vm.cc` - Virtual machine initialization and management
- `interpret.cc` - LPC bytecode interpreter
- `simulate.cc` - Simulation engine for object lifecycle
- `master.cc` - Master object management
- `simul_efun.cc` - Simulated external functions

**Compiler Layer** (`src/compiler/`)

- `compiler.cc` - LPC-to-bytecode compiler
- `grammar.y` - Grammar definition (Bison)
- `lex.cc` - Lexical analyzer
- `generate.cc` - Code generation

**Packages** (`src/packages/`)

- Modular functionality organized by feature (async, db, crypto, etc.)
- Each package has `.spec` files defining available functions
- Core packages: core, crypto, db, math, parser, sockets, etc.

**Networking** (`src/net/`)

- `telnet.cc` - Telnet protocol implementation
- `websocket.cc` - WebSocket support for web clients
- `tls.cc` - SSL/TLS encryption support
- `msp.cc` - MUD Sound Protocol support

### Build System

**CMake Configuration** (`CMakeLists.txt`, `src/CMakeLists.txt`)

- CMake 3.22+ required
- C++17 and C11 standards
- Jemalloc for memory management (recommended)
- ICU for UTF-8 support
- OpenSSL for crypto features

**Build Options** (key flags)

- `MARCH_NATIVE=ON` (default) - Optimize for current CPU
- `STATIC=ON/OFF` - Static vs dynamic linking
- `USE_JEMALLOC=ON` - Use jemalloc memory allocator
- `PACKAGE_*` - Enable/disable specific packages
- `ENABLE_SANITIZER=ON` - Enable address sanitizer for debugging

## Build Commands

### Development Build

```bash
# Standard development build
mkdir build && cd build
cmake ..
make -j$(nproc) install

# Debug build with sanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON
```

### Production Build

```bash
# Production-ready static build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSTATIC=ON -DMARCH_NATIVE=OFF
make install
```

### Package-Specific Builds

```bash
# Build without database support
cmake .. -DPACKAGE_DB=OFF

# Build with specific packages disabled
cmake .. -DPACKAGE_CRYPTO=OFF -DPACKAGE_COMPRESS=OFF
```

## Testing

### Unit Tests

```bash
# Run all tests (requires GTest)
cd build
make test

# Run specific test executable
./src/lpc_tests
./src/ofile_tests
```

### LPC Tests

```bash
# Run LPC test suite
cd testsuite
../build/bin/driver etc/config.test -ftest
```

### Integration Testing

```bash
# Run driver with test configuration
./build/bin/driver testsuite/etc/config.test
```

## Continuous Integration

FluffOS uses GitHub Actions for comprehensive CI/CD across multiple platforms and configurations.

### CI Matrix

**Ubuntu CI** (`.github/workflows/ci.yml`)

- **Compilers**: GCC and Clang
- **Build Types**: Debug and RelWithDebInfo
- **Platform**: ubuntu-22.04
- **Key Features**: SQLite support, GTest integration
- **Steps**: Install dependencies → CMake configure → Build → Unit tests → LPC testsuite

**macOS CI** (`.github/workflows/ci-osx.yml`)

- **Build Types**: Debug and RelWithDebInfo
- **Platform**: macos-14 (Apple Silicon)
- **Environment Variables**:
  - `OPENSSL_ROOT_DIR=/usr/local/opt/openssl`
  - `ICU_ROOT=/opt/homebrew/opt/icu4c`
- **Dependencies**: cmake, pkg-config, pcre, libgcrypt, openssl, jemalloc, icu4c, mysql, sqlite3, googletest

**Windows CI** (`.github/workflows/ci-windows.yml`)

- **Build Types**: Debug and RelWithDebInfo
- **Platform**: windows-latest with MSYS2/MINGW64
- **Build Options**:
  - `-DMARCH_NATIVE=OFF` (for portability)
  - `-DPACKAGE_CRYPTO=OFF`
  - `-DPACKAGE_DB_MYSQL=""` (disabled)
  - `-DPACKAGE_DB_SQLITE=1`
- **Dependencies**: mingw-w64 toolchain, cmake, zlib, pcre, icu, sqlite3, jemalloc, gtest

**Sanitizer CI** (`.github/workflows/ci-sanitizer.yml`)

- **Purpose**: Memory safety and bug detection
- **Compiler**: Clang only
- **Build Types**: Debug and RelWithDebInfo
- **Special Flag**: `-DENABLE_SANITIZER=ON`
- **Additional Dependencies**: libdw-dev, libbz2-dev

**Docker CI** (`.github/workflows/docker-publish.yml`)

- **Base Image**: Alpine 3.18
- **Build Configuration**: Static linking with `-DSTATIC=ON -DMARCH_NATIVE=OFF`
- **Registry**: GitHub Container Registry (ghcr.io)
- **Triggers**: Push to master, version tags (v*.*), pull requests

**Code Quality CI**

- **CodeQL Analysis** (`.github/workflows/codeql-analysis.yml`): Security vulnerability detection
- **Coverity Scan** (`.github/workflows/coverity-scan.yml`): Static analysis (weekly schedule + push)

**Documentation CI** (`.github/workflows/gh-pages.yml`)

- **Framework**: VitePress
- **Build**: Node.js 20 with npm
- **Deploy**: GitHub Pages
- **Path**: `docs/` directory

### Running CI-Equivalent Builds Locally

**Ubuntu Debug Build (GCC)**

```bash
export CC=gcc CXX=g++
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DPACKAGE_DB_SQLITE=2 ..
make -j$(nproc) install
make test
cd ../testsuite && ../build/bin/driver etc/config.test -ftest
```

**Ubuntu with Sanitizer (Clang)**

```bash
export CC=clang CXX=clang++
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DPACKAGE_DB_SQLITE=2 -DENABLE_SANITIZER=ON ..
make -j$(nproc) install
make test
```

**macOS Build**

```bash
mkdir build && cd build
OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/opt/homebrew/opt/icu4c" \
  cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPACKAGE_DB_SQLITE=2 ..
make -j$(sysctl -n hw.ncpu) install
make test
```

**Windows Build (MSYS2/MINGW64)**

```bash
mkdir build && cd build
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug \
  -DMARCH_NATIVE=OFF -DPACKAGE_CRYPTO=OFF \
  -DPACKAGE_DB_MYSQL="" -DPACKAGE_DB_SQLITE=1 ..
make -j$(nproc) install
```

**Docker Build (Static)**

```bash
docker build -t fluffos:local .
# Or build manually in Alpine container
cmake .. -DMARCH_NATIVE=OFF -DSTATIC=ON
make install
ldd bin/driver  # Should show "not a dynamic executable"
```

### CI Dependencies by Platform

**Ubuntu/Debian**

```bash
sudo apt update
sudo apt install -y build-essential autoconf automake bison expect \
  libmysqlclient-dev libpcre3-dev libpq-dev libsqlite3-dev \
  libssl-dev libtool libz-dev telnet libgtest-dev libjemalloc-dev \
  libdw-dev libbz2-dev  # For sanitizer builds
```

**macOS (Homebrew)**

```bash
brew install cmake pkg-config pcre libgcrypt openssl jemalloc icu4c \
  mysql sqlite3 googletest
```

**Windows (MSYS2 - MINGW64 shell)**

```bash
pacman --noconfirm -S --needed \
  mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-zlib mingw-w64-x86_64-pcre \
  mingw-w64-x86_64-icu mingw-w64-x86_64-sqlite3 \
  mingw-w64-x86_64-jemalloc mingw-w64-x86_64-gtest \
  bison make
```

**Alpine (Docker/Static)**

```bash
apk add --no-cache linux-headers gcc g++ clang-dev make cmake bash \
  mariadb-dev mariadb-static postgresql-dev sqlite-dev sqlite-static \
  openssl-dev openssl-libs-static zlib-dev zlib-static icu-dev icu-static \
  pcre-dev bison git musl-dev libelf-static elfutils-dev \
  zstd-static bzip2-static xz-static
```

## Development Workflow

### Code Generation

Several source files are auto-generated during build:

- `grammar.autogen.cc/.h` - From `grammar.y` (Bison)
- `efuns.autogen.cc/.h` - From package specifications
- `applies_table.autogen.cc/.h` - From applies definitions
- `options.autogen.h` - From configuration options

### Adding New Functions

1. Add function to appropriate package `.spec` file
2. Implement function in corresponding `.cc` file
3. Run build to regenerate autogenerated files
4. Add tests in `testsuite/` directory

### Debugging

```bash
# Build with debug symbols and sanitizer
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON

# Run with GDB
gdb --args ./build/bin/driver config.test

# Memory debugging with Valgrind
valgrind --leak-check=full ./build/bin/driver config.test
```

## Platform-Specific Notes

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt install build-essential bison libmysqlclient-dev libpcre3-dev \
  libpq-dev libsqlite3-dev libssl-dev libz-dev libjemalloc-dev libicu-dev \
  libgtest-dev  # For testing
```

### macOS

```bash
# Install dependencies
brew install cmake pkg-config mysql pcre libgcrypt openssl jemalloc icu4c \
  sqlite3 googletest  # Added sqlite3 and googletest for testing

# Build with environment variables (for Apple Silicon)
OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/opt/homebrew/opt/icu4c" cmake ..
# For Intel Macs, use:
# OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/usr/local/opt/icu4c" cmake ..
```

### Windows (MSYS2)

```bash
# Install MSYS2 packages (run in MSYS2 shell, then switch to MINGW64 shell for build)
pacman --noconfirm -S --needed \
  git mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-zlib mingw-w64-x86_64-pcre \
  mingw-w64-x86_64-icu mingw-w64-x86_64-sqlite3 \
  mingw-w64-x86_64-jemalloc mingw-w64-x86_64-gtest \
  bison make

# Note: PACKAGE_CRYPTO is typically disabled on Windows
# Build in MINGW64 terminal
cmake -G "MSYS Makefiles" -DMARCH_NATIVE=OFF \
  -DPACKAGE_CRYPTO=OFF -DPACKAGE_DB_MYSQL="" -DPACKAGE_DB_SQLITE=1 ..
```

## Key Directories

- `src/` - Main driver source code
- `src/packages/` - Modular package implementations
- `src/vm/` - Virtual machine and interpreter
- `src/compiler/` - LPC compiler
- `src/net/` - Network protocol implementations
- `testsuite/` - LPC test programs and configurations
- `docs/` - Documentation (Markdown and Jekyll)
- `build/` - Build output directory (auto-generated)

## Configuration Files

- `Config.example` - Example driver configuration
- `src/local_options` - Local build options (copy from `local_options.README`)
- `testsuite/etc/config.test` - Test configuration
- Package-specific `.spec` files define available functions

## Common Development Tasks

### Adding a New Package

1. Create directory in `src/packages/[package-name]/`
2. Add `CMakeLists.txt`, `.spec` file, and source files
3. Update `src/packages/CMakeLists.txt`
4. Add tests in `testsuite/`

### Modifying Compiler

1. Edit `src/compiler/grammar.y` for syntax changes
2. Regenerate grammar with Bison if available
3. Update corresponding compiler components

### Debugging Memory Issues

1. Build with `-DENABLE_SANITIZER=ON`
2. Use `mud_status()` efun in LPC for runtime memory info
3. Check `testsuite/log/debug.log` for detailed logs
