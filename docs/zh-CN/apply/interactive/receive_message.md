---
layout: default
title: interactive / receive_message
---

### 名称

    receive_message - 提供 message 外部函数的界面

### 语法

    void receive_message( string type, string message );

### 描述

    message() 外部函数调用玩家对象中的这个方法，参数 `type` 用来表明消息类别（say、tell、表情、战斗信息、房间描述等等），参数 `message` 是要处理的消息内容。 receive_message() 方法和 message() 外部函数配合使用可以提供一个智能的客户端信息显示机制。

### 参考

    catch_tell(4), message(3), receive(3), receive_snoop(4)

### 翻译

    雪风(i@mud.ren)
