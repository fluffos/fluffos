---
layout: doc
title: contrib / add_a
---
# add_a

### 名称

    add_a() - 为字符串加上正确的不定冠词

### 语法

    string add_a( string str );

### 描述

    返回在 `str` 前加上 "a " 或 "an " 之后的字符串，冠词依据其后单词的读音
    而非首字母来选择：元音用 "an"，但对若干特例做了处理——"us..."（"a user"、
    "a use"）、"hour..."（"an hour"），以及已经以 "a "/"an " 开头的字符串。
    仅由空格组成的字符串返回 "a "。

### 参考

    add_a(3), vowel(3)
