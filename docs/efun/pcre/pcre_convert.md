---
title: pcre / pcre_convert
---
# pcre_convert

### NAME

    pcre_convert() - turn a glob or POSIX pattern into a PCRE2 pattern

### SYNOPSIS

    string pcre_convert(string pattern, mapping options);

### DESCRIPTION

    Runs PCRE2's pattern converter and returns a PCRE2 pattern string.
    `options` must set exactly one of:

      glob              shell-style glob (`*`, `?`, `**`, `[...]`)
      posix_basic       POSIX basic regular expression
      posix_extended    POSIX extended regular expression

    Optional extra keys:

      glob_no_starstar
      glob_no_wild_separator
      no_utf_check

    The result is a normal PCRE2 pattern: pass it to `pcre_match()`,
    `pcre_replace()`, etc. UTF-8 conversion is on.

        string re = pcre_convert("*.c", (["glob": 1]));
        pcre_match("foo.c", re);

### SEE ALSO

    pcre_config(3), pcre_info(3), pcre_match(3)
