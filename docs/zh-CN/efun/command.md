---
layout: default
title: interactive / command
---

### 名称

    command() - 像玩家一样执行命令

### 语法

    int command( string str);

### 描述

    让当前对象执行玩家命令 `str`，如果执行失败返回0，否则返回一个代表执行指令消耗时间的数值，数值越大代表越耗时，此数值仅供参考，并不精确。

### 参考

    add_action(3), enable_commands(3)

### 翻译 ###

    雪风(i@mud.ren)
