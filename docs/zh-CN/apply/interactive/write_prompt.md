---
layout: default
title: interactive / write_prompt
---

### 名称

    write_prompt - 显示玩家界面命令提示符

### 语法

    void write_prompt( void );

### 描述

    如果玩家对象中存在 write_prompt() 方法，当需要显示提示符时会调用此方法。如果玩家在输入(in_input)或者编辑(in_edit)状态，驱动程序不会调用此方法。

### 作者

    Truilkan@TMI

### 翻译

    雪风(i@mud.ren)
