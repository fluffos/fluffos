---
layout: default
title: interactive / receive_snoop
---

### 名称

    receive_snoop - 获取传入的监听内容

### 语法

    void receive_snoop( string message ) ;

### 描述

    限驱动程序编译时定义了 RECEIVE_SNOOP 时有效。

    当一个玩家开始监听另一个玩家时，所有监听到的文本都会发给玩家对象的 receive_snoop() 方法，在这个方法中，你可以根据需要处理文本，常规做法是使用 receive() 外部函数显示监听的内容。

### 参考

    catch_tell(4), receive(3), receive_message(4)

### 翻译

    雪风(i@mud.ren)
