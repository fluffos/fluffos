---
layout: default
title: sockets / socket_connect
---

### 名称

    socket_connect() - 启动一个 socket 连接

### 语法

    int socket_connect( int s, string address, string read_callback, string write_callback );

### 描述

    参数 `s` 是一个 socket 连接，`s` 必须是 STREAM 模式 或 MUD 模式. 参数 `address` 是将要连接的 socket 地址，地址格式类似： "127.0.0.1 23"。

    参数 `read_callback` 是 socket 获取数据时驱动程序调用的函数名称，函数原型应该是以下格式：

    void read_callback(int fd, mixed message)

    其中参数 `fd` 是接受数据的 socket 连接，参数 `message` 是接收到的数据。

    参数 `write_callback` 是 socket 准备写入数据时驱动程序调用的函数名称，函数原型应该是以下格式：

    void write_callback(int fd)

    其中参数 `fd` 是将被写入数据的 socket 连接。

### 返回值

    成功时返回 EESUCCESS ，失败（出错）时返回代表以下意义的负值：

#### 错误

    EEFDRANGE      连接描述符（Descriptor）超出范围
    EEBADF         无效的连接描述符
    EESECURITY     试图违反安全性
    EEMODENOTSUPP  不支持的 Socket 模式
    EEISLISTEN     Socket 正在监听中
    EEISCONN       Socket 已连接
    EEBADADDR      地址格式有问题
    EEINTR         中断的系统调用
    EEADDRINUSE    地址已经被占用
    EEALREADY      操作已在进行中
    EECONNREFUSED  连接被拒绝
    EECONNECT      连接有问题

### 参考

    socket_accept(3), socket_close(3), socket_create(3)

### 翻译 ###

    雪风(i@mud.ren)
