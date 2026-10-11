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
    `start` 和返回值都是字节偏移，只能匹配单个字节（0..255）。当字节结果不等于字符结果时——
    查找的 ASCII 字符出现在非 ASCII 文本或 CR LF（算一个字符）之后，或者是一个更长字符的开头
    （组合字符序列、CR LF 中的 LF），或者在含有非 ASCII 字节的文本中查找大于 0x7F 的值——
    驱动会记录一条警告（每个调用位置一次）。查找 ASCII 字符且未找到、或在任何非 ASCII 文本之前
    找到时，结果本身是正确的。要按字符索引查找任意 Unicode 码点，请改用 strsrch(str, ch)。

### 翻译

    雪风(i@mud.ren)
