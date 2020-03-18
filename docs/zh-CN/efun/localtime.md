---
layout: default
title: system / localtime
---

### 名称

    localtime() - 转换时间戳为本地时间

### 语法

    mixed *localtime( int time );

### 描述

    localtime() 转换时间戳（使用 time() 外部函数返回）为一个代表本地时间的数组。数组的格式如下：

    ({秒, 分, 时, 日, 月, 年, 星期, 天, 时差, 时区, 夏令时})

    数组下标索引定义在驱动程序头文件 <localtime.h> 中，具体如下：
    #ifndef _LOCALTIME_H
    #define _LOCALTIME_H

    #define LT_SEC 0        // Seconds after the minute (0..59)
    #define LT_MIN 1        // Minutes after the hour (0..59)
    #define LT_HOUR 2       // Hour since midnight (0..23)
    #define LT_MDAY 3       // Day of the month (1..31)
    #define LT_MON 4        // Months since January (0..11)
    #define LT_YEAR 5       // Year (guarenteed to be >= 1900)
    #define LT_WDAY 6       // Days since Sunday (0..6)
    #define LT_YDAY 7       // Days since January 1 (0..365)
    #define LT_GMTOFF 8     // Seconds after GMT (UTC)
    #define LT_ZONE 9       // Timezone name
    #define LT_ISDST 10     // Is Daylight Saving Time?(0 || 1)

    #endif

    各字段类型和说明如下：
    int LT_SEC      秒数 (0..59)
    int LT_MIN      分钟 (0..59)
    int LT_HOUR     小时 (0..23)
    int LT_MDAY     日期 (1..31)
    int LT_MON      几月 (0..11)
    int LT_YEAR     年份 (必須大於 1900)
    int LT_WDAY     星期几 (0..6)
    int LT_YDAY     一年中的第几天 (0..365)
    int LT_GMTOFF   格林威治时间相差的秒数 (UTC)
    string LT_ZONE  本地时区名
    int LT_ISDST    是否夏令时

### 参考

    ctime(3), time(3), time_expression(3), uptime(3)

### 翻译

    雪风(i@mud.ren)
