---
title: ffi / ffi_sizeof
---
# ffi_sizeof

### 名称

    ffi_sizeof() - 标量类型码的字节大小

### 语法

    int ffi_sizeof( int type_code );

### 描述

    返回在当前平台上，`type_code`（<ffi.h> 中的类型码）所表示的标量 C 类型
    的字节大小。用于确定 ffi_alloc() 内存块的大小以及手工计算偏移量。

### 参考

    ffi_alloc(3), ffi_struct_layout(3), ffi_read(3)
