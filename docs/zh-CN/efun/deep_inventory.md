---
layout: default
title: objects / deep_inventory
---

### 名称

    deep_inventory() - 返回对象中所有嵌套对象清单的数组

### 语法

    object *deep_inventory( object ob | void );

### 描述

    返回对象 `ob` 中的所有对象及对象中的所有对象（层层递进查找，直到传回所有对象为止）的数组。如果没有指定参数 `ob`，默认是 this_object()。

### 参考

    first_inventory(3), next_inventory(3), all_inventory(3)

### 翻译

    雪风(i@mud.ren)
