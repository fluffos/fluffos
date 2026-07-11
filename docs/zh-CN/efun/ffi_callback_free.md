---
title: ffi / ffi_callback_free
---
# ffi_callback_free

### 名称

    ffi_callback_free() - 释放一个回调闭包

### 语法

    void ffi_callback_free( int cb );

### 描述

    释放回调句柄 `cb` 对应的 libffi 闭包。此调用是可选的——垃圾回收也会回收
    该闭包。当 C 仍持有其地址时，不要释放对应回调。

### 参考

    ffi_callback(3), ffi_callback_addr(3)
