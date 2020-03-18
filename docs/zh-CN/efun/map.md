---
layout: default
title: general / map
---

### 名称

    map()  - 通过指定函数修改一个映射、数组或字符串

### 语法

    mixed map( mapping | mixed* | string x, string fun, object ob, mixed extra, ... );
    mixed map( mapping | mixed* | string x, function f, mixed extra, ... );

### 描述

    如果第一个参数是一个映射，map() 的作用和 map_mapping() 一样。如果第一个参数是数组，map() 的作用和 map_array() 一样。如果第一个参数是字符串，map() 把字符串的每个字符转为整数传给函数，如果函数返回值为是非0整数，会转成字符替换原来的字符，如果是0保持不变。

### 参考

    filter_array(3), sort_array(3), map_array(3), map_mapping(3)

### 翻译 ###

    雪风(i@mud.ren)
