---
layout: default
title: efuns / strings / strlen
---

### 名称

    strwidth() - 返回字符串的显示宽度

### 语法

    int strwidth( string str );

### 描述

    strwidth() 返回字符串 'str' 的显示宽度。

    The driver uses rules defined in https://www.unicode.org/reports/tr11/tr11-36.html to calcuate character width.
    Control characters has no width, Wide characters, including emojis are two column wide. This rule is also used by sprintf() to layout strings.

### 参考

    strlen(3)
