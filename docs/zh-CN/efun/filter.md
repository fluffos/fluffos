---
layout: default
title: general / filter
---

### 名称

    filter() - 返回一个筛选过的数组或映射

### 语法

    mixed filter( mixed x, string fun, object ob, mixed extra, ... );
    mixed filter( mixed x, function f, mixed extra, ...);

### 描述

    如果第一个参数 `x` 是数组，filter() 的作用和 filter_array() 一样，如果第一个参数 `x` 是映射，filter() 的作用和 filter_mapping() 一样。如果第一个参数是字符串，filter() 会把每个字符转为整型传递给过滤函数，如果过滤函数返回 0 ，此字符被会过滤掉，否则正常返回。

### 参考

    filter_array(3), filter_mapping(3)

### 翻译

    雪风(i@mud.ren)
