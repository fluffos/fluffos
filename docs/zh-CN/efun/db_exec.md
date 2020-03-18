---
layout: default
title: db / db_exec
---

### 名称

    db_exec() - 执行一个 SQL 语句

### 语法

    mixed db_exec( int handle, string sql_query );

### 描述

    通过指定句柄执行 SQL 语句，如果执行成功会返回结果的行数，否则返回错误消息字符串。

### 参考

    db_commit(), db_fetch(3), db_rollback(3), valid_database(4)

### 翻译 ###

    雪风(i@mud.ren)
