---
title: pcre / pcre_info
---
# pcre_info

### NAME

    pcre_info() - compiled-pattern metadata

### SYNOPSIS

    mapping pcre_info(string pattern, void|int|mapping pcre_flags);

### DESCRIPTION

    Compiles `pattern` (with the same defaults and options as the other
    `pcre_*` efuns) and returns what PCRE2 knows about it:

      captures          int      number of capturing groups
      names             mapping  group name → group number
      minlength         int      minimum subject length that can match
      max_lookbehind    int
      backref_max       int
      match_empty       int      1 if the pattern can match an empty string
      has_cr_or_lf      int
      has_backslash_c   int
      newline           int      newline convention used at compile
      bsr               int
      size              int      compiled pattern size in bytes
      jit               int      1 if JIT code was generated
      jit_size          int      JIT code size (0 if not JITted)

    The optional second argument is the same int bits or mapping accepted
    by `pcre_match()`. Compilation errors are catchable.

    Callouts, DFA matching, and serialized bytecode are not exposed: those
    APIs need C callbacks or raw pointers, which LPC must not hold. See
    `pcre_config()`.

:::warning[Unicode properties are on by default]

Patterns compile with `PCRE2_UCP`, so `\d` matches any Unicode digit and `\w`
any letter. Patterns used to *validate* input now accept more than they did
before the PCRE2 upgrade. Pass `PCRE_NO_UCP` (or `([ "no_ucp": 1 ])`) to get the
ASCII-only classes back -- see [pcre_match](pcre_match) for the full note.

:::

### SEE ALSO

    pcre_config(3), pcre_convert(3), pcre_match(3)
