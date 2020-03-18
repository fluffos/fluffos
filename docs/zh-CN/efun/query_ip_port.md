---
layout: default
title: contrib / query_ip_port.pre
---

### 名称

    query_ip_port() - 返回指定玩家的连线端口

### 语法

    int query_ip_port(void | object ob);

### 描述

    返回指定玩家对象 `ob` 的连线端口，如果不指定玩家，默认为 this_player()。如果对象非在线玩家返回 0 。

### 参考

    interactive(3), this_player(3)

### 翻译 ###

    雪风(i@mud.ren)
