---
layout: default
title: master / valid_database
---

### 名称

    valid_database - 保护数据库相关外部函数

### 语法

    mixed valid_database( object caller, string func, mixed *info );

### 描述

    每个数据库操作相关外部函数在执行之前都会调用 valid_database() 方法。如果 valid_database() 返回 0 ，数据库外部函数会执行失败，如果返回 1 ，数据库相关外部函数会尝试执行。第一个参数 `caller` 是调用数据库相关外部函数的对象，第二个参数 `func` 是准备执行的数据库外部函数名（如：db_connect() 或者 db_commit()），第三个参数 'info' （如果适用）是一个附加信息数组，目前被用于 efun::db_connect() 和 efun::db_exec()，前一个函数中 `info` 的格式是 ({ string database, string host, string user }) ，后一个函数中 `info` 的格式是 ({ string sql_query }) 。

    如果禁止执行数据库外部函数，valid_database() 返回 0 。但对 efun::db_connect() 应该返回连接数据库的密码，或者返回 1 。

### 参考

    db_connect(3), db_exec(3)

### 翻译 ###

    雪风(i@mud.ren)
