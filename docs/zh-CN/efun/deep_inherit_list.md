---
layout: default
title: system / deep_inherit_list
---

### 名称

    deep_inherit_list() - 获取一个对象继承的所有对象(parent object)

### 语法

    string *deep_inherit_list( object obj );

### 描述

    返回对象 `obj` 继承（包括直接继承和间接继承）的所有对象的文件名的数组。省略 `obj` 默认对象为 this_object()。

### 参考

    inherit_list(3), inherits(3)

### 翻译 ###

    雪风(i@mud.ren)
