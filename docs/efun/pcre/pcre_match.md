---
layout: doc
title: pcre / pcre_match
---
# pcre_match

### NAME

    pcre_match() - regular expression handler

### SYNOPSIS

    mixed pcre_match(string|string *lines, string pattern, void|int flag, void|int pcre_flags);

### DESCRIPTION

    analog with regexp efun for backwards compatibility reasons but utilizing
    the PCRE library.

    The optional `pcre_flags` set PCRE options (e.g., `PCRE_I` case-insensitive,
    `PCRE_M` multiline). For string input, the 3rd argument is treated as
    `pcre_flags`; for array input, the 3rd argument remains the legacy flag and
    `pcre_flags` is the 4th argument. Defaults to 0.

### SEE ALSO

    regexp(3)
