---
layout: default
title: system / set_eval_limit
---

### 名称

    set_eval_limit() - set the maximum evaluation cost

### 语法

    int set_eval_limit( int );

### 描述

    set_eval_limit() with a nonzero argument, sets the maximum evaluation cost that is allowed for any one thread before a runtime error occurs. With a zero argument, it sets the current evaluation counter to zero, and the maximum cost is returned. set_eval_limit(-1) returns the remaining evaluation cost.

### 参考

    catch(3), error(3), throw(3), error_handler(4), eval_cost(3)
