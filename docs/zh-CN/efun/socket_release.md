---
layout: default
title: sockets / socket_release
---

### 名称

    socket_release() - 释放 socket 所有权给其它对象

### 语法

    int socket_release( int socket, object ob, string release_callback );

### 描述

    socket_release() 用来改变一个 socket 的所有权和控制权到其它对象。这在接受连接设置然后转换已接受的 socket 连接到其它对象以做后续处理的守护进程对象中很有用。

    socket 所有权的转移涉及当前所属对象和转移目标对象之间的一次握手，当 socket_release() 调用时启动握手。socket_release() 执行必要的安全性和完整性检查后调用对象 `ob` 中的 `release_callback` 方法，这个方法用来通知准备接收 `socket` 所有权的对象 `ob`，对象 `ob` 有责任在 `release_callback` 方法中调用 socket_acquire() 外部函数。如果 socket_acquire() 被调用，握手完成，`socket` 的所有权成功的移交给对象 `ob`。对象 `ob` 也可以不调用 socket_acquire() 以拒绝接收 `socket` 的所有权，在这种情况下 `socket` 的所有权不会改变，而且 `socket` 的当前所有者必须决定如何响应拒绝。

    如果 `socket` 的所有者转移成功，socket_release() 返回 EESUCCESS；如果对象 `ob` 拒绝授受，socket_release() 返回 EESOCKNOTRLSD。其它基于安全检查、错误的 socket 描述符等等错误码也可能返回。

### 参考

    socket_acquire(3)

### 翻译 ###

    雪风(i@mud.ren)
