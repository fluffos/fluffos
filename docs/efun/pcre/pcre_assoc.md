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

### SEE ALSO

    pcre_config(3), reg_assoc(3)
