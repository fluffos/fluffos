---
layout: default
title: master / get_bb_uid
---

### 名称

    get_bb_uid - 取得骨干用户ID(backbone uid)

### 语法

    string get_bb_uid( void );

### 描述

    在驱动程序编译时定义了 PACKAGE_UIDS 后有效。

    当游戏启动时驱动程序调用这个方法，在主控对象载入后，获取 MUD 定义当骨干用户ID。这个方法应该返回一个字符串，如："BACKBONE"。

### 参考

    get_root_uid(4)

### 翻译

    雪风(i@mud.ren)
