---
layout: default
title: interactive / gmcp
---

### 名称

    gmcp - 提供从客户端接收的GMCP数据的接口

### 语法

    void gmcp( string message ) ;

### 描述

    当驱动程序从客户端接收GMCP数据时调用此apply方法，此函数应放在玩家对象中，并可用和receive_message类似的方式处理数据。