---
layout: default
title: functions / bind
---

### 名称

    bind() - 改变一个函数指针的所属对象

### 语法

    function bind(function f, object ob)

### 描述

    返回一个和函数指针 `f` 完全相同的函数指针，但是所属对象从创建 `f` 的对象变为 `ob`。这在函数 `f` 的创建者被摧毁或 `f` 是需要由其它对象调用的外部函数时非常有用。

    示例:
```c
    void make_living(object ob)
    {
        function f;

        f = bind((: enable_commands :), ob);

        evaluate(f);
    }
```
    以上示例的效果和由对象 `ob` 自己执行 enable_commands() 外部函数完全一样。因为 bind() 允许你强制其它对象执行一些代码，这会造成一些安全风险，为了防止被滥用，需要 MASTER 对象中的 valid_bind() apply 方法返回1，否则 bind() 无法执行。

### 翻译 ###

    雪风(i@mud.ren)
