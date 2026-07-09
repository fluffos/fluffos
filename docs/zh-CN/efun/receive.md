---
layout: doc
title: interactive / receive
---
# receive

### 名称

    receive() - 向当前对象显示消息

### 语法

    void receive( string | buffer message );

### 描述

    这个外部函数是驱动程序 add_message() 函数的界面，其目的是向当前对象显示消息。message 可以是字符串，也可以是原始字节缓冲区(buffer)。通常，receive() 应该在 receive_snoop() 或 receive_message() 中被呼叫。

### 参考

    catch_tell(4), receive_message(4), receive_snoop(4), message(3)

### 翻译

    雪风(i@mud.ren)
