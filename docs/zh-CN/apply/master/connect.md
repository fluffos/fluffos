---
layout: default
title: master / connect
---

### 名称

    connect - 获取新连线玩家的游戏对象

### 语法

    object connect( void );
    object connect( int port );

### 描述

    当一个新用户连接游戏时驱动程序调用主控对象中当 connect() 方法。这个方法返回的对象作为初始用户对象，注意可以使用 exec() 外部函数把玩家连接从初始用户对象切换到其它对象。

    参数 `port` 为连线的端口。

### 参考

    exec(3), logon(4)

### 翻译

    雪风(i@mud.ren)
