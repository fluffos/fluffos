---
layout: doc
title: pcre / pcre_assoc
---
# pcre_assoc

### NAME

    pcre_assoc() - A regular pattern substring extractor

### SYNOPSIS

    mixed *pcre_assoc(string input, string *patterns, mixed *token_aray, void|mixed default, void|int pcre_flags);

### DESCRIPTION

    analog with reg_assoc efun for backwards compatibility reasons but utilizing
    the PCRE library.

    The optional `pcre_flags` set PCRE options (e.g., `PCRE_I` case-insensitive,
    `PCRE_M` multiline). Defaults to 0.

### SEE ALSO

    reg_assoc(3)
sssss
