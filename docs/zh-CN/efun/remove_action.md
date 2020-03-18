---
layout: default
title: interactive / remove_action
---

### 名称

    remove_action - 取消局部函数和指令的绑定

### 语法

    int remove_action( string fun, string cmd );

### 描述

    取消玩家指令 `cmd` 和函数 `fun` 的绑定。基本上，remove_action() 是 add_action() 的反向操作，当一个指令不再需要时，可以使用 remove_action() 移除。

### 返回值

    成功返回 1，失败返回 0。

### 参考

    add_action(3), query_verb(3), init(4)

### 翻译

    雪风(i@mud.ren)
