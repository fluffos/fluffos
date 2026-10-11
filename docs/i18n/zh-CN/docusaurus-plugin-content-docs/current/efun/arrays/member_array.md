---
title: arrays / member_array
---
# member_array

### 名称

    member_array()  -  返回指定元素在数组或字符串中的索引位置

### 语法

    int member_array( mixed item, mixed * | string arr, void | int start );

### 描述

    返回 `item` 在 数组 `arr` 中首次出现的位置，如果指定起始位置 `start`，则从指定位置之后开始查询，如果没有找到返回 -1。

    如果第二个参数是字符串，第一个参数必须是要查找的字符（int）。字符串形式按字节查找：
    `start` 和返回值都是字节偏移，只能匹配单个字节（0..255）。只有在 ASCII 文本范围内（CR LF
    算一个字符），字节偏移才等于字符索引；大于 0x7F 的值只能匹配 UTF-8 序列的一部分。因此当查找值
    大于 0x7F 且字符串不是 ASCII，或者 `start` 或匹配位置在非 ASCII 文本之后时，驱动会记录一条警告
    （每个调用位置一次）。查找 ASCII 字符且未找到、或在任何非 ASCII 文本之前找到时，结果本身是正确的。
    要按字符索引查找任意 Unicode 码点，请改用 strsrch(str, ch)。

### 翻译

    雪风(i@mud.ren)
