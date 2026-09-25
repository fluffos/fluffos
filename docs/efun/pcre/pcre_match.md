---
title: pcre / pcre_match
---
# pcre_match

### NAME

    pcre_match() - regular expression handler

### SYNOPSIS

    mixed pcre_match(string|string *lines, string pattern, void|int|mapping flag, void|int|mapping pcre_flags);

### DESCRIPTION

    analog with regexp efun for backwards compatibility reasons but utilizing
    the PCRE2 library. UTF-8, Unicode properties, and JIT are on by default.

    The optional `pcre_flags` are integer bits from `src/include/pcre_flags.h`
    or a mapping of named PCRE2 options (see `pcre_config()`). For string
    input, the 3rd argument is treated as `pcre_flags`; for array input, the
    3rd argument remains the legacy flag (or a mapping of flags) and
    `pcre_flags` is the 4th argument. Defaults to 0.

:::warning[Behavior change: Unicode properties are on by default]

Drivers before the PCRE2 upgrade compiled patterns with UTF matching only.
Every `pcre_*` efun now also sets `PCRE2_UCP`, so the character-class escapes
resolve against Unicode properties rather than ASCII:

* `\d` matches any `Nd` digit (`３`), not just `0-9`
* `\w` matches any letter (`café`), not just `[A-Za-z0-9_]`
* `\s`, `\b` and their negations widen to match

Mudlibs that use `\d` or `\w` to *validate* input (numeric IDs, object names,
player names) will accept strings they used to reject. Restore the old
behavior per call with the `PCRE_NO_UCP` flag bit or the `no_ucp` mapping
option:

```c
pcre_match(s, "^\\d+$", PCRE_NO_UCP);           // ASCII digits only
pcre_match(s, "^\\d+$", ([ "no_ucp": 1 ]));     // same, mapping form
```

:::

### SEE ALSO

    pcre_config(3), regexp(3)
