---
layout: default
title: mudlib / find_living
---

### 名称

    find_living() - 查找与给定名字匹配的生物

### 语法

    object find_living( string str );

### 描述

    首先查找标记为“活的”对象（即生物），然后查找ID `str`。生物是曾经呼叫过 enable_commands() 外部函数的对象，而且必须使用 set_living_name() 外部函数设置了名字，这些生物的名字会被存入 hash 表方便快速搜索。

### 参考

    living(3), livings(3), find_player(3), users(3), enable_commands(3), set_living_name(3)

### 翻译

    雪风(i@mud.ren)
