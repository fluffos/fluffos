---
layout: default
title: strings / sscanf
---

### 名称

    sscanf() - 从一个字符串匹配格式化输入

### 语法

    int sscanf( string str, string fmt, mixed var1, mixed var2 ... );

### 描述

    以格式 `fmt` 分析字符串 `str`，`fmt` 可以包含使用 `%d` 或 `%s` 分隔的字符串，每一个 `%d` 和 `%s` 按顺序对应一个变量参数 `var1`、`var2`...。`%d` 读入整数到变量，`%s` 读入字符串到变量。* 也可以用在格式定义符中（如 %*d 和 %*s），表示忽略对应的内容（不分配给变量）。LPC语言中的 sscanf() 函数和C语言中的同名函数功能类似，但又稍有不同，一方面不需要像C语言一样对普通变量使用取地址运算符 &，另一方面在LPC语言中 sscanf(str, "%s %s",  str1,  str2) 会把第一个单词分配给变量str1，剩余部分分配给str2，而在C语言中str2只会是第二个单词。请注意 `%s%s` 是不能使用的。

    返回值是匹配字符串的格式定义符数量。

### 参考

    explode(3), replace_string(3), strsrch(3)

### 翻译

    雪风(i@mud.ren)
