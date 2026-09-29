---
title: pcre / pcre_extract
---
# pcre_extract

### NAME

    pcre_extract() - extract matching parts

### SYNOPSIS

    string *pcre_extract(string input, string pattern, void|int|mapping include_named, void|int|mapping pcre_flags);

### DESCRIPTION

    Returns an array of captured groups specified in pattern.

    The optional third argument `include_named` defaults to 0. When non-zero,
    the return array always gets one extra element appended at the end: a
    mapping from group name to the captured string. If the pattern has no
    named groups (or none participated), the mapping is empty; otherwise only
    participating named groups are present.

    The optional fourth argument `pcre_flags` is integer bits from
    `src/include/pcre_flags.h` or a mapping of named PCRE2 options (see
    `pcre_config()`). A mapping as the third argument is treated as flags
    with `include_named` left off. Defaults to 0.

    Example (named groups):

        ({"alpha", "99", (["word": "alpha", "num": "99"])})

:::warning[Unicode properties are on by default]

Patterns compile with `PCRE2_UCP`, so `\d` matches any Unicode digit and `\w`
any letter. Patterns used to *validate* input now accept more than they did
before the PCRE2 upgrade. Pass `PCRE_NO_UCP` (or `([ "no_ucp": 1 ])`) to get the
ASCII-only classes back -- see [pcre_match](pcre_match) for the full note.

:::

### SEE ALSO

    pcre_version(3), pcre_replace(3)
