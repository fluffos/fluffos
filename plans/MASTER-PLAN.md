# MASTER PLAN — LPC Compiler Frontend Modernization

The single index for the compiler-frontend modernization arc. It combines and
supersedes-as-an-index the four plans in this directory (each remains the
detailed record for its own phase):

| Plan document | Scope | Status |
|---|---|---|
| `flex-lexer-migration.md` | Hand-written 2,840-line scanner → Flex-generated `lex.l`, in 6 phases | **DONE** |
| `self-review-vs-master.md` | Three review rounds over the migration branch; 14 bugs found | **DONE** (all 14 fixed + tested) |
| `repl-push-parser.md` | Reentrant lexer + pure/push Bison parser + `lpcshell` REPL binary | **DONE** (v1 REPL working; deeper items deferred, listed below) |
| `unified-push-lexer.md` | Merge the standalone preprocessor into the lexer's single scan | **DONE** |
| Phase 5 (section below, no separate doc) | lex.l thinning: Flex-infra-only actions, shared scan primitives, rule consolidation | **PLANNED** |
| Phase 6 (section below, no separate doc) | `CompileSession`: one state object threaded everywhere, a real nested-compile stack, clang-quality provenance diagnostics | **PLANNED** |

End state: **one push-driven pipeline, one scan, one token at a time.**

```
compile_file(stream, name, vm_context)                     [compiler.cc]
  └─ LexTokenStream (owns reentrant Flex scanner, RAII)    [LexStream.h / lexer_utils.cc]
       └─ next() → yylex → YY_INPUT ← ring buffer ← INPUT-SOURCE STACK
                                        [top]  macro-expansion splices (add_input)
                                               #include files (incstate_t stack)
                                        [base] main file / REPL string chunk
  └─ yypush_parse loop (no yyparse exists) → grammar_rules_*.cc → AST → icode → program_t
```

---

## What Was Done (chronological, with the key decisions)

### 1. Flex lexer migration (`flex-lexer-migration.md`)
- Replaced the hand-written switch-based scanner with a Flex DFA (`lex.l` →
  checked-in `lex.autogen.cc`), integrated via the same CMake pattern as
  Bison, feeding from the legacy `linked_buf_t` ring buffer through
  `YY_INPUT`.
- Strings, char literals, template literals (`` `...${expr}...` ``),
  comments, `(`-compound tokens, `$N` params, heredoc *openers* became
  native Flex rules with exclusive start conditions; heredoc *bodies* and
  include-stack popping stayed hand-written raw-`outp` helpers
  (`parseHeredoc()`, `parseEofOrIncludePop()`) — a heredoc's terminator is
  runtime data, structurally not a static Flex pattern.
- Documented intentional micro-deviations from the original scanner (octal
  `\8`/`\9` error instead of an embedded-NUL quirk, consolidated malformed
  surrogate-pair messages, `\u`/`\U` decoding no longer length-dependent).

### 2. Self-review vs master (`self-review-vs-master.md`)
- Three review rounds over the whole branch; 14 real bugs found, fixed, and
  pinned with tests — notably template-literal brace tracking with `({`,
  preprocessor/template interaction, CRLF handling everywhere
  (continuations, directives, string rules), template `MAX_TEMPLATE_NESTING`
  OOB (UBSan-confirmed), and scratchpad-lifetime bugs for template
  fragments (`scratch_large_alloc` vs `scratch_copy`).

### 3. Reentrancy + push parser + REPL (`repl-push-parser.md`)
- `lex.l` is `%option reentrant`; all lexer state lives in
  `compiler_context_t` reached via `yyget_extra(yyscanner)`. `grammar.y` is
  `%define api.pure full` + **`api.push-pull push`** — `yyparse()` no longer
  exists; `compile_file()` drives a hand-written
  `next()`/`yypush_parse()` loop with the `yypstate` under RAII.
- `LexTokenStream` consolidates scanner ownership + source loading +
  token pull behind `load(stream, session)` / `next(&yylval)`; reusable
  across `load()`s (a REPL keeps one for a whole session).
- `compile_file()` saves/restores the compiler's transient globals around
  each compile (groundwork for nested compiles) but the legacy reentrancy
  guard (`error("Object cannot be loaded during compilation.")`) is
  **deliberately kept** — the save/restore set is not yet complete (see
  Remaining Work).
- **`lpcshell`** (`src/main_lpcshell.cc`): a working interactive REPL binary
  (python-shell-like `>>>`/`...` prompts). v1 uses the **restart pattern**:
  each statement compiles as a fresh in-memory object via
  `load_object_from_source()` (simulate.cc), reusing `compile_file()`
  unchanged; variable *values* persist between statements by redeclaring
  known names as `mixed` globals and round-tripping values through
  `save_variable()`/`restore_variable()` in generated LPC text.
- Self-review of this phase caught and fixed: `YY_PENDING_LOOKAHEAD()`
  computed against the wrong buffer pointer (over-rewound `outp`), a
  2-vs-3-arg `extern "C"` signature drift on `lpc_lex_yy_input()` (UB
  masked by C linkage), and a parser-recovery quirk where a semantic error
  in one function garbles the next function in the same compile unit
  (worked around in lpcshell by ordering generated functions; see Remaining
  Work).

### 4. Unified push-driven lexer (`unified-push-lexer.md`) — the capstone
- **The standalone preprocessor is gone** (`preprocessor.cc`/`.h`,
  `PreprocessingLexStream`, the `#pragma`/`#line` sentinel markers all
  deleted). Preprocessing is rule actions inside the lexer's one scan;
  every byte is read once; each directive's effect applies at its exact
  position (mid-file `#pragma` toggles and `#line` are correct by
  construction, single `current_line` counter).
- **Preprocessing logic lives in `lexer_rules_pp.{h,cc}`** (per direction):
  macro table (`LpcMacroTable`, plain map), `#define`/`#undef`,
  `IfExprParser`, the `CondState` conditional stack inside **`LexerSession`**
  (the `shared_ptr` session that keeps `#define`s alive across REPL
  chunks), textual expansion with self-reference guards,
  `__LINE__`/`__FILE__`/`__DIR__` from the live scan position, the directive
  dispatcher, and the dead-branch classifier.
- **`#include` is stack-based again**: `lpc_lex_handle_include()` resurrects
  the legacy `incstate_t` push byte-for-byte (including the
  `save_file_info`/`current_line_base` arithmetic the untouched legacy pop
  assumes) and ends with the `refill_buffer()` that splices the included
  file in; `start_new_file()` delegates `__GLOBAL_INCLUDE_FILE__` to the
  same push, exactly like legacy. No recursion, no eager expansion — first
  token available after reading one line.
- **Macro expansion at identifier-resolution time**
  (`lpc_lex_resolve_identifier()`): expand → `add_input()` splice → rescan,
  with a `\x1e<name>` sentinel popping the active-expansion guard. Strings/
  templates/heredocs never reach identifier resolution, so "no expansion
  inside literals" needs no parallel quoting implementations.
- **`SC_COND_SKIP`**: false `#if` branches are consumed without tokenizing
  (dead code may be invalid on purpose), nesting tracked by a C++
  classifier on whole captured lines (no per-keyword Flex pattern
  fragility).
- **`vm_context_t`** (per direction): `compile_file()` takes a `vm_context`;
  null (unit tests, no VM booted) gates off every VM interaction on the
  compile path — `smart_log()` master applies, `yyerror()` mudlib-stats,
  `init_include_path()` master query — fixing the null-`sp` UB class.
- **Top-level `##` pasting rejected as invalid** (per direction): `##` is
  macro-body-only (`substitute()`); the ancient `X1##X2` top-level line in
  `testsuite/single/tests/compiler/succeed.c` was rewritten to proper
  macro-body pasting.
- Bugs the test battery caught during the cutover, all fixed:
  `__FILE__`/`__DIR__` lost their leading `/` (found by the LPC testsuite's
  `function_exists.c`; fixed with one shared `lpc_lex_builtin_macro()`),
  `#line` off-by-one after the newline-consumption reordering, include-push
  ordering vs Flex prefetch (the load-bearing rewind/consume/dispatch/flush
  order now documented at the lex.l rule), and "Missing #endif" leaving a
  reused session permanently skipping (conds stack now cleared on that
  error).
- Tests migrated (Stage 3): `test_compiler.cc`'s `pp()` harness drives the
  real lexer end-to-end and reconstructs comparable text from tokens — the
  ~78 `Preprocessor.*` tests kept their bodies but now assert through
  actual tokenization.

**Verification standard held throughout**: full unit-test suite (246 tests:
compiler/lexer/LPC/ofile) + full LPC mudlib testsuite (`driver-autotest`)
green at every committed stage; `lpcshell` re-verified interactively after
pipeline changes.

---

## Phase 5 (IN PROGRESS) — lex.l thinning, scanning consolidation & CompileSession

> **Scope note**: per direction, the `CompileSession` work (originally
> drafted as a separate Phase 6 below) is executed AS PART OF Phase 5.
> Execution order: 5.1–5.6 (scanning consolidation, fewer call sites for
> the state moves), then 6.1→6.5 (session type, diagnostics, state moves,
> nested compiles). The Phase 6 section below is the detailed spec for
> those later sub-stages; this banner and the checkboxes track combined
> progress.

**Principle**: `lex.l` should contain ONLY Flex-infrastructure interaction —
patterns, start-condition transitions (`BEGIN`), pushback (`yyless`), buffer
rewind/flush, and `return token`. Every multi-line action body becomes a
one-call delegation into `lexer_rules*.cc`. In the same
pass, consolidate the redundant scanning implementations: today the same
micro-scans (identifier, quoted string, comment, whitespace) are hand-rolled
in five-plus places, and several rule families are near-duplicated across
start conditions. Current inventory backing this: `lex.l` is 865 lines with
~30 rules carrying non-trivial inline bodies; the escape-decoding rules exist
three times (SC_STRING_BODY ~20, SC_TEMPLATE_BODY ~13 near-identical,
SC_CHAR_BODY ~10); the quoted-string skip loop is written five times across
the pp helpers.

**Counter-principle — no dumping ground**: `lexer_rules.cc` must NOT absorb
all of this and become one oversized grab-bag. Logic moves out of lex.l into
*topically grouped* files, following the precedent `grammar_rules_*.cc`
already set for the parser side. Target layout (adjust names as the work
clarifies, but keep one topic per file, each file small enough to read
top-to-bottom):
- `lexer_scan.{h,cc}` — the shared `string_view` micro-scan primitives of
  5.1 (identifier / whitespace / quoted / comment), dependency-free so both
  rule files and the pp helpers can use them.
- `lexer_rules.cc` — stays: numeric literals, `$N` params, operators-adjacent
  helpers, token metadata.
- `lexer_rules_strings.cc` (new) — the consolidated escape table + string/
  template/char accumulator logic of 5.3 (including the relocated
  `STR_CHECK_OVERFLOW` body and the brace/template-resume helpers of 5.5).
- `lexer_rules_pp.cc` — stays: directives, macro table, expansion,
  `IfExprParser` (which 5.4 grows); if it outgrows readability after 5.2/5.4,
  split the `#if`-evaluation cluster into `lexer_rules_ifexpr.cc` rather than
  letting it sprawl.
When a helper is only ever called from one rule family, it belongs in that
family's file — proximity to its callers beats a big "utils" pile.

### 5.1 Reuse via genuine Flex mechanisms, NOT a parallel C++ scan library

**Correction, made mid-implementation**: a first pass at this stage added a
standalone `lexer_scan.{h,cc}` — plain `std::string_view` helpers
(`skip_ws`/`scan_identifier`/`skip_quoted`/`skip_comment`) that the pp
helpers would call into. **Rejected and reverted.** Per direction: no such
file should exist; every case of duplicated scanning logic must be
consolidated by actually reusing Flex, not by writing a second,
hand-rolled scanning engine in C++ that happens to sit next to it. A
parallel string-walking library is exactly the kind of second engine this
whole arc (Phase 4 especially) has been eliminating — it doesn't matter
that it's small; the duplication it "fixes" (five quote-skip loops) is
better fixed by making those five call sites genuinely share one Flex
scan, the same way the rest of the codebase does.

**What "reuse via Flex" concretely means, landed and verified (246/246
unit tests + full `driver-autotest` green) as the working pattern:**
- [x] `%option stack` added to `lex.l`.
- [x] `SC_BLOCK_COMMENT` converted from plain `BEGIN(SC_BLOCK_COMMENT)` /
  `BEGIN(INITIAL)` to `yy_push_state(SC_BLOCK_COMMENT, yyscanner)` /
  `yy_pop_state(yyscanner)` — it now resumes wherever it was pushed from,
  instead of unconditionally returning to `INITIAL`.
  Real, previously-latent gap this fixes: `SC_FUNC_OPEN` (the state after
  `(:`) had no comment rule at all, so `(: /* x */ foo)` fell through to
  the catch-all and was misread as the start of an anonymous
  function-pointer body. Now `<INITIAL,SC_FUNC_OPEN>"/*"` push into the
  shared `SC_BLOCK_COMMENT` and pop back correctly. `//` line comments
  (self-contained, single match, no stack needed) are shared the same way
  via `<INITIAL,SC_FUNC_OPEN>"//"[^\n]*`.
- [ ] Extend the same `<STATE1,STATE2,...>"/*"` / `"//"` reuse to any other
  start condition where a comment can legally appear but isn't recognized
  today (audit `SC_HEREDOC_TERM`'s pre-terminator whitespace, `SC_COND_SKIP`'s
  directive sub-rule).

**What stays exactly where it is, and why:** `strip_directive_comments()`,
`collect_args()`, `substitute()`, `lpc_lex_expand_string()`,
`directive_name()`, `#line`'s payload parse, and `IfExprParser` remain
hand-rolled C++ over `std::string_view`, UNCHANGED from the current
implementation — see 5.1a below for why, and for the one place genuine
Flex reuse (`yy_scan_string`/buffer-stack) is worth exploring for them as
a stretch goal rather than a requirement.

#### 5.1a Why the pp text-processing functions are NOT a Flex-reuse target (by default)
These operate on text that is already fully captured into a
`std::string`/`string_view` *before* any of this code runs — a macro's
stored body (substituted differently at every invocation site, with
different arguments each time), a directive line's payload (captured
whole by the one anchored `lex.l` rule), an already-assembled argument
list, an `#if` expression. This is fundamentally different from the
comment-reuse case above: a comment is *pure text to discard*, found by
running the *same* DFA rules regardless of context — an ideal
push/pop-state fit. Macro substitution is *data transformation*
parameterized by the specific call site's arguments; nothing about
switching to Flex removes the need to assemble a new string per
invocation. Precedent already in the codebase: `IfExprParser` is
hand-written recursive descent, not Flex, for exactly this reason — a
different, much simpler grammar/evaluator operating on captured text.
Genuinely-live-stream raw-`outp` readers (heredocs, the function-like
macro argument collector in `lexer_utils.cc`'s `get_next_char` lambda) are
the opposite case again — structurally can't be static Flex patterns
(heredoc terminator is runtime data) or already read live from the ring
buffer (the arg collector) — and stay hand-rolled as today.
**Stretch goal, not required for 5.1 to be considered done**: Flex's
`yy_scan_string`/`yy_scan_bytes` + `yypush_buffer_state`/
`yypop_buffer_state` (a *buffer* stack, distinct from the *state* stack
used above) let the same generated scanner sub-scan an arbitrary
in-memory string through dedicated start conditions and switch back
cleanly — this could let `collect_args()`'s and `substitute()`'s
quote/identifier recognition literally reuse `lex.l`'s existing
identifier and quote-skip rules instead of re-expressing that grammar in
C++. Real complexity to weigh before attempting it: these functions
recurse (macro expansion inside macro expansion), so sub-scans would need
to nest via the buffer stack correctly, and the output is `std::string`
assembly (the substituted text), not tokens — Flex rule actions would be
building a string via `yyextra`-reachable state rather than `return`ing,
which is a materially different usage pattern than every other rule in
`lex.l`. Worth a spike; not blocking the rest of Phase 5.

### 5.2 One directive-line pipeline (the "# directive is per-line" cleanup)
- [x] **Done, verified (251/251 unit + driver-autotest clean twice +
  lpcshell)**: ONE `<INITIAL,SC_COND_SKIP>`-tagged anchored rule serves
  both scan modes; its action is buffer choreography + one call + the
  `BEGIN` switch. Single entry point `lpc_lex_on_directive(text, len,
  yyscanner, in_skip_mode) -> {kNone, kEnterSkip, kExitSkip}` folds +
  parses the captured line EXACTLY ONCE (previously the skip-classifier
  folded/parsed the name, then the dispatcher re-folded and re-parsed the
  same line from scratch for a dead-branch-ending `#elif`/`#else`/
  `#endif`) and counts embedded continuation newlines exactly once
  regardless of mode/dispatch path. `lpc_lex_dispatch_directive`,
  `lpc_lex_count_directive_newlines`, `lpc_lex_classify_skip_directive`,
  and `LpcSkipDirective` all internalized (static `dispatch_directive(dir,
  rest)` taking pre-parsed views + inline name checks); the pp header's
  public surface shrank by three functions and an enum. Skip-mode
  directive lines now also consume their terminating newline + flush,
  making buffer handling uniform across modes (same line totals -- the
  newline was previously counted by the skip-mode `\n` rule instead).

### 5.3 String / template / char escape consolidation
- [x] **Done, verified (246/246 unit + full driver-autotest green, 3
  consecutive clean runs)**: Flex multi-state rules
  `<SC_STRING_BODY,SC_TEMPLATE_BODY>pattern` (and the `pattern1 |`/
  `pattern2 { action }` same-action-continuation form where the two
  contexts' patterns genuinely differ, e.g. the plain-text-run regexes)
  for everything the two bodies actually share: every simple escape
  (`\n \t \r \b \a \e \\`), octal `\\[0-7]+`/bad-octal `\\[89]`, hex
  `\\x...`/bad-hex, backslash-newline continuations, bare `\r`, the
  unknown-escape `"\\".` fallback, and `<<EOF>>` (one rule, message text
  branches on `YY_START`). Differing bits that could NOT merge and stay
  their own rules: `\"` vs `` ` `` (delimiters), `\\"` vs `` \\` ``/`\\$`
  (context-specific escapes), the plain-text-run character classes
  (different exclusion sets), and — found empirically via a real test
  failure (`TemplateLiteral_Crlf_StripsCarriageReturn`) and NOT
  mergeable — the two-character `\r\n` sequence: string preserves it as a
  lone `\n`, template drops it with no append at all (bare `\n` already
  differed the same way; `\r\n` just isn't a shared-action case despite
  looking like one). `\u`/`\U` remain `<SC_STRING_BODY>`-only (templates
  don't support them). Net: this pass alone deleted roughly a dozen
  near-duplicate template rules; a dedicated escape-decode *table*
  function and moving `STR_CHECK_OVERFLOW`'s body out of the macro
  (below) are still open.
- [x] **Done**: one shared simple-escape table (`lpc_lex_simple_escape()`
  in lexer_rules.cc, covering `n t r b a e " ' \ $ \``) decodes for all
  THREE literal contexts. Which escapes each context RECOGNIZES stays
  expressed as that context's rule character class (so `\`` in a plain
  string still hits the unknown-escape warning), but the mapping exists
  once: string+template's ten per-escape rules collapsed to one action
  with four `|`-continued patterns; SC_CHAR_BODY's nine single-escape
  rules collapsed to one.
- [x] **Done**: the three copies of the char-literal "Illegal character
  constant; number=0; return L_NUMBER" recovery tail are one helper
  (`lpc_lex_char_error()`); char `\r\n`/`\n` escaped-newline rules merged
  via `|`-continuation.
- [x] **Done**: `STR_CHECK_OVERFLOW`'s 28-line body moved to
  `lexer_rules.cc` as `lpc_lex_accum_overflow()` (cap check + finalize +
  the template-nesting-undo logic, all documented at the declaration);
  the lex.l macro is now the minimal call + `BEGIN`/`return` skeleton it
  structurally must remain (a macro only because it returns out of the
  invoking rule).

### 5.4 #if expression state consolidation
- [x] **RESOLVED DIFFERENTLY -- the drafted design was analyzed and
  rejected as a semantic regression; the legitimate dedup landed.** What
  landed: the twin 15-line `#if`/`#elif` evaluation blocks in the
  dispatcher are one call, `lpc_lex_eval_if_expr(expr)` (which owns
  error reporting via `lexerror()` directly), and `IfExprParser` the
  class became the style-conforming `IfExprState` struct + free
  functions. Why the "evaluator resolves macros itself in `atom()`"
  half was rejected: (1) *precedence semantics* -- the two-phase design
  (textually expand THE WHOLE expression, then parse) is what the C
  preprocessor does; with `#define X 1+1`, `#if X * 2` must evaluate
  `1+1*2 = 3`, but an evaluator expanding `X` at atom level would
  compute `(1+1)*2 = 4`. (2) *the `in_if_expr` flag is load-bearing,
  pinned by tests* -- `defined()`/`efun_defined()` are `#if`-only
  operators (`DefinedIsPlainIdentifierOutsideIf`, a bug-fix test from
  the self-review rounds: a bare `defined` in ordinary macro expansion
  must stay a plain identifier), and their operands must be protected
  from expansion AT expansion time, including when delivered via a
  macro body -- both of which require the rewrite to live inside the
  expansion walk, exactly where it is. The flag stays, documented.

### 5.5 Identifier-path unification & raw-outp handoff hygiene
- [x] **Done, verified**: the `SC_FUNC_OPEN` identifier rule and the main
  identifier rule were two copies of the exact same pattern
  (`[a-zA-Z_][a-zA-Z0-9_]*`) with different actions; merged into one
  `<INITIAL,SC_FUNC_OPEN>` rule whose action branches on `YY_START` for
  the SC_FUNC_OPEN-specific pending-lookahead/flush dance, calling
  `lpc_lex_resolve_identifier()` exactly once either way. (A separate
  `IDENT` Flex *abbreviation* wasn't needed once the pattern itself is
  literally shared via the state list — abbreviations and multi-state
  tagging solve the same duplication two different ways; the tagging
  approach turned out sufficient here.)
- [x] **Attempted and reverted -- documented as a real architectural
  finding, not a dead end to retry casually**: eliminating the
  `\x1e[a-zA-Z_][a-zA-Z0-9_]*` sentinel rule by pushing a macro's
  expansion as a genuine Flex buffer (`yy_scan_string()` +
  `yypush_buffer_state()`, popped on that buffer's own `<<EOF>>`) instead
  of splicing it into the ring buffer. Two problems surfaced, in order:
  1. **A real Flex footgun, fixed along the way**: `yy_scan_string()`/
     `yy_scan_bytes()` unconditionally call `yy_switch_to_buffer()`
     internally, which REPLACES the current top-of-stack slot rather than
     leaving it alone -- composing it with `yypush_buffer_state()`
     therefore double-writes the same buffer pointer into two stack
     slots. Popping one frees the buffer out from under the other; ASan
     caught the resulting heap-use-after-free in `yy_load_buffer_state()`
     on the very first macro-expansion test (single push, single pop,
     immediate crash). Fixed by doing the "preserve current, advance
     stack top" half of `yypush_buffer_state()` manually first, so
     `yy_scan_string()`'s internal switch lands in an already-advanced,
     fresh slot instead of clobbering the live one.
  2. **A genuine, unfixable-at-this-scope incompatibility, which is why
     the whole approach was reverted**: `lpc_lex_expand_string()` can
     legitimately leave a bare, unexpanded reference to a *function-like*
     macro as literal text in its output (`#define A B` where `B` is
     itself `#define B(x) ...` -- `B` alone, with no `(...)` in the same
     span, can't be expanded at that point, so it's emitted as literal
     text: see the `result += id;` fallback). When Flex later re-matches
     that literal `B` as an identifier and resolves it as function-like,
     `lpc_lex_resolve_identifier()`'s function-like path reads its
     `(args)` via raw reads directly from `outp`/the ring buffer (the
     same category of reader as heredocs -- see 5.1's scope-boundary
     note). That's only correct when `outp` genuinely reflects "what Flex
     will scan next", which is true for the ring buffer but false the
     moment a separate `yy_scan_string()` buffer is what's actually
     feeding Flex: `outp` stays frozen wherever the original file
     happened to be, so the collector silently reads and consumes the
     WRONG file's bytes as the macro's arguments. This didn't show up in
     the unit suite (no test exercises that exact pattern) but broke
     `driver-autotest` non-deterministically -- a different real testsuite
     file failed with a bogus "syntax error" on each of several runs
     (`find_living.c`, then `add_action.c`+`catch.c` on a re-run), with no
     ASan report (pure logic corruption, not memory-unsafety, so nothing
     to catch) -- exactly consistent with "some unrelated later token
     sequence got silently consumed as fake macro arguments." Reverting
     to the ring-buffer splice + sentinel and re-running `driver-autotest`
     three times clean confirmed the diagnosis and the fix.
  The buffer-stack conclusion stands: that approach would require
  converting the function-like-macro argument collector to be
  buffer-stack-aware -- a materially bigger redesign.
- [x] **Sentinel ELIMINATED for real (third approach, landed + verified)**:
  per-OCCURRENCE blue paint. Two directive-shaped alternatives were
  analyzed first and rejected on concrete mechanics: a spliced
  `#<name>`-style marker matched by its own Flex rule collides with the
  anchored directive rule, because `YY_FLUSH_BUFFER` (mandatory before
  every `add_input()` splice) sets Flex's beginning-of-line flag -- a
  marker at the start of a splice (e.g. any self-referential expansion
  whose body BEGINS with the guarded name, or any empty-body expansion)
  would be captured as a whole "#..." directive LINE, swallowing real
  source text after it. A directive-line marker (`#pop_expansion name\n`)
  additionally injects newlines mid-expression and breaks line counting.
  The real insight: the sentinel's information ("this literal occurrence
  is a painted self-reference") is already known, per occurrence, inside
  `lpc_lex_expand_string()` at the moment it emits a guarded identifier
  literally. So: `compiler_context_t::pending_plain` (name -> count);
  `lpc_lex_expand_string()` gains a `guard_hits` out-param that counts
  exactly the guarded emissions reaching the final splice text (argument
  pre-expansion passes null internally -- its output is re-walked after
  substitute(), and only the re-walk's emissions reach the result, so
  counting both would double-count); `lpc_lex_resolve_identifier()`
  consumes one count per rescanned occurrence and resolves it as a plain
  identifier. No marker byte, no dedicated rule, no positional
  bookkeeping, no active-expansion stack -- and semantics moved CLOSER to
  the C preprocessor (paint sticks to the occurrence, not to a scan
  region). One deliberate, pinned behavior change fell out: an alias to a
  function-like macro (`#define APPLY F`) followed by `(args)` in the
  real stream now expands (C behavior) -- previously the in-band sentinel
  byte sat between the spliced `F` and the stream's `(`, silently
  blocking the arg collector. New tests pin the count bookkeeping
  (`BluePaintDoesNotStickAcrossUses`,
  `BluePaintCountsMultipleOccurrencesInOneBody`,
  `BluePaintMutualRecursionTerminates`,
  `BluePaintFunctionLikeSelfRefIgnoresFollowingParens`,
  `AliasToFunctionLikeExpandsWithStreamArgs`). Verified: 251/251 unit
  tests, `driver-autotest` clean 3 consecutive runs (same bar the
  buffer-stack attempt failed), lpcshell parity on the pre-change
  verification input. Known corner accepted + documented: a `#if 0` block
  inside spliced macro-argument text (SC_COND_SKIP consumes without
  resolving identifiers) could strand counts -- the positional sentinel
  had the exact same blind spot (its rule was INITIAL-only), and
  `lpc_lex_reset()` clears stale counts per compile.
- [ ] Standardize every raw-`outp` handoff on `lpc_lex_flush_lookahead()`:
  the SC_FUNC_OPEN identifier rule, both heredoc-start rules, and the
  directive rule still open-code `outp -= YY_PENDING_LOOKAHEAD()` +
  `YY_FLUSH_BUFFER` in various orderings (note: with `%option array`,
  `yytext` survives the flush, so flush-then-dispatch is safe and the
  directive rule's ordering comment can simplify). The duplicated
  empty-terminator check + `parseHeredoc()` call shared by the two
  heredoc-start rules folds into one helper.
- [ ] Template brace-depth tracking if-blocks in `{`, `}`, and `({` →
  two tiny ctx helpers (`lpc_lex_brace_open/close`), leaving the `}` rule
  with only its `BEGIN`/return skeleton.

### 5.6 Newline/line-count audit
- [x] **Done**: every `\n`-accounting site now routes through
  `lpc_lex_newline()` (both counters + refill check -- matching the legacy
  scanner, which counted `total_lines` in comments, string bodies, and
  paren-whitespace runs too; several modern sites had drifted to
  `current_line`-only, undercounting the compiled-lines/s stat) or through
  `lpc_lex_count_newlines()` (renamed from `lpc_lex_count_inner_newlines`,
  now full-range and counting both) for multi-char matched spans
  (compound-paren whitespace, SC_FUNC_OPEN whitespace -- whose inline
  per-char loop is gone). ONE deliberate, documented exception remains:
  the heredoc-terminator rule keeps a bare `current_line++`, because
  `lpc_lex_newline()`'s refill check may memmove the ring buffer and that
  rule's `outp -= YY_PENDING_LOOKAHEAD()` rewind reaches BEHIND the
  relocation point -- refill-before-rewind would land the rewind on stale
  bytes (comment at the rule). `lpc_lex_newline()`'s refill check was
  KEPT: it preserves the ring buffer's line-boundary headroom invariant
  that `add_input()` splicing relies on, independent of `YY_INPUT`'s own
  per-byte refills.
- [ ] Fold the test-metadata fields (`is_char_literal`, `is_template`)
  behind the helpers that produce those tokens instead of raw ctx pokes in
  lex.l actions. (Cosmetic; low priority.)

### 5.7 Acceptance criteria / verification — status at Phase 5 lex-work close
- [x] Action-body slimming: substantially met. Every loop is out of lex.l
  (SC_FUNC_OPEN's per-char count was the last, now
  `lpc_lex_count_newlines()`); the remaining multi-line actions are
  exactly the irreducible categories the criterion allowed for --
  raw-`outp` buffer choreography (directive rule, heredoc starts,
  SC_FUNC_OPEN identifier), start-condition transitions, and the
  template-resume branch in the `}` rule (BEGIN + brace-depth, which are
  both lex.l-only constructs).
- [x] One implementation each of: simple-escape decode
  (`lpc_lex_simple_escape`), directive-line parse
  (`lpc_lex_on_directive`), `#if` evaluation (`lpc_lex_eval_if_expr`),
  char-literal error recovery (`lpc_lex_char_error`), string-overflow
  recovery (`lpc_lex_accum_overflow`), newline counting
  (`lpc_lex_newline`/`lpc_lex_count_newlines`, one documented exception).
  DELIBERATELY NOT unified: the small hand-rolled ident/quote scans
  inside lexer_rules_pp.cc's text-processing functions -- the shared C++
  micro-scan library that would have deduplicated them (`lexer_scan.h`)
  was rejected per direction (5.1's correction) as a second scanning
  engine; the remaining duplication is contained within that one file.
- [x] File sizes: `lexer_rules.cc` 347, `lex.l` 822 (was 865 pre-Phase-5
  with FEWER features handled natively), `lexer_rules_pp.cc` 876 (over
  the ~600 guide; its natural split -- `lexer_rules_ifexpr.cc` for the
  evaluator cluster, per the counter-principle -- remains available if it
  grows further, but a split for its own sake wasn't done).
  `lexer_rules_strings.cc` and `lexer_scan.{h,cc}` were NOT created: the
  escape consolidation shrank the string/escape logic enough to stay put,
  and the scan library was rejected.
- [x] Verification held throughout: full unit suite (now 251 tests, 5 new
  blue-paint pins) + `driver-autotest` green after each landed sub-stage
  (multiple consecutive runs where the change touched macro splicing);
  `lpcshell` re-verified; `lex.autogen.cc` regenerated and committed with
  each lex.l change.

---

## Phase 6 (planned) — `CompileSession`: one state object, a real compile stack, clang-quality diagnostics

**Goal**: a single `CompileSession` object constructed by `compile_file()`
and threaded through compiler, lexer, and parser — replacing the scattered
globals those three currently share — and organized as a **stack**: every
compile triggered *by* the current compile (an `inherit` of an unloaded
parent, a future nested `#include`-of-object, a REPL statement compiling
support objects) pushes a child session that remembers its parent. With the
full chain in hand, diagnostics can finally report provenance the way clang
does:

```
/std/room.c:14: error: undefined identifier 'set_lite'
  in expansion of macro 'SETUP' (/include/setup.h:9)
  in file included from /std/room.c:3
  while compiling '/std/room' inherited by '/obj/tavern.c:1'
```

**Why the current shape can't do this** (how the three pieces work together
today):
- The lexer/parser/compiler communicate through file-scope globals in three
  files: position state (`current_file`/`current_line`/`current_line_base`/
  `current_line_saved`/`total_lines`/`pragmas`), the ring buffer + include
  stack (`cur_lbuf`/`outp`/`inctop`/`current_stream`), the pp session
  (`current_session`), the codegen state (`mem_block[]`, `comp_trees`,
  string tables, locals/type tables, function contexts), plus
  `compiler_vm_context`. Only `compiler_context_t` (per-scanner) and
  `LexerSession` (macro/cond state) are already objects.
- `compile_file()` fakes one level of nesting with an ~88-line manual
  `saved_*` save/restore block — and still keeps the hard guard
  (`error("Object cannot be loaded during compilation.")`) because the set
  is incomplete (`current_stream`, `inctop`, ring buffer, function-context
  stack, `inherit_file`, `current_session` are not covered).
- **The inherit dance destroys provenance**: `inherit` of an unloaded
  parent ABORTS the child compile (`inherit_file` global), `load_object()`
  compiles the parent as a brand-new top-level compile that has no idea a
  child triggered it, then the child recompiles from scratch (verified in
  `simulate.cc` load_object). Any error in the parent reports bare, with no
  "while compiling inherited by ..." context — and the child pays a full
  double compile.
- Errors are formatted eagerly as text (`yyerror` → `smart_log` prints
  immediately); there is no structured record carrying the include stack /
  macro-expansion stack / triggering-compile chain at the moment of the
  error, so no later stage can render a provenance chain.

### 6.1 Introduce the type + the stack (no behavior change)
- [ ] `struct CompileSession` in `compiler.h`: filename/object identity,
  `vm_context_t*` (subsumes the Phase-4 global), the
  `shared_ptr<LexerSession>`, a `CompileSession *parent` link, and — first
  iteration only — *references* to the existing globals rather than moved
  state, so the type can land without touching behavior.
- [ ] `compile_file()` constructs it, pushes/pops a
  `CompileSession *current_compile` top-of-stack global (RAII pusher, so
  longjmp/error unwinds via the existing DEFER pattern still pop).
- [ ] Depth guard: `MAX_COMPILE_DEPTH` on the stack replaces nothing yet
  (the old guard stays until 6.4) but reports the chain on overflow.

### 6.2 Structured diagnostics sink (the user-visible win, before any state moves)
- [ ] `Diagnostic {severity, file, line, message, notes[]}` collected into
  the session instead of eagerly printed; `yyerror()`/`yywarn()`/
  `lexerror()` become thin wrappers that capture, at the moment of error:
  the include stack snapshot (walk `inctop`), the active macro expansions
  (`compiler_context_t::active_expansions`, plus expansion-site lines), and
  the parent-session chain ("while compiling X, triggered by Y").
- [ ] One renderer produces today's exact `smart_log` text by default
  (testsuite compatibility), with the clang-style multi-line provenance
  form behind it; `lpcshell` and `test_compiler.cc` switch to reading
  structured diagnostics instead of scraping text / counting
  `num_parse_error` (which becomes `session->diags.error_count()`).
- [ ] The `#include` push records the *including* line in each `incstate_t`
  (it already saves file+line — surface it), and macro expansion records
  the expansion-site line alongside each `active_expansions` entry, so both
  chains render with locations.

### 6.3 Move the lexer's shared state into the session (kills two globals clusters)
- [ ] Position cluster: `current_file`/`current_file_id`/`current_line`/
  `current_line_base`/`current_line_saved`/`total_lines`/`pragmas` become
  session fields (accessor macros can keep the diff small in step one).
- [ ] Input cluster: `current_stream`, the ring buffer
  (`cur_lbuf`/`head_lbuf`/`outp`/`last_nl`), and the include stack
  (`inctop`/`incnum`) move behind a `LexerInput` member — this IS
  remaining-work item "per-instance ring buffer", done as a move-into-
  session rather than a standalone refactor. `LexTokenStream` becomes a
  session member (the scanner + `compiler_context_t` already travel with
  it).

### 6.4 Move the compiler/codegen globals; retire the save/restore block
- [ ] The ~88-line `saved_*` block's variables (`mem_block[]`,
  `comp_trees`, string_idx/string_tags/freed_string, locals/type tables,
  function contexts, `var_defined`, etc.) become session members;
  `compile_file()`'s save/restore block is DELETED — nesting is structural
  now (each nested compile is simply a new session on the stack).
- [ ] Lift the reentrancy guard (remaining-work item 2) — replaced by the
  depth guard from 6.1. `main_lpcshell.cc`, `simulate.cc`, and the test
  harnesses construct/receive sessions instead of relying on the globals
  being idle.

### 6.5 True nested compiles: replace the inherit abort-and-restart dance
- [ ] `inherit` of an unloaded object compiles the parent as a CHILD
  session mid-compile (push, compile, pop, continue the outer compile) —
  eliminating the discard-and-recompile double work in `load_object()` and
  giving the parent compile its "inherited by ..." provenance for 6.2.
  The `inherit_file` global and `simulate.cc`'s iterative reload loop go
  away. (Guarded by feature parity: identical testsuite results, including
  the deliberately-cyclic-inherit error cases.)
- [ ] Audit remaining mid-compile VM callouts (`valid_object`, master
  applies via `compiler_vm_context`) for re-entry into `compile_file()` —
  with the stack in place these become legal nested sessions instead of
  guard trips.

### 6.6 Verification & docs
- [ ] Same gate as every phase: full unit suite + `driver-autotest` green
  per sub-stage; `lpcshell` interactive check; ASan/UBSan build clean.
- [ ] New tests: diagnostic-provenance goldens (error inside include ×
  macro × inherited-parent combinations, nested two deep); nested-compile
  reentrancy test compiling an object whose inherit triggers a fresh
  compile while the outer one is mid-parse.
- [ ] Update `src/compiler/internal/README.md` module map + this plan's
  architecture diagram (the `compile_file` line becomes
  `CompileSession::run()`), and fold the superseded remaining-work items
  (2, 3) into this phase's record.

**Ordering note**: 6.1 + 6.2 are independent of Phase 5 and deliver the
diagnostics improvement early; 6.3–6.5 are large state moves best done
AFTER Phase 5's consolidation (fewer call sites to rewire once the
scanning logic is already grouped into few files).

---

## What Remains To Be Done

Ordered roughly by value. None of these block current functionality; all
current tests are green.

0. **Phase 5 above** — lex.l thinning & scanning-logic consolidation (the
   next planned work; fully itemized in its own section) — then **Phase 6**,
   the `CompileSession` stack + clang-quality diagnostics (also above).
   Phase 6 subsumes items 2 and 3 below.
1. **Real REPL symbol persistence (the "non-restart" REPL).** lpcshell's v1
   restart pattern re-compiles a synthetic object per statement and fakes
   variable persistence textually. The real design: keep one compile
   session open across statements — persistent symbol table/locals, feeding
   each statement's tokens into a fresh `yypush_parse` while retaining
   compiler state. Touches symbol.cc / grammar_rules_decls.cc lifetime
   assumptions; needs its own design pass. (From `repl-push-parser.md`
   Phase 3, explicitly deferred.)
2. **Lift the compile reentrancy guard** → now Phase 6.4. The save/restore
   block in `compile_file()` doesn't yet cover `current_stream`, the
   include stack (`inctop`/`incnum`), `cur_lbuf`/`head_lbuf`, the
   function-context stack, `inherit_file`, or `current_session` — itemized
   in `repl-push-parser.md`. Phase 6 replaces the block with session
   members instead of completing it.
3. **Per-instance ring buffer** → now Phase 6.3. The ring buffer + include
   stack are still file-scope statics in `lexer_utils.cc`, so two
   `LexTokenStream`s alive at once would corrupt each other. Fine today
   (the guard serializes compiles); becomes the `LexerInput` session member.
4. **Parser error-recovery quirk**: a semantic error mid-function can
   garble the *next* function in the same compile unit (bogus
   "unexpected '}'"). Worked around in lpcshell by generation order; never
   root-caused in `grammar_rules*.cc` — likely pre-existing, worth a
   focused investigation.
5. **Per-include `#if` balance checking (minor).** An unbalanced `#if`
   opened inside an `#include`d file now leaks to the parent and is
   reported at top-level EOF — legacy-scanner parity, but the old
   standalone preprocessor caught it per-file. A conds-depth snapshot in
   `incstate_t`, compared at pop, would restore per-file reporting.
6. **lpcshell v1 rough edges** (listed in `repl-push-parser.md` Phase 4):
   failed trial-expression compiles still print their syntax error before
   the statement-form retry; expression results auto-print even for
   statement-intent input; only single-variable declarations are
   recognized (`int x, y;` is not).
7. **REPL cross-chunk `#if` (optional).** `LexerSession` could allow an
   `#if` spanning REPL statements (the cond stack lives in the session),
   currently disallowed to match old behavior — revisit only if wanted.
8. **`add_input` splice cap.** Single macro-expansion splices are capped at
   ~64KB (`DEFMAX-10`, legacy semantics). Revisit only if a real mudlib
   trips it ("Macro expansion buffer overflow").
