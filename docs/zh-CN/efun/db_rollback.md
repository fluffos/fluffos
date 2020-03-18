---
layout: default
title: db / db_rollback
---

### 名称

    db_rollback() - 回滚上一个数据库事务

### 语法

    int db_rollback( int handle );

### 描述

    对于事务数据库，这将回滚最后一组操作。

    回滚成功返回 1，失败返回 0 。

### 说明

    Not yet implemented!

### 参考

    db_commit(3), db_exec(3), valid_database(4)

### 翻译 ###

    雪风(i@mud.ren)
