---
layout: default
title: interactive / receive
---

### 名称

    receive() - 向当前对象显示消息

### 语法

    int receive( string message );

### 描述

    这个外部函数是驱动程序 add_message() 函数的界面，其目的是向当前对象显示消息。如果当前对象是玩家(interactive)返回 1，否则返回 0 。通常，receive() 应该在 receive_snoop() 或 receive_message() 中被呼叫。

### 参考

    catch_tell(4), receive_message(4), receive_snoop(4), message(3)

### 翻译

    雪风(i@mud.ren)
