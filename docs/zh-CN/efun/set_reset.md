---
layout: default
title: system / set_reset
---

### 名称

    set_reset - 修改一个对象的重置时间

### 语法

    varargs void set_reset( object ob, int time );

### 描述

    设置对象 `ob` 的重置时间为从现在开始的 `time` 秒后。如果省略 `time`，对象 `ob` 的重置时间会设置为驱动通用重置时间，其公式为：

    reset_time = current_time + reset_time / 2 + random(reset_time / 2)

### 参考

    reset(4)

### 翻译 ###

    雪风(i@mud.ren)
