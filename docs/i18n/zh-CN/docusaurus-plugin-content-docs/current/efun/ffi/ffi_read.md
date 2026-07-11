---
title: ffi / ffi_read
---
# ffi_read

### 名称

    ffi_read() - 从 buffer 按类型读取一个标量

### 语法

    mixed ffi_read( buffer mem, int offset, int type_code );

### 描述

    从 `mem` 的字节偏移 `offset` 处读取一个 `type_code`（<ffi.h> 中的类型码）
    所表示的标量：整数/指针类型返回 int，FFI_FLOAT/FFI_DOUBLE 返回 float。
    配合 ffi_struct_layout() 可按符号名读取 C 结构体字段。

### 参考

    ffi_write(3), ffi_struct_layout(3), ffi_sizeof(3)
