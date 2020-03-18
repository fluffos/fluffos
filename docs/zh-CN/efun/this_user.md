---
layout: default
title: interactive / this_player
---

### 名称

    this_player() | this_user() - 返回当前玩家对象

### 语法

    object this_player( int flag );
    object this_user( int flag );

### 描述

    返回呼叫当前函数的玩家对象。需要注意的是：即使是从玩家对象本身呼叫 this_player() 和 this_object()，返回值也可能不同。如果使用 this_player(1)，返回呼叫此函数的玩家对象在某些情况下返回值和使用 this_player() 不同（比如系统管理员使用 command() 函数强制玩家使用指令时）。

### 参考

    this_object(3)

### 翻译

    雪风(i@mud.ren)
