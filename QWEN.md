# QWEN.md

本文档为 QWEN Code 在处理本仓库代码时提供指导。

## 项目概述

FluffOS 是一款基于 MudOS 最后一个版本（v22.2b14）开发的 LPMUD 驱动程序，包含了 10 多年来的错误修复和性能增强。它支持所有基于 LPC 的 MUD，只需极少的代码改动，并包含现代特性，如 UTF-8 支持、TLS、WebSocket 协议、异步 IO 和数据库集成。

## 架构

### 核心组件

**驱动层**（`src/`）
- `main.cc` - 驱动程序可执行文件的入口点
- `backend.cc` - 主游戏循环和事件处理
- `comm.cc` - 网络通信层
- `user.cc` - 用户/会话管理
- `symbol.cc` - 符号表管理
- `ofile.cc` - 对象文件处理

**虚拟机层**（`src/vm/`）
- `vm.cc` - 虚拟机初始化和管理
- `interpret.cc` - LPC 字节码解释器
- `simulate.cc` - 对象生命周期的模拟引擎
- `master.cc` - 主对象管理
- `simul_efun.cc` - 模拟外部函数

**编译器层**（`src/compiler/`）
- `compiler.cc` - LPC 到字节码的编译器
- `grammar.y` - 语法定义（Bison）
- `lex.cc` - 词法分析器
- `generate.cc` - 代码生成

**包**（`src/packages/`）
- 按功能（异步、数据库、加密等）组织的模块化功能
- 每个包都有 `.spec` 文件，定义可用函数
- 核心包：core（核心）、crypto（加密）、db（数据库）、math（数学）、parser（解析器）、sockets（套接字）等

**网络**（`src/net/`）
- `telnet.cc` - Telnet 协议实现
- `websocket.cc` - 用于 Web 客户端的 WebSocket 支持
- `tls.cc` - SSL/TLS 加密支持
- `msp.cc` - MUD 声音协议支持

### 构建系统

**CMake 配置**（`CMakeLists.txt`、`src/CMakeLists.txt`）
- 需要 CMake 3.22+
- 支持 C++17 和 C11 标准
- 推荐使用 Jemalloc 进行内存管理
- ICU 用于 UTF-8 支持
- OpenSSL 用于加密功能

**构建选项**（关键标志）
- `MARCH_NATIVE=ON`（默认）- 针对当前 CPU 进行优化
- `STATIC=ON/OFF` - 静态链接与动态链接
- `USE_JEMALLOC=ON` - 使用 jemalloc 内存分配器
- `PACKAGE_*` - 启用/禁用特定包
- `ENABLE_SANITIZER=ON` - 启用地址 sanitizer 用于调试

## 构建命令

### 开发构建
```bash
# 标准开发构建
mkdir build && cd build
cmake ..
make -j$(nproc) install

# 带 sanitizer 的调试构建
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON
```

### 生产构建
```bash
# 生产就绪的静态构建
cmake .. -DCMAKE_BUILD_TYPE=Release -DSTATIC=ON -DMARCH_NATIVE=OFF
make install
```

### 特定包构建
```bash
# 不带数据库支持的构建
cmake .. -DPACKAGE_DB=OFF

# 禁用特定包的构建
cmake .. -DPACKAGE_CRYPTO=OFF -DPACKAGE_COMPRESS=OFF
```

## 测试

### 单元测试
```bash
# 运行所有测试（需要 GTest）
cd build
make test

# 运行特定测试可执行文件
./src/lpc_tests
./src/ofile_tests
```

### LPC 测试
```bash
# 运行 LPC 测试套件
cd testsuite
../build/bin/driver config.test
```

### 集成测试
```bash
# 使用测试配置运行驱动程序
./build/bin/driver testsuite/etc/config.test
```

## 开发工作流

### 代码生成
构建过程中会自动生成多个源文件：
- `grammar.autogen.cc/.h` - 来自 `grammar.y`（Bison）
- `efuns.autogen.cc/.h` - 来自包规范
- `applies_table.autogen.cc/.h` - 来自应用定义
- `options.autogen.h` - 来自配置选项

### 添加新函数
1. 将函数添加到相应包的 `.spec` 文件中
2. 在对应的 `.cc` 文件中实现函数
3. 运行构建以重新生成自动生成的文件
4. 在 `testsuite/` 目录中添加测试

### 调试
```bash
# 构建带有调试符号和 sanitizer 的版本
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON

# 使用 GDB 运行
gdb --args ./build/bin/driver config.test

# 使用 Valgrind 进行内存调试
valgrind --leak-check=full ./build/bin/driver config.test
```

## 平台特定说明

### Ubuntu/Debian
```bash
# 安装依赖
sudo apt install build-essential bison libmysqlclient-dev libpcre3-dev \
  libpq-dev libsqlite3-dev libssl-dev libz-dev libjemalloc-dev libicu-dev
```

### macOS
```bash
# 安装依赖
brew install cmake pkg-config mysql pcre libgcrypt libevent openssl jemalloc icu4c

# 使用环境变量构建
OPENSSL_ROOT_DIR="/usr/local/opt/openssl" ICU_ROOT="/usr/local/opt/icu4c" cmake ..
```

### Windows（MSYS2）
```bash
# 安装 MSYS2 包
pacman -S git mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-zlib mingw-w64-x86_64-libevent \
  mingw-w64-x86_64-pcre mingw-w64-x86_64-icu mingw-w64-x86_64-openssl

# 在 MINGW64 终端中构建
cmake -G "MSYS Makefiles" ..
```

## 关键目录

- `src/` - 主驱动程序源代码
- `src/packages/` - 模块化包实现
- `src/vm/` - 虚拟机和解释器
- `src/compiler/` - LPC 编译器
- `src/net/` - 网络协议实现
- `testsuite/` - LPC 测试程序和配置
- `docs/` - 文档（Markdown 和 Jekyll）
- `build/` - 构建输出目录（自动生成）

## 配置文件

- `Config.example` - 示例驱动程序配置
- `src/local_options` - 本地构建选项（从 `local_options.README` 复制）
- `testsuite/etc/config.test` - 测试配置
- 特定包的 `.spec` 文件定义可用函数

## 常见开发任务

### 添加新包
1. 在 `src/packages/[package-name]/` 中创建目录
2. 添加 `CMakeLists.txt`、`.spec` 文件和源文件
3. 更新 `src/packages/CMakeLists.txt`
4. 在 `testsuite/` 中添加测试

### 修改编译器
1. 编辑 `src/compiler/grammar.y` 以更改语法
2. 如果可用，使用 Bison 重新生成语法
3. 更新相应的编译器组件

### 调试内存问题
1. 使用 `-DENABLE_SANITIZER=ON` 构建
2. 在 LPC 中使用 `mud_status()` 外部函数获取运行时内存信息
3. 查看 `testsuite/log/debug.log` 获取详细日志