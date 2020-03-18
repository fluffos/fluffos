---
layout: default
title: parsing / query_verb
---

### 名称

    query_verb() - 返回当前执行的命令名称

### 语法

    string query_verb( void );

### 描述

    返回当前指令的名称，如果不是通过指令执行的，返回 0 。这个函数可以用来判断在好几个命令会执行同一个函数时到底是哪一个命令呼叫的函数。

### 参考

    add_action(3)

### 翻译

    雪风(i@mud.ren)
