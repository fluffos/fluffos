---
layout: default
title: system / max_eval_cost
---

### 名称

    max_eval_cost() - returns the maximum evaluation cost

### 语法

    int max_eval_cost();

### 描述

    max_eval_cost() returns the number of instructions that can be executed before the driver decides it is in an infinite loop.

### 参考

    catch(3), error(3), throw(3), error_handler(4), set_eval_limit(3), reset_eval_cost(3)
