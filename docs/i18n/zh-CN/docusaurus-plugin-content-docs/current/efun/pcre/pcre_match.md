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

:::warning[行为变更：默认启用 Unicode 属性]

PCRE2 升级之前，驱动只以 UTF 模式编译模式串。现在每个 `pcre_*` efun 都会附加
`PCRE2_UCP`，字符类转义按 Unicode 属性而非 ASCII 解析：

* `\d` 匹配任意 `Nd` 数字（`３`），不再只是 `0-9`
* `\w` 匹配任意字母（`café`），不再只是 `[A-Za-z0-9_]`
* `\s`、`\b` 及其取反形式的匹配范围同样扩大

如果 mudlib 用 `\d` 或 `\w` 来*校验*输入（数字 ID、对象名、玩家名），它会开始
接受以前会拒绝的字符串。可以用 `PCRE_NO_UCP` 标志位或 `no_ucp` mapping 选项
逐次调用恢复旧行为：

```c
pcre_match(s, "^\\d+$", PCRE_NO_UCP);           // 仅 ASCII 数字
pcre_match(s, "^\\d+$", ([ "no_ucp": 1 ]));     // 同上，mapping 形式
```

:::

### SEE ALSO

    pcre_config(3), regexp(3)
