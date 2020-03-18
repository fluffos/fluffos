---
layout: default
title: interactive / query_encoding
---

### 名称

    query_encoding() - 返回当前玩家的输入输出编码

### 语法

    string query_encoding();

### 描述

    获取当前玩家的输入输出编码。

    注意：您得到的名称是ICU内部名称，它与 set_encoding() 返回的名称相同，可能是您传入的名称，也可能不是，但它们引用的是相同的编码。

### 参考

    set_encoding(3)
