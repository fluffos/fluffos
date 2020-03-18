---
layout: default
title: interactive / snoop
---

### 名称

    snoop() - 监听一个在线玩家

### 语法

    varargs object snoop( object snooper, object snoopee );

### 描述

    当使用2个参数时，对象 `snooper` 开始监听对象 `snoopee`。当第二个参数省略时，关闭 `snooper` 的所有监听。通常使用模拟外部函数控制 snoop() 的安全性。

    如果函数执行成功，使用2个参数时返回 `snoopee`，只有一个参数时返回 `snooper`。如果执行失败返回 0 。

### 参考

    query_snoop(3), query_snooping(3)

### 翻译

    雪风(i@mud.ren)
