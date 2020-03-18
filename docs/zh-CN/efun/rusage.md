---
layout: default
title: internals / rusage
---

### 名称

    rusage() - 报告系统调用 getrusage() 收集的信息

### 语法

    mapping rusage( void );

### 描述

    这个外部函数返回通过 getrusage() 系统调用收集的信息，报告中的时间单位是毫秒。

    这里有个 rusage() 的使用示例：

    void create()
    {
        mapping info;

        info = rusage();
        write("user time = " + info["utime"] + "ms\n");
        write("system time = " + info["stime"] + "ms\n");
    }

    rusage() 外部函数返回的映射键名包括: utime, stime, maxrss, ixrss, idrss, isrss, minflt, majflt, nswap, inblock, oublock, msgsnd, msgrcv, nsignals, nvcsw, nivcsw

### 参考

    time_expression(3), function_profile(3), time(3), uptime(3)

### 翻译 ###

    雪风(i@mud.ren)
