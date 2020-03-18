---
layout: default
title: contrib / copy.pre
---

### 名称

    copy - 可以递归的复制一个变量值

### 语法

    mixed copy(mixed arg);

### 说明

    返回变量 `arg` 的复制。变量只能是以下类型：
    - array
    - buffer (if compiled into driver)
    - class
    - mapping

    例如：
    mapping a, b = ({ 1 });
    a = b;
    a[0] = 2;
    printf("%O %O\n", a, b);

    结果是： ({ 2 }) ({ 2 })

    mapping a, b = ({ 1 });
    a = copy(b);
    a[0] = 2;
    printf("%O %O\n", a, b);

    结果是： ({ 2 }) ({ 1 })

### 翻译 ###

    雪风(i@mud.ren)
