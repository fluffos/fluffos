# FluffOS Issue Triage — July 2026

Full review of all 101 open issues. Every issue was checked against the current
master source tree (and git history / CI configuration) before any action was
taken. Result: **50 issues closed** (19 already fixed or implemented, 31
obsolete / superseded / not planned), **51 remain open**, and the **top 10
fix candidates** below are ranked by user impact and effort.

---

## Top 10 issues to fix (ranked)

### 1. #927 — `pcre_match_all` infinite loop → driver OOM
`src/packages/pcre/pcre.cc:507-527` advances with `offset = ovector[1]` and has
no zero-width-match handling: a pattern like `(\w*)` matching empty never
advances, appending match arrays until the OOM killer fires. Any LPC code can
take down the driver with a common regex.
**Fix (small):** standard PCRE idiom — on `ovector[1] == offset`, retry at the
same offset with `PCRE_NOTEMPTY | PCRE_ANCHORED`, and on failure advance one
character; add a testsuite case for `pcre_match_all("hi there", "(\\w*)")`.

### 2. #1047 — segfault on error with "interactive catch tell" enabled
`tell_object` → `tell_npc` runs `APPLY_CATCH_TELL` via plain `apply()` from the
`print_prompt` path (`src/comm.cc:1241`, `src/vm/internal/simulate.cc:2179-2206`)
with no error context set up; `error_handler` then dereferences a null
`current_error_context->save_csp` (`simulate.cc:1968`). Remote users can
trigger a driver crash on any lib using this config option.
**Fix (small-medium):** null-guard `current_error_context` in
`error_handler`/`throw_error`, and/or route the prompt-path apply through
`safe_apply`.

### 3. #1036 — sqlite crasher (double `sqlite3_finalize`)
`SQLite3_execute` (`src/packages/db/db.cc:956-961`) finalizes the statement on
a `sqlite3_get_table` failure but does not null `c->SQLite3.results`; the next
`db_exec` finalizes the already-freed statement in `SQLite3_cleanup`.
**Fix (small):** null the pointer (and reset row state) after the finalize;
audit close/cleanup for the same pattern.

### 4. #1088 + #1051 (duplicates) — "BUG: inherit function is undefined or prototype" spam and broken `::` resolution
In `find_matching_function` (`src/compiler/internal/compiler.cc:1155`), a
prototype-only table entry (e.g. from a header included by several sibling
inherits) triggers the warning and `return 0` **without** searching that
program's own inherits — spamming one warning per sibling and making legitimate
`name::func()` calls fail with "Unable to find the inherited function".
Multiple users hit this; bisected to commit 6bc96f745.
**Fix (small-medium):** on a `FUNC_UNDEFINED|FUNC_PROTOTYPE` hit, fall through
to search `prog->inherit[]` for the real definition; only diagnose if the whole
lookup fails. Fix under #1088 and close #1051 as duplicate. Add an LPC
testsuite case mirroring the #1051 repro.

### 5. #1127 — `foreach` silently truncates mappings/arrays > 65535 entries
The remaining-iteration count is stored in `svalue_t.subtype`, an
`unsigned short` (`src/vm/internal/base/svalue.h:41`;
`interpret.cc:2776, 2803, 2840`). With the default max mapping size of 150000,
a 65536-entry mapping iterates zero times — silent data loss.
**Fix (small):** the loop-counter stack slot is a `T_NUMBER` temporary — store
the count in `u.number` instead; update F_FOREACH / F_NEXT_FOREACH /
F_EXIT_FOREACH and add a >65536-entry test.

### 6. #1014 — parser package crasher (negative `pop_n_elems` in `restore_context`)
`restore_context` computes `pop_n_elems(sp - econ->save_sp)` with no sign check
(`src/vm/internal/base/interpret.cc:5146`); in release builds a negative count
(reported n=-215, from parser `direct_*` applies that throw) walks garbage and
segfaults.
**Fix (medium):** clamp/guard the negative case as a safety net (pop only when
`sp > econ->save_sp`, log otherwise), then chase the parser-side stack
accounting on the error path.

### 7. #162 — fd leak on aborted compile ("mud is fubar" via fd exhaustion)
Include fds are now slurped-and-closed, but the main file's fd is still leaked
when `error()` unwinds mid-compile: `load_object()` opens at
`src/vm/internal/simulate.cc:501` and only closes at :517 after
`compile_file_fd()` returns, with no scope guard.
**Fix (small):** wrap the fd in a `DEFER` scope guard per AGENTS.md §4 (or
close it right after the slurp in `start_new_file_fd`).

### 8. #1054 — `terminal_colour` breaks UTF-8 (byte-based wrapping)
`src/packages/contrib/contrib.cc:748` uses byte length (`SHARED_STRLEN`) as
column width and both wrap passes walk bytes (`:799`), so non-ASCII text wraps
at wrong columns and can split mid-codepoint (mojibake). Fresh user reports in
2026. The building block `u8_truncate_below_width()` already exists and is used
by sprintf.
**Fix (medium-large):** keep byte lengths for memcpy, add a parallel
column-width computation via `u8_width()`, advance the wrap loops by
codepoint/EGC with separate byte and column counters; test with CJK,
box-drawing, ZWJ emoji, and ASCII pinning.

### 9. #968 — `explode(a, a)` drops fields with "reversible explode string"
When the leading-delimiter loop consumes the whole string,
`explode_string()` returns `the_null_array` unconditionally
(`src/vm/internal/base/array.cc:301`), even in reversible mode — breaking the
documented `implode(explode(s, d), d) == s` invariant. Maintainer-acknowledged.
**Fix (small):** in the `!sourcelen` early-return, when reversible, return
`num_leading + 1` empty strings; add explode/implode round-trip tests.

### 10. #992 — `remove_action()` can't remove function-expression actions
`add_action` accepts `string | function` but `remove_action` is declared
`int remove_action(string, string)` (`src/packages/core/core.spec:82`) and the
implementation explicitly skips `V_FUNCTION` sentences
(`src/packages/core/add_action.cc:588-604`) — an API asymmetry with no
workaround.
**Fix (small-medium):** widen the spec to `string | function` and match
`V_FUNCTION` sentences by comparing the `funptr_t*`; add an efun test.

### Runners-up (good next picks)
- **#808** — two concrete db bugs still present: MySQL date/time columns fetch
  as 0 (missing `FIELD_TYPE_*` cases) and `SQLite3_fetch`'s row-advance loop
  breaks after the first step (single-call `db_fetch(h, N)` returns the wrong
  row). Small-medium, but needs a live MySQL to verify.
- **#445** — the include line-number half is fixed by the compiler rewrite;
  what remains is adding the function name to two `yywarn`s at
  `compiler.cc:1339/1342`, then close. Tiny.
- **#936** — `pluralize()`: `*ff`→"bluves", `*is`→"penes". Small exception-table fix.
- **#1007** — `strwidth()` over-counts ZWJ/skin-tone emoji (per-codepoint
  wcwidth instead of per-EGC). Medium.
- **#1177** — replace remaining `whashstr` with `std::hash` (3 call sites), delete `hash.cc/.h`. Small refactor.
- **#967** — rc option to gate preload output (`vm.cc:59`). Small; #469 already closed as its duplicate.

---

## Closed as already fixed / implemented (19)

| # | Title (short) | Evidence |
|---|---|---|
| #1199 | Layout-sensitive include corruption | July 2026 front-end rewrite removed the persistent ring buffer; state reset per compile |
| #1009 | #define errors cut out code | Snippets now read from real file buffer; pinned by `Diagnostics.ExpansionErrorAttributesInvocationColumn` |
| #1112 | Ubuntu 24.04 build | CI runs six ubuntu-24.04 matrix entries, green |
| #1072 | TLS listen-socket crasher | Server-side TLS accept implemented (`socket_efuns.cc:764-789`) |
| #406 | Telnet buffer overflow | libtelnet parses ENVIRON; no fixed sprintf buffers remain |
| #844 | sprintf ANSI width | "sprintf add_justified ignore ANSI colors" rc option, default on |
| #1183 | max_eval_cost doc error | Fixed in PR #1211 |
| #343 | Safe IO | save_object writes `.tmp` + atomic rename (`object.cc:1564-1606`) |
| #377 | Coverity scans | `.github/workflows/coverity-scan.yml` weekly + master; CodeQL too |
| #290 | Port/type predefine | `sys_network_ports()` efun |
| #232 | Deep-copy mapping efun | `copy()` in contrib, pinned by `copy.lpc` |
| #209 | db tests in testsuite | `db*.lpc` + `async_db_exec.lpc` in ctest gate |
| #200 | Logging behavior | debug.log buffering, boot banner, backward-cpp traces |
| #695 | Default parameter values | `int f(int x: (: 1 :))` + `0default_args.lpc` |
| #920 | Class member reflection | `classes(ob,1)` + `fetch/store_class_member` |
| #780 | `\|\|=` and `&&=` operators | `lexer.l:651,656` + syntax tests |
| #840 | Telnet over websocket | `binary` ws subprotocol (`ws_telnet.cc`, PR #1097) |
| #1040 | libbz2-dev in docs | `docs/build.md:58,311` |
| #447 | Code formatting tool | `tools/lpc-syntax/format.mjs` + grammar contract |

## Closed as obsolete / superseded / not planned (31)

- **Superseded by newer issues:** #199 → #1164 (STL containers), #352 → #1168 (GC), #469 → #967 (preload output; exact duplicate), #108/#109/#110/#546/#353 → svalue_t-constrained STL ideas that stalled for a decade (pointers to #1164/#1168 left in comments)
- **Stale design sketches (2013-2019), rewritten out from under:** #61 (JIT), #124 (evbuffer), #279 (prog cache), #328 (ldmud hooks), #381 (embed other VM), #390 (living hash), #545 (OO umbrella)
- **Resolved-by-events / working as intended:** #737 (Yearly Plan 2022), #789 (swapping removed from driver), #787 (comma precedence — reported output was correct), #912 (casts are annotations, now documented), #341 (2019 in-thread resolution), #292 (BEGIN/COMMIT via db_exec), #80 (testsuite umbrella; ~390 efun tests + CI gate now)
- **Not planned:** #375 (phone-home telemetry), #421 (embed Node.js), #449 (efun-list efun), #741 (embedded SSH), #812 (pragmas covered by master applies), #783 (call_out funptr covers), #969 (conflicts with `.`/`->` syntax), #835 (GSL), #942 (bundle web client)

## Kept open (22 beyond the fix lists)

- **Bugs/design needing maintainer decision:** #993 (int/float coercion — contributor patch waiting on promotion-semantics decision), #1071 (CRLF is one EGC per UAX#29 — needs tailored break iterator decision), #1104 (this_player in async — two competing designs in-thread), #788 (class trailing declarator), #605 (`%*=s` edge), #660 (parser pkg UTF-8 — large rework), #832 (needs fresh repro post-rewrite), #930 (needs-info; no mechanism in current backend), #126 (bounded leak, FIXME in place), #712 (bounded by call-depth check now; cheap hardening possible), #382 (likely fixed, retest before closing), #179 (clean_up arg vs docs mismatch)
- **Live feature requests:** #909 (compile_string — security design open), #917/#918 (clean_up efuns), #900 (member_array predicate), #921 (lpcshell script mode), #922 (console), #923 (enum), #924 (UTF-8 identifiers), #777/#784 (object member access / transient objects), #1045 (env-var efuns w/ allowlist)
- **Modernization discussions:** #1200 (multithreading), #1164 (STL), #1167 (O_DESTRUCTED), #1168 (GC), #1169 (LWO), #1105 (svalue_t operators), #365 (fuzzing — newly feasible via lpcc), #380 (eval limit off-Linux), #450 (computed goto), #467 (dwlib: port or delete), #547 (call_stack dead code — needs behavior decision)
