---
layout: default
title: interactive / set_encoding
---

### 名称

    set_encoding() - 设置当前玩家的输入输出编码

### 语法

    string set_encoding( string encoding | void );

### 描述

    设置当前玩家的输入/输出编码。

    如果给定的编码名称不可用，会抛出错误。可用编码取决于你的 ICU 版本，一般来说，中文使用 GBK。

    如果没有指定编码，会重置玩家的编码为 UTF-8。

    返回值为 ICU 规范的编码名称，与 query_encoding() 返回值相同。

### 参考

    query_encoding(3)
