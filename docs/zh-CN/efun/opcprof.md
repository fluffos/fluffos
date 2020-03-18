---
layout: default
title: internals / opcprof
---

### 名称

    opcprof() - reports statistics on calling frequencies of various efuns

### 语法

    void opcprof( string | void );

### 描述

    注意：需要驱动程序编译时 #define OPCPROF

    This function dumps a list of statistics on each efunction and eoperator. If no argument is specified, then the information will be dumped to files named /OPCPROF.efun and /OPCPROF.eoper. If an argument is specified, then that name is used as the filename for the dump.

### 参考

    function_profile(3)
