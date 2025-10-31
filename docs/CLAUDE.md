# CLAUDE.md - Documentation Guide for FluffOS

This file provides guidance to Claude Code (claude.ai/code) and other AI assistants when working with FluffOS documentation.

## Documentation Structure

The FluffOS documentation is organized into several categories:

### 1. Apply Documentation (`/docs/apply/`)

Applies are callback functions that the driver calls on LPC objects in response to various events.

**Directory Structure:**
```
/docs/apply/
  ├── index.md                    # Apply overview
  ├── interactive/                # Player/connection-related applies
  │   ├── index.md               # List of interactive applies
  │   ├── logon.md               # Called when player connects
  │   ├── net_dead.md            # Called when connection drops
  │   ├── process_input.md       # Input preprocessing
  │   ├── receive_message.md     # Message handling
  │   ├── gmcp.md, mxp_tag.md    # Protocol-specific applies
  │   └── ...
  ├── master/                     # Master object applies
  │   ├── index.md               # List of master applies
  │   ├── connect.md             # New connection handling
  │   ├── crash.md               # Crash/error handling
  │   ├── compile_object.md      # Compilation control
  │   ├── valid_*.md             # Security validation applies
  │   └── ...
  └── object/                     # Standard object applies
      ├── index.md               # List of object applies
      ├── create.md              # Object creation
      ├── init.md                # Object initialization
      ├── reset.md               # Object reset
      └── ...
```

**Apply Documentation Template:**
```markdown
---
layout: doc
title: category / apply_name
---
# apply_name

### NAME
    apply_name - brief description

### SYNOPSIS
    return_type apply_name(arg_type arg, ...);

### DESCRIPTION
    Detailed description of when this apply is called and what it does.

    **Arguments:**
    - `arg`: Description of each argument

    **Return Value:**
    - Description of expected return value and its effect

### EXAMPLE
    ```c
    return_type apply_name(arg_type arg) {
        // Example implementation
    }
    ```

### SEE ALSO
    related_apply(4), related_efun(3)

### NOTE
    Additional notes, caveats, or requirements
```

**Finding Applies in Source:**
- Primary source: `/src/vm/internal/applies` - Lists all defined applies
- Implementation: Search for `APPLY_NAME` or `apply(APPLY_NAME, ...)` in source
- Special applies: Some applies (like `heart_beat`, `is_living`) are handled specially by the compiler

### 2. Efun Documentation (`/docs/efun/`)

Efuns (external functions) are built-in C/C++ functions callable from LPC code.

**Directory Structure:**
```
/docs/efun/
  ├── index.md                    # Master list of all efuns by category
  ├── arrays/                     # Array manipulation
  ├── async/                      # Asynchronous operations
  ├── buffers/                    # Buffer operations
  ├── calls/                      # Function calling & control flow
  ├── contrib/                    # Contributed functions
  ├── crypto/                     # Cryptographic functions
  ├── db/                         # Database operations
  ├── ed/                         # Editor functions
  ├── external/                   # External program execution
  ├── filesystem/                 # File operations
  ├── floats/                     # Floating-point math
  ├── functions/                  # Function pointers
  ├── general/                    # General utilities
  ├── interactive/                # Player interaction
  ├── internals/                  # Driver internals/debugging
  ├── mappings/                   # Mapping operations
  ├── mudlib/                     # Mudlib-specific functions
  ├── numbers/                    # Integer operations
  ├── objects/                    # Object management
  ├── parsing/                    # String parsing
  ├── pcre/                       # Regular expressions
  ├── sockets/                    # Network sockets
  ├── strings/                    # String manipulation
  └── system/                     # System functions
```

**Efun Documentation Template:**
```markdown
---
layout: doc
title: category / efun_name
---
# efun_name

### NAME
    efun_name - brief description

### SYNOPSIS
    return_type efun_name(arg_type arg, ...);

### DESCRIPTION
    Detailed description of what the function does.

### ARGUMENTS
    - `arg`: Description of each argument

### RETURN VALUE
    Description of what the function returns

### ERRORS
    Conditions that cause errors

### EXAMPLES
    ```c
    // Example usage
    mixed result = efun_name(arg1, arg2);
    ```

### SEE ALSO
    related_efun(3), related_apply(4)

### NOTES
    Additional information, platform notes, availability
```

**Finding Efuns in Source:**
- Package specs: `/src/packages/*//*.spec` - Defines function signatures
- Implementation: `/src/packages/*/*.cc` - Contains C++ implementation
- Auto-generated: `/src/vm/internal/base/efuns.autogen.*` - Generated from specs

**Package to Category Mapping:**
- `core` package → multiple categories (general, interactive, contrib)
- `crypto` package → crypto/
- `db` package → db/
- `parser` package → parsing/
- `pcre` package → pcre/
- `sockets` package → sockets/
- `async` package → async/
- Math functions → floats/, general/
- Matrix functions → general/ (3D operations)

### 3. CLI Documentation (`/docs/cli/`)

Command-line tools included with FluffOS.

**Tools to Document:**
- `driver` - Main server executable
- `lpcc` - LPC compiler
- `symbol` - Symbol/object loader
- `portbind` - Privileged port binding utility
- `o2json` / `json2o` - Object serialization tools
- `generate_keywords` - IDE integration helper

**CLI Documentation Template:**
```markdown
---
layout: doc
title: cli / tool_name
---
# cli / tool_name

Brief description.

## Usage
    ./tool_name [options] arguments

## Arguments/Options
    | Option | Description |
    |--------|-------------|
    | `-x`   | Description |

## Description
    Detailed explanation

## Examples
    ```bash
    # Example usage
    ./tool_name -x file.c
    ```

## See Also
    - [related_tool](related.md)
```

### 4. Driver Documentation (`/docs/driver/`)

Documentation about driver internals, architecture, and configuration.

**Topics:**
- `config.md` - Configuration file format and options
- `adding_efuns.md` - How to add new efuns
- `stackmachine.md` - VM architecture
- `parse_tree.md` - Compiler internals
- `call_into_vm.md` - VM integration
- `malloc.md` - Memory management

### 5. Build Documentation (`/docs/`)

Root-level documentation about building and deploying FluffOS.

**Key Files:**
- `build.md` - Comprehensive build guide for all platforms
- `build_v2017.md` - Legacy build instructions
- `index.md` - Main documentation landing page
- `bug.md` - Bug reporting guidelines

### 6. Concepts Documentation (`/docs/concepts/`)

High-level explanations of LPC and MUD concepts.

**Topics:**
- `/concepts/general/lpc.md` - LPC language overview
- `/concepts/general/objects.md` - Object system
- `/concepts/general/oop.md` - Object-oriented programming in LPC
- `/concepts/general/preprocessor.md` - LPC preprocessor
- `/concepts/general/simul_efun.md` - Simulated efuns

### 7. LPC Language Documentation (`/docs/lpc/`)

LPC language syntax and features.

**Topics:**
- `/lpc/types/` - Data types (arrays, mappings, classes, etc.)
- `/lpc/constructs/` - Language constructs (loops, conditionals, etc.)

## Documentation Workflow

### Finding Undocumented Features

**1. Check Applies:**
```bash
# List all applies
cat src/vm/internal/applies

# Search for apply implementation
grep -r "APPLY_APPLY_NAME" src/

# Check if documented
find docs/apply -name "*apply_name*"
```

**2. Check Efuns:**
```bash
# List efuns in a package
cat src/packages/package_name/*.spec

# Find efun implementation
grep -r "F_EFUN_NAME\|void f_efun_name" src/packages/

# Check if documented
find docs/efun -name "*efun_name*"
```

**3. Check CLI Tools:**
```bash
# List built executables
grep "add_executable" src/CMakeLists.txt

# Check if documented
ls docs/cli/*.md
```

### Adding New Documentation

**For a New Apply:**
1. Identify the apply in `/src/vm/internal/applies`
2. Find implementation by searching for `APPLY_NAME`
3. Determine category: interactive, master, or object
4. Create markdown file: `docs/apply/category/name.md`
5. Update index: `docs/apply/category/index.md`
6. Use the apply documentation template

**For a New Efun:**
1. Find efun in package spec file: `src/packages/*//*.spec`
2. Find implementation: `src/packages/*/*.cc`
3. Determine appropriate category (see mapping above)
4. Create markdown file: `docs/efun/category/name.md`
5. Update index: `docs/efun/index.md` (add to category list)
6. Update category index: `docs/efun/category/index.md`
7. Use the efun documentation template

**For a New CLI Tool:**
1. Find source: `src/main_*.cc`
2. Understand usage from source code
3. Create markdown file: `docs/cli/name.md`
4. Use the CLI documentation template

### Verifying Documentation Completeness

**Check all applies are documented:**
```bash
# List applies
cat src/vm/internal/applies | grep -v "^#" | cut -d: -f1 | while read apply; do
  name=$(echo $apply | tr '[:upper:]' '[:lower:]')
  if ! find docs/apply -name "${name}.md" | grep -q .; then
    echo "Missing: $apply"
  fi
done
```

**Check package efuns are documented:**
```bash
# For each package spec
for spec in src/packages/*/*.spec; do
  pkg=$(basename $(dirname $spec))
  echo "Checking package: $pkg"
  grep -E "^[a-z_]+\s+[a-z_]+\(" $spec
done
```

### Documentation Standards

**File Naming:**
- Use lowercase with underscores: `function_name.md`
- Match the LPC function/apply name exactly
- Apply files: `docs/apply/category/name.md`
- Efun files: `docs/efun/category/name.md`

**Front Matter:**
```yaml
---
layout: doc
title: category / name
---
```

**Markdown Style:**
- Use `###` for NAME, SYNOPSIS, DESCRIPTION sections
- Use backticks for code: \`function_name()\`
- Use code blocks with language: \`\`\`c
- Use bullet lists with `-` for consistency

**Cross-References:**
- Apply references: `apply_name(4)`
- Efun references: `efun_name(3)`
- Format as: `[name](../path/name.md)` or just `name(3)`

**Code Examples:**
- Always provide practical examples
- Use correct LPC syntax
- Show both basic and advanced usage when relevant
- Include comments for clarity

## Common Documentation Issues

### 1. Outdated Information

**Signs:**
- References to removed features
- Incorrect function signatures
- Platform info that contradicts CI workflows

**Fix:**
- Compare with source code (`.spec` files, `.cc` implementations)
- Check CI workflows in `.github/workflows/`
- Verify build instructions match CI environment

### 2. Missing Documentation

**Common gaps:**
- New features added without docs
- Protocol-specific applies (MXP, GMCP, ZMP, MSDP, MSP)
- Newer hash algorithms in crypto package
- CLI tools that aren't user-facing
- Compile-time optional features

**Detection:**
- Compare `/src/vm/internal/applies` with `docs/apply/*/index.md`
- Compare package specs with `docs/efun/*/index.md`
- Check `src/CMakeLists.txt` executables vs `docs/cli/`

### 3. Incorrect Examples

**Validation:**
- Ensure LPC syntax is correct
- Verify efun signatures match spec files
- Test that examples would actually work

### 4. Broken Cross-References

**Common causes:**
- Files renamed but references not updated
- Obsolete features still referenced
- Incorrect relative paths

**Fix:**
- Use `grep -r "old_name" docs/` to find references
- Update or remove obsolete references
- Verify links: `[name](path.md)` format

## Testing Documentation Changes

**Local Preview:**
```bash
cd docs
npm install
npm run dev
# Visit http://localhost:5173
```

**Check for Broken Links:**
```bash
# Find all markdown links
grep -r "\[.*\](.*\.md)" docs/ --include="*.md"

# Verify referenced files exist
```

**Verify Index Files:**
```bash
# Check apply indices match files
for idx in docs/apply/*/index.md; do
  dir=$(dirname $idx)
  echo "Checking $dir"
  # Compare index entries with actual files
done
```

## Package-Specific Notes

### Crypto Package
- **Documents:** Modern hash algorithms (SHA-3, BLAKE2, SM3)
- **Location:** `docs/efun/crypto/`
- **Note:** Disabled by default on Windows
- **Version dependencies:** Document OpenSSL version requirements

### Parser Package
- **Documents:** Parse commands, sentence parsing
- **Location:** `docs/efun/parsing/`
- **Special:** Has dedicated error handling apply
- **Note:** Complex package with many internal applies

### Async Package
- **Documents:** Asynchronous I/O operations
- **Location:** `docs/efun/async/`
- **Functions:** async_read, async_write, async_getdir, async_db_exec

### Protocol Applies
- **Location:** `docs/apply/interactive/`
- **Protocols:** GMCP, MXP, MSDP, MSP, ZMP
- **Pattern:** Usually `protocol_enable()` and `protocol()` or `protocol_tag()`
- **Enable flags:** Document corresponding config options

## VitePress Configuration

The documentation is built using VitePress. Key files:

- `docs/.vitepress/config.mts` - VitePress configuration
- `docs/package.json` - Dependencies and build scripts
- Navigation is auto-generated from directory structure

## Contributing Documentation

When adding documentation:

1. **Research:** Check source code for accurate information
2. **Template:** Use appropriate template (apply/efun/CLI)
3. **Examples:** Include practical, tested examples
4. **Cross-refs:** Add relevant see-also links
5. **Indices:** Update all relevant index files
6. **Test:** Preview locally with VitePress
7. **Commit:** Use clear commit message describing changes

## Documentation Maintenance Checklist

When updating documentation:

- [ ] Check applies list matches `src/vm/internal/applies`
- [ ] Verify efun specs match package `.spec` files
- [ ] Confirm CLI tools match `CMakeLists.txt` executables
- [ ] Validate all index files are up-to-date
- [ ] Test code examples for correctness
- [ ] Check cross-references aren't broken
- [ ] Verify build instructions match CI workflows
- [ ] Update version compatibility notes
- [ ] Preview changes with VitePress local server

## Quick Reference

**Find apply implementation:**
```bash
grep -r "APPLY_NAME" src/
```

**Find efun implementation:**
```bash
grep -r "F_EFUN_NAME\|void f_efun_name" src/packages/
```

**List all packages:**
```bash
ls src/packages/
```

**Check CI-validated build commands:**
```bash
cat .github/workflows/ci.yml
```

**Generate keyword list for IDE integration:**
```bash
cd build
./generate_keywords
cat keywords.json
```

---

This guide should help maintain accurate, complete, and consistent documentation for FluffOS. When in doubt, always verify against the source code!
