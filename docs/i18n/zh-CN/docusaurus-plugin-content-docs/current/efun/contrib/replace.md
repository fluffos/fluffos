---
title: contrib / replace
---
# replace

### 名称

    replace() - 替换子串，或批量替换多个子串

### 语法

    string replace( string str, string *|string from, string|void to );

### 描述

    两种形式：

    - `replace(str, from, to)` 返回把 `str` 中每一处 `from` 都替换为 `to` 后的
      结果（这是普通字符串形式，等同于 replace_string() 外部函数）。
    - `replace(str, arr)` 中 `arr` 是形如 `({ from1, to1, from2, to2, ... })`
      的偶数长度数组，依次施加每一组替换。

    当替换对数组长度为奇数时报错。

### 参考

    replace_string(3), replace_dollars(3)
