---
layout: default
title: interactive / terminal_type
---

### 名称

    terminal_type - 通知mudlib用户的终端类型

### 语法

    void terminal_type( string term );

### 描述

    这个 apply 在玩家连线对象上自动调用，变量 `term` 被设置为用户的终端类型，如果客户端一直没有响应（比如客户端不是telnet协议），这个apply不再被调用。

### 翻译 ###

    雪风@mud.ren
