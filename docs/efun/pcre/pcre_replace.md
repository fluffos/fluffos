---
layout: doc
title: pcre / pcre_replace
---
# pcre_replace

### NAME

    pcre_replace()

### SYNOPSIS

    string pcre_replace(string input, string pattern, string *replacements, void|int pcre_flags);

### DESCRIPTION

    returns a string where all captured groups have been replaced by the
    elements of the replacement array. Number of subgroups and the size of the
    replacement array must match.

    The optional `pcre_flags` lets you set PCRE options (e.g., `PCRE_I` for
    case-insensitive, `PCRE_M` for multiline). Defaults to 0.

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_extract(3)
