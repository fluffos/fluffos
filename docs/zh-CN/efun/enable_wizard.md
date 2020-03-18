---
layout: default
title: mudlib / enable_wizard
---

### 名称

    enable_wizard() - 把巫师(wizard)的特权赋予当前玩家

### 语法

    void enable_wizard( void );

### 描述

    本函数仅在驱动编译时未定义 NO_WIZARD 的情况下有效。

    任何互动对象（玩家）呼叫 enable_wizard() 外部函数都会让此对象呼叫 wizardp() 外部函数的返回值为真（1）。enable_wizard() 可以让此玩家拥有以下三项特权：

    1. 当驱动编译时定义了 RESTRICTED_ED ，可以使用 ed 的受限模式（restricted  modes）。
    2. 可以收到运行时详细错误信息。
    3. 可以使用 trace() 和 traceprefix() 外部函数。

### 参考

    disable_wizard(3), wizardp(3)

### 翻译

    雪风(i@mud.ren)
