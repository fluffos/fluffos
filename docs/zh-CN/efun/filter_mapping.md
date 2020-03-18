---
layout: default
title: mappings / filter_mapping
---

### 名称

    filter_mapping() - 通过一个函数过滤映射（mapping）变量中的元素

### 语法

    mapping filter_mapping( mapping map, string fun, object ob, mixed extra, ... );
    mapping filter_mapping( mapping map, function f, mixed extra, ... );

### 描述

    返回经 `ob->fun()` 或函数指针 `f` 处理后的映射。映射的每一组 key 和 value 都会做为参数传递给过滤函数，如果有附加参数 `extra`，也会按顺序传递给过滤函数。函数返回 0 此组元素会被过滤掉，否则正常返回。过滤函数的参数形式： fun(mixed key, mixed value, mixed extra,...);

    说明：此函数本质上就是 filter() 外部函数。

### 参考

    filter(3), filter_array(3), sort_array(3), map(3)

### 翻译

    雪风(i@mud.ren)
