---
layout: default
title: master / author_file
---

### 名称

    author_file - 确定给定对象的创造者(author)

### 语法

    string author_file (string file);

### 描述

    这个方法必须存在于主控对象(master object)中。驱动程序中的创造者(author)统计函数调用它来确定给定对象应与哪个创造者关联，这个方法的判断方式完全依据 mudlib 设计者的意图。应该注意的是分配给对象的作者会收到对象的所有行为信息（错误、心跳等等）。

    在游戏启动时驱动程序通过调用 author_file(__MASTER_FILE__) 询问主控对象的创造者。

### 参考

    author_stats(3), domain_stats(3), domain_file(4)

### 翻译

    雪风(i@mud.ren)
