---
title: ffi / ffi_symbol
---
# ffi_symbol

### 名称

    ffi_symbol() - 解析符号的原始地址

### 语法

    int ffi_symbol( int lib, string name );

### 描述

    在库 `lib` 中解析符号 `name`，以整数形式返回其原始本地地址；未找到则
    返回 0。

    如果只是要“调用”一个 C 函数，并不需要此函数——请使用 ffi_prepare()，它
    会自行解析符号。ffi_symbol() 用于取得数据符号的地址（再用 ffi_peek()
    读取），或取得某个函数的地址以作为 FFI_POINTER 参数传给 C。

    调用会经过主控对象的 valid_ffi("symbol", name, caller) 检查。

### 参考

    ffi_prepare(3), ffi_peek(3), ffi_address(3), valid_ffi(4)
