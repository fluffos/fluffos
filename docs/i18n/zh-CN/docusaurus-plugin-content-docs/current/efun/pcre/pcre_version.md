---
title: pcre / pcre_version
---
# pcre_version

### NAME

    pcre_version() - 返回编译进驱动的 PCRE 库版本

### SYNOPSIS

    string pcre_version(void);

### DESCRIPTION

    返回 PCRE2 库版本（例如 `10.42 2022-12-11`）。库带 JIT 时字符串以
    ` JIT` 结尾。完整能力用 `pcre_config()`。

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_config(3), pcre_extract(3), pcre_replace(3)
