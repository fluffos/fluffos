---
layout: default
title: sockets / socket_close
---

### 名称

    socket_close() - 关闭一个 socket

### 语法

    int socket_close( int s );

### 描述

    socket_close() 关闭 socket `s`，这将释放一个 socket 位置以供使用。

### 返回值

    成功返回 EESUCCESS ，失败返回代表以下错误的负值：

#### 错误

    EEFDRANGE      连接描述符（Descriptor）超出范围
    EEBADF         无效的连接描述符
    EESECURITY     试图违反安全性

### 参考

    socket_accept(3), socket_create(3)

### 翻译 ###

    雪风(i@mud.ren)
