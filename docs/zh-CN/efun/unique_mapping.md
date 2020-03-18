---
layout: default
title: mappings / unique_mapping
---

### 名称

    unique_mapping() - 基于一个函数从一个数组创建一个映射

### 语法

    mapping unique_mapping( mixed *arr, string fun, object ob, mixed extra, ... );
    mapping unique_mapping( mixed *arr, function f, mixed extra, ... );

### 描述

    基于一个函数从一个数组使用以下方式创建一个映射：

    以函数返回值相同的成员组成的数组做为映射的值（value），以函数的返回值做为映射的键（key）

### 参考

    filter_array(3), sort_array(3), map(3)

### 翻译

    雪风(i@mud.ren)
