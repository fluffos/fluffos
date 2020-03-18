---
layout: default
title: objects / query_heart_beat
---

### 名称

    query_heart_beat() - 查询指定对象的心跳状态

### 语法

    int query_heart_beat( object ob );

### 描述

    返回指定对象通过 set_heart_beat() 外部函数设置的心跳数值。如果没有心跳，返回 0。

    如果不指定对象 `ob`，默认对象为 this_object()。

### 参考

    heart_beat(4), set_heart_beat(3)

### 翻译 ###

    雪风(i@mud.ren)
