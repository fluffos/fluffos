---
layout: doc
title: general / preprocessor
---
# preprocessor

## LPC Preprocessor Manual

The preprocessor is a front end to the LPC compiler that provides such
handy features as:

    o   sharing definitions and code (#include)
    o   macros (#define, #undef)
    o   conditional compilation (#if, #ifdef, #ifndef, #else, #elif, #endif)
    o   diagnostics (#echo, #error, #warn)
    o   compiler specific (#pragma)

The first three are identical to C usage, so those already familiar with
C may want to just skim the last few sections of this document.

(The `@`/`@@` text-block syntax was historically documented here. It is a
string-literal feature rather than a preprocessor directive, and is now
covered under [text blocks](../../lpc/constructs/text_blocks.md).)

Note:

    For those directives that begin with '#' (such as #include), the
    '#' symbol must start in the first column (of the line).

Sharing Definitions and Code

This facility is provided through the #include directive.

Syntax 1:

    #include <file.h>

Syntax 2:

    #include "file.h"

Notes:

    The '#include <file.h>' form looks for the file, 'file.h' in the
    driver's configured system include directories (the
    [`include directories`](../../driver/config.md) config option,
    a colon-separated list of paths).

    The '#include "file.h"' form looks for the file, 'file.h' in the
    same directory as the file that is including it.

The #include statement is a way to textually include one file into another.
Putting a statement such as '#include "file.h"' in a file gives the same
effect as if the contents of file.h had been directly entered into the file
at the point where the #include statement occurred.  Included files are
recompiled each time the object that include's them is recompiled.  If the
included file contains variables or functions of the same name as variables
in the file doing the including, then a duplicate-name error will occur at
compile time (in the same way that the error would occur if you simply typed
in file.h rather than using #include).

### Macros

Macro definitions are used to replace subsequent instances of a given
word with a different sequence of text.  Reasons for doing so include
hiding implementation details, reducing the number of keystrokes, and
ease in changing constants.

Syntax 1:

    #define identifier token_sequence

Syntax 2:

    #define identifier(id_list) token_sequence

Notes:

    As a matter of convention, identifiers are usually capitalized to
    emphasize their presence in the code, and defined close to the
    start of program, or in a separate header file which you #include.

    The second case allows identifiers in the id_list to be substituted
    back into the token_sequence.

Example:

```c
// Create a 40 cell array of integers and initialize each cell
// to its cell number times 2,
//   i.e. stack[0] = 0, stack[1] = 2, stack[2] = 4, etc

#define STACKSIZE 40
#define INITCELL(x) 2*x

int *stack;

create() {
    int i;

    stack = allocate(STACKSIZE);

    for (i = 0; i < STACKSIZE; i++)
    stack[i] = INITCELL(i);
}
```

Lastly, it's sometimes useful to undefine (i.e. make the compiler forget
about) a macro.  The following directive is then used:

Syntax:

    #undef identifier

Note:

    It's perfectly acceptable to undefine an identifier that hasn't been
    defined yet.

### Conditional Compilation

These directives can add flexibility to your code.  Based on whether an
identifier is defined (or not defined), variations of the code can be
produced for different effects.  Applications include selective admin
logging and support for multiple drivers (or versions of the same driver).

Syntax:

    #ifdef <identifier>
    #ifndef <identifier>
    #if <expression>
    #elif <expression>
    #else
    #endif

Note:

    <identifier> refers to an identifier that has been (or could be) defined
    by your program, a file you have included, or a symbol predefined by
    the driver.

    <expression> is a constant expression that evaluates to a boolean
    condition.  The expression may contain any legal combination of the
    following:
    operators: ||, &&, >>, <<,
               +, -, *, /, %,
               &, |, ^, !, ~,
               ==, !=, <, >, <=, >=, ?:
    parentheses for grouping: (, )
    calls of the form: defined(identifier)

    Also, `#ifdef identifier` can be considered shorthand for:
    `#if defined(identifier)`

    `#ifndef identifier` can be considered shorthand for:
    `#if !defined(identifier)`

    `#elif expression` can be considered shorthand for the sequence:
    #else
    #if expression
    #endif

Example 1:

```c
// Using #if 0 allows you to comment out a block of code that
// contains comments.  One reason to do so may be to keep a copy
// of the old code around in case the new code doesn't work.
#if 0
// In this case, the constant expression evaluates
// (or is) 0, so the code here is not compiled

write(user_name + " has " + total_coins + " coins\n");
#else
// This is the alternate case (non-zero), so the code
// here _is_ compiled

printf("%s has %d coins\n", user_name, total_coins);
#endif
```

Example 2:

```c
// __VERSION__ is predefined by the driver as the FluffOS version string.
// Fall back gracefully if some other driver ever compiles this code.
#ifdef __VERSION__
string version() { return __VERSION__; }
#else
string version() { return "unknown"; }
#endif
```

### Diagnostics

The '#echo' directive prints a message to the driver's debug log while the
file is being compiled.  This facility is useful for diagnostics and
debugging.

Syntax:

    #echo This is a message

The '#warn' directive emits a compiler warning, and '#error' aborts
compilation with an error.  Each takes the rest of the line as its message:

    #warn this code path is deprecated
    #error unsupported configuration

Note:

    For all three, the rest of the line (or end-of-file, whichever comes
    first) is the message, and is printed verbatim.  It's not necessary to
    enclose the text with quotes.

### Compiler Specific

This facility performs implementation-dependent actions.

Syntax:

    #pragma keyword

At this time the following control keywords are recognized:

    o   strict_types        (off by default)
    o   save_types          (on by default)
    o   warnings            (on by default)
    o   optimize            (on by default)
    o   show_error_context  (on by default)

The defaults come from the driver's build-time configuration
(DEFAULT_PRAGMAS in local_options), so a given mud may enable or disable
others. #pragma no_keyword (e.g. #pragma no_warnings) turns a pragma off for
the current file.

Notes:

    'strict_types' informs the compiler that the return value from
    call_other()'d functions must be casted

    'save_types' causes the compiler to retain function argument type
    information in the compiled object; this is required for type-checking
    calls into the object from strict_types code, and is discarded otherwise
    to save memory

    'warnings' enables certain warnings about things in your LPC code which
    probably won't behave the way you intended

    'optimize' takes a second pass over the compiled code to improve it
    slightly

    'show_error_context' adds more text to error messages indicating where on
    the line the error occurred
