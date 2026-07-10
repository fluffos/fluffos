---
title: pcre / pcre_replace_callback
---
# pcre_replace_callback

### NAME

    pcre_replace_callback() - replace captured groups using a callback

### SYNOPSIS

    string pcre_replace_callback(string subject, string pattern,
                                 function fun,
                                 mixed extra..., void|int pcre_flags);

    string pcre_replace_callback(string subject, string pattern,
                                 string fun, object|string ob,
                                 mixed extra..., void|int pcre_flags);

### DESCRIPTION

    Returns a copy of `subject` in which each captured group of
    `pattern` has been replaced by the value returned from invoking
    `fun`.

    The callback is called once for each capture group in the
    pattern, left-to-right. Group 0 (the whole match) is never
    passed. Each invocation receives:

      string  matched   - the text captured by this group
      int     index     - the 0-based index of this capture group
                          (PCRE group 1 -> 0, group 2 -> 1, ...)
      ...extra          - any additional arguments supplied at the
                          call site, forwarded verbatim

    The callback's return value replaces the captured text in the
    output. If the callback returns a non-string value (including 0),
    the original captured text is used unchanged.

    When `fun` is a function pointer, no `ob` argument is supplied.
    When `fun` is a string, the next argument must name an object
    (either an `object` value or its filename) in which to look up
    the function.

    If `pattern` does not match `subject`, the callback is not
    invoked and `subject` is returned unchanged.

    The optional `pcre_flags` argument sets PCRE options:

      PCRE_I   case-insensitive matching
      PCRE_M   multiline (^ and $ match at line breaks)
      PCRE_S   dotall (`.` also matches newline)
      PCRE_U   ungreedy quantifiers
      PCRE_X   extended (ignore unescaped whitespace and `#` comments)
      PCRE_A   anchored matching

    Flags may be combined with `|`. Defaults to 0. When supplied,
    `pcre_flags` must be the final argument and an int. Because a
    trailing int is always consumed as `pcre_flags`, an extra
    argument intended for the callback must not be a bare int at the
    tail of the argument list.

### EXAMPLES

    // One match, multiple capture groups; callback fires per group.
    string label(string found, int group_index) {
        return sprintf("[%d:%s]", group_index, found);
    }

    string s = pcre_replace_callback("foo-bar-baz",
                                     "(\\w+)-(\\w+)-(\\w+)",
                                     "label", this_object());
    // s == "[0:foo]-[1:bar]-[2:baz]"

    // Function-pointer form (no `ob` argument).
    string s = pcre_replace_callback("foo-bar-baz",
                                     "(\\w+)-(\\w+)-(\\w+)",
                                     (: sprintf("[%d:%s]", $2, $1) :));
    // s == "[0:foo]-[1:bar]-[2:baz]"

    // Forwarding extra arguments to the callback.
    string wrap(string found, int idx, string prefix, string suffix) {
        return prefix + found + suffix;
    }

    string s = pcre_replace_callback("hello world",
                                     "(world)",
                                     "wrap", this_object(),
                                     "<", ">");
    // s == "hello <world>"

    // No match returns the subject unchanged.
    string s = pcre_replace_callback("hello", "(xyz)",
                                     "wrap", this_object());
    // s == "hello"

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_extract(3), pcre_match(3),
    pcre_match_all(3), pcre_replace(3), pcre_version(3)
