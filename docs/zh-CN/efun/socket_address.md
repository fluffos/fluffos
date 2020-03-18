---
layout: default
title: sockets / socket_address
---

### 名称

    socket_address() - 返回 efun socket 的远程连接地址

### 语法

    string socket_address( int s );

### 描述

    socket_address()  返回 efun socket `s` 的远程连接地址，返回地址格式类似："127.0.0.1 23"

### 返回值

    成功返回地址字符串，失败返回一个空字符串。

### 参考

    socket_connect(3), socket_create(3), resolve(3), query_host_name(3), query_ip_name(3), query_ip_number(3)

### 翻译 ###

    雪风(i@mud.ren)
