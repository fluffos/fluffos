---
layout: default
title: db / db_close
---

### 名称

    db_close() - 关闭一个数据库连接

### 语法

    int db_close( int handle );

### 描述

    通过给定句柄关闭数据库连接。

    关闭成功返回 1 ，失败返回 0 ，如果指定句柄不存在会报错。

### 参考

    db_connect(3), db_status(3), valid_database(4)

### 翻译 ###

    雪风(i@mud.ren)
