---
layout: default
title: interactive / commands
---

### 名称

    commands() - 返回当前对象可以使用的行为指令的一些资料

### 语法

    mixed *commands( void );

### 描述

    返回当前对象可用的指令资料，这是一个包括每个可用指令 4 项资料的二维数组，第 1 项是通过 add_action() 外部函数增加的指令的名称，第 2 项是指令标志（add_action() 命令的第三个参数，通常为0），第 3 项是定义此指令的对象，第 4 项是指令呼叫的函数（函数指针）。

### 参考

    add_action(3), enable_commands(3), disable_commands(3)

### 翻译 ###

    雪风(i@mud.ren)
