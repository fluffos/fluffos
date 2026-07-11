---
title: ffi / ffi_peek
---
# ffi_peek

### 名称

    ffi_peek() - 从原始外部地址复制字节到 buffer

### 语法

    buffer ffi_peek( int address, int nbytes );

### 描述

    从原始本地地址 `address` 起复制 `nbytes` 个字节到一块新的、自有的 buffer
    中。这是让某个外部指针背后的字节（例如 C 函数返回的 char*）变成 LPC 值的
    唯一途径。`nbytes` 传 -1 表示复制一个以 NUL 结尾的字符串（有内部上限）。
    取得字节后若为文本，用 string_decode() 解码。

### 参考

    ffi_call(3), ffi_symbol(3), ffi_address(3), string_decode(3)
