---
layout: doc
title: ffi / ffi_write
---
# ffi_write

### 名称

    ffi_write() - 向 buffer 按类型写入一个标量

### 语法

    void ffi_write( buffer mem, int offset, int type_code, mixed value );

### 描述

    将 `value` 按 `type_code`（<ffi.h> 中的类型码）写入 `mem` 的字节偏移
    `offset` 处：整数/指针类型的 `value` 为 int，FFI_FLOAT/FFI_DOUBLE 为
    float。用于在 ffi_call() 之前填充结构体字段和出参。

### 参考

    ffi_read(3), ffi_struct_layout(3), ffi_alloc(3)
