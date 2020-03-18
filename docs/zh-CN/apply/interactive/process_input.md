---
layout: default
title: interactive / process_input
---

### 名称

    process_input - 获取(并可以修改)用户的输入指令

### 语法

    mixed process_input( string arg );

### 描述

    如果玩家对象中存在 process_input() 方法，驱动程序会把玩家输入的每一行内容发送给此方法。利用 process_input() 方法可以很容易的实现指令历史记录等功能，也可以用来限制管理指令的使用，还可以在指令解析前修改玩家的输入。

    如果 process_input() 方法返回字符串，此字符串会取代玩家的输入；如果返回 0 则保持玩家输入的内容不变；如果返回 1 则完全过滤掉此输入内容，不再做任何处理。

### 作者

    Truilkan@TMI

### 翻译

    雪风(i@mud.ren)
