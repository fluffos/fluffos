---
layout: default
title: calls / previous_object
---

### 名称

    previous_object() - 返回呼叫当前函数的对象或对象列表

### 语法

    mixed previous_object( int | void );

### 描述

    返回呼叫当前函数的对象指针（如果存在）。请注意：呼叫局部函数不会改变 previous_object() 为当前对象。如果传递一个正整数为参数，就会传回先前第几个呼叫的对象，previous_object(0) 等于 previous_object()，previous_object(1) 是 previous_object() 的 previous_object()，以此类推。而 previous_object(-1) 返回包括所有 previous_object 的对象数组。

### 参考

    call_other(3), call_out(3), origin(3)

### 翻译 ###

    雪风(i@mud.ren)
