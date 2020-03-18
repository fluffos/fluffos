---
layout: default
title: sockets / socket_listen
---

### 名称

    socket_listen() - 监听一个 socket 连接

### 语法

    int socket_listen( int s, string listen_callback );

### 描述

    为了接收连接，需要先使用 socket_create() 创建 socket，并使用 socket_listen() 进入监听模式，最后使用 socket_accept() 接收连接。socket_listen() 仅仅在 STEAM 或 MUD 模式有效。

    参数 `listen_callback` 是监听中的 socket 收到连接请求时驱动程序调用的函数名称，函数原型格式如下：

    void listen_callback(int fd)

    其中参数 `fd` 是正在监听中的 socket 连接。

### 返回值

    成功时返回 EESUCCESS，失败（错误）时返回代表以下意义的负值：

#### 错误

    EEFDRANGE      连接描述符（Descriptor）超出范围
    EEBADF         无效的连接描述符
    EESECURITY     试图违反安全性
    EEMODENOTSUPP  不支持的 Socket 模式
    EENOADDR       Socket 没有绑定地址
    EEISCONN       Socket 已连接
    EELISTEN       监听有问题

### 参考

    socket_accept(3), socket_connect(3), socket_create(3)

### 翻译 ###

    雪风(i@mud.ren)
