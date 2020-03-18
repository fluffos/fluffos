---
layout: default
title: buffers / write_buffer
---

### 名称

    write_buffer() - 将缓冲区写入文件或读入数据源到缓冲区

### 语法

    int write_buffer( string dest, int start, mixed source );
    int write_buffer( buffer dest, int start, mixed source );

### 描述

    If 'dest' is a file, then 'source' must be an int (and will be  written
    to the file in network-byte-order), a buffer, or a string, and 'source'
    will be written to the file 'dest' starting at byte # 'start'.

    If 'dest' is a buffer, then 'source' will be written  into  the  buffer
    starting  at  byte  # 'start' in the buffer.  If 'source' is an int, it
    will be written in network-byte-order.

### 参考

    read_buffer(3), allocate_buffer(3)

### 作者

    Truilkan

### 翻译

    雪风(i@mud.ren)
