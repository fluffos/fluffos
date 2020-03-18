---
layout: default
title: strings / strsrch
---

### 名称

    strsrch() - 在一个字符串中寻找子字符串

### 语法

    int strsrch( string str, string substr | int char, int flag );

### 描述

    strsrch() 在字符串 `str` 中寻找第一个子字符串 `substr` 出现的的位置，如果第三个参数（可选参数） `flag` 是 -1 代表最后一次出现的位置。如果第二个参数是整数，会寻找 ascii 字符（类似C语言中的 strchr() 和 strrchr()）。无法从空字符串或 null 值中寻找。

### 返回值

    返回第一个（或最后一个）匹配的字符串的位置，起始位置从 0 开始，如果返回 -1 代表没有匹配。

### 参考

    explode(3), sscanf(3), replace_string(3), regexp(3)

### 翻译

    雪风(i@mud.ren)
