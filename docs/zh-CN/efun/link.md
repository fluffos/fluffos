---
layout: default
title: filesystem / link
---

### 名称

    link() - 把一个文件链接(link)到另一个文件

### 语法

    void link( string original, string reference );

### 描述

    创建一个从 `reference` 到 `original` 的链接。这个外部函数会触发主控对象调用 valid_link(original, reference) 方法，如果 valid_link() 返回 0，link() 会失败，如果 valid_link() 返回 1，当和 link() 使用相当参数的 rename() 外部函数执行成功时 link() 成功返回 1。

    注意：这个外部函数创建的是硬链接不是软链接（符号链接）。

### 参考

    rm(3), rmdir(3), rename(3), mkdir(3), cp(3)

### 翻译

    雪风(i@mud.ren)
