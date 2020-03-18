---
layout: default
title: arrays / allocate
---

### 名称

    allocate() - 配置一个数组

### 语法

    varargs mixed *allocate(int size, void | mixed value);

### 描述

    配置一个有 `size` 个元素的数组。 元素个数必须大于 0 且小于系统上限（通常约10000）。所有元素的值都初始化为 0。

    如果指定第二个参数且参数不是函数指针，值被初始化为指定的参数。如果第二个参数是函数指针，会以数组每个元素的下标为参数调用函数，把函数运算后的结果做为数组对应元素的初始值。

    例如: allocate( 5, (: $1 + 3 :) ) => ({ 3, 4, 5, 6, 7 })

### 返回值

    allocate() 返回数组。

### 参考

    sizeof(3), allocate_mapping(3)

### 翻译

    雪风(i@mud.ren
