---
layout: default
title: contrib / fetch_variable.pre
---

### 名称

    fetch_variable() - 获取对象中指定全局变量的值

### 语法

    mixed fetch_variable(string arg, object ob | void);

### 描述

    返回对象 `ob` 中定义的全局变量 `arg` 的值，如果不指定对象 `ob`，默认为 this_object()。请注意：变量 `arg` 必须是指定对象中已定义的，否则会报错。

    此外部函数要求驱动程序编译时定义了 PACKAGE_CONTRIB 包。

### 参考

    restore_variable(3), store_variable(3), restore_object(3)

### 翻译

    雪风(i@mud.ren)
