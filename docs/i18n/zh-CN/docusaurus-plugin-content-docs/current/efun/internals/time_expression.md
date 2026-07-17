---
title: internals / time_expression
---
# time_expression

### 名称

    time_expression - 返回求值一个表达式或语句块所耗费的真实时间

### 语法

    int time_expression( expr );
    int time_expression { statements }

### 描述

    time_expression 是一个语言结构（编译器关键字，类似 catch），
    而不是普通的 efun。它接受两种主体写法，与 catch 共享同一语法：

    - 括号表达式形式：`time_expression(expr)` 对 <expr> 求值。
    - 语句块形式：`time_expression { statements }` 执行该语句块。

    两种形式都以 int 返回主体求值期间经过的真实（挂钟）时间，
    单位为微秒。主体表达式本身的值会被丢弃。返回值的精度不一定
    是 1 微秒；实际上，精度很可能低得多。

    在 time_expression 语句块中使用 `break` 或 `continue` 跳出
    是编译期错误。

### 示例

    ```c
    int usec = time_expression( users() );

    int usec = time_expression {
        for (int i = 0; i < 1000; i++)
            do_something();
    };
    ```

### 参考

    rusage(3), function_profile(3), time(3), catch(3)
