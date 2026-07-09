---
layout: doc
title: ffi / ffi_struct_layout
---
# ffi_struct_layout

### 名称

    ffi_struct_layout() - 计算 C 结构体的字段布局

### 语法

    mixed *ffi_struct_layout( int *field_types );

### 描述

    给定一个标量类型码数组（<ffi.h> 中的类型码），每个元素对应结构体的一个
    字段，按平台对齐规则返回 `({ total_size, ({ offset0, offset1, ... }) })`。
    用 total_size 确定 ffi_alloc() 内存块的大小，用偏移量配合 ffi_read()/
    ffi_write()。tools/ffi 生成器会从 C 头文件生成这些数组。

### 参考

    ffi_alloc(3), ffi_read(3), ffi_write(3), ffi_sizeof(3)
