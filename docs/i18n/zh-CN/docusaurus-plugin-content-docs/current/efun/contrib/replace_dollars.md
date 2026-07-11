---
title: contrib / replace_dollars
---
# replace_dollars

### 名称

    replace_dollars() - 展开字符串中的 $ 标记

### 语法

    string replace_dollars( string str, string *pairs );

### 描述

    扫描 `str` 中以 `$` 开头的标记，用 `pairs`——一个交替给出“标记”与其
    “替换文本”的偶数长度数组，例如
    `({ "$name", "阿宝", "$place", "码头" })`——进行替换。未匹配任何标记的
    文本原样保留；若没有任何标记匹配，则返回原字符串。

### 参考

    replace(3), replace_string(3)
