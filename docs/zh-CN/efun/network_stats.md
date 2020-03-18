---
layout: default
title: contrib / network_stats.pre
---

### 名称

    network_stats() - 返回所有已取名的生物对象

### 语法

    mapping network_stats();

### 描述

    返回网络相关数据统计的映射，内容如下：

    - incoming packets total : int
    - incoming volume total : int
    - outgoing packets total : int
    - outgoing volume total : int

    - incoming packets port X : int
    - incoming volume port X : int
    - outgoing packets port X : int
    - outgoing volume port X : int

    如果驱动编译时定义了 PACKAGE_SOCKETS，还会有以下内容：

    - incoming packets sockets : int
    - incoming volume sockets : int
    - outgoing packets sockets : int
    - outgoing volume sockets : int

### 翻译 ###

    雪风(i@mud.ren)
