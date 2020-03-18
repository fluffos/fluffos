---
layout: default
title: strings / replace_string
---

### 名称

    replace_string() - 替换字符串中所有符合条件的内容

### 语法

    string replace_string( string str, string pattern, string replace );
    string replace_string( string str, string pattern, string replace, int max );
    string replace_string( string str, string pattern, string replace, int first, int last );

### 描述


    replace_string() 返回字符串 `str` 中所有 `pattern` 替换成 `replace` 的新字符串。如果没有可替换的 `pattern` 返回原字符串 `str`。请注意：如果替换后字符串超过系统最大长度会报错。

    replace_string() 可以使用 `replace` 为空的字符串来移除字符串 `str` 中的指定字符。如：replace_string("  1  2  3  ", " ", "") 会返回 "123"。

    第四和第五个参数为可选参数，其作用如下：

    如果只指定第四个参数，为最大替换次数（基数从1开始），如果值为 0 代表替换所有，否则为替换指定次数，如： replace_string("xyxx", "x", "z", 2) 会返回 "zyzx"。

    如果指定第五个参数，那么第四个和第五个参数代表替换范围。如果 `first` < 1，代表从头开始全部替换；如果 `last` == 0 或 `last` > 字符串最大长度代表替换到字符串末尾；如果 `first` > `last` 不替换任何内容。如：replace_string("xyxxy", "x", "z", 2, 3) 会返回 "xyzzy"。

### 参考

    sscanf(3), explode(3), strsrch(3)

### 作者

    Zak@TMI-2 wrote the range constraint additions.

### 翻译

    雪风(i@mud.ren)
