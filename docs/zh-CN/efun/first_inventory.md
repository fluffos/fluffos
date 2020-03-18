---
layout: default
title: objects / first_inventory
---

### 名称

    first_inventory() - 返回一个对象中包含的第一个物品

### 语法

    object first_inventory( mixed ob | void );

### 描述

    返回对象 `ob` 内容物品中的第一个物品，参数 `ob` 既可以是对象类型也可以是字符串类型(对象的文件名)，如果没有指定参数 `ob`，默认是 this_object()。

### 参考

    file_name(3), next_inventory(3), all_inventory(3), deep_inventory(3)

### 翻译

    雪风(i@mud.ren)
