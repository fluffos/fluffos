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

    Two forms:

    **Array** (legacy): replace the capture groups of the first match. The
    number of subgroups and the size of the replacement array must match.

    **String** (PCRE2 substitute): replace every match using a PCRE2
    replacement template. `$0` / `$&` is the whole match, `$1`…`$n` are
    groups, `${name}` is a named group, `$$` is a literal `$`. Extended
    escapes (`\u` `\l` `\U` `\L` `\E`) are on. Unset or unknown groups
    become empty strings. Pass `(["replace_global": 0])` for first-match
    only, or `(["replace_literal": 1])` to treat the template as plain text.

    The optional last argument is integer bits from `src/include/pcre_flags.h`
    or a mapping of named PCRE2 options (see `pcre_config()`). Defaults to 0.
    UTF-8, Unicode properties, and JIT are on unless you pass `PCRE_NO_UCP`
    or `(["no_ucp": 1])`.

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_config(3), pcre_extract(3), pcre_replace_callback(3)
