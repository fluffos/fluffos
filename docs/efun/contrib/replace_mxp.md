---
title: contrib / replace_mxp
---
# replace_mxp

### NAME

    replace_mxp() - escape a string for MXP

### SYNOPSIS

    string replace_mxp( string str );

### DESCRIPTION

    Returns `str` escaped for MXP output: `&`, `<` and `>` become their
    entity forms (`&amp;`, `&lt;`, `&gt;`) and each newline is turned into
    a secure-line MXP `<BR>` tag. The result is capped at the driver's
    maximum string length.

### SEE ALSO

    replace_html(3)
