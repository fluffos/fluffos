---
title: constructs / operators
---

# Operators

LPC is C-like. This page lists the extensions you will not find in K&R C.
Indexing, ranges, and lvalues are covered in depth under
[substructures](../types/substructures). Nullish coalescing is also on
[if](if).

## Arithmetic, logic, bitwise

`+` `-` `*` `/` `%` `++` `--` work on `int` and (where it makes sense)
`float`. `+` also concatenates strings, arrays, and buffers.

`==` `!=` `<` `>` `<=` `>=` · `&&` `||` `!` · `&` `|` `^` `~` `<<` `>>`

`?:` is the ternary operator.

Compound assignment: `+=` `-=` `*=` `/=` `%=` `&=` `|=` `^=` `<<=` `>>=`
plus the logical forms `||=` `&&=` `??=` (short-circuit; see [if](if)).

## Nullish coalescing (`??`)

`a ?? b` yields `a` when `a` is defined, otherwise `b` (and only then
evaluates `b`). Use `||` when every falsy value should fall through;
use `??` when only **undefined** should.

```c
int timeout = cfg["timeout"] ?? 60;
```

## Optional mapping access (`?.` / `.?`)

These forms apply to **mappings**. If the receiver is not a mapping, the
expression is `0` instead of a runtime error.

```c
mixed v = cfg?.timeout;     // same idea as cfg["timeout"] when cfg is a mapping
mixed w = cfg?.["timeout"];
mixed x = cfg.?["timeout"];
```

They do not turn a missing object into `0` the way JavaScript `?.` does on
objects. For objects, use a null check or `objectp()`.

## Indexing and ranges

```c
arr[0]          // first element (0-based)
arr[<1]         // last element
str[1..3]       // inclusive slice
str[1..]        // from 1 through the end
str[<2..<1]     // last two characters
```

Slices are lvalues: `arr[1..2] = ({ 9 });` splices. Full rules:
[substructures](../types/substructures).

## Spread (`...`)

A trailing `...` in a call or array literal expands an array into
individual arguments / elements:

```c
mixed *args = ({ 1, 2, 3 });
foo(args...);               // foo(1, 2, 3)
mixed *more = ({ 0, args... });
```

A parameter list may end with `...` on a `varargs` function; see
[modifiers](modifiers).

## Member access

`obj->fn(args)` calls a function on another object (`call_other`).
`class_value.member` and `class_value->member` read class fields
([classes](../types/classes)).

`efun::name(...)` calls the driver efun even if a simul_efun wrapped it.
`parent::name(...)` / `::name(...)` reach inherited implementations.
See [shadowing](shadowing).

## Function pointers

`(: fn, extra :)` and `(: $1 + $2 :)` build function values.
`(*fp)(...)` and `evaluate(fp, ...)` call them. See
[function type](../types/function).
