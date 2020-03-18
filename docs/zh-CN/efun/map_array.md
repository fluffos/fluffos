---
layout: default
title: arrays / map_array
---

### 名称

    map_array() - 返回一个通过指定函数修改数组元素的新数组

### 语法

    mixed *map_array(mixed *arr, string fun, object ob, mixed extra, ... );
    mixed *map_array(mixed *arr, function f, mixed extra, ... );

### 描述

    返回由数组 `arr` 中的元素经过对象 `ob` 中的方法 `fun` 或函数指针 `f` 处理后的元素组成的新数组。如果有额外参数 `extra`，也会做为参数传递给处理函数。

    主要功能：foreach (index) arr[index] = ob->fun(arr[index], extra);

    如果 'arr' 不是数组，函数返回0。

### 参考

    filter_array(3), sort_array(3), map(3)

### 翻译

    雪风(i@mud.ren)
