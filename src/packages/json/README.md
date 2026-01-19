# JSON Package

Native JSON parsing and stringification for FluffOS, providing high-performance alternatives to the pure LPC implementation.

## Functions

### `mixed json_parse(string json_text)`

Parse a JSON string and convert it to LPC values (arrays and mappings).

**Parameters:**
- `json_text` - A JSON-formatted string

**Returns:**
- Parsed LPC value (string, number, real, array, or mapping)

**Examples:**
```lpc
// Simple values
string name = json_parse("\"Alice\"");      // "Alice"
int age = json_parse("30");                  // 30

// Arrays
int* nums = json_parse("[1, 2, 3]");        // ({ 1, 2, 3 })

// Objects (become mappings)
mapping user = json_parse("{\"name\": \"Bob\", \"id\": 42}");
// user["name"] == "Bob"
// user["id"] == 42

// Nested structures
mapping data = json_parse("{\"items\": [1, 2, 3], \"meta\": {\"count\": 3}}");
```

**JSON Type Conversion:**
| JSON Type | LPC Type | Notes |
|-----------|----------|-------|
| `true` | T_NUMBER | 1 |
| `false` | T_NUMBER | 0 |
| `null` | T_NUMBER | 0 |
| Number | T_NUMBER or T_REAL | Integer vs float based on JSON format |
| String | T_STRING | |
| Array | T_ARRAY | |
| Object | T_MAPPING | JSON keys (strings) become mapping indices |

**Error Handling:**
Raises an error on invalid JSON.

---

### `string json_stringify(mixed value, int|void indent)`

Convert LPC values to JSON string format.

**Parameters:**
- `value` - Any LPC value (string, number, array, mapping, etc.)
- `indent` - (Optional) Indent level for pretty-printing. If omitted or negative, produces compact JSON.

**Returns:**
- JSON-formatted string

**Examples:**
```lpc
// Compact output (default)
string json = json_stringify(([ "x": 10, "y": 20 ]));
// {"x":10,"y":20}

// Pretty-printed with 2-space indent
string pretty = json_stringify(([ "x": 10, "y": 20 ]), 2);
// {
//   "x": 10,
//   "y": 20
// }

// Pretty-printed with 4-space indent
string pretty4 = json_stringify(([ "x": 10, "y": 20 ]), 4);
// {
//     "x": 10,
//     "y": 20
// }
```

**LPC Type Conversion:**
| LPC Type | JSON Type | Notes |
|----------|-----------|-------|
| T_STRING | string | Properly escaped |
| T_NUMBER (0/1) | boolean (false/true) | Only 0 and 1 |
| T_NUMBER (other) | number | |
| T_REAL | number | |
| T_ARRAY | array | |
| T_MAPPING | object | Keys must be strings for JSON compatibility |
| Others | null | Objects, functions, etc. become null |

**Error Handling:**
Raises an error on serialization failure (e.g., circular references would be caught).

---

## Performance

The native JSON implementation is significantly faster than the pure LPC version:

- **Parse:** ~4-5x faster
- **Stringify:** ~4-5x faster

Benchmarks (on 199KB JSON file):
- `json_parse()`: ~4,500 eval cost
- `json_decode()` (sefun): ~22,000 eval cost

---

## Features

### Pretty Printing

Unlike the pure LPC implementation, `json_stringify()` supports built-in pretty-printing with customizable indentation:

```lpc
// Automatic formatting with 2-space indent
string pretty = json_stringify(data, 2);

// Or 4-space indent
string pretty = json_stringify(data, 4);
```

### Unicode Support

Full Unicode support for strings in both parsing and stringification:

```lpc
json_parse("\"Hello, 世界\"")     // Works fine
json_stringify(([ "greeting": "😄" ]))  // Works fine
```

### Roundtrip Safety

Data can be safely round-tripped through JSON:

```lpc
mixed original = ([ "items": ({1,2,3}), "meta": ([ "count": 3 ]) ]);
string json = json_stringify(original);
mixed restored = json_parse(json);
// original and restored are equivalent
```

---

## Limitations

- **Mapping keys:** Only string keys are supported when converting mappings to JSON (JSON spec requirement). Non-string keys are skipped during stringification.
- **Object types:** LPC objects, functions, and other unsupported types are converted to `null` in JSON output.
- **Circular references:** May cause issues; not explicitly handled.

---

## Configuration

The JSON package is enabled by default in `src/CMakeLists.txt`:

```cmake
option(PACKAGE_JSON "json package" ON)
```

To disable it:
```bash
cmake .. -DPACKAGE_JSON=OFF
```

---

## Implementation Details

- Uses **nlohmann::json** library (bundled with FluffOS)
- Implemented in C++ for performance
- Direct svalue manipulation for efficient LPC integration
- Automatic memory management with proper reference counting

---

## Testing

The test suite includes comprehensive tests for both parsing and stringification:

```bash
# Run JSON tests
cd build
./bin/driver ../testsuite/etc/config.test -ftest std/json
```

Tests cover:
- Basic types (strings, numbers, booleans, null)
- Arrays and objects
- Nested structures
- Unicode handling
- Roundtrip safety
- Error conditions
