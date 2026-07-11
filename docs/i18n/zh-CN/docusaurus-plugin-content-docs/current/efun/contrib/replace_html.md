---
title: contrib / replace_html
---
# replace_html

### 名称

    replace_html() - 转义字符串以用于 HTML

### 语法

    string replace_html( string str );

### 描述

    返回把 `str` 中的 HTML 特殊字符转义后的结果：`&` 变为 `&amp;`，
    `<` 变为 `&lt;`，`>` 变为 `&gt;`，`"` 变为 `&quot;`。结果长度受驱动的
    最大字符串长度限制。

### 参考

    replace_mxp(3)
