---
layout: doc
title: pcre / pcre_replace
---
# pcre_replace

### NAME

    pcre_replace()

### SYNOPSIS

    string pcre_replace(string, string, string *, void|int pcre_flags);

### DESCRIPTION

    returns a string where all captured groups have been replaced by the elements
    of the replacement array. Number of subgroups and the size of the replacement
    array must match.

    可选参数 `pcre_flags` 用于设置 PCRE 选项（如 `PCRE_I` 大小写不敏感，
    `PCRE_M` 多行等），默认 0。

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_extract(3)
