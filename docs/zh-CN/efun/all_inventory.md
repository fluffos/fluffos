---
layout: default
title: objects / all_inventory
---

### 名称

    all_inventory() - 返回一个对象中的所有内容对象（inventory）

### 语法

    object *all_inventory( object ob | void );

### 描述

    返回在指定对象 `ob` 中的所有对象组成的数组。如果没有指定参数 `ob` ，默认对象为 this_object()。

### 参考

    first_inventory(3), next_inventory(3), deep_inventory(3)

### 翻译 ###

    雪风(i@mud.ren)
