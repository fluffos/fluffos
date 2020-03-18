---
layout: default
title: general / nullp
---

### 名称

    nullp() - 检测给定变量是否是空（null）

### 语法

    int nullp( mixed arg );

### 描述

    如果变量 'arg' 是空值（null）返回 1。 'arg' 在以下几种情况为 null:

    1. 变量没有初始化。
    2. 变量指向一个已经被摧毁的对象。(好像哪怕被摧毁，也会返回0)
    3. 变量是函数的形式参数，但在调用时没有传递实参。
    4. 变量是 call_other 外部函数呼叫对象中不存在的函数的返回值。
    5. 变量是映射变量中不存在的元素。（即 arg = map[不存在的key]）

    本质上 nullp 等于 undefinedp。

### 参考

    mapp(3),  stringp(3),  pointerp(3),  objectp(3),  intp(3),  bufferp(3),
    floatp(3), functionp(3), undefinedp(3), errorp(3)

### 翻译

    雪风(i@mud.ren)
