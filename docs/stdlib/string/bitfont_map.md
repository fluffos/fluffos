---
layout: default
title: stdlib / bitmap_font
---

### NAME

    bitmap_font - display english and chinese(simplified) character with bitmap font


### SYNOPSIS

    string bitmap_font(string str);
    string bitmap_font(string str, int size);
    string bitmap_font(string str, int size, string fill, string bg);
    string bitmap_font(string str, int size, string fill, string bg, string fcolor, string bgcolor);

### ARGUMENTS

    str - string to display
    size - optional: bitmap font size, only suport 12/14/16, default 12
    fill - optional: String that display foreground, default '8'
    bg - optional: String that display background, default '-'
    fcolor - optional: Foreground color
    bgcolor - optional: Background color

### DESCRIPTION

    display english and chinese(simplified) character with bitmap font.
    if size is 16, the result will be random font type.

### RETURN VALUE

    bitmap font string

### EXAMPLES

example 1:

    printf("%s\n",bitmap_font("测试abc"));

display:

    8------8---8-----8------8-------------------------------
    -8-888888--8------8-----8-88------------88--------------
    ---8---8-8-8------------8----------------8--------------
    8--8-8-8-8-8-------888888888------888----8-88-----8888--
    -8-8-8-8-8-8----88------8--------8---8---88--8---8---8--
    --88-8-8-8-8-----8--88888---------8888---8---8---8------
    -8-8-8-8-8-8-----8----8-8--------8---8---8---8---8------
    88-8-8-8-8-8-----8----8--8-------8---8---8---8---8---8--
    -8---8---8-8-----8--8-8--8-8------8888--88888-----888---
    -8--8-8----8-----8-8--88-8-8----------------------------
    -8-8---8-8-8-----88-88----88----------------------------
    -88-------8----------------8----------------------------

example 2:

    printf("%s\n",bitmap_font("测试abc", 16));

display:

    ------------88-----------88-8---------------------------
    -8--88888---88---88------88-88--------------------------
    -88-88888-8-88----88-----88--8----------888-------------
    --8-8---8-8-88----88-8888888888----------88-------------
    ----8-8-8-8-88-------8888888888----------88-------------
    -8--8-8-8-8-88--8888-----88------8888----8888----88888--
    -88-8-8-8-8-88--8888-888888---------88---88-88--88---88-
    --8-8-8-8-8-88----88-888888------88888---88--88-88------
    ----8-8-8-8-88----88--88--88----88--88---88--88-88------
    --8-8-8-8-8-88----88--88--88----88--88---88--88-88------
    --8---8---8-88----88--88--88----88--88---88--88-88---88-
    --8--88-8---88----888-8888-8-88--888-88--88888---88888--
    -88--8--88--88----88888888-8888-------------------------
    -8--88---8-888----88-88-----888-------------------------
    -8--8------88----------------8--------------------------
    --------------------------------------------------------

example 3:

    printf("%s\n",bitmap_font("你好", 16, "HI", "  "));

display:

```



      HI    HI                          HI        HI
      HI    HIHIHIHIHIHIHIHI            HI    HIHI  HIHIHI
    HIHI    HI          HI            HI                HI
  HIHIHI  HI            HI            HI            HIHI
      HI          HI              HIHIHIHIHI        HI
      HI          HI                HI    HIHIHIHIHIHIHIHIHIHI
      HI      HI  HI                HI    HI          HI    HI
      HI    HI    HI    HI            HIHI            HI
      HI  HI      HI      HIHI        HIHIHI        HI
      HI          HI              HIHI      HI      HI
      HI          HI              HI            HIHIHI


```
