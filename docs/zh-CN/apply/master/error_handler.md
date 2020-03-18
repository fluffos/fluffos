---
layout: default
title: master / error_handler
---

### 名称

    error_handler - 主控对象中处理错误的函数

### 语法

    void error_handler( mapping error, int caught );

### 描述

    此方法在驱动程序编译时定义了 MUDLIB_ERROR_HANDLER 才有效。

    这个方法允许 mudib 代替驱动程序处理错误，映射 `error` 中的内容如下：

    ([
        "error"   : string,     // 错误
        "program" : string,     // 出错程序
        "object"  : object,     // 当前对象
        "line"    : int,        // 错误所在行
        "trace"   : mapping*    // 错误追溯
    ])

    每行错误追溯是一个包括以下内容的映射：

    ([
        "function"  : string,   // 函数名
        "program"   : string,   // 程序
        "object"    : object,   // 对象
        "file"      : string,   // 行号指向的文件
        "line"      : int       // 行号
    ])

    如果错误被 catch() 外部函数捕获，标识符参数 `caught` 值为 1 。

### 参考

    catch(3), error(3), throw(3), log_error(4)

### 作者

    Beek

### 翻译

    雪风(i@mud.ren)
