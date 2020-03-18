---
layout: default
title: calls / origin
---

### 名称

    origin() - 检测当前函数是如何调用的

### 语法

    string origin( void );

### 描述

    返回一个表示当前函数的调用方式的字符串，具体定义可以驱动源码的 `include/origin.h` 头文件中找到：

    #define ORIGIN_BACKEND "driver" /* backwards compat */
    #define ORIGIN_DRIVER "driver"
    #define ORIGIN_LOCAL "local"
    #define ORIGIN_CALL_OTHER "call_other"
    #define ORIGIN_SIMUL_EFUN "simul"
    #define ORIGIN_INTERNAL "internal"
    #define ORIGIN_EFUN "efun"
    /* pseudo frames for call_other function pointers and efun pointer */
    #define ORIGIN_FUNCTION_POINTER "function_pointer"
    /* anonymous functions */
    #define ORIGIN_FUNCTIONAL "functional"

### 参考

    previous_object(3)

### 翻译 ###

    雪风(i@mud.ren)
