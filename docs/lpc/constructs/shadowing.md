---
title: constructs / shadowing
---
# shadowing

Shadowing is what happens when you use the same name twice. A local variable
called `count` in a function that also has a global called `count`. A function
called `strlen` in an object, when `strlen` is already an efun. An inherited
object that has a `name` variable, and your object declares `name` too.

Sometimes LPC stops you. Sometimes it lets you through and quietly picks one.
This page is about which is which, and what actually gets picked.

If you only remember one thing, remember this: **LPC decides what a name means
by looking at how you used it, not just where you declared it.** `foo` and
`foo()` on the same line can refer to two completely different things.

## One name, six possible meanings

When the compiler sees an identifier, it can find as many as six different
things wearing that name:

* a **local** variable (including function parameters)
* a **global** variable (your object's own, or one it inherited)
* a **function** defined in your object (its own, or inherited)
* a **simul_efun** — a function your mudlib provides to every object
* an **efun** — a function the driver provides
* a **class** name

These don't overwrite each other. They coexist. This compiles with no warning
at all, and all three `foo`s are real and distinct:

```c
class foo { int x; }
int foo = 1;
int foo() { return 2; }

void demo() {
    class foo c = new(class foo);   // the class
    foo = foo();                    // the variable, then the function
    c->x = 3;
}
```

That's not a trick example — it's the normal state of affairs. Classes in
particular never collide with anything, because you can only ever mention a
class after the word `class` or `new`, so there's no ambiguity to resolve.

The other five *can* be ambiguous, and how LPC resolves them is the whole story.

## Three questions, three different answers

There are three ways you can write a bare name, and each one searches that list
in a different order.

**Using it as a value** — `x = foo;`

```
local  ->  global  ->  function  ->  simul_efun  ->  efun
```

**Calling it** — `foo();`

```
function  ->  simul_efun  ->  efun  ->  local  ->  global
```

**Making a function pointer** — `(: foo :)`

```
local (error!)  ->  global  ->  function  ->  simul_efun  ->  efun
```

Look at the middle one carefully. In call position, **functions and efuns are
checked before your variables**. This is the single most surprising thing about
shadowing in LPC, so here it is as running code:

```c
int strlen = 5;    // a global variable named after an efun

mixed probe() {
    write(strlen);              // 5        -- your variable
    write(strlen("abcdefg"));   // 7        -- the efun. Not your variable.
}
```

Same name, same scope, same line of thinking — two different bindings. Neither
one produces a warning.

## The trap that bites hardest

Take that one step further. Suppose you write a function that accepts a
callback, and you happen to name the parameter after an efun:

```c
int apply_to(function member_array, mixed *arr) {
    return member_array(1, arr);   // you meant your callback
}
```

Your callback is never called. `member_array` is an efun, and in call position
the efun wins, so the driver's `member_array()` runs instead. The compiler is
perfectly happy. Worse, it will warn you that the parameter is *unused* —
which is true, and is the only clue you get.

The same thing happens with a local:

```c
void demo() {
    function sizeof = (: 77 :);
    mixed a = sizeof;                // your function pointer
    int   b = sizeof(({ 1, 2, 3 })); // 3 -- the efun ran, not your closure
}
```

**Practical rule:** never name a variable of type `function` after a function
that already exists. If a name is callable anywhere in your mud — efun,
simul_efun, or a function in the same object — don't reuse it for a callback
variable.

Function pointers get their own flavour of this. `(: foo :)` is a *third*
lookup order, and it does two unhelpful things:

```c
function demo() {
    int strlen;
    return (: strlen :);   // ERROR: Illegal to use local variable in a functional.
}
```

That error is real, and it's confusing the first time, because `(: strlen :)`
would have been a perfectly good reference to the efun if the local hadn't
existed. And when the collision is with a *global* instead of a local, you get
no error at all — the function pointer silently captures the variable's value
rather than the efun:

```c
int strlen = 5;

void demo() {
    write(evaluate((: strlen :)));   // 5, not a call to strlen
}
```

## What LPC will not let you do

These are hard compile errors. You'll see them immediately, which makes them
the easy half of the topic.

**Two locals with the same name, alive at the same time.**

```c
void demo() {
    int x = 1;
    int x = 2;   // error: Illegal to redeclare local name 'x'
}
```

**A nested block does not get you out of it.** This is the big difference from
C, and it catches people constantly:

```c
void demo() {
    int x = 1;
    if (something) {
        int x = 2;   // error: Illegal to redeclare local name 'x'
    }
}
```

In C that inner `x` would be a fresh variable. In LPC it's an error. The rule
is about *lifetime*, not braces — two blocks that don't overlap are fine:

```c
void demo() {
    { int x = 1; }   // fine
    { int x = 2; }   // also fine -- the first x is already gone
}
```

Loop variables count as locals, so this fails too:

```c
void demo() {
    int i = 5;
    for (int i = 0; i < 3; i++) { }   // error: Illegal to redeclare local name 'i'
}
```

And so do parameters — a parameter is just a local that arrived from outside:

```c
void demo(int p) {
    int p = 2;   // error: Illegal to redeclare local name 'p'
}
```

**Defining the same function twice in one file.**

```c
int foo() { return 1; }
int foo() { return 2; }   // error: Redeclaration of function 'foo'.
```

**Overriding anything marked `nomask`.** That's what `nomask` is for — an
inherited object saying "this one is not yours to replace":

```c
// in /std/thing.c
nomask int identity() { return 1; }

// in your object
inherit "/std/thing";
int identity() { return 2; }   // error: Illegal to redefine 'nomask' function 'identity'.
```

It works the same for variables (`Illegal to redefine 'nomask' variable`).

**Reserved words.** You cannot name anything `switch`, `int`, `return` and so
on. These never reach the name lookup at all — the compiler sees them as
keywords and you get a plain syntax error.

## What LPC will let you do

Everything else. Here's each case and what it actually means.

### A local shadowing a global — fine, and normal

```c
int count = 10;

int demo() {
    int count = 20;
    return count;      // 20
}
```

No warning, and this is genuinely common and fine. The one thing to know is
that **the global is now unreachable inside that function.** There is no `::`
escape hatch for variables the way there is for functions. If you need both,
rename one, or read the global through a small accessor function defined
elsewhere in the object.

### A function shadowing an efun or simul_efun — fine, and deliberate

```c
int strlen(string s) { return 999; }

void demo() {
    write(strlen("abc"));         // 999 -- your function
    write(efun::strlen("abc"));   // 3   -- the driver's
}
```

This is a supported way to specialise behaviour for one object. `efun::name`
reaches past it to the driver's version (your mudlib's master object can veto
this through `valid_override`, so some muds restrict it).

The precedence chain here is worth stating plainly: **your object's own
function beats a simul_efun, and a simul_efun beats an efun.** That's how a
mudlib provides its own `strlen` mud-wide, and how a single object can then
override even that.

**But the override only applies from the point the compiler has seen it.** Each
call is bound where it is written, and at that moment the compiler only knows
about the functions declared above it. A call that appears *before* your
`strlen` still gets the efun:

```c
int before() { return strlen("abcd"); }   // 4   -- the efun

int strlen(string s) { return 999; }

int after()  { return strlen("abcd"); }   // 999 -- your function
```

Both compile without a warning, and the same file now means two different
things by `strlen()` depending on the line number. Put a **prototype** at the
top of the file and both calls bind to your version:

```c
int strlen(string s);

int before() { return strlen("abcd"); }   // 999 -- your function now

int strlen(string s) { return 999; }
```

This is why "wins for the whole file" is only true when you prototype it, and
it's the single reason to bother prototyping in a file that overrides an efun
or simul_efun. Note that it applies to overriding your *own* functions too —
any call written above a definition binds to whatever that name meant at that
point, which for a plain unprototyped helper is nothing at all and gets you an
undefined-function error instead.

### A global with the same name as a function — fine

```c
int state = 1;
int state() { return 2; }
```

Variables and functions genuinely are separate. `state` is the variable,
`state()` is the function. This is legal, and it's also exactly the sort of
thing that makes a file hard to read six months later.

### Overriding an inherited function — fine, and the point of inheriting

```c
inherit "/std/thing";
string query_long() { return "something else"; }
```

Standard practice. If you want the inherited version too, call it by name:
`thing::query_long()` names a specific parent, and `::query_long()` means
"whichever parent has it" — handy when there's only one.

If you inherit the same function from *two* different parents, the **last
inherit statement wins**. You only get told about it if the file has
`#pragma warnings`:

```
warning: query_long() inherited from both /std/armor.c and /std/weapon.c;
using the definition in /std/armor.c.
```

Without that pragma, it's silent. If you're doing multiple inheritance and the
parents overlap, turn the pragma on and read what it says.

### Declaring a global that an inherited object already has — legal, but don't

This is the one case where "allowed" is genuinely a bad idea, and it's worth
spelling out because the compiler only gives you a warning:

```c
inherit "/std/thing";   // which has:  int a = 111;
int a = 222;            // warning: Redeclaration of global variable 'a'.
```

Your object now has **two** variables called `a`. Your code sees yours; the
inherited object's code sees its own. So far so tolerable.

Then you call `save_object()`, and the save file gets two lines with the same
key — the inherited one first, yours second:

```
a 111
a 222
```

On `restore_object()`, both lines are matched by name, and name matching finds
the *first* one — so both values land in the inherited variable, and the second
line wins. The inherited `a` ends up holding *your* saved value, and your own
`a` is never restored at all: it comes back **undefined**, not zero.

That distinction matters when you go looking for the bug. An undefined int
prints as `0` and compares equal to `0`, so nothing looks wrong until you test
it properly:

```c
// after restore_object()
a                  // 0     -- looks fine
undefinedp(a)      // 1     -- it was never actually restored
```

Nothing errors. Nothing warns. The data is just quietly wrong from then on.

If you inherit something and want a variable of your own, give it a different
name. If you want the inherited one, use it — it's already there.

## Reaching the thing you shadowed

| You shadowed | How to reach the original |
|---|---|
| An efun | `efun::name(...)` |
| A simul_efun | `efun::name(...)` — reaches the efun underneath it |
| An inherited function | `parent::name(...)`, or `::name(...)` for "whichever parent has it" |
| **A variable** | **No syntax exists. Rename one of them.** |

Note the last row. Function shadowing is recoverable; variable shadowing is
not. That asymmetry is the reason variable shadowing deserves more caution than
function shadowing, even though function shadowing is the one that looks
scarier.

## A short checklist

* Don't name a `function` variable or parameter after any existing function.
  It will not be callable.
* Don't redeclare a variable your inherited object already declares. It breaks
  `save_object` / `restore_object` silently.
* Do use `nomask` on things nobody should override. It turns a subtle bug into
  a compile error, which is a trade you want.
* Do turn on `#pragma warnings` in objects that inherit from more than one
  place.
* Remember that nested blocks don't give you a fresh scope for a name. If you
  want a temporary in an `if` body, pick a name you aren't already using.
* When a name is doing two jobs and you can't tell which one a line means:
  rename it. LPC will let you keep going. Whoever reads it next won't be able to.

## See also

* [inherit](inherit) — how inheritance brings names into your object
* [function types](../types/function) — function pointers and `(: :)` syntax
* [simul_efun](../../concepts/general/simul_efun) — mudlib-wide function overrides
* [pragma](../preprocessor/pragma) — including `#pragma warnings`
