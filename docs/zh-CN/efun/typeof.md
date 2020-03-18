---
layout: default
title: general / typeof
---

### 名称

    typeof() - 返回一个表达式或变量的数据类型

### 语法

    string typeof( mixed var );

### 描述

    返回表达式或变量的数据类型，具体类型在驱动文件<type.h>中定义如下：

    T_INT         "int"
    T_STRING      "string"
    T_ARRAY       "array"
    T_OBJECT      "object"
    T_MAPPING     "mapping"
    T_FUNCTION    "function"
    T_FLOAT       "float"
    T_BUFFER      "buffer"
    T_CLASS       "class"

    T_INVALID       "*invalid*"
    T_LVALUE        "*lvalue*"
    T_LVALUE_BYTE   "*lvalue_byte*"
    T_LVALUE_RANGE  "*lvalue_range*"
    T_ERROR_HANDLER "*error_handler*"
    T_FREED         "*freed*"
    T_UNKNOWN       "*unknown*"

    请注意，在LPC中所有数据类型的变量如果没有初始化，默认值都为0，typeof() 返回值都是 int 型。

### 参考

    allocate(3), allocate_mapping(3), strlen(3)

### 翻译

    雪风(i@mud.ren)
