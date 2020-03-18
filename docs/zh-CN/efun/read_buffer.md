---
layout: default
title: buffers / read_buffer
---

### 名称

    read_buffer() - 以缓冲区类型(buffer)传回文件的内容，或以字符串类型传回缓冲区的内容

### 语法

    buffer read_buffer( string src, int start, int len );
    string read_buffer( buffer src, int start, int len );

### 描述

    如果 `src` 是一个字符串（文件名），会从文件第 `start` 字节开始读取长度 `len` 字节的内容并以缓冲区类型返回，如果没有指定起始和长度，会读取整个文件。

    如果 `src` 是一个缓冲区，会从缓冲区第 `start` 字节开始读取长度 `len` 字节的内容并以字符串类型返回。

    请注意，你能从文件读取到缓冲区的最大字节数通过运行时配置文件的 `maximum byte transfer` 参数控制。

### 参考

    write_buffer(3), allocate_buffer(3), bufferp(3), read_bytes(3), write_bytes(3)

### 作者

    Truilkan

### 翻译

    雪风(i@mud.ren)
