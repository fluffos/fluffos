---
title: constructs / catch
---

# catch and throw

`catch` is a **compiler keyword**, not an efun, even though a man page
also lives under [efun/calls/catch](../../efun/calls/catch). It evaluates
a body and returns `0` on success or the error value on failure.

Two forms share the grammar of `time_expression`:

```c
mixed err = catch(expr);
mixed err = catch {
    statement;
    statement;
};
```

The value of a successful body is discarded. Driver errors become a
string that starts with `*`. `throw(value)` returns that value from the
nearest `catch`. `throw(0)` cannot be distinguished from "no error",
because `catch` already uses `0` for success.

```c
mixed err = catch(load_object("/obj/missing"));
if (err)
    write("load failed: " + err + "\n");
```

```c
mixed err = catch {
    foreach (string file in files)
        load_object(file);
};
```

`break` and `continue` may not leave a `catch` block (compile error).
`catch` is not cheap — use it where an error would corrupt state, not
around every line.

`await` is illegal inside `catch(...)`. In an `async` function use
`acatch` instead; see [async](async).

Related efuns: [error](../../efun/system/error),
[throw](../../efun/calls/throw). Master apply:
[error_handler](../../apply/master/error_handler).
