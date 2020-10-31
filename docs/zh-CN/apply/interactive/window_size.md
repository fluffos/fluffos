---
layout: default
title: interactive / window_size
---

### 名称

    window_size - 报告用户窗口大小

### 语法

    void window_size(int width, int height);

### 描述

    window_size() is called with the user's window size, as reported by telnet
    negotiation.  If the user's client never responds to the query, this is
    never called.
