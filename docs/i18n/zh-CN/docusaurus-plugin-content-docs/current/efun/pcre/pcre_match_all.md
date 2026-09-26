---
title: pcre / pcre_match_all
---
# pcre_match_all

### NAME

    pcre_match_all() - 查找所有匹配

### SYNOPSIS

    mixed pcre_match_all(string input, string pattern, void|int|mapping pcre_flags);

### DESCRIPTION

    类似 PHP 的 preg_match_all，返回一个字符串数组的数组，包含所有匹配及其捕获分组。

    可选参数 `pcre_flags` 用于设置 PCRE 选项（如 `PCRE_I` 大小写不敏感，`PCRE_M` 多行等），默认 0。

### Example

    // https://tools.ietf.org/html/rfc3986#appendix-B
    pcre_match_all("http://www.ics.uci.edu/pub/ietf/uri/#Related",
                       "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?" , PCRE_M));

    返回的数组第 1 项是完整匹配，后续依次为各捕获分组。

:::warning[默认启用 Unicode 属性]

模式以 `PCRE2_UCP` 编译，因此 `\d` 匹配任意 Unicode 数字、`\w` 匹配任意字母。
用于*校验*输入的模式会比 PCRE2 升级之前接受更多字符串。传入 `PCRE_NO_UCP`
（或 `([ "no_ucp": 1 ])`）可恢复仅 ASCII 的字符类 —— 详见
[pcre_match](pcre_match)。

:::
