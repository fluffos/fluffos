---
title: contrib / vowel
---
# vowel

### 名称

    vowel() - 判断一个字符是否为元音

### 语法

    int vowel( int c );

### 描述

    若字符编码 `c` 是 ASCII 元音（a、e、i、o、u，不分大小写）则返回 1，
    否则返回 0。常与 add_a() 一起用于选择冠词。

### 参考

    add_a(3)
