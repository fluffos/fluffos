---
layout: default
title: functions / evaluate
---

### 名称

    evaluate() - 执行一个函数指针

### 语法

    mixed evaluate(mixed f, ...)

### 描述

    如果 `f` 是一个函数会使用剩余的参数调用 `f`，否则直接返回 `f`。 evaluate(f, ...) 的作用和直接调用  f(...) 相同。

### 翻译 ###

    雪风(i@mud.ren)
