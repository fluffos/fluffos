---
layout: default
title: interactive / disable_commands
---

### 名称

    disable_commands() - 让一个活的（living）对象变成非活的（non-living）

### 语法

    int disable_commands( void );

### 描述

    让一个活着的对象变成非活着的状态，也就是说：add_action() 无效，living() 返回值为0，而且，如果对象是玩家，将变的无法使用任何指令（input_to()依然可用）。

    调用 disable_commands() 函数还有一个附加效果是清理掉此对象的所有行为指令，包括此对象本身定义的和来自其它对象的。

### 返回值

    disable_commands()函数永远返回0

### 参考

    enable_commands(3)

### 翻译

    雪风(i@mud.ren)
