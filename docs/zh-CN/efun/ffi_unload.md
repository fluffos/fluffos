---
layout: doc
title: ffi / ffi_unload
---
# ffi_unload

### 名称

    ffi_unload() - 释放库句柄

### 语法

    void ffi_unload( int lib );

### 描述

    释放由 ffi_load() 返回的库句柄。由该库准备出来的函数句柄随之失效。
    驱动关闭时也会自动释放所有库。

### 参考

    ffi_load(3), ffi_status(3)
