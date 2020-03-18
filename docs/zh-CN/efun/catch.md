---
layout: default
title: calls / catch
---

### 名称

    catch() - 捕获运行时错误

### 语法

    mixed catch( mixed expr );

### 描述

    执行表达式 `expr`，如果没有错误会返回0，如果有标准错误，会返回一个以 `*` 开头的包括错误信息的字符串。

    外部函数 throw() 用来马上抛出一个错误并返回非零值，可以和 catch 配合使用。另外 catch() 本质上并不是外部函数而是一个编译器指令。

    catch() 比较消耗资源，请不要随意使用，建议只用在出错时可能会造成严重问题的地方。

### 参考

    error(3), throw(3), error_handler(4)

### 翻译 ###

    雪风(i@mud.ren)
