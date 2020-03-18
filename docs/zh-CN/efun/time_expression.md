---
layout: default
title: internals / time_expression
---

### 名称

    time_expression() - return the amount of real time that an expression took

### 语法

    int time_expression( mixed expr );

### 描述

    Evaluate <expr>. The amount of real time that passed during the evaluation of <expr>, in microseconds, is returned. The precision of the value is not necessarily 1 microsecond; in fact, it probably is much less precise.

### 参考

    rusage(3), function_profile(3), time(3)
