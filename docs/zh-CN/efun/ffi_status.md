---
title: ffi / ffi_status
---
# ffi_status

### 名称

    ffi_status() - FFI 句柄计数

### 语法

    mapping ffi_status();

### 描述

    返回一个用于内省与泄漏检查的映射，其整数值分别位于键 "libraries"
    （已打开的 ffi_load() 句柄数）、"functions"（已准备的 ffi_prepare() 句柄
    数）和 "callbacks"（存活的 ffi_callback() 闭包数）之下。

### 参考

    ffi_load(3), ffi_prepare(3), ffi_callback(3)
