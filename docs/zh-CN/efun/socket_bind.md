---
layout: default
title: sockets / socket_bind
---

### 名称

    socket_bind() - 绑定 IP 和端口到 socket 连接

### 语法

    int socket_bind( int s, int port );

### 描述

    socket_bind() 为未名称的 socket 指定名称， 当通过 socket_create() 创建一个 socket 连接后，它存在于命名空间中但没有指定名称（译者注：简单的说就是有 IP 没端口），socket_bind() 会请求把端口 `port` 分配给 socket `s`。

### 返回值

    成功时返回 EESUCCESS，失败（错误）时返回代表以下意义的负值：

#### 错误

    EEFDRANGE      连接描述符（Descriptor）超出范围
    EEBADF         无效的连接描述符
    EESECURITY     试图违反安全性
    EEISBOUND      Socket 已经绑定（被命名）
    EEADDRINUSE    地址已经被占用
    EEBIND         绑定（命名）出问题
    EEGETSOCKNAME  获取 socket 名称（getsockname）出问题

    注：所有错误名称定义在驱动程序源文件的 `socket_err.h` 头文件中。

### 参考

    socket_connect(3), socket_create(3), socket_listen(3)

### 翻译 ###

    雪风(i@mud.ren)
