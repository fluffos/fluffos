---
layout: doc
title: json
---

Native JSON parsing and stringification with performance optimizations.

## Efuns

- [json_parse](json_parse.md) - Parse JSON string to LPC values
- [json_stringify](json_stringify.md) - Convert LPC values to JSON string

## Overview

The JSON package provides native C++ implementations of JSON parsing and
stringification, offering significant performance improvements over pure LPC
implementations.

**IMPORTANT:** This package is **opt-in** and not enabled by default. To use these
efuns, compile FluffOS with `-DPACKAGE_JSON=ON`.

### Performance

Both functions are approximately **4-5x faster** than their pure LPC counterparts:

| Operation | Native | Sefun | Speedup |
|-----------|--------|-------|---------|
| Parse (199KB) | ~4,500 eval | ~22,000 eval | 5x |
| Stringify (199KB) | ~5,000 eval | ~23,000 eval | 4.6x |

### Features

- **Full JSON support**: Parse and generate RFC 8259 compliant JSON
- **Pretty-printing**: `json_stringify()` supports optional indentation
- **Unicode**: Full UTF-8 support for strings
- **Type conversion**: Seamless conversion between JSON and LPC types
- **Roundtrip safe**: Data can be safely round-tripped through JSON

### Quick Start

```c
// Parse JSON
mapping user = json_parse("{\"name\": \"Alice\", \"age\": 30}");

// Stringify with pretty printing
string json = json_stringify(user, 2);
```

### Type Mapping

**JSON → LPC:**

- `null` → 0 (number)
- `true` / `false` → 1 / 0 (number)
- `123` → 123 (number)
- `3.14` → 3.14 (real)
- `"text"` → "text" (string)
- `[...]` → ({ ... }) (array)
- `{...}` → ([ ... ]) (mapping)

**LPC → JSON:**

- Numbers → numbers
- Strings → strings
- Arrays → arrays
- Mappings → objects (string keys only)
- Booleans → true/false (1/0)
- null/nil → null
- Other types → null
