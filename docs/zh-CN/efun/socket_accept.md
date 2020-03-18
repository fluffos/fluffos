---
layout: default
title: sockets / socket_accept
---

### 名称

    socket_accept() -在一个 socket 上接受连接

### 语法

    int socket_accept( int s, string read_callback, string write_callback );

### 描述

    参数 `s` 是使用 socket_create() 创建的已调用 socket_bind() 绑定地址并通过 socket_listen() 进入监听状态的 socket。 socket_accept() 从待处理的连接队列中提取第一个连接创建一个和 `s` 有相同属性的新的 socket 并分配一个新的文件描述符，如果队列中不存在待处理的连接，socket_accept() 返回一个下面描述的错误。已接受的 socket 用来从已连接到它的 socket 中读数据或向其中写数据，它不能用来接受别的连接，原始的socket `s` 会保持打开状态以接受新的连接。

    参数 `read_callback` 是当新的 socket （不是正在接收中的 socket）收到数据时驱动程序调用的函数名称，这个函数的原型应该是以下格式：

    void read_callback(int fd);

    其中参数 `fd` 是已准备好接收数据的 socket。

    参数 `write_callback` 是当新的 socket （不是正在接收中的 socket） 准备好写入数据时驱动程序调用的函数名称，这个函数的原型应该是以下格式：

    void write_callback(int fd);

    其中参数 `fd` 是已准备好被写入数据的 socket。

    注意：当新的socket 异常关闭时，正在接收中的 socket （不是新的 socket）中的 close_callback 方法会被调用，和调用 socket_close() 的结果不同，close_callback 函数原型应该是以下格式：

    void close_callback(int fd)

    其中参数 `fd` 是被关闭的 socket 连接。

### 返回值

    socket_accept() 在成功时为接受的 socket 返回一个非负描述符，失败时返回一个错误值，可以使用 socket_error() 外部函数获取错误值的文字描述。

#### 错误

    EEFDRANGE      连接描述符（Descriptor）超出范围
    EEBADF         无效的连接描述符
    EESECURITY     试图违反安全性
    EEMODENOTSUPP  不支持的 Socket 模式
    EENOTLISTN     Socket 没有开启监听
    EEWOULDBLOCK   操作会阻塞
    EEINTR         中断的系统调用
    EEACCEPT       接收出问题
    EENOSOCKS      没有空余的 efun sockets

### 参考

    socket_bind(3), socket_connect(3), socket_create(3), socket_listen(3)

### 翻译 ###

    雪风(i@mud.ren)
