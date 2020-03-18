---
layout: default
title: sockets / socket_acquire
---

### 名称

    socket_acquire() - 获得 socket 的所有权

### 语法

    int socket_acquire( int socket, string read_callback, string write_callback, string close_callback );

### 描述

    socket_acquire() 在 socket_release() 把 `socket` 的所有权和控制权转移到新对象时被呼叫用来完成握手。 socket_release() 呼叫新拥有者对象中的 `release_callback` 方法用来通知对象它想移交 `socket` 的所有权。新对象有责任决定是否愿意接收 `socket`，如果愿意，就调用 socket_acquire() 外部函数完成转移，如果不愿意回调只是简单的回应而不完成握手。

    在前一种情况下，握手完成，新对象成为 `socket` 的拥有者，读、写或关闭回调函数的参数都参照新对象中的函数，在新对象中的这些函数都需要定义以保证驱动程序知道调用哪些函数。后一种情况下 socket_release() 返回 EESOCKNOTRLSD 以保证原所有者可以做适合的处理。

    socket_acquire() 应该仅在 `release_callback` 方法中调用并且只使用传递过来的这个 `socket`。

### 参考

    socket_release(3)

### 翻译 ###

    雪风(i@mud.ren)
