---
layout: doc
title: ffi / ffi_free
---
# ffi_free

### 名称

    ffi_free() - 释放一块本地内存

### 语法

    void ffi_free( buffer mem );

### 描述

    释放由 ffi_alloc() 分配的 buffer。此调用是可选的——当没有任何 LPC 值引用
    它时，垃圾回收也会回收该内存块。释放后再使用 `mem` 会报错。

### 参考

    ffi_alloc(3)
