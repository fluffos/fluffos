---
layout: default
title: system / error
---

### 名称

    error - 生成一个运行时间错误信息

### 语法

    void error( string err );

### 描述

    当 error() 调用时会生成一个运行时错误消息 `err`，暂停当前线程，并把错误追踪信息记录到 debug 日志中。错误消息会附加前缀 `*`。

### 参考

    catch(3), throw(3), error_handler(4)

### 翻译 ###

    雪风(i@mud.ren)
