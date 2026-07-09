---
layout: doc
title: ffi / ffi_prepare
---
# ffi_prepare

### 名称

    ffi_prepare() - 描述并准备一个可调用的 C 函数

### 语法

    int ffi_prepare( int lib, string name, int ret_type, int *arg_types );

### 描述

    在 `lib` 中解析 `name`，按所描述的签名构建平台调用帧，并返回一个可供
    ffi_call() 使用的函数句柄。`ret_type` 及 `arg_types` 的每个元素都是
    <ffi.h> 中的类型码（FFI_INT32、FFI_DOUBLE、FFI_POINTER 等）。

    调用会经过主控对象的 valid_ffi("prepare", name, caller) 检查。若符号不存在
    或类型码非法则报错。

### 参考

    ffi_call(3), ffi_load(3), ffi_sizeof(3), valid_ffi(4)
