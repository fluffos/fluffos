---
layout: default
title: strings / set_bit
---

### 名称

    set_bit() - 设置字符串位域的指定位为 1

### 语法

    string set_bit( string str, int n );

### 描述

    返回字符串 `str` 的第 `n` 位（bit）设置为 1 后的新字符串，注意原始字符串 `sr` 不会被改变。

    `n` 的上限在运行时配置文件中限制，如果需要，新字符串的长度会自动增加。可打印字符串的每个字节长度为 6 位。

### 参考

    clear_bit(3), test_bit(3)

### 翻译

    雪风(i@mud.ren)
