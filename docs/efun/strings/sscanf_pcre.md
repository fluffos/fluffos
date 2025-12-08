---
layout: doc
title: strings / sscanf (PCRE mode)
---
# sscanf (PCRE mode)

### DESCRIPTION

When the runtime config line `sscanf use pcre : 1` is set (or
`set_config(__RC_SSCANF_USE_PCRE__, 1)` is called) and the driver is built with
`PACKAGE_PCRE`, the `%(...)` specifier in `sscanf` is parsed with PCRE instead
of the legacy V8-style regexp engine. All other specifiers (`%s %d %f %x`)
behave the same. Match counts still include skipped specifiers (`%*`).

Key compatibility notes:

- PCRE recognizes escapes like `\d \w \s \b (?i) (?: ) (?= )` and
  backreferences (`\1` etc.). Patterns that relied on unknown `\x` escapes
  silently becoming literals may change meaning or now error.
- Matching remains leftmost-first, but PCRE syntax applies (e.g., `(?i)`
  works, `(a)\1` is a backref).
- The match must still begin at the current input position for a bare `%(...)`
  (anchored).

### EXAMPLES

Enable via runtime config:

```text
sscanf use pcre : 1
```

Skip + capture:

```c
string a, b;
sscanf(":test hi", "%*([:/])%([\\w]+) %s", a, b); // returns 3; a=="test", b=="hi"
```

Case-insensitive with inline modifier:

```c
string a, b;
sscanf("Hello hELLo", "%((?i)hello) %((?i)hello)", a, b); // returns 2
```

Backreference:

```c
string m;
sscanf("aba", "%((.)b\\1)", m); // returns 1; m=="aba"
```

Numeric via PCRE capture:

```c
string num;
sscanf("abc123", "%*([a-z]+)%([0-9]+)", num); // returns 2; num=="123"
```

### SEE ALSO

[sscanf](sscanf.html), [pcre_match](../pcre/pcre_match.html), [regexp](regexp.html)
