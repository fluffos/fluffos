---
layout: default
title: mudlib / living
---

### 名称

    living() - 检测给定对象是否是活的（living）

### 语法

    int living( object ob );

### 描述

    如果对象 `ob` 是活的（living）返回 1。关于 living 的定义：如果一个对象呼叫了 `enable_commands()` 外部函数，这个对象就活了。

### 参考

    interactive(3), enable_commands(3)

### 翻译

    雪风(i@mud.ren)
