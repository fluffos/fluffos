---
layout: doc
title: constructs / include
---
# include

```c
#include "defs.h"
#include <mudlib.h>
```

`#include` textually splices another file into the one being compiled:
the effect is exactly as if the contents of the header had been typed
at the point of the directive. Each object that includes a file gets
its own compiled copy of whatever the file contains — so headers are
for `#define`s, prototypes and shared declarations, while shared
*code* belongs in an [inherit](inherit)ed object.

The `"file"` form searches relative to the including file's directory
first and then the include path; the `<file>` form searches only the
include path. For the full resolution rules, macro file names,
trailing comments, and nesting limits, see the preprocessor reference:
[#include](../preprocessor/include).
