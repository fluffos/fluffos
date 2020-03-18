---
layout: default
title: ed / ed_cmd
---

### 名称

    ed_cmd() - send a command to an ed session

### 语法

    string ed_cmd(string cmd)

### 描述

    This efun is available only if __OLD_ED__ is not defined.

    The  command  'cmd' is sent to the active ed session, and the resulting
    output is returned.
