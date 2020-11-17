---
layout: default
title: sockets / socket_create
---

### 名称

    socket_create() - 创建一个 efun socket

### 语法

    int socket_create( int mode, string read_callback );
    int socket_create( int mode, string read_callback, string close_callback );

### 描述

    socket_create() 创建一个 efun socket。参数 `mode` 决定要创建哪种模式的 socket。目前支持的 socket 模式为:

    MUD         使用 TCP 协议传输 LPC 类型数据
    STREAM      使用 TCP 协议传输原始数据
    DATAGRAM    使用 UDP 协议
    ------------------------------------------------
    #define MUD                 0 // for sending LPC data types using TCP protocol.
    #define STREAM              1 // for sending raw data using TCP protocol.
    #define DATAGRAM            2 // for using UDP protocol.
    #define STREAM_BINARY       3
    #define DATAGRAM_BINARY     4
    ------------------------------------------------

    参数 `read_callback` 是在UDP协议下 socket 接收数据后驱动程序调用的函数名称，此函数原型应该是以下格式：

    void read_callback(int fd, mixed message, string addr);

    其中参数 `fd` 是接收到数据的 socket 连接；参数 `message` 是接收到的数据，在非二进制模式下返回值是 utf-8 格式的字符串，在二进制模式下返回值是 buffer；第三个参数 `addr` 为客户端地址。

    参数 `close_callback` 是在TCP协议下 socket 意外关闭时驱动程序调用的函数名称（如不是通过 socket_close() 关闭）。此函数原型应该是以下格式：

    void close_callback(int fd);

    其中参数 `fd` 是被关闭的 socket 连接。注意：close_callback() 不能在 DATAGRAM 模式下使用。

### 返回值

    成功时返回一个非负连接描述符，失败时返回以下意义的负值：

#### 错误

    EEMODENOTSUPP  不支持的 Socket 模式
    EESOCKET       创建 socket 时的问题
    EESETSOCKOPT   设置 socket 选项（setsockopt）时的问题
    EENONBLOCK     设置非阻塞模式（non-blocking mode）时的问题
    EENOSOCKS      没有空余的 efun sockets
    EESECURITY     试图违反安全性

    以上错误定义在驱动程序源文件的 `socket_err.h` 头文件中,具体文件内容如下：
    -----------------------------------------
    #ifndef _SOCKET_ERRORS_H
    #define _SOCKET_ERRORS_H

    #define EESUCCESS 1       /* Call was successful */
    #define EESOCKET -1       /* Problem creating socket */
    #define EESETSOCKOPT -2   /* Problem with setsockopt */
    #define EENONBLOCK -3     /* Problem setting non-blocking mode */
    #define EENOSOCKS -4      /* UNUSED */
    #define EEFDRANGE -5      /* Descriptor out of range */
    #define EEBADF -6         /* Descriptor is invalid */
    #define EESECURITY -7     /* Security violation attempted */
    #define EEISBOUND -8      /* Socket is already bound */
    #define EEADDRINUSE -9    /* Address already in use */
    #define EEBIND -10        /* Problem with bind */
    #define EEGETSOCKNAME -11 /* Problem with getsockname */
    #define EEMODENOTSUPP -12 /* Socket mode not supported */
    #define EENOADDR -13      /* Socket not bound to an address */
    #define EEISCONN -14      /* Socket is already connected */
    #define EELISTEN -15      /* Problem with listen */
    #define EENOTLISTN -16    /* Socket not listening */
    #define EEWOULDBLOCK -17  /* Operation would block */
    #define EEINTR -18        /* Interrupted system call */
    #define EEACCEPT -19      /* Problem with accept */
    #define EEISLISTEN -20    /* Socket is listening */
    #define EEBADADDR -21     /* Problem with address format */
    #define EEALREADY -22     /* Operation already in progress */
    #define EECONNREFUSED -23 /* Connection refused */
    #define EECONNECT -24     /* Problem with connect */
    #define EENOTCONN -25     /* Socket not connected */
    #define EETYPENOTSUPP -26 /* Object type not supported */
    #define EESENDTO -27      /* Problem with sendto */
    #define EESEND -28        /* Problem with send */
    #define EECALLBACK -29    /* Wait for callback */
    #define EESOCKRLSD -30    /* Socket already released */
    #define EESOCKNOTRLSD -31 /* Socket not released */
    #define EEBADDATA -32     /* sending data with too many nested levels */
    #define ERROR_STRINGS 33  /* sizeof (error_strings) */

    #endif /* SOCKET_ERRORS_H */
    -----------------------------------------

### 参考

    socket_accept(3), socket_bind(3), socket_close(3), socket_connect(3), socket_listen(3), socket_write(3), valid_socket(4)

### 翻译 ###

    雪风(i@mud.ren)
