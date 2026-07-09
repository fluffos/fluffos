---
layout: doc
title: ffi / ffi_alloc
---
# ffi_alloc

### 名称

    ffi_alloc() - 分配一块本地内存并以 buffer 返回

### 语法

    buffer ffi_alloc( int nbytes );

### 描述

    返回一个 `nbytes` 字节、已清零的 buffer，其存储即为本地内存块本身，
    适合用于结构体、出参以及传给 ffi_call() 的指针参数。该内存块像任何 buffer
    一样被引用计数并由垃圾回收释放；ffi_free() 可提前释放。

### 参考

    ffi_free(3), ffi_read(3), ffi_write(3), ffi_address(3), ffi_struct_layout(3)
