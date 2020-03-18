---
layout: default
title: system / find_call_out
---

### 名称

    find_call_out() - 查询一个计划的呼叫剩余延迟时间

### 语法

    int find_call_out( string func ):

### 描述

    寻找最新一条函数 `func` 延迟呼叫并返回剩余延迟时间。如果没有找到返回 -1。

### 参考

    call_out(3), remove_call_out(3), set_heart_beat(3)

### 翻译 ###

    雪风(i@mud.ren)
