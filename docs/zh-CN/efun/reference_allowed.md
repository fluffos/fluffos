---
title: contrib / reference_allowed
---
# reference_allowed

### 名称

    reference_allowed() - Discworld playtester 引用检查

### 语法

    int reference_allowed( object referee, string|object|void referrer );

### 描述

    Discworld mudlib 辅助函数：查询 `referrer`（默认为 this_player()，若给出
    字符串则按名字查找）是否被允许持有对 `referee` 的引用，会参考 mudlib 的
    playtester 与 player 处理器。当这些处理器不存在时返回 0，因此仅在提供了
    它们的 mudlib 中才有意义。

### 参考

    this_player(3)
