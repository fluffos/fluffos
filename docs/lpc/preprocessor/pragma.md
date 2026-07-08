---
layout: doc
title: preprocessor / pragma
---
# #pragma

`#pragma` toggles per-file compiler flags. Each flag has a positive
form and a `no_`-prefixed negative form (`#pragma no_warnings`), and
applies from the directive to the end of the file being compiled. An
unrecognized pragma is a warning, not an error.

| Pragma | Effect |
|---|---|
| `strict_types` | Enforce strict type checking: functions must declare return types, and calls/assignments are type-checked. Strongly recommended for all new code. |
| `warnings` | Enable compiler warnings (`no_warnings` silences them for the rest of the file). |
| `save_types` | Keep the type information of the program after compilation, so objects inheriting it can type-check calls into it. |
| `optimize` | Enable extra tree optimization for this file. |
| `show_error_context` | Legacy flag (on by default). It predates the clang-style diagnostics, which already show source context, so it no longer changes how compiler diagnostics are rendered (see [diagnostics](../diagnostics)). |

```c
#pragma strict_types
#pragma no_warnings
```
