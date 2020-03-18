---
layout: default
title: filesystem / read_file
---

### 名称

    read_file() - 读取一个文件到字符串

### 语法

    string read_file( string file );
    string read_file( string file, int start_line );
    string read_file( string file, int start_line, int number_of_lines );

### 描述

    从文件 `file` 读取指定行数文本到字符串。第二、三个参数为可选参数，如果只指定第一个参数会读取整个文档。

   `start_line` 指定你希望从文档的第几行开始读，`number_of_lines ` 为要读取的行数，如果起始行数超过文件尾行或要读取的行数为负，函数返回 0 。

### 参考

    write_file(3), read_buffer(3)

### 翻译

    雪风(i@mud.ren)
