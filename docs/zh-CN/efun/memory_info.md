---
layout: default
title: internals / memory_info
---

### 名称

    memory_info - 获取指定对象或总体内存使用情况的信息

### 语法

    varargs int memory_info( object ob );

### 描述

    如果指定参数 `ob` ，memory_info() 返回对象 `ob` 的内存占用量，如果不指定参数，memory_info() 返回所有对象的内存占用量。请注意，返回值为近似值，MUD 的内存占用量并不完全等于系统分配给 MUD 的内存。

### 参考

    debug_info(3), malloc_status(3), mud_status(3)

### 翻译

    雪风(i@mud.ren)
