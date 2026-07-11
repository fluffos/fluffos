---
title: contrib / replace_html
---
# replace_html

### NAME

    replace_html() - escape a string for HTML

### SYNOPSIS

    string replace_html( string str );

### DESCRIPTION

    Returns `str` with the HTML-significant characters escaped:
    `&` becomes `&amp;`, `<` becomes `&lt;`, `>` becomes `&gt;`, and
    `"` becomes `&quot;`. The result is capped at the driver's maximum
    string length.

### SEE ALSO

    replace_mxp(3)
