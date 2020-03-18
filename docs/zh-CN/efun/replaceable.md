---
layout: default
title: contrib / replaceable.pre
---

### 名称

    replaceable() - 判断指定对象是否可以使用 replace_program()

### 语法

    int replaceable(object ob, void | string *fun);

### 描述

    string *fun defaults to ({ "create", "__INIT" }) and contains a list of functions which may be ignored for checking。

    checks if object defines any functions itself (beside create and __INIT)

    如果对象 `ob` 中定义了局部函数，返回 0，否则返回 1 。

### 参考

    replace_program(3), query_replaced_program(3)
