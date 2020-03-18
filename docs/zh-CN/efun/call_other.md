---
layout: default
title: calls / call_other
---

### 名称

    call_other() - 呼叫在其它对象中的函数（方法）

### 语法

    unknown call_other( object ob, string func, ... );
    unknown call_other( object ob, mixed *args);
    unknown call_other( object *obs, string func, ... );
    unknown call_other( object *obs, mixed *args );

### 描述

    呼叫其它对象中的方法，因为返回值为调用方法的返回值，所以在编译阶段无法判断返回值的类型，最好在使用时做类型检查。字符串 `func` 是对象 `ob` 中被调用的方法名称，而第3、4……个等参数会做为第1、2……个参数按顺序传递给 `func`。

    如果 call_other 函数第一个对象参数是数组，那么 call_other 会逐一呼叫它们的 `func`，返回值也是一个数组。

    如果 call_other 函数的第二个参数是数组，那么数组的第一个元素必须是要调用的方法名字符串 `func` ,剩余的元素则是要传入方法的参数。

    这里有一个使用 call_other() 更好的方式:

    object ob | object *obs -> func ( ... );

    如：
    call_other(ob, "query", "name");

    可以写成：
    ob->query("name");

    如果第二个参数使用数组，以上示例可以写成：
    call_other(ob, ({ "query", "name" }));

    而第一个参数是数组的示例如下：
    users()->quit();

### 翻译 ###

    雪风(i@mud.ren)
