---
layout: default
title: master / valid_socket
---

### 名称

    valid_socket - 保护 socket 相关外部函数的使用

### 语法

    int valid_socket( object caller, string func, mixed *info );

### 描述

    每个 socket 相关外部函数在执行之前都会调用主控对象中的 valid_socket() 方法。如果 valid_socket() 返回 0 ，socket 外部函数执行失败。如果 valid_socket() 返回 1 ，socket 外部函数会尝试成功。第一个参数 `caller` 是调用 socket 外部函数的对象，第二个参数 `func` 是被调用的 socket 外部函数（如 socket_write() 或 socket_bind() 等）。第三个参数是一个信息数组，数组第一个元素是是文件描述符，第二个元素是 socket 所属对象，第三个元素是 socket 远程连接地址，第四个元素是 socket 绑定的端口。

### 翻译 ###

    雪风(i@mud.ren)
