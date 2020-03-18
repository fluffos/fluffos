---
layout: default
title: calls / remove_call_out
---

### 名称

    remove_call_out() - 移除一个延迟中的呼叫

### 语法

    int remove_call_out( string fun );
    int remove_call_out();

### 描述

    在第一种模式下移除当前对象中下一个即将呼叫的函数 `fun`，返回值是剩余延时，如果指定函数没有被延时呼叫返回 -1。

    在第二种模式下移除当前对象中所有的延时函数，返回值固定为0。

### 参考

    call_out(3), call_out_info(3).

### 翻译

    雪风(i@mud.ren)
