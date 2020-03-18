---
layout: default
title: objects / clonep
---

### 名称

    clonep() - 检测给定变量是否指向一个复制对象

### 语法

    int clonep( void | mixed arg );

### 描述

    如果参数 `arg` 是对象（objectp()为真）并且设置了 O_CLONE 标志(flag)，返回真（1）。驱动程序会给通过 new() 或 clone_object() 外部函数创建的对象设置 O_CLONE 标志。通过 call_other() 或 load_object() 载入的蓝图对象不是复制对象。

    对复制对象，file_name() 外部函数的返回值会包括一个 `#`。

    如果 clonep() 不指定参数，默认对象为 this_object()。

### 参考

    virtualp(3), userp(3), wizardp(3), objectp(3), new(3), clone_object(3), call_other(3), file_name(3)

### 翻译

    雪风(i@mud.ren)
