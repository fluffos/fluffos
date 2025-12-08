---
layout: doc
title: pcre / pcre_extract
---
# pcre_extract

### NAME

    pcre_extract() - extract matching parts

### SYNOPSIS

    string *pcre_extract(string input, string pattern, void|int include_named, void|int pcre_flags);

### DESCRIPTION

    Returns an array of captured groups specified in pattern.

    The optional third argument `include_named` defaults to 0. When non-zero,
    the return array always gets one extra element appended at the end: a
    mapping from group name to the captured string. If the pattern has no
    named groups (or none participated), the mapping is empty; otherwise only
    participating named groups are present.

    The optional fourth argument `pcre_flags` sets PCRE options (e.g., `PCRE_I`
    for case-insensitive, `PCRE_M` for multiline). Defaults to 0.

    Example (named groups):

        ({"alpha", "99", (["word": "alpha", "num": "99"])})

### SEE ALSO

    pcre_version(3), pcre_replace(3)
