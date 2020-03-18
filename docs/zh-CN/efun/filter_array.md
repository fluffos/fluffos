---
layout: default
title: arrays / filter_array
---

### 名称

    filter_array() - 返回一个筛选过的子数组

### 语法

    mixed *filter_array( mixed *arr, string fun, object ob, mixed extra, ... );
    mixed *filter_array( mixed *arr, function f, mixed extra, ...);

### 描述

    filter_array() 返回数组 `arr` 中成功通过对象 `ob` 中的方法 `fun` 或函数指针 `f` 处理的元素组成的子数组。数组 `arr` 的每个元素都会做为参数传递给函数过滤，如果有额外参数 `extra`，也会传给过滤函数。如果过滤函数结果为1，元素会在子数组返回，否则会被过滤掉。如果 `arr` 不是数组，filter_arry()返回0。

    说明：此函数本质上就是 filter() 外部函数。

### 返回值

    过虑后的子数组或0

### 参照

    filter(3), sort_array(3), map_array(3)

### 翻译

    雪风(i@mud.ren)
