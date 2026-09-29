---
title: pcre / pcre_assoc
---
# pcre_assoc

### NAME

    pcre_assoc() - A regular pattern substring extractor

### SYNOPSIS

    mixed *pcre_assoc(string input, string *patterns, mixed *token_aray, void|mixed default, void|int|mapping pcre_flags);

### DESCRIPTION

    analog with reg_assoc efun for backwards compatibility reasons but utilizing
    the PCRE2 library.

    The optional `pcre_flags` are integer bits from `src/include/pcre_flags.h`
    or a mapping of named PCRE2 options (see `pcre_config()`). Defaults to 0.

:::warning[Unicode properties are on by default]

Patterns compile with `PCRE2_UCP`, so `\d` matches any Unicode digit and `\w`
any letter. Patterns used to *validate* input now accept more than they did
before the PCRE2 upgrade. Pass `PCRE_NO_UCP` (or `([ "no_ucp": 1 ])`) to get the
ASCII-only classes back -- see [pcre_match](pcre_match) for the full note.

:::

### SEE ALSO

    pcre_config(3), reg_assoc(3)
