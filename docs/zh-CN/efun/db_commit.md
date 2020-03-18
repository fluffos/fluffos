---
layout: default
title: db / db_commit
---

### 名称

    db_commit() - 提交上一个数据库事务

### 语法

    int db_commit(int handle );

### 描述

    对于事务数据库，这将提交最后一组操作。

    提交成功返回 1，失败返回 0 。

### 说明

    Not yet implemented!

### 参考

    db_exec(3), db_rollback(3), valid_database(4)

### 翻译 ###

    雪风(i@mud.ren)
