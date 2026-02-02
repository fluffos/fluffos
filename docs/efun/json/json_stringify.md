---
layout: doc
title: json / json_stringify
---
# json_stringify

### NAME

    json_stringify - convert LPC values to JSON string

### SYNOPSIS

    string json_stringify(mixed value);
    string json_stringify(mixed value, int indent);

### DESCRIPTION

    Converts LPC values to JSON-formatted strings. This is a native C++
    implementation providing significantly better performance than the
    pure LPC json_encode() sefun, plus additional features like
    built-in pretty-printing.

    **NOTE:** This efun requires the JSON package to be enabled at compile time.
    Enable it with `-DPACKAGE_JSON=ON` in your CMake configuration.

    LPC types are converted to JSON types as follows:
    - LPC numbers (non-boolean) → JSON numbers
    - LPC numbers 0/1 (as booleans) → JSON false/true
    - LPC strings → JSON strings (properly escaped)
    - LPC reals → JSON numbers
    - LPC arrays → JSON arrays
    - LPC mappings → JSON objects (string keys only)
    - Other types (objects, functions) → JSON null

### ARGUMENTS

- `value` - The LPC value to stringify
- `indent` - (Optional) Indentation level for pretty-printing
  - If omitted or negative: compact JSON (no whitespace)
  - If 0 or positive: pretty-printed JSON with specified indent

### RETURN VALUE

Returns a JSON-formatted string representation of the value.

### ERRORS

Generates an error on serialization failure.

### EXAMPLES

```c
// Compact output (default)
string json = json_stringify(([ "x": 10, "y": 20 ]));
// Result: {"x":10,"y":20}

// Pretty-printed with 2-space indent
string pretty = json_stringify(([ "x": 10, "y": 20 ]), 2);
// Result:
// {
//   "x": 10,
//   "y": 20
// }

// Pretty-printed with 4-space indent
string pretty4 = json_stringify(({ 1, 2, 3 }), 4);
// Result:
// [
//     1,
//     2,
//     3
// ]

// Complex nested structures
mixed data = ([
  "user": "Alice",
  "age": 30,
  "scores": ({ 85, 90, 88 }),
  "metadata": ([ "level": 5, "active": 1 ])
]);
string json = json_stringify(data);

// Roundtrip (parse then stringify)
string original_json = read_file("data.json");
mixed parsed = json_parse(original_json);
string serialized = json_stringify(parsed);
// serialized is equivalent to original_json (possibly different formatting)
```

### FEATURES

**Pretty-Printing**
Unlike the pure LPC `json_encode()` sefun, `json_stringify()` supports
built-in pretty-printing with customizable indentation. This is useful
for debugging, logging, and creating human-readable JSON output.

**Unicode Support**
Full Unicode support for strings:

```c
json_stringify(([ "greeting": "😄" ]))  // Works fine
```

**Type Conversion**
Automatic and sensible conversion of LPC values to JSON equivalents.
Only string keys are supported in mappings (JSON specification requirement).

### PERFORMANCE

The native `json_stringify()` is approximately **4-5x faster** than the
pure LPC `json_encode()` sefun:

- `json_stringify()`: ~5,000 eval cost (199KB roundtrip)
- `json_encode()`: ~23,000 eval cost (same operation)

### NOTES

- **Mapping keys**: Only string keys are preserved in JSON. Non-string keys
  are skipped during serialization.
- **Unsupported types**: LPC objects, functions, and other unsupported types
  are converted to JSON `null`.
- **Circular references**: May cause issues; not explicitly handled.

### SEE ALSO

[json_parse](json_parse.md) - Parse JSON strings to LPC values
