---
title: pcre / pcre_config
---
# pcre_config

### NAME

    pcre_config() - PCRE2 库能力

### SYNOPSIS

    mapping pcre_config(void);

### DESCRIPTION

    返回本驱动所链接的 PCRE2 库实际提供的能力：`version`、`jit`、
    `jit_target`、`unicode`、`unicode_version`、`newline`、`bsr`、
    `link_size`、`match_limit`、`depth_limit`、`heap_limit`、
    `never_backslash_c`。

    其它 `pcre_*` efun 的最后一个参数可以是 `src/include/pcre_flags.h`
    里的整型位，也可以是命名选项 mapping。默认开启 UTF-8、Unicode 属性
    （UCP）和 JIT。键名见英文页。未知键会 `error()`。超出
    `match_limit` / `depth_limit` / `heap_limit` 是可 catch 的错误。

    不要用 FFI 或 mudlib stdlib 再包一层 PCRE2：驱动已经链接
    `libpcre2-8`，并负责编译缓存、JIT 和字符串寿命。把
    `pcre_flags.h` 拷进 mudlib include 目录，直接调这些 efun。

### SEE ALSO

    pcre_version(3), pcre_match(3), pcre_replace(3)
