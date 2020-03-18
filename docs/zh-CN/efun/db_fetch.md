---
layout: default
title: db / db_fetch
---

### 名称

    db_fetch() - 从数据库查询结果集中获取数据数组

### 语法

    mixed *db_fetch( int handle, int row );

### 描述

    从数据库查询的结果集中返回指定行的数据列数组，如果失败会返回错误消息字符串。

### 示例

    mixed dbconn, rows, *res;

    if (stringp(dbconn = db_connect("192.168.10.10", "homestead", "homestead")))
    {
        write(dbconn); /* error */
    }
    else
    {
        rows = db_exec(dbconn, "SELECT name, phone FROM users limit 20");
        if (!rows)
            write("查询结果为空\n");
        else if (stringp(rows))
            write(rows); /* error */
        else
            for (int i = 1; i <= rows; i++)
            {
                res = db_fetch(dbconn, i);
                write(res[0] + " " + res[1] + "\n");
            }

        db_close(dbconn);
    }

### 参考

    db_exec(3), valid_database(4)

### 翻译 ###

    雪风(i@mud.ren)
