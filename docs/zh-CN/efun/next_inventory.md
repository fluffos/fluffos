---
layout: default
title: objects / next_inventory
---

### 名称

    next_inventory() - 返回和对象在相同环境中的后一个对象

### 语法

    object next_inventory( object ob );

### 描述

    返回和对象 `ob` 环境相同的后一个对象。

    先进入环境的对象顺序靠后，所以 next_inventory() 取得的是比指定对象先进入环境的对象，如果指定对象是第一个进入环境，哪怕环境中有其它对象进入返回值也是 0。

### 参考

    first_inventory(3), all_inventory(3), deep_inventory(3)

### 翻译 ###

    雪风(i@mud.ren)
