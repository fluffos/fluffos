---
title: pcre / pcre_match
---
# pcre_match

### NAME

    pcre_match() - regular expression handler

### SYNOPSIS

    mixed pcre_match(string | string *, string, void | int | mapping flag, void | int | mapping pcre_flags);

### DESCRIPTION

    analog with regexp efun for backwards compatibility reasons
    but utilizing the PCRE library.

    可选参数 `pcre_flags` 是 `src/include/pcre_flags.h` 的整型位，或命名
    选项 mapping（见 `pcre_config()`）。默认为 0。字符串输入时第 3 个参数
    视为 `pcre_flags`；数组输入时第 3 个参数仍是旧 flag（或 mapping），
    第 4 个参数才是 `pcre_flags`。默认开启 UTF-8、UCP 和 JIT。

### SEE ALSO

    pcre_config(3), regexp(3)
