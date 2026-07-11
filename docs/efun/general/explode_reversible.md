---
title: general / explode_reversible
---
# explode_reversible

### NAME

    explode_reversible() - split a string on a delimiter without losing empty tokens

### SYNOPSIS

    string *explode_reversible(string str, string delimiter);

### DESCRIPTION

    Splits `str` into an array of substrings on each occurrence of
    `delimiter`, exactly like explode(), but losslessly: it keeps the empty
    tokens that plain explode() discards. Empty pieces produced by a leading
    delimiter, a trailing delimiter, or two adjacent delimiters are all
    preserved as empty strings in the result.

    Because no information is dropped, the split is exactly reversible:

        implode(explode_reversible(str, delimiter), delimiter) == str

    holds for any `str` and any non-empty `delimiter`. Use this when you need
    to round-trip a string through a split/join cycle unchanged, where
    explode()'s pruning of empty fields would corrupt the data.

### EXAMPLE

    string *parts = explode_reversible("a,,b,", ",");
    // parts == ({ "a", "", "b", "" })
    // implode(parts, ",") == "a,,b,"

    explode("a,,b,", ",");
    // ({ "a", "b" })  -- empty fields dropped, not reversible

### SEE ALSO

    explode(3), implode(3)
