---
layout: doc
title: ffi / ffi_callback_addr
---
# ffi_callback_addr

### 名称

    ffi_callback_addr() - 回调闭包的原始地址

### 语法

    int ffi_callback_addr( int cb );

### 描述

    以整数形式返回回调句柄 `cb`（来自 ffi_callback()）所对应闭包的原始本地
    代码地址——即作为 FFI_POINTER 参数传给 C 的那个值。

### 参考

    ffi_callback(3), ffi_callback_free(3), ffi_call(3)
