---
layout: default
title: functions / functionp
---

### 名称

    functionp() - 判断给定变量是否是函数指针，如果是再判断为何种型态

### 语法

    int functionp( mixed arg );

### 描述

    如果 `arg` 是函数指针返回非零针，否则返回0。函数指针是函数类型的变量，如：

    f = (: func, args... :);

    返回值为驱动程序 "include/function.h" 中定义的函数指针类型。

    function pointer type        value
    ---------------------        -----
    call_other                   FP_CALL_OTHER
    lfun                         FP_LOCAL
    efun                         FP_EFUN
    simul_efun                   FP_SIMUL
    functional                   FP_FUNCTIONAL

    此外，在某些情况下会有以下值：
    (有参数)                  FP_HAS_ARGUMENTS
    (创建函数的对象被摧毁)     FP_OWNER_DESTED
    (无法被绑定)              FP_NOT_BINDABLE

    最后一组值为位元值（bit values），可以用来做位运算测试，FP_MASK 可以忽略某些位测试函数指针的基本型态。

    示例:

    测试一个函数值是否是外部函数（efun）指针：

    if ((functionp(f) & FP_MASK) == FP_EFUN) ...

    测试函数是否有参数：

    if (functionp(f) & FP_HAS_ARGUMENTS) ...

    `function.h` 文件具体内容如下：
    /* codes returned by the functionp() efun */

    #ifndef _FUNCTION_H
    #define _FUNCTION_H

    #define FP_LOCAL 2
    #define FP_EFUN 3
    #define FP_SIMUL 4
    #define FP_FUNCTIONAL 5

    /* internal use */
    #define FP_G_VAR 6
    #define FP_L_VAR 7
    #define FP_ANONYMOUS 8

    /* additional flags */
    #define FP_MASK 0x0f
    #define FP_HAS_ARGUMENTS 0x10
    #define FP_OWNER_DESTED 0x20
    #define FP_NOT_BINDABLE 0x40

    #endif

### 参考

    mapp(3), stringp(3), pointerp(3), objectp(3), intp(3), bufferp(3), floatp(3), nullp(3), undefinedp(3), errorp(3), bind(3)

### 翻译

    雪风(i@mud.ren)
