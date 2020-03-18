---
layout: default
title: interactive / query_ip_name
---

### 名称

    query_ip_name() - 返回指定玩家对象IP的PTR记录

### 语法

    string query_ip_name( object ob );

### 描述

    返回玩家 `ob` IP 的 DNS PTR 记录。每当服务器有新的玩家连线都有触发一个异步 DNS 反向查寻。

    在查询结束前，这个函数的返回值和 'query_ip_number(3)' 一样。

    在查询结束后，如果查询成功，函数会返回对象 `ob` IP 的 DNS PTR 值。如果查询失败，（比如网络问题，这个 IP 没有配置 PTR 记录。）这个函数的返回值还是和 'query_ip_number(3)' 一样。

    查询结果会被缓存，所以这个函数不会有额外开销。

### 参考

    query_ip_number(3), query_host_name(3), resolve(3), socket_address(3)

### 翻译

    雪风(i@mud.ren)
