---
title: contrib / replace_mxp
---
# replace_mxp

### 名称

    replace_mxp() - 转义字符串以用于 MXP

### 语法

    string replace_mxp( string str );

### 描述

    返回把 `str` 转义为 MXP 输出后的结果：`&`、`<`、`>` 变为对应的实体
    （`&amp;`、`&lt;`、`&gt;`），每个换行符转换为安全行的 MXP `<BR>` 标记。
    结果长度受驱动的最大字符串长度限制。

### 参考

    replace_html(3)
