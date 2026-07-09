---
layout: doc
title: ffi / ffi_address
---
# ffi_address

### 名称

    ffi_address() - buffer 的原始本地地址

### 语法

    int ffi_address( buffer mem );

### 描述

    以整数形式返回 `mem` 首字节的原始本地地址——用于作为指针值传给 C，或作
    指针比较。该地址仅在 `mem` 存活期间有效。

### 参考

    ffi_alloc(3), ffi_peek(3), ffi_call(3)
