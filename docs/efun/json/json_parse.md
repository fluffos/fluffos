---
layout: doc
title: json / json_parse
---
# json_parse

### NAME

    json_parse - parse JSON string to LPC values

### SYNOPSIS

    mixed json_parse(string json_text);

### DESCRIPTION

    Parses a JSON-formatted string and converts it to equivalent LPC values.
    This is a native C++ implementation providing significantly better
    performance than the pure LPC json_decode() sefun.

    **NOTE:** This efun requires the JSON package to be enabled at compile time.
    Enable it with `-DPACKAGE_JSON=ON` in your CMake configuration or local_options.

    JSON types are converted to LPC types as follows:
    - JSON numbers → LPC int or real (based on format)
    - JSON strings → LPC strings
    - JSON arrays → LPC arrays
    - JSON objects → LPC mappings (string keys only)
    - JSON true/false → LPC number (1/0)
    - JSON null → LPC number (0)

### ARGUMENTS

- `json_text` - A JSON-formatted string to parse

### RETURN VALUE

Returns the parsed LPC value. The type depends on the top-level JSON type:

- Scalar JSON values return scalar LPC types
- JSON arrays return LPC arrays
- JSON objects return LPC mappings

### ERRORS

Generates an error if the JSON is malformed or invalid.

### EXAMPLES

```c
// Parse simple values
int num = json_parse("42");                    // 42
string str = json_parse("\"hello\"");          // "hello"
int flag = json_parse("true");                 // 1

// Parse arrays
int* nums = json_parse("[1, 2, 3]");           // ({ 1, 2, 3 })
mixed* mixed_arr = json_parse("[1, \"two\", 3.0]");

// Parse objects as mappings
mapping user = json_parse("{\"name\": \"Alice\", \"age\": 30}");
// user["name"] == "Alice"
// user["age"] == 30

// Parse nested structures
mapping data = json_parse(
  "{\"items\": [1, 2, 3], \"meta\": {\"count\": 3}}"
);
// data["items"][0] == 1
// data["meta"]["count"] == 3

// Unicode support
string greeting = json_parse("\"Hello, 世界\"");  // Works fine
```

### PERFORMANCE

The native `json_parse()` is approximately **4-5x faster** than the pure LPC
`json_decode()` sefun:

- `json_parse()`: ~4,500 eval cost (199KB file)
- `json_decode()`: ~22,000 eval cost (same file)

### SEE ALSO

[json_stringify](json_stringify.md) - Convert LPC values to JSON
