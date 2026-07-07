---
layout: doc
title: preprocessor / define
---
# #define and #undef

`#define` creates a **macro**: a name that is replaced by a body of
text wherever it appears later in the file.

## Object-like macros

```c
#define MAX_HP 100
#define GREETING "Welcome to " MUD_NAME
```

Every later occurrence of the name (as a whole identifier — not inside
strings, comments, or longer identifiers) is replaced by the body.

## Function-like macros

A parameter list attached **directly** to the name (no space before
`(`) makes the macro function-like:

```c
#define SQUARE(x) ((x) * (x))
#define MSG(who, text) tell_object(who, text "\n")
```

Arguments are collected with full nesting awareness — commas inside
parentheses, strings, or character literals do not split arguments.
Parenthesize parameters in the body (as above) to avoid precedence
surprises at the use site.

Inside a macro body, `#param` produces the argument's text as a string
literal, and `a ## b` pastes two tokens together. `##` may not appear
at the very start or end of a body.

## Expansion and rescan

A macro's body is rescanned after substitution, so macros may reference
other macros freely. A macro that (directly or indirectly) references
itself stops expanding at the self-reference instead of recursing
forever, as in C.

Because `#if` expressions are evaluated over the same token stream,
arithmetic keeps C precedence across macro boundaries:

```c
#define X 1+1
#if X*2 == 3   // 1+1*2 -- true
```

## Multi-line macros

End a line with `\` to continue the definition:

```c
#define LONG_MACRO(x) do { \
    write(x);              \
} while (0)
```

## Redefinition

Redefining a macro with a **different** body is allowed: the compiler
emits a warning (with a note pointing at the previous definition) and
the new definition takes effect. Redefining with an identical body is
silent. Redefining or `#undef`-ing a **predefined** macro (`__FILE__`,
`FLUFFOS`, ...) is a compile error.

## #undef

```c
#undef MAX_HP
```

Removes a user macro definition; the name is ordinary text again.
`#undef` of an unknown name is harmless.
