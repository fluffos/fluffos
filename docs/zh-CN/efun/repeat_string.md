---
layout: default
title: contrib / repeat_string.pre
---

### 名称

    repeat_string() - 以指定的次数重复一个字符串,并返回新生成的字符串

### 语法 ###

    string repeat_string( string str, int i );

### 描述 ###

    以指定的次数 `i` 重复一个字符串 `str`,并返回新生成的字符串。如果 `i` 小于等于 0 返回空字符串。如果 `i` 大于 0 ，请注意生成的字符串长度不能超出系统最大长度。

### 翻译

    雪风(i@mud.ren)
