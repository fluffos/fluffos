---
layout: default
title: contrib / store_variable.pre
---

### 名称

    store_variable() - 设置指定对象全局变量的值

### 语法

    void store_variable(string arg, mixed value, object ob | void);

### 描述

    设置对象 `ob` 中定义的全局变量 `arg` 的值为 `value`，如果不指定对象 `ob`，默认为 this_object()。请注意：变量 `arg` 必须是指定对象中已定义的，否则会报错。

    此外部函数要求驱动程序编译时定义了 PACKAGE_CONTRIB 包。

### 参考

    restore_variable(3), fetch_variable(3), save_object(3)

### 翻译

    雪风(i@mud.ren)
