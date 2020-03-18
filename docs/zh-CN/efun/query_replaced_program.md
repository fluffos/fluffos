---
layout: default
title: contrib / query_replaced_program.pre
---

### 名称

    query_replaced_program() - return the path to the object it was replaced with

### 语法

    string query_replaced_program(void | object ob);

### 描述

    if object called replace_program this function returns the path to the object it was replaced with, if no ob, obiect defaults to this_object()

### 参考

    replace_program(3), replaceable(3)
