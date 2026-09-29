---
title: pcre / pcre_version
---
# pcre_version

### NAME

    pcre_version() - returns the version of the compiled PCRE library used

### SYNOPSIS

    string pcre_version(void);

### DESCRIPTION

    Returns the PCRE2 library version (for example `10.42 2022-12-11`).
    When the library was built with JIT, the string ends with ` JIT`.
    For the full capability mapping (Unicode, default limits, JIT target)
    use `pcre_config()`.

### SEE ALSO

    pcre_assoc(3), pcre_cache(3), pcre_config(3), pcre_extract(3), pcre_replace(3)
