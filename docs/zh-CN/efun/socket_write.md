---
layout: default
title: sockets / socket_write
---

### 名称

    socket_write() - 从 socket 发送信息

### 语法

    int socket_write( int s, mixed message );
    int socket_write( int s, mixed message, string address );

### 描述

    socket_write() 在 socket `s` 上发送消息 `message`，如果 socket `s` 是 STREAM 或 MUD 模式，socket 必须已经连接且不能指定参数 `address`，如果 socket `s` 是 DATAGRAM 模式, 地址参数 `address` 必须指定，地址格式类似: "127.0.0.1 23"。

### 返回值

    成功时返回 EESUCCESS，失败（错误）时返回代表以下意义的负值：

#### 错误

    EEFDRANGE      连接描述符（Descriptor）超出范围
    EEBADF         无效的连接描述符
    EESECURITY     试图违反安全性
    EENOADDR       Socket 没有绑定地址
    EEBADADDR      地址格式有问题
    EENOTCONN      Socket 没有连接
    EEALREADY      操作已在进行中
    EETYPENOTSUPP  不支持的对象类型
    EEBADDATA      发送嵌套级别过多的数据
    EESENDTO       sendto 有问题
    EEMODENOTSUPP  不支持的 Socket 模式
    EEWOULDBLOCK   操作会阻塞
    EESEND         send 有问题
    EECALLBACK     等待回调中

### 参考

    socket_connect(3), socket_create(3)

### 翻译 ###

    雪风(i@mud.ren)
