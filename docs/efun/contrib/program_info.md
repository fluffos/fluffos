---
layout: default
title: contrib / program_info.pre
---

### NAME

    program_info

### SYNOPSIS

    mapping program_info(void | object);

### DESCRIPTION

    object defaults to ALL objects

    returns mapping consinsting of:

    - header size : int
    - code size : int
    - function size : int
    - var size : int
    - class size : int
    - inherit size : int
    - saved type size : int
    - total size : int
