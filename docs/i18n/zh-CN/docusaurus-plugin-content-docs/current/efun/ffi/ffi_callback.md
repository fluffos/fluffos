---
title: ffi / ffi_callback
---
# ffi_callback

### 名称

    ffi_callback() - 把 LPC 函数暴露给 C 作为回调

### 语法

    int ffi_callback( function fn, int ret_type, int *arg_types );

### 描述

    用给定的返回类型码与参数类型码（<ffi.h> 中的类型码）把 LPC 函数指针
    `fn` 包装进一个 libffi 闭包，返回一个回调句柄。把该句柄的 ffi_callback_addr()
    作为 FFI_POINTER 参数传给 C，C 库（qsort() 的比较器、事件处理器等）便可
    回调进 LPC。

    调用会经过主控对象的 valid_ffi("callback", 0, caller) 检查。

### 参考

    ffi_callback_addr(3), ffi_callback_free(3), ffi_call(3), valid_ffi(4)
