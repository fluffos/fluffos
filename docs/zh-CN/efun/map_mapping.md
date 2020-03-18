---
layout: default
title: mappings / map_mapping
---

### 名称

    map_mapping() - 通过一个函数修改映射的元素

### 语法

    mapping map_mapping( mapping map, string fun, object ob, mixed extra, ... );
    mapping map_mapping( mapping map, function f, mixed extra, ... );

### 描述

    返回一个和映射 `map` 键相同但值为由 `ob->fun()` 或函数指针 `f` 处理后的返回值构成的新映射。映射的每一组元素的键和值都会作为参数呼叫处理函数，处理函数形式参数如： fun(mixed key, mixed value, mixed extra,...);

### 参考

    filter_array(3), sort_array(3), map(3)

### 翻译

    雪风(i@mud.ren)
