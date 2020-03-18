---
layout: default
title: master / domain_file
---

### 名称

    domain_file - 确定给定对象的域(domain)

### 语法

    string domain_file( string file );

### 描述

    这个方法必须存在于主控对象中，驱动程序中的域统计函数调用它来确定给定对象应该和什么域关联。这个方法的判断方式完全依据 mudlib 设计者的意图，应该注意的是分配给对象的域会收到对象的所有行为信息（错误、心跳等等）。

    在游戏启动是驱动程序会调用 domain_file("/") 询问骨干域。

### 参考

    author_stats(3), domain_stats(3), author_file(4)

### 作者

    Wayfarer@Portals

### 翻译

    雪风(i@mud.ren)
