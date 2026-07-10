---
title: types / strings
---
# strings

LPC strings hold Unicode text, stored as UTF-8. There is no separate
"wide string" or "unicode string" type — every `string` value is UTF-8,
and every string operator and efun interprets it that way.

## UTF-8 Native Strings

### What counts as one character

String lengths and positions are measured in **user-perceived characters**
— Unicode *extended grapheme clusters* as defined by
[UAX #29](https://www.unicode.org/reports/tr29/) — not in bytes and not in
code points. A character that occupies several bytes (`你`, `é`) or even
several code points (an emoji such as `👍🏽` built from a base emoji plus a
skin-tone modifier) still counts as **one** character:

```c
strlen("abc")   == 3
strlen("你好")  == 2    // 6 bytes of UTF-8, 2 characters
strlen("👍🏽")    == 1    // 2 code points, 1 character
sizeof("你好")  == 2    // same rule as strlen()
```

This applies consistently across the string operations:

| Operation | Unit |
|-----------|------|
| `strlen(str)`, `sizeof(str)` | characters (grapheme clusters) |
| `str[i]` indexing | character index; yields the code point as an `int` |
| `str[i] = c` assignment | replaces the character at that index |
| `str[a..b]`, `str[<a..<b]` ranges | character indices |
| `explode()`, `implode()` | splits/joins at character boundaries |
| `strsrch()` | matches only at character boundaries, returns a character index |
| `strwidth(str)` | display columns (see below) |

Because ranges work on whole characters, slicing can never cut a
multi-byte sequence in half and produce invalid UTF-8:

```c
string s = "naïve 👍🏽 text";
s[0..4]    // "naïve"
s[6..6]    // "👍🏽"  — the whole emoji, not half of it
```

Indexing with `str[i]` returns the Unicode code point of the character as
an integer (so `"abc"[0] == 'a'` still holds). If the character at that
index is a multi-code-point grapheme cluster (like `👍🏽`), plain indexing
raises a runtime error — use a one-character range (`str[i..i]`) to
extract it as a string instead.

### Length vs. display width

`strlen()` tells you how many characters a string has; it does **not**
tell you how wide the string renders on screen. East Asian full-width
characters and emoji occupy two terminal columns
([UAX #11](https://www.unicode.org/reports/tr11/)), control characters
occupy none. Use `strwidth()` when aligning output:

```c
strlen("你好")   == 2
strwidth("你好") == 4    // two full-width characters

sprintf("%-10s|", ...)   // sprintf pads by display width, not strlen
```

### String literals

Source files are UTF-8, so any Unicode text can be written directly in a
string literal. Escape sequences are available for characters that are
awkward to type:

```c
string s1 = "€ 3,50";            // literal UTF-8 text
string s2 = "\u20ac 3,50";      // \uXXXX — 4-hex-digit code point: "€ 3,50"
string s3 = "\ud83d\ude00";     // astral characters via a surrogate pair: "😀"
string s4 = "\x41";              // \x hex escape: "A"
```

### Validity and raw bytes

Strings are expected to always contain valid UTF-8. Operations that walk
a string (ranges, `sizeof`, searches) validate it and raise a runtime
error such as `Invalid UTF-8 string` if it is malformed. Byte sequences
that are not UTF-8 text — binary file contents, packets in a legacy
encoding — belong in a `buffer`, not a string.

### Other encodings at the boundary

Everything inside the driver is UTF-8; other encodings exist only at the
edges, converted on the way in and out:

* **Interactive connections**: `set_encoding()` gives a player a
  per-connection encoding (e.g. `"GBK"`). Output is transcoded from UTF-8
  to that encoding and input is transcoded back automatically, so mudlib
  code only ever sees UTF-8. `query_encoding()` returns the current
  setting; no setting means the connection speaks UTF-8.
* **Everything else** (files, sockets, databases): convert explicitly
  with `string_encode(str, enc)` → `buffer`, `string_decode(buf, enc)` →
  `string`, or `buffer_transcode()` for buffer-to-buffer conversion.

The set of accepted encoding names comes from the ICU library the driver
is built against.

### SEE ALSO

    strlen(3), strwidth(3), sizeof(3), string_encode(3), string_decode(3),
    buffer_transcode(3), set_encoding(3), query_encoding(3)

---

## Template Literals

Template literals provide string interpolation using backtick delimiters and
`${expression}` placeholders. The expression inside the placeholder is
evaluated and coerced to a string at runtime.

### Syntax

```c
string name = "Alice";
int count = 3;

`Hello, ${name}!`              // "Hello, Alice!"
`You have ${count} items.`     // "You have 3 items."
`Sum: ${1 + 2}`                // "Sum: 3"
```

### Type Coercion

Values are coerced to strings as follows:

| Type    | Coercion                                            |
|---------|-----------------------------------------------------|
| string  | Used as-is                                          |
| int     | Decimal representation                              |
| float   | `%g` format (trailing zeros removed: 3.14 not 3.140000) |
| other   | LPC literal representation (similar to `%O`)        |

### Escape Sequences

Template literals support the same escape sequences as regular strings
(`\n`, `\t`, `\\`, `\xHH`, octal), plus:

    \`      literal backtick
    \$      literal dollar sign (prevents interpolation)

### Newline Handling

Newlines within a template literal are collapsed (removed):

```c
`line one
line two`
```

produces `"line oneline two"`.

### Adjacent Concatenation

Template literals can be placed adjacent to other template literals or
regular strings to concatenate them, just like regular string literals:

```c
`Hello, ${name}! ` `How are you?`
`Count: ${n}` " items"
"Hello, " `${name}!`
```

### Examples

```c
// Basic interpolation
string msg = `${this_body()->query_name()} says hi!`;

// Multiple expressions
string status = `${name} has ${count} items worth ${value} gold.`;

// Expressions
string info = `Total: ${price * quantity}`;

// Multiline with adjacent concat
string long_msg = `Dear ${name}, `
                  `your order of ${count} items `
                  `has been shipped.`;
```

### SEE ALSO

    sprintf(3)

---

## String Sub-Ranging

string sub ranging - comments by Grey@TMI-2

You can take a substring from a variable ( str ) by using the substring
operation ( str[n1..n2] ). Positive values are taken from the left and
values with `<` from the right. If a value is greater than the length of
the string it will be treated as being equal to the length of the string.

All positions are measured in characters (grapheme clusters, see
[UTF-8 Native Strings](#utf-8-native-strings) above), not bytes, so
sub-ranging is safe on any Unicode text.

If the two values are equal ( str[n1..n1] ) then the character at that
position ( n1 ) is returned. If both values point to positions beyond
the same end of the string the null string ( "" ) is returned. If the
position pointed to by the first value is after the one pointed to by
the second then the null string is also returned.

Examples:

    str = "abcdefg"

    str[0..0] ==       "a"
    str[0..<1] ==      "abcdefg"
    str[<4..<2] ==     "def"
    str[<7..6] ==      "abcdefg"
    str[3..2] ==       ""
    str[2..] ==        "cdefg"
