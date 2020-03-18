---
layout: default
title: master / valid_shadow
---

### 名称

    valid_shadow - 控制哪些对象可以被投影

### 语法

    int valid_shadow( object ob );

### 描述

    当一个对象尝试使用 shadow() 外部函数投影对象 `ob` 时，驱动程序会调用主控对象中的 valid_shadown() 方法。参数 `ob` 是 previous_object() 尝试投影的目标。如果不允许被投影， valid_shadow() 应该返回 0 ，在这种情况下 shadow() 会返回 0 并投影失败。如果 valid_shadow() 返回 1，允许投影。

### 参考

    shadow(3), query_shadowing(3)

### 翻译 ###

    雪风(i@mud.ren)
