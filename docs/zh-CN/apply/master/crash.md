---
layout: default
title: master / crash
---

### 名称

    crash - 当驱动程序崩溃时在主控对象中调用的方法

### 语法

    void crash( string crash_message, object command_giver, object current_object );

### 描述

    当驱动程序崩溃时，驱动程序会调用主控对象中的 crash() 方法。这个方法提供了一种在驱动程序崩溃前关闭游戏并保证玩家和其它重要数据安全的方式。这也让你可以记录各种有用的信息，像是什么信号造成驱动程序崩溃、哪些对象是活动的、当前玩家是谁等等。

### 参考

    shutdown(3)

### 翻译

    雪风(i@mud.ren)
