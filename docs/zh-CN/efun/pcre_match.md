---
layout: doc
title: pcre / pcre_match
---
# pcre_match

### NAME

    pcre_match() - regular expression handler

### SYNOPSIS

    mixed pcre_match(string | string *, string, void | int flag, void | int pcre_flags);

### DESCRIPTION

    analog with regexp efun for backwards compatibility reasons
    but utilizing the PCRE library.

    可选参数 `pcre_flags` 用于设置 PCRE 选项（如 `PCRE_I` 大小写不敏感，
    `PCRE_M` 多行等），默认为 0。字符串输入时第 3 个参数视为 `pcre_flags`；
    数组输入时第 3 个参数仍是旧 flag，第 4 个参数才是 `pcre_flags`。

### SEE ALSO

    regexp(3)
