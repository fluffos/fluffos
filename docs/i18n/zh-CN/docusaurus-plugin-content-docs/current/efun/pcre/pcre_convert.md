---
title: pcre / pcre_convert
---
# pcre_convert

### NAME

    pcre_convert() - 把 glob 或 POSIX 模式转成 PCRE2 模式

### SYNOPSIS

    string pcre_convert(string pattern, mapping options);

### DESCRIPTION

    `options` 必须设置 `glob`、`posix_basic` 或 `posix_extended` 之一。
    返回值是普通 PCRE2 模式，可交给 `pcre_match()` / `pcre_replace()`。

### SEE ALSO

    pcre_config(3), pcre_info(3), pcre_match(3)
