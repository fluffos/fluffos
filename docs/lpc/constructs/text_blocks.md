---
title: constructs / text blocks
---
# text blocks

Text blocks (the `@` and `@@` operators) are a string-literal shorthand that
makes it easier to write large blocks of formatted text — help messages, room
descriptions, and the like — without escaping newlines or quoting every line.

They are lexical syntax, not preprocessor directives: `@` produces a `string`
and `@@` produces a `string *` (an array of strings). The result is an ordinary
value that can be used anywhere such a value is expected.

Syntax 1:

```text
@marker
<... text block ...>
marker
```

Syntax 2:

```text
@@marker
<... text block ...>
marker
```

Notes:

- `@` — produces a string suitable for `write()`
- `@@` — produces an array of strings, suitable for the body pager

These are used by prepending '@' (or '@@') before an end marker word. This is
followed by your formatted text, as you would have it appear to the user. The
text block is terminated by the end marker word on a line by itself, without the
'@' (or '@@').

With '@', the block becomes a single string in which every line — including the
last line before the terminator — is terminated by a '\n', so the resulting
string ends with a trailing newline. With '@@', the block becomes an array of
strings, one element per line, without any embedded newlines.

Example 1:

```c
int help() {
    write( @ENDHELP
This is the help text.
It's hopelessly inadequate.
ENDHELP
    );
    return 1;
}
```

Is equivalent to:

```c
int help() {
    write( "This is the help text.\nIt's hopelessly inadequate.\n" );
    return 1;
}
```

Example 2:

```c
int help() {
    this_player()->more( @@ENDHELP
This is the help text.
It's hopelessly inadequate.
ENDHELP
    , 1);
    return 1;
}
```

Is equivalent to:

```c
int help() {
    this_player()->more( ({ "This is the help text.",
    "It's hopelessly inadequate." }), 1);
    return 1;
}
```
