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

### SEE ALSO

    pcre_config(3), regexp(3)
