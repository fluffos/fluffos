---
title: 排障
---

# 排障

本页先讲编译和启动失败，再讲如何提交有用的崩溃报告。若还没跑起来，先看
[从零到一个能连上的 MUD](start) 或 [LLM 约定](llm)。

## 编译

**CMake 找不到 ICU、OpenSSL、PCRE、jemalloc、MySQL、SQLite 或 PostgreSQL。**
按 [构建指南](build) 安装对应 `-dev` / Homebrew / MSYS2 包，装完后清空 `build/` 再跑 CMake。

**CMake 太旧。** 需要 3.22+。Ubuntu 22.04+ 即可。

**WSL 极慢。** 树必须在 Linux 文件系统（`~/fluffos`），不要 `/mnt/c/...`。

**Windows 上 crypto / MySQL 报错。** 文档里的 MSYS2 开关会关掉它们：
`-DPACKAGE_CRYPTO=OFF -DPACKAGE_DB_MYSQL="" -DPACKAGE_DB_SQLITE=1`。

## 启动与连接

**`Bad mudlib directory`。** 当前目录不对。`config.test` 的 mudlib 是 `./`：

```bash
cd testsuite
../build/bin/driver etc/config.test
```

**端口占用 / `Address already in use`。** 4000–4003 已被占用。不要同时跑两个驱动。

**打印版本后立刻退出。** 看控制台和 `testsuite/log/debug.log`。常见原因：找不到 master、`/single/master` 编译失败、`include directories` 写错。

**Telnet 连上立刻断开。** `master::connect()` 克隆 login 对象失败。同一份日志。

## 消毒器与 Valgrind

随机崩溃多半是更早的内存破坏。Linux 上优先用 **Clang + AddressSanitizer + UBSan**：

```bash
export CC=clang CXX=clang++
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON ..
make -j"$(nproc)" install
```

Ubuntu 还需要 `libdw-dev` `libbz2-dev`。Valgrind 更慢，只在不能跑 ASan 时再用。不要把 Valgrind 当默认第一步。

## 提交 Issue

到 [GitHub Issues](https://github.com/fluffos/fluffos/issues)，务必包含：

1. 启动时打印的**版本行**（`master` 提交或 tag）
2. 操作系统与编译器
3. **完整**控制台和 `debug.log` 片段，不要转述
4. 能复现的最小 LPC（如有）
5. 内存问题：Debug 消毒器构建的 ASan/UBSan 日志
