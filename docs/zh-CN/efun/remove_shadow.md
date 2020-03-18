---
layout: default
title: contrib / remove_shadow.pre
---

### 名称

    remove_shadow() - 移除对象的投影

### 语法

    int remove_shadow(void | object ob);

### 描述

    移除对象 `ob` 上的投影，如果对象没有投影或被投影返回 0 ，否则返回 1 。如果不指定对象默认为 this_object()。

    此外部函数仅在驱动编译时没有定义 NO_SHADOWS 时有效。

### 参考

    shadow(3), query_shadowing(3)

### 翻译 ###

    雪风(i@mud.ren)
