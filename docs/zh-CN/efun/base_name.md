---
layout: default
title: contrib / base_name.pre
---

### 名称

    base_name() - 返回一个对象的基本文件名

### 语法

    stirng base_name( void );
    string base_name(string | object ob);

### 描述

    返回指定对象的基本文件名（不包括 `#` 以后的部分）。如果不指定参数，默认对象为 this_object()。

    这是 FLUFFOS 新增的外部函数，在 MUDOS 中请用模拟外部函数实现。

### 参考

    file_name(3)

### 翻译 ###

    雪风(i@mud.ren)
