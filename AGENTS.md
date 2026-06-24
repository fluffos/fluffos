# FluffOS Agent Guide (AGENTS.md)

Welcome to FluffOS. This guide provides future AI coding agents with essential codebase information, architecture context, Package API standards, and Continuous Integration setup.

---

## 1. Codebase Architecture & Key Components

FluffOS is a high-performance LPMUD driver and game engine. Its codebase is structured as follows:

* **`src/`**: Core driver and event dispatcher.
  - `mainlib.cc` / `backend.cc`: Engine initialization, main game loop, and network event dispatching.
  - `comm.cc`: Network socket read/write, Telnet, and connection handling.
  - `user.cc`: Connection session states.
* **`src/vm/`**: The execution VM and LPC bytecode interpreter.
  - `interpret.cc`: The core bytecode interpreter loop (`eval_instruction`).
  - `simulate.cc`: Object loading, cloning, destruction, and simulation event loops.
  - `vm.cc`: VM startup and master object callbacks.
* **`src/compiler/`**: The LPC parser, compiler, and code generator.
  - `grammar.y` / `grammar.autogen.cc`: Bison rules compiling LPC scripts to bytecode.
  - `lex.cc` / `generate.cc`: Lexer and VM instructions generator.
* **`src/packages/`**: Modular packages implementing C++ external functions (efuns).
* **`testsuite/`**: Core LPC test cases, std library objects, and configurations. Note: Although these files use the `.c` extension, they are written in LPC, not standard C.

---

## 2. Package API & VM Stack Guidance

When creating or modifying packages (efuns) under `src/packages/`, you must follow these rules:

### Unified Inclusion
* All package source files **must** include `"base/package_api.h"` as their first header. Avoid importing VM, interpreter, or compiler headers directly.

### Spec Files
* Package functions are declared in `[package_name].spec` files using LPC-like syntax, e.g.:
  `int db_close(int);`
  The `make_func` parser compiler compiles these specs into C++ glue code (`efuns.autogen.cc`).

### VM Stack & Argument Unpacking
LPC arguments and return values are passed on the VM evaluation stack using `svalue_t` structures. The stack pointer is `sp`.

#### Navigating the Stack:
* The last argument is at `sp`. The first argument is at `sp - (num_args - 1)`.
* For variable-argument functions, the number of arguments passed is available via the global `st_num_arg`.

#### Accessing Arguments:
* `sp->u.number` or `(sp - n)->u.number`: Accessing integer values.
* `sp->u.string` or `(sp - n)->u.string`: Accessing string values.
* `sp->u.real` or `(sp - n)->u.real`: Accessing float/real values.
* `sp->u.arr`: Accessing arrays (`array_t *`).
* `sp->u.map`: Accessing mappings (`mapping_t *`).
* `sp->u.buf`: Accessing buffers (`buffer_t *`).

#### Modifying the Stack & Returning Values:
* Use `pop_stack()` or `pop_n_elems(n)` to clean up input arguments.
* Push return values to the stack:
  - `push_number(val)` / `push_real(val)`
  - `push_malloced_string(str)` / `push_constant_string(str)`
  - `put_malloced_string(str)` / `put_array(arr)` / `put_mapping(map)` (these replace the top stack element directly).

---

## 3. Memory Management & Reference Counting

LPC variables are dynamically typed and managed via the `svalue_t` structure. Memory safety is critical:

### Reference Counting
* Compound types (`T_ARRAY`, `T_MAPPING`, `T_BUFFER`, `T_CLASS`, and ref-counted `T_STRING` strings) are reference-counted.
* If you copy a pointer to one of these types into another structure, you must increment its ref count (e.g., `arr->ref++`).
* If you overwrite or destroy a reference, you must decrement its ref count or use `free_svalue(sval, tag)` to clean it up safely.

### String Allocations
* **Constant Strings (`STRING_CONSTANT`)**: Not ref-counted or freed (point directly to literal read-only memory).
* **Malloced Strings (`STRING_MALLOC`)**: Uniquely copied and freed via `FREE()`. Allocated using `string_copy()`.
* **Shared Strings (`STRING_SHARED`)**: Stored in a global string table, immutable, and ref-counted. Generated using `make_shared_string()`.

---

## 4. Error Handling & Stack Unwinding Safety

> [!CAUTION]
> **Longjmp Memory Leaks**: FluffOS uses longjmps for error handling. Calling the `error()` function throws an LPC exception that immediately unwinds the VM stack, bypassing normal C++ return paths.
>
> **Prevention**: Any raw heap pointer (`new` or `malloc`) allocated before calling `error()` or any nested VM function that could trigger an error **will be leaked**.
>
> **Remedy**: Always use RAII containers (`std::unique_ptr`, `std::shared_ptr`) or C++ scope guards (`DEFER` / `Neargye/scope_guard`) to guarantee resource deallocation during unwinding.

---

## 5. CMake & Compilation Constraints

* **Header Probing Location**: Do not move or reorder the `# System headers` block in `src/CMakeLists.txt` (currently around line 187). It must remain in place to preserve configure-time check ordering.
* **Header Guards**: Do not modify `HAVE_*` macro guards in C++ source files unless explicitly required.
* **Autogenerated Files**:
  - `grammar.autogen.cc`/`.h` is generated from Bison `grammar.y`.
  - `efuns.autogen.cc`/`.h` is generated from spec files using the `make_func` tool.
  - `applies_table.autogen.cc`/`.h` is generated using the `build_applies` tool.
  - `options.autogen.h` is generated using the `make_options_defs` tool.

---

## 6. Continuous Integration (CI) Environment

FluffOS uses GitHub Actions for CI on pull requests and pushes to `master`.

* **Unified CI** (`.github/workflows/ci.yml`):
  All platforms and configurations are defined as matrix entries in a single workflow:
  - **Ubuntu** (`ubuntu-24.04`): GCC and Clang, Debug and RelWithDebInfo builds.
  - **Ubuntu + Sanitizers** (`ubuntu-24.04`): Clang with `-DENABLE_SANITIZER=ON` (Address & Undefined Behavior), Debug and RelWithDebInfo.
  - **macOS** (`macos-14`, Apple Silicon): Default Clang, Debug and RelWithDebInfo.
  - **Windows** (`windows-latest`): MSYS2/MinGW64 via `msys2/setup-msys2@v2`, Debug and RelWithDebInfo.
  - **Flow** (all platforms): Install dependencies → CMake configure → Build → GTest unit tests → LPC testsuite.
* **Docker CI** (`.github/workflows/docker-publish.yml`):
  - Builds a Docker image and pushes to `ghcr.io` on tagged releases and master merges.

---

## 7. LPC Testsuite Conventions

* **Efun Tests**: Any new or modified external function must have matching LPC test scripts added to the `testsuite/` directory under `testsuite/single/tests/efuns/`.
* **Testing Targets**:
  - `driver-testsuite`: Boots the local driver pointing to the test configuration.
  - `driver-fulltest`: Runs the LPC test suite and reports results before exiting.

---

## 8. LPC Language & Runtime Concepts for Agents

When editing compiler, VM, or package features, keep these structural mechanics in mind:

### Mudlib vs Driver Separation
* **The Driver (FluffOS)**: Written in C++, executes as the operating system, virtual machine, and compiler for LPC. It exposes built-in commands as "external functions" (efuns).
* **The Mudlib**: Written in LPC, contains the game logic, rooms, user logins, and rule definitions. It sits in a separate folder (e.g. `testsuite/` or game folders) and is loaded by the driver.

### Simulated Efunctions (Simul_efuns)
* If an object makes a global function call (e.g. `foo()`) that is not declared inside the object and is not a built-in driver efun, the compiler resolves it as a simulated efun call.
* Simulated efuns are written in LPC and defined in a single file loaded at boot.
* Overriding built-in efuns is allowed in LPC by creating a simul_efun with the same name. To bypass a simul_efun override and invoke the driver efun directly, use the `efun::` prefix (e.g. `efun::move_object()`).

### Applies (Driver-to-LPC Callbacks)
* "Applies" are standard callbacks that the driver VM invokes on LPC objects during specific runtime events (e.g., `create` during cloning, `init` when entering a room, `clean_up` during sweep collections).
* Applies are mapped using the `applies_table.autogen.cc` lookup tables.

---

## 9. Documentation (`docs/`)

The FluffOS documentation site lives in the `docs/` directory and is built with **Docusaurus 3** (React-based).

### Framework & Build

* **Framework**: Docusaurus 3 (`@docusaurus/preset-classic`). Do **not** confuse with VitePress — the site was migrated from VitePress and `.vitepress/` is a dead directory that can be ignored.
* **Node**: Use NVM. The active version is managed via `~/.nvm`. Always `source "$NVM_DIR/nvm.sh"` before running npm commands.
* **Key config files**:
  - `docs/docusaurus.config.ts` — site config, navbar, footer, docs plugin path
  - `docs/sidebars.ts` — sidebar tree (Docusaurus `SidebarsConfig` format, NOT VitePress format)
  - `docs/src/css/custom.css` — Infima CSS variable overrides
* **Build commands** (run inside `docs/`):
  ```bash
  npm run build    # production build → docs/build/
  npm run dev      # dev server on 0.0.0.0
  ```
* **Docs plugin config**: `path: '.'` and `routeBasePath: '/'` — markdown files live directly in `docs/`, not a subdirectory.

### Markdown Compatibility

* `markdown.format: 'detect'` is set in `docusaurus.config.ts` so `.md` files use standard CommonMark (not MDX).
* Even so, bare `{...}` patterns in prose text (not inside fenced code blocks) are still parsed as JSX expressions and will cause SSG build failures. Escape them as `\{...\}` if they appear in non-code contexts.

### Config Docs (auto-generated)

`docs/driver/config.md` is **generated** from `src/base/internal/rc.cc`. Do not edit it by hand. Regenerate with:
```bash
python3 docs/gen_config_docs.py
```
CI fails if the file is stale (`.github/workflows/config-docs.yml`).

### Sidebar Format

Docusaurus sidebars use a different format than VitePress:
* Links are **doc IDs** (relative file path without extension), not `.html` URLs
* Category with a landing page: `link: { type: 'doc', id: 'path/index' }`
* Auto-generated from directory: `{ type: 'autogenerated', dirName: 'concepts' }`

---

## 10. Windows Environment Requirements (CRITICAL)

When working on a Windows host, FluffOS compiles and runs **exclusively** within the **MSYS2 / MinGW64** environment.

> [!CAUTION]
> **MSYS2 Environment Constraint**: Before proposing or executing any command (such as compilation, configuration, CMake, Make, Bison, or Git operations) on a Windows system, you **must ensure that `MSYS2_ROOT` is defined** (e.g., `E:\msys64`).
> 
> All commands **must be executed within that MSYS2 environment** (by prepending MSYS2 MinGW64 binary directories to the environment `PATH` or running through MSYS2 bash). Do NOT use standard Windows-installed binaries (like native Git, native CMake) or execute commands directly inside standard Windows Command Prompt or native PowerShell without MSYS2 paths active.
> 
> If `MSYS2_ROOT` is not configured, you **must refuse to run any commands** and prompt the user to configure the paths or launch via `open-editor-msys2.bat` (see README.md).

---

## 10. WSL Environment Requirements (CRITICAL)

WSL provides a **native Linux** environment. FluffOS runs natively under WSL distributions (Ubuntu, Alpine) using the standard Linux build toolchain — not MSYS2.

> [!CAUTION]
> **Mapped Directory Detection**: If the workspace path starts with `/mnt/` (inside WSL) or resides on a Windows drive letter path (e.g. `E:\src\fluffos`) being accessed via WSL, the codebase is on a **mapped Windows filesystem**. Cross-filesystem I/O across the Windows/Linux boundary is extremely slow.
> 
> **Agent action**: If you detect the user is operating in a mapped directory, you **must warn them** and suggest they:
> 1. Move or clone the repository into the WSL distribution's native Linux volume (e.g. `/home/user/fluffos`).
> 2. Use `open-editor-wsl.bat` (see README.md) to relaunch the editor from the native WSL path.
> 3. Access the native volume in Windows Explorer via `\\wsl.localhost\<distro>\home\user\fluffos`.
