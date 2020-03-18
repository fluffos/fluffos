---
layout: default
title: calls / throw
---

### 名称

    throw() - 强制在对象中发生错误

### 语法

    void throw(mixed);

### 描述

    throw() 外部函数常用来在对象中强制报错，throw() 和 catch() 联合使用可以让程序在运行时报错时选择显示哪种错误消息。以下是典型用法：

    string err;
    int rc;

    err = catch(rc = ob->move(dest));
    if (err) {
        throw("move.c: ob->move(dest): " + err + "\n");
        return;
    }

### 参考

    catch(3), error(3), error_handler(4)

### 翻译

    雪风(i@mud.ren)
