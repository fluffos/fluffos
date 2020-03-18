---
layout: default
title: objects / virtualp
---

### 名称

    virtualp() - 检测指定变量是否是虚拟对象

### 语法

    int virtualp( object arg );

### 描述

    如果参数 `arg` 是对象并且有 O_VIRTUAL 标志(flag)，返回 1。由主控对象（master()）中 compile_object() 方法创造出来的对象，驱动程序都会设置 O_VIRTUAL 标志。

### 参考

    clonep(3), userp(3), wizardp(3), objectp(3), new(3), clone_object(3), call_other(3), file_name(3)

### 翻译

    雪风(i@mud.ren)
