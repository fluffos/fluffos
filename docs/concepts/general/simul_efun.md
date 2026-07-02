---
layout: doc
title: general / simul_efun
---
# simul_efun

There is a mechanism to allow the mudlib to simulate efunctions (simulated
efuns, or simul_efuns). All simulated efuns are defined in a single object whose
path is given by the [`simulated efun file`](../../driver/config.md) config
option.

When compiling an object, if a bareword function call (not a call_other) is
found that is neither defined in the object nor a built-in efun, the compiler
resolves it as a simulated efun: it compiles a direct call into the simul_efun
object's function of that name, using a dedicated instruction rather than a
general call_other. Because the simul_efun's prototype is known at compile time,
the return value does not have to be typecast — even under `#pragma
strict_types` — since the compiler already knows its type.

Simulated efuns have many uses. One is that it becomes possible to make major
changes (and even removals) to the behavior of efuns without modifying the
driver, by defining a simul_efun with the same name as an efun. Suppose you wish
to modify the behavior of the `move_object()` efun. You could do so by defining a
simulated efun of the same name that performs various restrictive checks and then
calls `efun::move_object()`. The `efun::` prefix is necessary so that the driver
calls the real `move_object` efun rather than recursively calling the simul_efun
it is defined in. (The master object's
[`valid_override()`](../../apply/master/valid_override.md) apply controls which
objects may use the `efun::` prefix to bypass a simul_efun.) Simulated efuns are
also useful for adding functions that many different objects need to call but
which aren't appropriate for inclusion in an inherited file.

Only public functions in the simul_efun object are exported as simul_efuns. A
function declared `private` or `protected` (or one that is only a prototype) is
not made available as a simul_efun to other objects.
