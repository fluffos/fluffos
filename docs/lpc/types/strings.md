---
title: types / strings
---
# strings

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

You can take a substring from a variable ( str ) buy using the substring
operation ( str[n1..n2] ). Positive values are taken from the left and
values with `<` from the right. If a value is greater than the length of
the string it will be treated as being equal to the length of the string.

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
