---
layout: doc
title: ffi / ffi_error
---
# ffi_error

### 名称

    ffi_error() - 最近一次 FFI 错误信息

### 语法

    string ffi_error();

### 描述

    返回当前线程最近一次的 FFI 错误信息——例如某次 ffi_load() 返回 0 的原因
    ——若从未发生错误则返回空字符串。FFI 接口上的外部函数在被误用时会抛出
    LPC 错误；ffi_error() 报告的是那些以返回 0 来表示的失败。

### 参考

    ffi_load(3), ffi_symbol(3), ffi_status(3)
