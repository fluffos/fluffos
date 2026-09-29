---
title: strings / trim
---
# trim

### 名称

    trim() - 移除字符串两侧的空白字符或其他预定义字符

### 语法

    string trim( string str );
    string trim( string str, string ch);

### 描述

    移除字符串 `str` 两侧的空格或其他预定义字符 `ch`，并返回一个新的字符串。

    第二参数是 Unicode 字符集合，不是字节集合。`trim("《三字经》", "　")`
    不会切坏书名号：U+3000 与 U+300A 的 UTF-8 前缀相同，但是两个不同的字符。

### 参考

    ltrim(3), rtrim(3)
