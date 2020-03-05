---
layout: default
title: efuns / strings / strlen
---

### NAME

    strwidth() - returns the display width of a string

### SYNOPSIS

    int strwidth( string str );

### DESCRIPTION

    strwidth() returns the number of columns to display string 'str'.

    The driver uses rules defined in https://www.unicode.org/reports/tr11/tr11-36.html to calcuate character width.
    Control characters has no width, Wide characters, including emojis are two column wide. This rule is also used by
    sprintf() to layout strings.

### SEE ALSO

    strlen()

