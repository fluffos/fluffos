---
layout: default
title: contrib / query_num.pre
---

### 名称

    query_num() - 返回指定数值的英文

### 语法

   string query_num(int num);
   string query_num(int num, int max);

### 描述

    此外部函数返回数值 `num` 的英文读法。如果 num > 99999 或指定参数 `max`且 num > max 或 num < 0 ，函数返回 `many`。

### 翻译

    雪风(i@mud.ren)
