---
layout: default
title: system / eval_cost
---

### 名称

    eval_cost() - returns the evaluation cost remaining

### 语法

    int eval_cost();

### 描述

    eval_cost() returns the number of instructions that can be executed before the driver decides it is in an infinite loop.

### 参考

    catch(3), error(3), throw(3), error_handler(4), set_eval_limit(3), reset_eval_cost()
