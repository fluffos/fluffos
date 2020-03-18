---
layout: default
title: master / flag
---

### 名称

    flag - 处理驱动程序启动时指定的 mudlib 特定标志

### 语法

    void flag( string flag );

### 描述

    驱动程序使用 -f flag 启动时标志 `flag` 会作为参数调用主控对象中的 flag(flag) 方法。例如使用以下方式启动驱动程序：./driver -fdebug config.ini 会调用主控对象中的 flag("debug") 方法。

### 翻译

    雪风(i@mud.ren)
