---
title: ffi / ffi_load
---
# ffi_load

### 名称

    ffi_load() - 加载本地共享库

### 语法

    int ffi_load( string path );

### 描述

    打开 `path` 处的共享库（通过 dlopen/LoadLibrary），返回一个正的库句柄，
    供 ffi_symbol() 与 ffi_prepare() 使用。失败时返回 0，可调用 ffi_error()
    获取失败原因。

    当 `path` 为空字符串时，打开驱动自身的进程映像，从而访问已经链接进来的
    C 符号，例如 libc/libm 中的 sqrt、abs 等。

    每次调用都会经过主控对象的 valid_ffi("load", path, caller) 检查，默认主控
    对象会拒绝。仅当驱动编译时启用了 package_ffi（__PACKAGE_FFI__）时才存在
    此外部函数。

### 参考

    ffi_symbol(3), ffi_prepare(3), ffi_unload(3), ffi_error(3), valid_ffi(4)
