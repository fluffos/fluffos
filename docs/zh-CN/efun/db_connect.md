---
layout: default
title: db / db_connect
---

### 名称

    db_connect() - 打开一个数据库连接

### 语法

    mixed db_connect( string host, string db );
    mixed db_connect( string host, string db, string user );
    mixed db_connect( string host, string db, string user, int type );

### 描述

    创建一个新的数据库连接到 `host` 上的数据库， `db` 是要连接的数据库，如果不指定用户名 `user` 将使用驱动编译时设置的值做登录ID， `type` 用来选择数据库类型，可用值取决于驱动编译时的配置，如果不指定类型将使用编译时设置的默认数据库类型。

    数据库密码通过主控对象中的 valid_database() 方法设置。

    连接成功返回一个新的数据库连接句柄（整型），否则返回失败消息（字符串型）。

    FlufFOS 支持 MYSQL, SQLITE3 和 PostgreSQL。

    When compiling driver, you need to pass -DPACKAGE_DB=ON and
        -DPACKAGE_DB_MYSQL=X or
        -DPACKAGE_DB_SQLITE=X or
        -DPACKAGE_DB_POSTGRESQL=X
     and you should also pass -DPACKAGE_DB_DEFAULT_DB=X which should be one
     of the value above. If one value is an empty string, the driver support for
     that DB is disabled.

     X here means an integer representing the "type" parameter in db_connect().

     Driver prvoides following pre-defines constant to lib for DB types.

    __USE_MYSQL__   is what -DPACKAGE_DB_MYSQL=<value> is, default to be 1
    __USE_SQLITE3__ is what -DPACKAGE_DB_SQLITE=<value> is, default to be not defined.
    __USE_POSTGRE__ is what -DPACKAGE_DB_POSTGRESQL=<value> is, default to be not defined.
    __DEFAULT_DB__  is what -DPACKAGE_DB_DEFAULT_DB=<value> is, default to be 1

### 参考

    db_close(3), db_status(3), valid_database(4)

### 翻译

    雪风(i@mud.ren)
