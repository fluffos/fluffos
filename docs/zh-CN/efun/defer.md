---
layout: default
title: functions / defer
---

### 名称

    defer() - 当前函数运行结束后执行指定函数

### 语法

    void defer(function f)

### 描述

    在当前函数执行结束后调用函数 `f`

### 示例

    void create()
    {
      ::create();

      defer( (: enable_commands :) );
    }

    The effect the defer() function has is it will cause the
    enable_commands() efun to be called after the execution of the
    create() function ends.  The argument passed to defer() can be
    any function type.
