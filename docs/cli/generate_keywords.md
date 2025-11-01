---
layout: doc
title: cli / generate_keywords
---
# cli / generate_keywords

`generate_keywords` is a development utility that generates a JSON file containing metadata for all LPC efuns (external functions).

## Usage

```bash
./generate_keywords
```

## Description

This tool extracts information about all available efuns from the compiled FluffOS driver and generates a `keywords.json` file containing structured metadata about each function. This file is useful for:

- **IDE Integration**: Language servers and code editors can use this file to provide autocomplete and type hints
- **Documentation Generation**: Automated documentation tools can extract function signatures
- **Static Analysis**: Code analysis tools can validate LPC code against available efuns
- **API Reference**: Developers can query available functions and their signatures

## Output Format

The tool generates a `keywords.json` file in the current directory with the following structure:

```json
[
  {
    "name": "function_name",
    "type": "efun",
    "returns": "return_type",
    "min_args": 0,
    "max_args": 2,
    "args_types": [
      ["type1", "type2"],
      ["type3"]
    ]
  },
  ...
]
```

## Output Fields

- **name**: The efun name
- **type**: Always "efun" for external functions
- **returns**: The return type of the function
- **min_args**: Minimum number of required arguments
- **max_args**: Maximum number of arguments (-1 for variadic)
- **args_types**: Array of argument type possibilities for each parameter position

## Example Output

```json
[
  {
    "name": "write",
    "type": "efun",
    "returns": "void",
    "min_args": 1,
    "max_args": 1,
    "args_types": [
      ["mixed"]
    ]
  },
  {
    "name": "hash",
    "type": "efun",
    "returns": "string",
    "min_args": 2,
    "max_args": 2,
    "args_types": [
      ["string"],
      ["string"]
    ]
  }
]
```

## Requirements

- Must be run from the FluffOS build directory
- Requires write permissions in the current directory
- The driver must be successfully compiled with all packages

## Use Cases

**VSCode/Language Server Integration:**
```javascript
// Language server can parse keywords.json to provide:
// - Autocomplete for efun names
// - Parameter hints
// - Type checking
```

**Documentation Automation:**
```bash
# Generate keywords.json then process it
./generate_keywords
node doc-generator.js keywords.json > efun-reference.md
```

## See Also

- [driver](driver.md) - Main FluffOS driver executable
- [lpcc](lpcc.md) - LPC compiler utility

## Notes

- This is a development/build tool, not needed for runtime operation
- The output reflects the exact efuns available in the compiled build
- Package-specific efuns only appear if their packages are enabled
- Run this after any build configuration changes to update the keywords
