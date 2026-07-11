# FluffOS Agent Guide (AGENTS.md)

Welcome to FluffOS. This guide provides future AI coding agents with essential codebase information, architecture context, Package API standards, and Continuous Integration setup.

---

## 1. Codebase Architecture & Key Components

FluffOS is a high-performance LPMUD driver and game engine. Its codebase is structured as follows:

* **`src/`**: Core driver and event dispatcher.
  - `mainlib.cc`: Engine initialization.
  - `backend.cc`: The gametick/event queues (event-loop-agnostic core); `backend_libevent.cc` is the native blocking loop, `wasm/backend_wasm.cc` the host-driven WASM loop.
  - `comm.cc`: Transport-agnostic connection handling (users, command queue, input_to, prompts, snoop). It talks to sockets only through the `Transport` interface (`net/transport.h`).
  - `net/`: The byte-transport layer -- `transport_libevent.cc` (bufferevent/TLS/websocket transports + listening ports + socket reads), `telnet.cc` (protocol, portable), `net_compat.h` (platform type decls).
  - `user.cc`: Connection session states.
  - `wasm/`: WebAssembly target files (JS transport, exported entry points, crash-handler stub).
* **`src/vm/`**: The execution VM and LPC bytecode interpreter.
  - `interpret.cc`: The core bytecode interpreter loop (`eval_instruction`).
  - `simulate.cc`: Object loading, cloning, destruction, and simulation event loops; also `recompile_object()` (in-place program update: recompiles a source file and swaps the program into the live master copy and all clones, carrying variables over by name -- see docs/concepts/general/hot_reload.md).
  - `vm.cc`: VM startup and master object callbacks.
* **`src/compiler/`**: The LPC parser, compiler, and code generator.
  - `grammar.y` / `grammar.autogen.cc`: Bison rules compiling LPC scripts to bytecode.
  - `lexer.l` / `generate.cc`: Lexer and VM instructions generator.
* **`src/packages/`**: Modular packages implementing C++ external functions (efuns). `packages/jsbridge/` (WASM target only) is the LPC <-> page-JavaScript bridge: `js_eval`/`js_call` outbound, `js_export` + the page's `fluffos.callLPC()` inbound; async deliveries run on the next tick and the pending tables hold/mark refs like the resolve() efun.
* **`testsuite/`**: Core LPC test cases, std library objects, and configurations (see `testsuite/README.md`). LPC sources use the `.lpc` extension; the driver resolves an explicit `.c`/`.lpc` exactly and prefers `.lpc` (falling back to `.c`) for extension-less names — pinned by `testsuite/single/tests/efuns/dual_extension.lpc`.

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

### Object Variable Block & Allocation Tags
* An object's global variables are a SEPARATE allocation from the `object_t` itself (`ob->variables`, `TAG_OBJ_VARS`, always >= 1 svalue, sized to `prog->num_variables_total`) -- this is what lets `recompile_object()` swap a program with a different variable count onto a live object. Allocate via `allocate_object_variables()`; `tot_alloc_object_size` accounting uses `sizeof(object_t) + (max(n,1)-1)*sizeof(svalue_t)`.
* When introducing a new DMALLOC tag, wire it into the debug-build walkers in `packages/develop/checkmemory.cc` (a `DO_MARK` at the owning object's mark site plus an orphan-report case), or every testsuite run on a Debug build will flag the blocks. Pick a free `TAG_PERMANENT + n` slot in `base/internal/debugmalloc.h`.
* `destruct_object()` SWEEPS the VM stack (`remove_object_from_stack`): any efun that runs arbitrary LPC (applies, `__INIT`) before cleaning up an object argument must free it with `free_svalue(sp, ...)`, never `free_object(&sp->u.ob, ...)` -- the slot may have become a plain number 0.

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

* **Header Probing Location**: Do not move or reorder the `# System headers` block in `src/CMakeLists.txt`. It must remain in place to preserve configure-time check ordering.
* **Header Guards**: Do not modify `HAVE_*` macro guards in C++ source files unless explicitly required.
* **Autogenerated Files**:
  - `grammar.autogen.cc`/`.h` is generated from Bison `grammar.y`.
  - `efuns.autogen.cc`/`.h` is generated from spec files using the `make_func` tool.
  - `applies_table.autogen.cc`/`.h` is generated using the `build_applies` tool.
  - `options.autogen.h` is generated using the `make_options_defs` tool.
* **WASM / Emscripten build**: the driver cross-compiles to WebAssembly and runs a full mudlib in the browser (the page is the telnet client). **`src/wasm/README.md` is the architecture doc** (Transport interface, host-driven tick loop, per-target implementation files, package on/off matrix, roadmap); **`docs/build-wasm.md` is the user workflow** (deps via `tools/wasm/build-deps.sh`, the `native-tools` + `wasm` CMake presets, mudlib packaging via `tools/wasm/pack-mudlib.sh`). Per-connection byte transport is the `Transport` interface (`src/net/transport.h`); per-target singletons (event loop, TLS, DNS resolver, crash handler) are selected at link time (`*_libevent.cc` vs `src/wasm/*.cc` / `*_stub.cc`) -- do not add `#ifdef __EMSCRIPTEN__` to shared logic files. CI's `wasm` job runs the LPC testsuite inside the wasm driver under node; testsuite files for optional packages must guard themselves with `#ifdef __PACKAGE_*__`.

---

## 6. Continuous Integration (CI) Environment

FluffOS uses GitHub Actions for CI on pull requests and pushes to `master`.

* **Unified CI** (`.github/workflows/ci.yml`):
  All platforms and configurations are defined as matrix entries in a single workflow:
  - **Ubuntu** (`ubuntu-24.04`): GCC and Clang, Debug and RelWithDebInfo builds.
  - **Ubuntu + Sanitizers** (`ubuntu-24.04`): Clang with `-DENABLE_SANITIZER=ON` (Address & Undefined Behavior), Debug and RelWithDebInfo.
  - **macOS** (`macos-14`, Apple Silicon): Default Clang, Debug and RelWithDebInfo.
  - **Windows** (`windows-latest`): MSYS2/MinGW64 via `msys2/setup-msys2@v2`, Debug and RelWithDebInfo.
  - **WASM** (`ubuntu-24.04`, separate `wasm` job): latest emsdk (deps cache keyed on the resolved `emcc -dumpversion`) + cached wasm-built ICU → the two cross-build presets → the LPC testsuite running *inside the wasm driver* under node. The release workflow (`release.yml`) has a matching `build-wasm` job that ships `fluffos-<version>-wasm.zip` (driver + web terminal + `pack-mudlib.sh`).
  - **Flow** (native platforms): Install dependencies → CMake configure → Build → GTest unit tests → LPC testsuite.
* **Docker CI** (`.github/workflows/docker-publish.yml`):
  - Builds a Docker image and pushes to `ghcr.io` on tagged releases and master merges.

---

## 7. LPC Testsuite Conventions

* **Efun Tests**: Any new or modified external function must have matching LPC test scripts added to the `testsuite/` directory under `testsuite/single/tests/efuns/`.
* **Testing Targets**:
  - `driver-testsuite`: Boots the local driver pointing to the test configuration.
  - `driver-fulltest` / `driver-autotest`: Runs the LPC test suite and reports results before exiting.
* **Harness facts that bite:** LPC fixture files must live OUTSIDE `testsuite/single/tests/` (use `/clone/...` or write them at runtime under `/data/...`) -- the runner executes every `tests/**/*.lpc` in RANDOMIZED order. Tests that register global state (e.g. `master->set_compile_hooks()`) must tear it down UNCONDITIONALLY: ASSERT macros record-and-continue, so wrap the body in `catch(run_checks())`, clean up, then re-`error()`. `master::flag()` sits on the call stack for the entire run (so e.g. the master object can never be recompiled from inside a test; use a `call_out` that fires post-run and `shutdown(-1)`s on failure -- call_outs only fire after the suite in FULL runs, never in single-file runs, which shut down synchronously). A plain `-ftest:` filter needs the FULL test path; suite runs dirty `testsuite/aw_test.txt` and `testsuite/trace_test.json` (restore before committing). `file_name(ob)` returns a leading slash.
* **The LPC suite is a real pass/fail gate, registered with ctest.** the ctest test is named `testsuite` — `ctest -R testsuite` (equivalently the `driver-autotest` CMake target) runs `driver etc/config.test -ftest`; CI runs `ctest -LE testsuite` (GTest) and `ctest -L testsuite` (this suite) as separate steps. The runner (`testsuite/command/tests.lpc`) prints gtest-style `[ RUN ]/[ OK ]/[ FAILED ]` blocks with per-file timing; **failed checks are recorded and the run continues** (one run reports every failure), then a recap lists the failed files and the driver exits nonzero. A clean run prints `Checks succeeded.` and exits 0 — that pair is the sound pass signal. Filter runs with `-ftest:single/tests/efuns/foo.lpc` (one file) or `-ftest:efuns/dual*` (glob). When touching the lexer/parser, run the suite 2–3× (it randomizes test file order) and prefer both a Debug ASan build and a `RelWithDebInfo` build (some issues are release-only).

---

## 8. LPC Language & Runtime Concepts for Agents

**When writing or reviewing LPC code, consult `docs/lpc/` first** — it is the maintained language reference, kept in sync with driver behavior: source-file/extension resolution (`docs/lpc/source-files.md`), reading compiler diagnostics (`docs/lpc/diagnostics.md`), the full preprocessor reference (`docs/lpc/preprocessor/` — define/include/conditionals/pragma), and the types/constructs pages. Testsuite conventions live in `testsuite/README.md`.

When editing compiler, VM, or package features, keep these structural mechanics in mind:

### Mudlib vs Driver Separation
* **The Driver (FluffOS)**: Written in C++, executes as the operating system, virtual machine, and compiler for LPC. It exposes built-in commands as "external functions" (efuns).
* **The Mudlib**: Written in LPC, contains the game logic, rooms, user logins, and rule definitions. It sits in a separate folder (e.g. `testsuite/` or game folders) and is loaded by the driver.

### Source File Extensions (`.lpc` / `.c`)
* LPC source files may use either `.lpc` (preferred) or `.c` (legacy). Resolution, implemented in `load_object()` (`src/vm/internal/simulate.cc`) and pinned by `testsuite/single/tests/efuns/dual_extension.lpc`:
  - An **explicit extension is exact** — `load_object("/foo.c")` probes only `foo.c`; no `.lpc` lookup, and vice versa. On a miss the master's `compile_object()` virtual hook gets the *stripped* name; if it declines, the load returns `0`.
  - **Extension-less names prefer `.lpc`, fall back to `.c`.**
  - **Object identity is extension-blind**: object names carry no extension (`filename_to_obname()` and `otable.cc basename()` strip either spelling), so any spelling of a loaded object's name finds it — the registry is consulted before the filesystem. `prog->filename` keeps the real extension of the compiled file.
* Do **not** append `".c"` to names before loading (the old pattern); pass names through and let `load_object()` resolve. Suffix handling in `save_object`/`restore_object`, `replace_program()`, `function_exists()`, and `children()` must treat both spellings equivalently.

### Simulated Efunctions (Simul_efuns)
* If an object makes a global function call (e.g. `foo()`) that is not declared inside the object and is not a built-in driver efun, the compiler resolves it as a simulated efun call.
* Simulated efuns are written in LPC and defined in a single file loaded at boot.
* Overriding built-in efuns is allowed in LPC by creating a simul_efun with the same name. To bypass a simul_efun override and invoke the driver efun directly, use the `efun::` prefix (e.g. `efun::move_object()`).

### Applies (Driver-to-LPC Callbacks)
* "Applies" are standard callbacks that the driver VM invokes on LPC objects during specific runtime events (e.g., `create` during cloning, `init` when entering a room, `clean_up` during sweep collections).
* Applies are mapped using the `applies_table.autogen.cc` lookup tables.
* **Compile-time master applies** `inherit_program(from, path, priv)` and `include_file(compiled, from, path)` are consulted for every inherit statement / #include directive: string return redirects the path, array-of-strings return supplies the source text itself, any other return denies. They run MID-COMPILE (same precedent as `valid_override`/`get_include_path`): implementations must never trigger another compile. Reference pages in docs/apply/master/; the dependency graph they expose powers the hot-reload daemon (`testsuite/single/hot_reload.lpc`).

### Hot Reload (`recompile_object()`)
* `recompile_object(master_copy)` recompiles the source and swaps the program into the live master copy and every clone -- no destruct, identity preserved, variables carried by name (private included; new program's `__INIT` runs first, then surviving names get old values). Works for the master object, the simul_efun object (their cached dispatch tables `master_applies`/`simuls` are rebuilt against the new program BEFORE its `__INIT` runs; simul indices are name-stable by design) and virtual objects (the backing program is what recompiles).
* Invariants when touching this machinery: refuse while any live frame executes the old program (bytecode/variable indices are layout-relative); void pending `replace_program()` entries at each target's swap point (`cancel_pending_replace_program` -- entries are computed against the program being replaced and old code can register one mid-update); function pointers that depend on the owner's layout (`FP_LOCAL`, `FP_FUNCTIONAL`) carry an `owner_gen` snapshot of `ob->prog_generation` and error cleanly when stale; `FP_LOCAL` funptrs store their creation program and account `func_ref` against IT (creation/destruction must stay symmetric -- decrementing `owner->prog` corrupts counts after a swap).

---

## 9. Documentation (`docs/`)

The FluffOS documentation site lives in the `docs/` directory and is built with **Docusaurus 3** (React-based).

### Framework & Build

* **Framework**: Docusaurus 3 (`@docusaurus/preset-classic`). Do **not** confuse with VitePress or Jekyll — the site was migrated from those frameworks and their leftover directories (`.vitepress/`, `_layouts/`, `css/`) have been removed.
* **Node**: Use NVM. The active version is managed via `~/.nvm`. Always `source "$NVM_DIR/nvm.sh"` before running npm commands.
* **Key config files**:
  - `docs/docusaurus.config.ts` — site config, navbar, footer, docs plugin path
  - `docs/sidebars.ts` — hand-authored sidebar skeleton (Docusaurus `SidebarsConfig` format, NOT VitePress format); imports `sidebars.generated.json`
  - `docs/sidebars.generated.json` + `docs/sidebar_meta.json` — generated sidebar trees and their curated metadata (see "Sidebar" below)
  - `docs/src/css/custom.css` — Infima CSS variable overrides
* **Build commands** (run inside `docs/`):
  ```bash
  npm run build    # production build → docs/build/
  npm run dev      # dev server on 0.0.0.0
  ```
* **Docs plugin config**: `path: '.'` and `routeBasePath: '/'` — markdown files live directly in `docs/`, not a subdirectory.

### Markdown Compatibility

* `markdown.format: 'detect'` is set in `docusaurus.config.ts` so `.md` files use standard CommonMark (not MDX), while `.mdx` files get full MDX.
* Even so, bare `{...}` patterns in prose text (not inside fenced code blocks) are still parsed as JSX expressions and will cause SSG build failures. Escape them as `\{...\}` if they appear in non-code contexts.
* Curated top-level pages use MDX (`docs/index.mdx` landing page with a card grid; `docs/build.mdx` with per-platform `<Tabs>`); admonitions (`:::note` / `:::tip` / `:::warning[Title]`) work in both `.md` and `.mdx`. In `.mdx`, `<https://...>` autolinks are NOT supported (parsed as JSX) — use `[text](url)`.

### Config Docs (auto-generated)

`docs/driver/config.md` is **generated** from `src/base/internal/rc.cc`. Do not edit it by hand. Regenerate with:
```bash
python3 docs/gen_config_docs.py
```
CI fails if the file is stale (`.github/workflows/config-docs.yml`).

### Sidebar (partly generated)

The sidebar fully expands to every page. It has two layers:
* **`docs/sidebars.ts`** — the hand-authored skeleton (Getting Started, the `lpc/` tree, Historical) which **splices in** the generated trees.
* **`docs/sidebars.generated.json`** — generated by `docs/gen_sidebar.py` from the reference doc trees (`efun/`, `apply/`, `stdlib/`, `concepts/`, `driver/`, `cli/`, `zh-CN/`) plus the curated labels/descriptions/ordering in `docs/sidebar_meta.json`. Do not hand-edit the JSON.

After adding, removing, or moving any page in those trees, run:
```bash
python3 docs/gen_sidebar.py
```
CI fails if the file is stale (`.github/workflows/docs-sidebar.yml`).

Those trees have **no `index.md` files** (the old `gen_index.py`/`update_index.sh` machinery is gone): each category landing page is a Docusaurus `generated-index` card page whose title/description come from `sidebar_meta.json`. `lpc/index.md` is hand-written and stays.

Sidebar entry format notes:
* Links are **doc IDs** (relative file path without extension), not `.html` URLs
* Category with a hand-written landing page: `link: { type: 'doc', id: 'path/index' }`
* Category with an auto-generated card landing page: `link: { type: 'generated-index', title, description, slug }`

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

---

## 11. Compiler Front-End (`compiler/internal/`)

**Read `src/compiler/internal/README.md` first** — it is the authoritative, maintained architecture document (module responsibilities, the data-flow diagram, header-ordering constraints). This section is the agent-facing summary of the load-bearing facts.

### Single-scan, Flex-native lexer + preprocessor
* **There is no separate preprocessor and no hand-rolled input buffer.** Preprocessing is a set of Flex rule actions inside the driver's ONE scan over the source; every byte is read exactly once. The old standalone `preprocessor.cc` engine and the old `outp`/ring-buffer are both gone.
* **Input rides on Flex's own buffer stack, and every input is an in-memory buffer.** The main file is slurped and installed as the base buffer (`lpc_lex_set_source`); macro expansions, pushbacks, and `#include` contents are pushed as in-memory Flex buffers (`lpc_lex_push_string_buffer`), popped at their `<<EOF>>`. There is no byte-stream abstraction and no `YY_INPUT` refill. Raw mid-rule reads (heredoc bodies, function-like macro-argument collection) go through `lpc_lex_getc()` (a wrapper over the generated `yyinput()`), which transparently pops a drained splice buffer into its parent.
* **Line/column tracking is native Flex state** (`%option yylineno` + a `YY_USER_ACTION` column). `current_line` is a macro over a reference to the innermost real buffer's per-buffer counter, so isolation across expansions and includes is automatic.
* **Macro expansion is rescan-driven**: `lpc_lex_resolve_identifier()` pushes the raw substituted body as a buffer and nested references expand when the rescan reaches them; self-reference termination is the set of live expansion-buffer frames. `#if`/`#elif` expressions are evaluated over **tokens** pulled through the scanner (not a private character walk), so `#define X 1+1` + `#if X*2` is `1+1*2` with correct C precedence. Redefining a macro with a different body is a **non-fatal warning**, not an error.

### The lexer.l purity rule (enforced)
* **`lexer.l` contains ONLY Flex interactions** — patterns, start-condition transitions (`BEGIN`), pushback (`yyless`), and the minimal trailer primitives that must dereference the generated scanner's private types (`yyguts_t`, `yyinput()`, the buffer-stack internals): buffer install/teardown, `lpc_lex_getc()`, the nested `#if`-expression pull, and the raw buffer accessors. ALL policy lives in `lexer_rules.cc` (token shaping), `lexer_rules_pp.cc` (preprocessing), or `lexer_utils.cc` (buffer/include bookkeeping, macro resolution, the tagged flex allocator, splice push/pop policy). When adding a rule, put anything that only needs `yytext`/`yylval`/`yyget_extra()` in a `lexer_rules*.cc` helper; put anything expressible over the public reentrant API in `lexer_utils.cc`.

### Grammar carries some lexical decisions (don't reintroduce lexer state for them)
* Array/mapping opens `({`/`([` are ordinary `'(' '{'` / `'(' '['` token pairs the grammar pairs; the `(: name` first-class-function split is decided by LALR lookahead, not a start condition. `grammar.y`'s `%expect` counts the intentional (documented) conflicts — changing those productions means re-running `bison -Wcounterexamples` and updating the count.
* **Token inventory is deliberately minimal.** Same-precedence operator families share one value-carrying token (opcode in `yylval`: `L_EQ_NE`, `L_SHIFT`, `L_INC_DEC`, `L_ORDER`); single-char operators are plain char tokens (`'!'`, `'.'`). Add an `L_*` token only for a real new feature or a distinct grammar position.

### Diagnostics
* Every `yyerror()`/`yywarn()` captures a structured `Diagnostic` (position + column, source snippet + caret, `#include` chain, macro-expansion chain, operand ranges via Bison `%locations`, fix-it hints) and reports it **clang-style by default** via `report_compile_diagnostic()`. `lpcshell` reads `compiler_diags` directly.
* **Never pass untrusted text as a printf format.** `lexerror(s)`, `yyerror`, `yywarn`, and VM `error()` are printf-style; source-derived text (`#error` payloads, macro/verb names, filenames) must be an ARGUMENT (`yywarn("%s", s)`), never the format string (a `%` in the text is a crash / CodeQL `cpp/tainted-format-string`).

### The scratchpad arena (allocation during a compile)
* **All transient compile strings live on the monotonic arena** (`scratchpad.h`): build them as `ScratchString`/`ScratchVector` (allocation = pointer bump; individual deallocation = no-op; `scratch_destroy()` bulk-frees at compile end, so `error()` unwinds leak nothing). Materialize a parser token with `scratch_new_string()` — the `%union`'s `string` member is `ScratchString *`; SHARED strings on the value stack use the separate `shared_string` member (see `grammar.y`'s `function` production) so scratch-vs-shared lifetime is visible in the grammar.
* **Never put compile-surviving state on the arena**: the macro table (`PpMacro`/`LpcMacroTable`), predefines, `Diagnostic` records (lpcshell reads them after the arena resets), and program data stay `std::string`/heap — copy out at the boundary. Arena-backed members of objects that survive the compile (scanner-context accumulators) must be re-initialized per compile (`lpc_lex_reset_context`).

### One compile state; staged outputs
* **`g_compile` (CompileState, compiler.h) is THE compiler state object**: compile identity (filename, vm_context), the preprocessor state held DIRECTLY (`g_compile.macros` = user #defines only, `g_compile.conds` = the #if stack; predefines live in a shared version-cached table and never enter the per-compile table -- `start_new_file(..., keep_macros=true)` retains #defines across REPL chunks with ZERO per-compile setup allocation), the diagnostics stream, and all one-shot diagnostic context. Legacy spellings (`compiler_diags`, `compiler_vm_context`, `compiler_pending_*`) are inline references into it -- do not add new loose compiler globals; add members.
* **Every compile stage has an output mode** via `lpcc`: `-E` (preprocessed source -- token-reconstructed, since the single-scan design has no textual pp artifact), `--tokens` (line:col kind spelling), `--ast` (parse trees via dump_tree before codegen), `-O0` (tree optimizer off -> dump_prog shows PRE-optimization bytecode), and the default full optimized `dump_prog` disassembly. Pre-parse stages live in `compiler/internal/stage_output.cc`.

### Utilities & testing
* **`lexer_utils.cc` / `lexer_utils.h`** also own include-path management, path normalization, and the predefine registry (`add_predefine`, `add_quoted_predefine`, `inc_open` — `std::string_view` interfaces; `inc_path`/`inc_list` are `std::vector<std::string>`, no manual GC marking).
* Compiler front-end tests are `src/tests/test_compiler.cc` (end-to-end through the real lexer) and `src/tests/test_lexer.cc` (token-level). When testing `#undef` of a predefined value, register the predefine (e.g. `FLUFFOS`) via `add_predefine` in setup — the full driver runtime is not initialized in unit tests.
* **The grammar ships as a machine contract**, all under `tools/lpc-syntax/`: `grammar.ebnf` (three layers: hand-authored Lexical + Preprocessor from `grammar_lexical.ebnf.in`, generated Syntax from `grammar.y`) and `lpc-grammar.json` (consumed by the JS tokenizer/highlighter/formatter and the VS Code extension in `tools/lpc-syntax/vscode/`). Regenerate with the `generate_ebnf` CMake target whenever `grammar.y` or the reserved-word/operator tables change — an uncategorized new token FAILS regeneration by design. JS tests: `node tools/lpc-syntax/test.mjs`.
* **CI regenerates the scanner and parser from `lexer.l`/`grammar.y` on every platform** (flex ≥ 2.6, bison ≥ 3.8 are installed in every CI environment), so a `lexer.l`/`grammar.y` change is validated by the toolchain, not just the checked-in `*.autogen.*` copies. Keep a clean regeneration warning-free.

---

## 12. Header Inclusion Guidelines

To maintain code health and consistency, FluffOS enforces the following global include rules:

* **First Include ("base/std.h")**: All code files in the driver (excluding files in `src/base/` and package implementations in `src/packages/`) **must** include `"base/std.h"` as their very first line, followed by a blank line to clearly separate it from other includes.
* **No Other Base Includes**: Other headers inside `src/base/` should not be included directly. Headers inside `src/base/internal/` must include necessary dependencies themselves.
* **Package Include ("base/package_api.h")**: All package source files must include `"base/package_api.h"` as their first header, and **must not** include `"base/std.h"` again.

