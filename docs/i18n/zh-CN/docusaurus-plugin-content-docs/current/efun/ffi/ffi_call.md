---
title: ffi / ffi_call
---
# ffi_call

### 名称

    ffi_call() - 调用已准备好的 C 函数

### 语法

    mixed ffi_call( int func, mixed *args );

### 描述

    调用由 ffi_prepare() 返回的函数句柄 `func`。`args` 的元素个数必须与准备
    时的参数个数完全一致；每个元素是 int、float 或 buffer（绝不能是 string
    ——LPC 字符串是 UTF-8 原生的，不会被隐式转换为 char*），并与准备时的
    参数类型码相对应。

    返回值取决于 `ret_type`：标量类型返回 int 或 float；FFI_POINTER 结果在
    拥有其内存时返回 buffer，否则以整数形式返回原始外部指针；FFI_VOID 返回 0。
    参数个数或类型不匹配时会报错，而不会用错误的调用帧去调用 C。

### 参考

    ffi_prepare(3), ffi_peek(3), ffi_alloc(3), string_encode(3), string_decode(3)
