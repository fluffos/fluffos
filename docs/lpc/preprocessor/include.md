---
title: preprocessor / include
---
# #include

```c
#include "defs.h"
#include <mudlib.h>
```

`#include` splices the named file into the compile at the point of the
directive, exactly as if its contents had been typed there. Included
files are re-read on every recompile of the including object.

## Search order

* **`#include "file"`** — first resolved **relative to the directory
  of the including file**, then (if not found there) through the
  include path.
* **`#include <file>`** — resolved through the include path only.

The include path is the config file's `include directories` list; the
master apply `get_include_path(file)` can override it per compiled
file. Names may contain subdirectories (`<sys/net.h>`), but `..` is
not allowed. Every candidate path is subject to the master's
`valid_read` check.

## Directive line details

Text after the closing `"` or `>` is ignored, so trailing comments are
fine:

```c
#include <mudlib.h> /* mud info defines */
```

The file name may also be produced by a macro:

```c
#define CONFIG "local.h"
#include CONFIG
```

Includes nest (an included file may `#include` further files) up to a
driver-enforced depth limit; diagnostics inside an included file print
the full `In file included from ...` chain (see
[diagnostics](../diagnostics)).

## #include vs inherit

`#include` copies text into each including object — every object gets
its own compiled copy. [`inherit`](../constructs/inherit) shares one
compiled program among all inheritors. Use header files for
definitions (`#define`s, prototypes) and `inherit` for code.
