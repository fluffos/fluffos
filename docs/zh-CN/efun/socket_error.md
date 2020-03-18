---
layout: default
title: sockets / socket_error
---

### 名称

    socket_error() - 返回 socket 错误的文字描述

### 语法

    string socket_error( int error );

### 描述

    socket_error() 返回 socket 错误 `error` 的文字描述

### 返回值

    报错时返回错误描述，否则返回 "socket_error: invalid error number"。

### 参考

    socket_create(3), socket_connect(3)

### 翻译 ###

    雪风(i@mud.ren)
