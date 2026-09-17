---
title: pcre / pcre_replace
---
# pcre_replace

### NAME

    pcre_replace()

### SYNOPSIS

    string pcre_replace(string input, string pattern, string *replacements, void|int|mapping pcre_flags);
    string pcre_replace(string input, string pattern, string template, void|int|mapping pcre_flags);

### DESCRIPTION

    两种形式：

    **数组**（旧）：替换第一次匹配的各个捕获组，组数必须与数组长度一致。

    **字符串**（PCRE2 substitute）：替换每一次匹配。`$0` / `$&` 是整次匹配，
    `$1`…`$n` 是捕获组，`${name}` 是命名组，`$$` 是字面 `$`。扩展转义
    （`\u` `\l` `\U` `\L` `\E`）默认打开。未设置或未知的组变成空串。
    `(["replace_global": 0])` 只替换第一次；`(["replace_literal": 1])`
    把模板当普通文本。

    可选最后一个参数是 `src/include/pcre_flags.h` 的整型位，或命名选项
    mapping（见 `pcre_config()`）。默认开启 UTF-8、Unicode 属性（UCP）
    和 JIT。`PCRE_NO_UCP` 或 `(["no_ucp": 1])` 可关掉 UCP。

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_config(3), pcre_extract(3), pcre_replace_callback(3)
