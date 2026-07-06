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
- [x] Audit done, conclusion: NO further extension. `SC_COND_SKIP`
  already consumes dead-branch text wholesale (comments included);
  recognizing comments in `SC_HEREDOC_TERM`'s pre-terminator whitespace
  would CHANGE the language (the legacy scanner treats `/*` there as
  body text), violating the behavior-preservation rule.

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
- [x] ~~Standardize every raw-`outp` handoff on
  `lpc_lex_flush_lookahead()`~~ — OBSOLETE, resolved more completely by
  7.3: there are no raw-outp handoffs (or `outp`, or
  `lpc_lex_flush_lookahead()`) left to standardize.
- [x] Template brace-depth tracking if-blocks in `{`, `}`, and `({` →
  two ctx helpers (`lpc_lex_brace_open/close`, lexer_rules.cc), leaving
  the `}` rule with only its `BEGIN`/return skeleton. (Landed with the
  8.1/8.2 lex.l-purity pass.)

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
- [x] Test-metadata fields (`is_char_literal`, `is_template`): CLOSED as
  won't-do — `is_template` is already set inside the lexer_rules.cc
  helpers, and `is_char_literal` is a single ctx assignment at the one
  token-production point; a helper would add indirection, not clarity.

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

## Phase 6 (rescoped) — `CompileSession` identity + clang-quality diagnostics; reentrancy is a NON-goal

> **SCOPE CORRECTION (per direction, after researching `load_object()`)**:
> the compiler does NOT need to be reentrant, and the inherit
> abort-and-restart mechanism is the accepted design, not a defect. The
> original Phase 6 draft below framed nested compiles / lifting the
> reentrancy guard / per-instance ring buffers as goals — all of that is
> **dropped**. What Phase 6 is actually for: compile identity + structured
> diagnostics with provenance (6.1/6.2, LANDED), with the remaining items
> reduced to optional diagnostics polish.
>
> **How the auto-load inherit chain actually works** (researched in
> `simulate.cc` / `grammar_rules.cc` / `compiler.cc`):
> 1. `rule_inheritence()` (grammar_rules.cc): `inherit "foo"` where foo
>    isn't loaded just sets the `inherit_file` global and lets the compile
>    run to a CLEAN abandon — `epilog()` sees the flag, suppresses
>    warnings ("they can be wrong, since we didn't parse the entire
>    file"), discards the partial program, returns null.
> 2. `load_object()` (simulate.cc): sees `inherit_file`, frees the partial
>    prog, RECURSIVELY `load_object(parent)` — recursion is at the
>    object-loading level and bounded by `__INHERIT_CHAIN_SIZE__` via
>    `num_objects_this_thread` — then reloads the child from scratch,
>    with an explicit re-check ("Beek's guard") for the case where loading
>    the parent already loaded the child via the parent's `create()`.
> 3. **`compile_file()` is therefore never re-entered**: every compile
>    fully completes (success or clean abandon) before the next begins;
>    nesting exists only in `load_object()`'s C-stack recursion AROUND
>    completed compiles. The `guard`/`current_file` check exists solely to
>    reject pathological LPC re-entry DURING a live compile (a master
>    apply like valid_override, or an error handler, calling
>    load_object mid-compile) — and rejecting is correct.
> 4. **Why abort-and-restart beats nested compiles** (the decisive
>    argument, visible in Beek's guard): `load_object(parent)` runs
>    arbitrary LPC — the parent's `create()` — which can itself load more
>    objects, move objects, call anything. Under a nested-compile design
>    all of that LPC would execute MID-COMPILE of the child, which is
>    exactly the reentrancy nightmare the guard exists to prevent. The
>    restart design guarantees LPC only ever runs BETWEEN compiles, at the
>    cost of recompiling the child once per first-load of a missing
>    parent (bounded, first-load-only, cheap relative to the safety).
>
> Consequences for the sub-stages below: 6.1 and 6.2 stand as landed
> (identity, diagnostics, include-stack provenance — none of it depends on
> nesting). 6.3/6.4/6.5 as originally drafted are RETIRED; see their
> updated sections. The old "What Remains" items about lifting the guard
> and per-instance ring buffers are likewise dropped.

**Landed goal**: a `CompileSession` per compile carrying identity +
diagnostics anchoring, and structured `Diagnostic` records with provenance
captured at report time:

```
/std/room.c:14: error: undefined identifier 'set_lite'
  in file included from /std/room.c:3
```

(The "while compiling X, triggered by Y" chain note exists in the capture
code but is empty in practice — compile_file never nests. If cross-load
provenance — "loading '/std/room' because '/obj/tavern' inherits it" — is
ever wanted, the right home is a small name chain in `load_object()`'s
recursion, NOT compiler nesting; listed under optional follow-ups.)

### 6.1 Introduce the type + the stack (no behavior change)
- [x] **Done, verified (251/251 unit + driver-autotest + lpcshell)**:
  `struct CompileSession` in `compiler.h` (filename, `vm_context_t*`,
  `parent` link, `depth`, and a `chain()` provenance formatter —
  "triggered by A, triggered by B"), with the full migration plan
  documented at the type. `compile_file()` constructs one and
  pushes/pops the `CompileSession *current_compile` top-of-stack global;
  the pop lives in the existing DEFER scope guard, so error()-thrown
  unwinds pop correctly alongside the saved_* restores.
  `MAX_COMPILE_DEPTH` (32) is checked BEFORE the legacy guard flag is
  set (so an over-deep reject can't wedge the guard) and reports the
  chain; with the legacy nesting guard still in place only depth 0
  occurs today. The LexerSession handle deliberately NOT added to the
  struct yet — compile_file-level callers don't supply one (each compile
  makes a fresh session inside start_new_file()); it joins the struct in
  6.3 when the lexer state moves.

### 6.2 Structured diagnostics sink (the user-visible win, before any state moves)
- [x] **Core landed, verified (255/255 unit incl. 4 new Diagnostics tests +
  driver-autotest clean twice + lpcshell)**: `Diagnostic {is_warning,
  file, line, message, notes[]}` captured by `yyerror()`/`yywarn()` (and
  therefore `lexerror()`) into the `compiler_diags` global at the moment
  of the report — provenance notes hold the live `#include` stack (new
  `lpc_lex_include_stack()` accessor walking `inctop`, naming each
  includer and its `#include` line) and the compile-session chain
  (`current_compile->chain()`, empty until nested compiles exist).
  Cleared per compile/REPL chunk in `start_new_file()`. A global rather
  than a session member so unit harnesses driving the lexer without
  `compile_file()` still capture. Default `smart_log` text output is
  unchanged; `render_diagnostic()` produces the clang-style multi-line
  form. Macro-expansion provenance notes are NOT possible anymore by
  design — the blue-paint counters (5.5) eliminated the runtime "currently
  inside expansion of X" stack that the old sentinel design maintained;
  re-adding expansion provenance would need a dedicated (cheap) name
  stack, noted as follow-up if ever wanted.
- [x] **Real attribution bug found by the new tests and fixed**: directive
  errors (`#error`, bad `#if` expressions, unresolvable `#include`s, ...)
  attributed to the line AFTER the directive — the lex.l rule consumes and
  counts the terminating newline BEFORE dispatching (load-bearing for the
  include push), so `current_line` had already advanced when the error
  fired; the old preprocessor attributed `#error` to its own line. Fixed
  via `compiler_directive_start_line` (documented in compiler.h): set to
  the directive's own first line around the dispatch calls, used by
  `yyerror()`/`yywarn()` when nonzero, zero everywhere else. This
  deliberately corrects the smart_log TEXT output too (directive errors
  now name the right line); full LPC testsuite verified unaffected.
- [x] ~~Remaining (deferred)~~ — DONE since: lpcshell reads/renders
  `compiler_diags` (8.7), and the clang-style rendering IS the default
  driver output (8.4). The unit harnesses still also consult
  `num_parse_error`, which remains a valid signal.

### 6.3 / 6.4 / 6.5 — RETIRED (reentrancy is a non-goal)

The state-move stages (lexer position/ring-buffer/include-stack into the
session; codegen globals into the session; retiring the `saved_*` block;
lifting the reentrancy guard; inherit-as-nested-compile) existed to make
`compile_file()` nestable. Per direction — and confirmed by the
`load_object()` research in the scope-correction banner above — the
compiler has no need to be reentrant: the inherit chain is handled
entirely by `load_object()`'s bounded, recursive abort-and-reload, the
guard correctly rejects the only genuine re-entry vector (LPC running
mid-compile trying to load), and abort-and-restart is SAFER than nesting
(parent `create()` LPC would otherwise run mid-compile). All three
stages are retired, not deferred.

What survives of their content, all optional and low priority:
- The ~88-local `saved_*` save/restore block in `compile_file()` is now
  known to be pure per-compile cleanup, not nesting support (nesting
  never happens). It could be SIMPLIFIED to plain re-initialization on
  entry — clearer intent, less code — but it is correct as is; cosmetic.
- Cross-load provenance for diagnostics ("loading '/std/room' because
  '/obj/tavern' inherits it"): if ever wanted, a small filename chain
  pushed/popped in `load_object()`'s recursion (it is already recursive,
  already bounded by `__INHERIT_CHAIN_SIZE__`), feeding a note into
  `compiler_diags` capture — NOT compiler nesting. The existing
  `CompileSession.parent`/`chain()` machinery stays as the anchor a
  load-chain note could hang off, and costs nothing meanwhile.
- Two `LexTokenStream`s still cannot be alive concurrently (file-scope
  ring buffer); irrelevant while compiles are serialized by design, and
  documented in the README's known-scope-boundary note.

### 6.6 Verification & docs
- [x] Verification gate held for 6.1/6.2 (unit suite + driver-autotest +
  lpcshell per stage; the whole build runs under ASan/UBSan).
- [x] lpcshell reads/renders `compiler_diags` (landed with Phase 8.7's
  quiet-flag consumer work).
- [x] `src/compiler/internal/README.md` updated: CompileSession +
  structured diagnostics + the settled inherit-via-load_object design in
  the compile-driver module notes; stale sentinel/IfExprParser/dispatcher
  references scrubbed alongside.
- [x] Load-chain provenance note — LANDED: load_object() sets
  `compiler_next_load_reason` just before recursively loading an
  inherited file; the nested compile's start_new_file() consumes it into
  a compile-scoped slot and every diagnostic of that compile carries
  "while loading '/child' inherited by '/parent'" as a note. One-shot
  hand-off, so an error() unwind can never leave a stale reason on an
  unrelated later compile. Verified live through lpcshell (broken
  inherited file shows the note under the child's caret diagnostic).

---

## Phase 7 (planned) — the Flex-native endgame: no `outp`, native buffers, native line tracking, pp on lex interfaces

**Direction**: keep simplifying lex.l and its support code; delete
unnecessary complication; move the remaining hand-rolled machinery onto
Flex's own mechanisms; rework `lexer_rules_pp` to work through the lex
interfaces instead of its own string processing (except genuinely
temporary scratch text). **Hard constraint: no behavior changes** — every
stage lands behavior-preserving, gated on the full battery.

### 7.1 Shorthand-alias deletion + line-drift fix — DONE (483df091)
- [x] The `#define str_accum (yyextra->str_accum)` alias block (and its
  `#undef` mirror) deleted; rule actions name `yyextra->field` directly.
- [x] Validation of the newline machinery en route found a REAL bug,
  fixed + pinned by two tests: a macro argument spanning a newline
  double-counted it (once at collection, again when the spliced copy was
  rescanned) — `__LINE__` drifted +1 per such line
  (`MultiLineMacroArgKeepsLineCount`). Collected argument text now
  carries a space in place of the newline; the count stays with the
  consumption (required — an unused parameter's text never reappears:
  `DroppedMultiLineArgKeepsLineCount`). Residual accepted corner: a raw
  newline inside a QUOTED macro argument still double-counts (string
  bytes can't be altered).

### 7.2 Native flex↔bison integration & line tracking — ✅ COMPLETE (bison-bridge + native yylineno; %locations stays an explicit non-goal)
- [x] **`%option bison-bridge` — DONE**: replaces the hand-rolled
  `YY_DECL int yylex(union YYSTYPE *yylval_param, void *yyscanner)` +
  `#define yylval (*yylval_param)` pair with flex's own bison-compatible
  generation (identical signature and parameter name — which is exactly
  why the hand-rolled version had chosen `yylval_param`). Under
  bison-bridge the `yylval` macro evaluates to a POINTER, so rule actions
  now write `yylval_param->field` explicitly (28 sites), which also reads
  more honestly. Verified: 257/257 unit + driver-autotest + lpcshell.
  `%option bison-locations` (yylloc) deliberately NOT enabled — see the
  `%locations` caveat below.

**`yylineno` — LANDED (lines are native Flex state now).** Every byte
flows through Flex, so `%option yylineno` counts everything: newlines in
matched text (whitespace/string/comment/skip rules, directive-line
continuations) and newlines handed out through `yyinput()` (the raw
readers, the directive's terminating newline). What landed:
- [x] **`current_line` is a macro over `int &lpc_lex_current_line_ref()`**
  (lex.l trailer): a REFERENCE to the innermost REAL frame's native
  counter — the top-most INCLUDE buffer's `yy_bs_lineno`, or the base
  (main file) buffer's; splice buffers' counters are synthetic and
  skipped. Every existing read/write/++/-- across the compiler (grammar
  actions, `#line`, the collector's no-invocation restore, diagnostics)
  operates on native storage UNCHANGED at the call sites. When no
  scanner/buffer is live the reference falls back to
  `lpc_lex_line_fallback`, which end_new_file() loads with the final
  value (legacy last-value semantics for fatal() notes and test
  harnesses).
- [x] **Isolation across expansions and includes is automatic**: the
  per-buffer counters freeze/resume with the buffer stack, exactly as
  analyzed. `__LINE__` inside an expansion reads the invocation line for
  free (resolve-time expansion + real-frame accessor); expansion text
  still contains no newlines, so the synthetic axis never even advances.
- [x] **Manual counting stripped everywhere**: `lpc_lex_newline()` keeps
  only the total_lines statistic + the dead-frame purge; the collector,
  parseHeredoc, and the directive fold-loop lost their `current_line++`
  (kept total_lines where legacy counted it). The include push/pop
  arithmetic no longer touches `current_line` at all: the push computes
  its `save_file_info`/`current_line_base` terms from locals (the
  parent's counter freezes untouched), the pop feeds the include's final
  native count (captured just before `yypop_buffer_state`) to
  `save_file_info` and lets the parent's counter resume by itself.
- [x] **Two flex facts encoded in the helpers**: `yy_scan_buffer()`
  (behind `yy_scan_bytes`) leaves `yy_bs_lineno/column` UNINITIALIZED —
  `lpc_lex_push_string_buffer()` sets 1/0 explicitly; and flushing
  preserves a buffer's counters, so `lpc_lex_reset()` re-arms the reused
  base buffer to line 1 explicitly.
- [x] Verified: all 264 unit tests (the `__LINE__`/#line/heredoc/
  multi-line-arg pins all held), driver-autotest ×3, lpcshell smoke.
  `yycolumn` maintenance rides with Phase 8.1 (needs YY_USER_ACTION;
  the push/reset helpers already initialize the per-buffer column).
- [x] Bison `%locations`/`yylloc`: landed additively in 8.3 (plumbing +
  operand ranges); primary attribution unchanged, exactly as scoped
  here.

### 7.3 Eliminate `outp` (and with it the ring buffer) — ✅ COMPLETE (A–D all landed)
The staged core, now finished: `outp` and the ring buffer no longer
exist. Input is entirely Flex-native — the base buffer bulk-reads the
main stream via YY_INPUT; splices and #include contents are in-memory
buffers on Flex's buffer stack. The stages, as landed:
- [x] **A. Raw readers → `yyinput()`** — LANDED. All mid-rule raw reads
  now go through `lpc_lex_getc()` (a lex.l-trailer wrapper over the
  generated-static `yyinput()`; declared in lex.h): `parseHeredoc()` was
  rewritten as a getc line-reader (replacing `get_text_block`/
  `get_array_block`/`NEWCHUNK` — ~9KB of chunk machinery deleted; text
  form builds the string directly into the scratchpad, array form
  splices `"l1", "l2", })` with deliberately-NO-newline separators,
  fixing the legacy `",\n"` double line count — pinned by
  `HeredocTextFormKeepsLineCount`/`HeredocArrayFormKeepsLineCount`); the
  macro-argument collector's `get_next_char` is getc-based (its restore
  path re-splices the recorded consumed text instead of the old
  `outp = saved_outp` rewind, which could dangle across a
  `refill_buffer()` memmove — a latent corruption fixed by this stage);
  the `(:` one-byte peek likewise. Both heredoc call sites in lex.l lost
  their `outp -= YY_PENDING_LOOKAHEAD(); YY_FLUSH_BUFFER;` prelude — and
  the heredoc-terminator newline now counts via the uniform
  `lpc_lex_newline()` (it was the one documented exception while the
  rewind still followed it). Rewind/flush choreography now survives ONLY
  at the splice sites (`lpc_lex_flush_lookahead()` before `add_input()`)
  and the directive-rule/EOF-rule resume points — all of which stage B/C
  convert to buffer pushes. This removes the exact blocker that sank the
  earlier buffer-stack experiment (5.5's post-mortem: the collector read
  `outp` while a pushed buffer was feeding Flex).
- [x] **B. Splices → buffer pushes** — LANDED. Every splice is now a
  pushed in-memory Flex buffer via `lpc_lex_push_string_buffer()` (lex.l
  trailer; uses the 5.5-documented fixed composition: do
  `yypush_buffer_state`'s "flush old position, advance stack top" half
  manually FIRST so `yy_scan_bytes()`'s internal switch lands in a fresh
  slot). Converted sites: object-like + function-like macro expansions,
  builtin (`__LINE__`) splices, the collector's no-parenthesis restore,
  `old_func()`'s name pushback (now takes the name as a parameter —
  yytext can be freed by a pop mid-peek, so `lpc_lex_resolve_identifier`
  copies it once up front), the `(:` re-splice and one-byte peek
  pushback, and parseHeredoc's array splice + trailing-content pushback.
  Pops happen in the `<<EOF>>` rules — ALL of them: the six
  state-specific `<<EOF>>` rules (comment/string/char/func-open/heredoc)
  pop a drained splice and continue scanning in-state, so constructs
  spanning a splice boundary keep working — or inside `lpc_lex_getc()`
  (pop-and-continue), which is what lets a function-like macro's
  `(args)` be collected across a splice boundary (`APPLY(3)`), the exact
  5.5 killer. Expansion provenance frames switched from byte-extent
  accounting to buffer lifetime (one frame per expansion buffer, marked
  dead at pop, same linger-until-newline purge; `lpc_lex_debit/
  credit_expansion` deleted). The directive rule's outp choreography is
  now guarded by splice depth 0. Already-deleted ahead of D:
  `add_input()`, `lpc_lex_flush_lookahead()` (nothing splices into the
  ring anymore — it only carries file bytes). `lpc_lex_reset()` pops
  leftover splice buffers from aborted compiles. Verified: 264 unit
  tests under ASan/UBSan, driver-autotest ×3 clean (the 5.5 experiment's
  non-determinism gate), lpcshell smoke.
- [x] **C. Includes → buffer pushes** — LANDED (simpler than planned: no
  per-buffer stream stack needed; the whole include file is slurped and
  pushed via `lpc_lex_push_string_buffer(LPC_BUF_INCLUDE)`, with the
  ring's guaranteed-trailing-newline invariant preserved by
  auto-append). `incstate_t`/`inctop`/`incnum` became a
  `std::vector<IncState>{line,file,file_id}` metadata stack
  (`lpc_lex_include_stack()` keeps working); the pop is the buffer's
  `<<EOF>>` (or a `lpc_lex_getc()` pop-through), whose INCLUDE branch
  restores current_file/current_line and closes the
  save_file_info/current_line_base arithmetic verbatim. Kind stack:
  PLAIN/EXPANSION/INCLUDE (`lpc_lex_top_buffer_kind()`). The directive
  rule consumes its terminating newline per source: base buffer = ring
  dance (flush now BEFORE dispatch — flushing after gutted the
  freshly-pushed include buffer, ASan-caught), include buffer = one
  `yyinput()`, splice = nothing to consume. `current_stream` never
  switches anymore: the ring carries ONLY the main file, so
  refill_buffer()'s entire include-splicing branch, the TERM_* front-pop,
  and end_new_file's linked-buffer walk were deleted;
  parseEofOrIncludePop() kept only the main-EOF tail. The global include
  file is pushed by start_new_file() AFTER the main file's first ring
  fill — which surfaced a real footgun fixed in
  lpc_lex_push_string_buffer(): pushing before the first yylex() call
  must materialize Flex's lazily-created base buffer first, or the push
  lands in slot 0 AS the base and its pop leaves yy_c_buf_p dangling
  (ASan-verified UAF). Include lines are no longer subject to the ring's
  MAXLINE "Line too long" cap (buffer form has no line-length
  assumption); the main file still is until D. Verified: 264 unit tests
  under ASan/UBSan, driver-autotest ×4 clean, lpcshell smoke.
- [x] **D. Ring buffer deletion** — LANDED. `linked_buf_t`/`head_lbuf`/
  `cur_lbuf`/`refill_buffer()`/`outp`/`last_nl`/`YY_PENDING_LOOKAHEAD()`
  all deleted (`add_input()`/`lpc_lex_flush_lookahead()` already fell in
  B); CompileSession's inert save/restore dropped its outp/last_nl
  fields. YY_INPUT now BULK-reads the main stream straight into Flex's
  base buffer (the one-byte trickle existed only to bound prefetch for
  the raw-outp rewind arithmetic — all gone), preserving the ring's
  guaranteed-trailing-'\n' invariant at EOF. The directive rule's three
  newline paths collapsed to ONE: consume via `yyinput()` (which
  maintains the BOL flag) for base and include sources alike; splices
  have no newline to consume. Dead-frame purge moved from the YY_INPUT
  byte loop to `lpc_lex_newline()` (with bulk reads, buffer-fill time no
  longer approximates consumption time — the '\n' RULE is the true line
  boundary). The error-context snippet is re-sourced from Flex's current
  buffer (`lpc_lex_error_context_from_buffer` in lex.l's trailer, walking
  yy_ch_buf around yy_c_buf_p with the yy_hold_char substitution),
  reached through a compile-scoped `active_scanner` slot (fine: the
  compiler is deliberately non-reentrant). The main file is no longer
  subject to the ring's MAXLINE "Line too long" cap either. Verified:
  264 unit tests under ASan/UBSan, driver-autotest ×3 clean, lpcshell
  smoke incl. an error-path caret block.
Open questions from the staging, as resolved: the `add_input` 64KB
splice cap is moot (buffer pushes have no such cap; parseHeredoc keeps
its explicit DEFMAX block cap as the size sanity check, and the string
literal accumulator keeps its own overflow cap); the ring's
`MAXLINE`-derived "Line too long" caps are simply GONE (neither main
files nor includes have a line-length limit anymore — Flex buffers make
no line-geometry assumption); performance moved the right direction
(YY_INPUT went from one byte per call to bulk reads).

### 7.4 `lexer_rules_pp` reworked onto lex interfaces — ✅ COMPLETE
Today the pp layer re-implements scanning as private string walking
(`lpc_lex_expand_string`'s ident/quote scans, `collect_args` over text,
the `IfExprState` char-level evaluator, the `in_if_expr` rewrite mode).
Target: the SCANNER is the only thing that scans; pp keeps only genuinely
temporary text (a `#define` body awaiting use, substitute()'s
param-splice/stringize/paste assembly — inherently textual scratch).
Riding on 7.3-A/B:
- [x] **Raw-body splices, rescan-driven nesting** — LANDED. Expansion
  pushes the macro's RAW (substituted) body as a buffer; nested
  references resolve when the rescan reaches them, one buffer + one
  provenance frame per level (`NestedExpansionChainNotes` now asserts
  the full innermost-first chain the flattened textual design couldn't
  produce). The self-reference guard is the LIVE expansion frames
  themselves (`lpc_lex_name_guarded`); the per-occurrence blue paint
  (`pending_plain`, `guard_hits`) is deleted and all its tests pass
  unchanged on the buffer-lifetime guard. `MAX_EXPANSION_NESTING` (128)
  caps pathological chains. Two honest deviations from the sketch:
  (1) ARGUMENT pre-expansion stays textual — it IS C's "arguments are
  fully expanded first" step, and it's what lets
  `SECOND(1, SECOND(2, 3))`'s inner call expand while the outer's
  buffer guards the name (`substitute()` now takes raw + expanded
  vectors: # and ## operands get the RAW spelling — more C-correct than
  the old expand-everything — plain references get the expanded form).
  (2) `lpc_lex_expand_string` therefore survives, slimmed, for exactly
  three textual consumers: argument pre-expansion, #if/#elif
  expressions (until the token-stream stage below), and #include's
  unquoted-filename form. En-route fix: the 8.2 snippet backscan must
  treat NUL as a line boundary — flex's `yyinput()` NULs every byte it
  consumes ("preserve yytext"), so a directive line's terminating
  newline survives only as NUL residue and the walk otherwise splices
  the previous line into the snippet.
- [x] **`#if`/`#elif` as token streams** — LANDED. The expression text
  is pushed as a `LPC_BUF_IF_EXPR` buffer whose `<<EOF>>` returns a
  dedicated `LPC_IFEXPR_END` pseudo-token (never continues into the
  parent input -- the one pushed-buffer kind that doesn't), and
  `lpc_lex_eval_if_expr(expr, yyscanner)` pulls TOKENS through
  `lpc_lex_ifexpr_next()` (a lex.l-trailer helper that scans each token
  under INITIAL and restores the caller's start condition -- a
  dead-branch `#elif` evaluates from SC_COND_SKIP). Numbers arrive via
  the real literal decoders (hex/binary/char escapes included); macro
  references expand through the ordinary rescan machinery into a FLAT
  sequence, so `#define X 1+1` + `#if X*2` = 3 (verified end-to-end);
  `defined()`/`efun_defined()` operands are pulled with
  `ctx->suppress_expansion` set (C's unexpanded-name requirement),
  retiring the `in_if_expr` text-rewrite mode entirely. The evaluator
  kept the old walker's precedence/ternary/error structure over an
  `IfTok` vector. Semantics drift, deliberate and pinned: `#if` literals
  follow LPC's OWN number grammar now -- no octal (leading zero =
  decimal, matching code proper), no U/L suffixes (the old char walker
  used C strtol rules). Found en route: `trim()` on an rvalue
  `std::string` picks a strutils overload returning a dangling view
  (ASan stack-use-after-return) -- bind to a local first.
- [x] What stays textual (the sanctioned scratch), as settled: macro
  bodies at rest, `substitute()`'s parameter splice + `#` stringize +
  `##` paste, `stringize()`, `trim()`, directive-line fold/name/rest
  parsing, function-like ARGUMENT pre-expansion, and #include's
  unquoted-filename expansion (the last two being
  `lpc_lex_expand_string`'s only remaining callers).

### 7.5 Scratchpad — investigated, VERDICT: KEEP
The scratchpad is not an unnecessary complication; it is the
compile-lifetime arena for token STRING VALUES flowing through Bison's
value stack (`yylval.string` from string/template/identifier paths).
Two properties are load-bearing: (1) `YYSTYPE` is a C union and cannot
own a `std::string`, so values must be raw pointers with externally
managed lifetime; (2) compiles abort via `error()` unwinds from
arbitrary depths — the arena's bulk-free-at-compile-end is what makes
every such path leak-free without per-token ownership plumbing through
every grammar action and error branch. Replacing it would be high-risk
re-plumbing with zero user-visible benefit. (Its internal
free-only-if-last trick stays documented in scratchpad.cc.)

---

## lex.l purity — the standing rule and its enforcement rounds

**Rule (by direction): lex.l contains ONLY interactions with flex** —
patterns, start-condition transitions, and the trailer helpers that
exist solely because the generated scanner's types (yyguts_t, buffer
internals, static yyinput) are private to that translation unit. ALL
substantial logic lives in lexer_rules.cc / lexer_rules_pp.cc /
lexer_utils.cc.

Round 2 (after the 8.1/8.2 round moved the buffer walks out):
- [x] The seven <<EOF>> pop-or-else copies collapsed onto one trailer
  decision helper (`lpc_lex_pop_splice_if_any`) -- single definition of
  the IF_EXPR hard-edge condition.
- [x] The directive rule's newline-consumption policy moved to a trailer
  helper (`lpc_lex_consume_directive_newline`); the rule action is now
  dispatch + BEGIN transitions only.
- [x] The duplicated heredoc-start tail (terminator validation +
  parseHeredoc hand-off) folded into `lpc_lex_start_heredoc`
  (lexer_utils.cc) -- closing the old 5.x fold-into-one-helper note.
- [x] String-close validation/copy moved to `lpc_lex_string_close`
  (lexer_rules.cc), matching the template-close helper's shape.
- [x] `lpc_lex_reset` keeps only the flex half; context-field resets
  moved to `lpc_lex_reset_context` (lexer_rules.cc).
Remaining conditionals in the rules section are all flex skeletons:
pop-or-else at <<EOF>>, YY_START branches, act→BEGIN switches, and
one-line helper-retry returns.

## Phase 8 (planned) — Expressive diagnostics: clang-style caret, ranges, fix-its, macro notes

**Direction**: the lexer/parser must be able to produce clang-quality
diagnostics — precise `file:line:col`, the source line with a caret at
the exact problem point, `~~~` range highlighting of related
sub-expressions, "expanded from" macro chains, and fix-it hints:

```
/std/room.c:14:9: error: invalid operands to binary expression ('int' and 'mapping')
  total = count + rates;
          ~~~~~ ^ ~~~~~
/include/setup.h:9:22: note: expanded from macro 'SETUP'
```

**Ground rules** (REVISED per direction): (1) ~~default text stays
byte-identical~~ — diagnostic text is NOT an API; the clang-shaped format
IS the default output now. All richness flows through the 6.2
`Diagnostic`/`compiler_diags`/`render_diagnostic()` substrate. (2) Column
tracking still rides on Phase 7.3 (same raw-read bypass gating as
`yylineno`) — but expansion provenance turned out NOT to need 7.3: exact
byte-extent accounting on the existing ring-buffer splices delivers it
today (see 8.4). The flex Multiple-Input-Buffers manual
(https://westes.github.io/flex/manual/Multiple-Input-Buffers.html) is the
reference for the eventual 7.3 buffer-stack shape, which can replace the
extent accounting 1:1; it also confirms `yy_create_buffer(NULL, size)` is
legal with a custom YY_INPUT (validates 7.3-C's LexStream-fed include
buffers) and that `yy_scan_string` "also switch[es] to the new buffer" —
the auto-switch behind the 5.5 experiment's double-slot footgun.

### 8.1 Column tracking — ✅ LANDED
- [x] `YY_USER_ACTION` maintains `yycolumn` exactly as analyzed (start
  col snapshotted into `compiler_context_t::token_start_column`, then
  advance/recompute past the match); per-buffer `yy_bs_column` gives
  automatic isolation across expansion/include pushes. New wrinkle found
  at implementation: `yyless()` adjusts `yylineno` but NOT `yycolumn`,
  so every yyless goes through `LPC_YYLESS()` which rewinds the column.
- [x] Bypass sites wired: `lpc_lex_getc()` advances/resets the column
  per raw-read character; the directive rule resets it after its
  consumed terminating newline.
- [x] Tabs count one column (renderer prints tabs as one space so the
  caret aligns). `Diagnostic.column` (1-based token START, 0 = unknown).
- [x] Expansion attribution per the plan: frames stamp the invocation
  position at push; an in-expansion diagnostic carries the OUTERMOST
  invocation's column (clang's "expansion location"). Pinned by
  `ExpansionErrorAttributesInvocationColumn`.

### 8.2 Caret + snippet rendering — ✅ LANDED (capture-time snapshot design)
- [x] Design (b) chosen as predicted: `Diagnostic.snippet` captures the
  diagnosed physical line at report time from the innermost REAL Flex
  buffer — the same frame `current_line` reads, so line/column/snippet
  are mutually consistent (invocation line while a splice scans). Found
  a real subtlety: the held-out NUL at the live scan position can mask
  the line's own newline; the walk resolves the held char BEFORE
  deciding the line continues (otherwise the next line splices into the
  snippet).
- [x] `render_diagnostic()` emits `file:line:col:` + indented snippet +
  caret; whole-line directive reports (#error) carry neither
  (`DirectiveErrorsCarryNoColumn`). The PRAGMA_ERROR_CONTEXT append in
  report_compile_diagnostic() retired — the snippet block IS the default
  format (prepare_logs keeps the legacy block for apply.cc's runtime
  path only).
- [x] lex.l-purity constraint (re-asserted by direction): the buffer
  walks live in lexer_utils.cc, built on three raw introspection
  primitives in lex.l's trailer (`lpc_lex_buffer_count/lineno/extents`)
  that exist only because the generated buffer types are private to
  that translation unit.
- [x] Severity colors: `render_diagnostic(d, color)` emits ANSI
  severity/caret/fix-it coloring; lpcshell passes `isatty(1)`, the
  driver's log path stays plain.

### 8.2 Caret + snippet rendering
(The original planning boxes for this section are superseded by the
LANDED items above: capture-time snapshot chosen and shipped; color
shipped; the PRAGMA_ERROR_CONTEXT append retired in favor of the
always-on snippet — `prepare_logs` keeps the legacy block only for
apply.cc's runtime trace path.)

### 8.3 Ranges: the parser half (Bison locations) — ✅ LANDED (acceptance target achieved)
- [x] `%option bison-locations` + `%locations` are ON: yylex carries
  `YYLTYPE*`, `YY_USER_ACTION` fills the span from 8.1's counters (real
  frame line, 1-based token columns), `YYLLOC_DEFAULT` merges bottom-up,
  and the push loop passes `&yylloc` to `yypush_parse`. Two flex↔bison
  landmines found and defused: (1) YY_USER_ACTION must write through the
  frame's OWN `yylloc_param`, never the `yylloc` macro (`yyg->yylloc_r`)
  -- a nested yylex (the #if evaluator's pulls) repoints yylloc_r at a
  shorter-lived location and the suspended outer frame writes through
  the stale pointer (ASan stack-use-after-return); `lpc_lex_ifexpr_next`
  additionally restores the saved lval/lloc pointers. (2) The
  checked-in normalized `grammar.autogen.h` shadows the freshly
  generated one for lex.autogen.cc's include, and it only syncs
  POST_BUILD of driver -- a grammar-signature change needs one manual
  NORMALIZE to break the chicken-and-egg.
- [x] The acceptance target renders EXACTLY (verified live):
  `error: Invalid argument types to '+' ( int vs mapping )` with the
  operand lines below as `~~~~~ ^ ~~~~~` -- `Diagnostic::Range` spans
  from `@lhs`/`@rhs`, the caret moved onto the operator from `@2`, all
  attached ADDITIVELY by a set/clear pair around `rule_expr_add` in the
  `'+'` action (one-shot, cleared in the same action, so a range can
  never leak to an unrelated report). Primary line attribution stays
  current_line-at-report, exactly per the ground rules. Further binary
  operators can adopt the same two-call pattern site by site.
- [ ] Still open (deliberately gated, unchanged): widening `parse_node_t`
  beyond `short line` for ranges that survive PAST the reduce (icode-time
  errors) — measure node-size impact first; likely a 32-bit location-id
  into a compile-lifetime table.

### 8.4 Macro expansion notes + include chains + new default format — LANDED
- [x] **"during expansion of macro 'F' (defined at /file:line)" notes,
  live in the default output**, delivered WITHOUT waiting for 7.3 via
  exact byte-extent accounting on the ring-buffer splices: each splice
  pushes an `ExpansionFrame {name, def site, remaining bytes}`; every
  byte handed to Flex (and every byte the raw argument collector
  consumes) debits the innermost non-exhausted frame; rewinds
  (`lpc_lex_flush_lookahead`, the collector's no-parenthesis restore, the
  SC_FUNC_OPEN rewind) credit it. **Linger policy, found by test**: a
  token's bytes are all handed before its rule action runs, so an error
  on a splice's FINAL token would fire with its frame already popped
  (`Diagnostics.ExpansionChainNote` caught it) — exhausted frames
  therefore linger until the next newline (splices never span lines),
  giving within-the-line provenance granularity, matching current_line
  itself. 7.3-B's buffer-per-expansion can replace this mechanism 1:1
  later.
- [x] **Include chains render clang-style**: "In file included from
  /file:line:" prefix lines, outermost first, before the main line — the
  Diagnostic struct now carries typed `included_from` and `expansions`
  fields plus free-form `notes`.
- [x] **`PpMacro` def sites** (`def_file`/`def_line`, stamped from
  `compiler_directive_start_line` during `#define` dispatch) feed both
  the expansion notes and a "previous definition of 'FOO' was at
  /file:line" note on the redefinition warning (via the new
  `compiler_pending_notes` queue any report site can use).
- [x] **The clang-shaped format is the DEFAULT driver output** (per
  direction — text is not an API): `yyerror()`/`yywarn()` now report via
  `report_compile_diagnostic()` (render + PRAGMA_ERROR_CONTEXT snippet
  block + debug_message + master APPLY_LOG_ERROR, same vm_context
  gating); `prepare_logs`' main line (still used by apply.cc's runtime
  trace warnings) modernized to `/file:line: error|warning: msg` for
  consistency. Full LPC testsuite verified unaffected.
- [x] **Latent corruption bug found & fixed while wiring the collector
  accounting**: the function-like-macro argument collector's
  no-parenthesis restore was `outp = saved_outp` — a raw pointer that
  dangles when the whitespace probe crosses a line boundary and
  `refill_buffer()` memmoves the buffer (reachable: macro name near a
  refill boundary, no call parens). Restore now records the probed bytes
  and pushes them back via `add_input()`, correct under any buffer
  movement.
- [x] Tests: `ExpansionChainNote`, `NestedExpansionChainNotes` (documents
  that text-level flattening attributes to the outer macro),
  `ExpansionInsideIncludeCombined` (both axes at once, order pinned),
  `RedefinitionCarriesPreviousDefinitionNote`,
  `NoStaleExpansionNoteAfterSpliceConsumed` (pins the purge). Verified:
  262/262 unit, driver-autotest clean (multiple runs), lpcshell, plus a
  live lpcc demo showing the full chain.

### 8.5 Fix-it hints — ✅ LANDED (field + renderer + first producers)
- [x] `Diagnostic::FixIt {col_start, col_end, replacement}` +
  `compiler_pending_fixits` (same one-shot contract as pending notes);
  the renderer prints clang-style replacement lines under the caret.
  First producers: the unknown-escape warning suggests dropping the
  backslash (`UnknownEscapeCarriesFixIt`), and unknown `#pragma` names
  within edit distance 2 of a real one get a "did you mean" note
  (`UnknownPragmaSuggestsNearest`). More producers attach as sites are
  touched -- the bar stays "only when the fix is unambiguous".
- (was) `Diagnostic` gains `fixits[] {range, replacement}`; the renderer
  prints the replacement under the caret line. Initial producers, chosen
  for payoff-per-effort: "expected ';'" parser recovery, unknown-pragma
  suggestions (edit distance over the known-pragma table),
  redefinition/`#undef` notes from 8.4's definition sites, and the
  heredoc/char-literal recovery paths that already know exactly what was
  malformed.

### 8.6 Wording precision pass
- [x] First batch landed (with the 8.5 work): the unknown-escape
  warning names the sequence ("Unknown escape sequence '\\q'."), the
  illegal-character error stops printing a quoted garbage glyph for
  nonprintables, unknown #pragma gets a did-you-mean note. The
  binary-op '+' error already names both operand types and now lets the
  caret/tildes do the pointing (8.3). Remaining texts continue
  site-by-site under the same rule -- change wording only where a site
  is being touched anyway, since every text is mudlib-visible.

### 8.7 Delivery & gating
- [x] **First structured consumer: lpcshell — LANDED, and it fixes a
  documented rough edge.** REPL compiles run with the new
  `compiler_diags_quiet` flag set (report_compile_diagnostic captures but
  neither prints nor master-reports), and lpcshell renders
  `compiler_diags` itself via `render_diagnostic()`: a doomed
  trial-expression parse's errors are now genuinely SILENT (previously
  they sprayed the console before the statement-form retry — the
  long-standing lpcshell rough edge), only the final failed attempt's
  clang-style diagnostics print (each compile clears the previous one's
  records), and warnings surface even on success. Runtime errors keep the
  driver's own reporting path. Closes 6.6's deferred item.
- [x] Driver-side default: the clang-shaped format IS the default (the
  opt-in gating was dropped with the text-is-not-API direction; see the
  revised ground rules). LPC testsuite verified against it.
- [x] (bookkeeping) The dependency order played out as planned and is
  now historical: 7.3 → 7.2-lines → 8.1 columns → 8.2 caret/snippet →
  8.4 chains (landed early via extents, later simplified onto buffer
  lifetime) → 8.3 locations/ranges → 8.5/8.6 polish.

---

## What Remains To Be Done

Ordered roughly by value. None of these block current functionality; all
current tests are green.

0. Phases 5 and 6 above are CLOSED (5's lex consolidation landed; 6
   rescoped per direction — reentrancy is a non-goal, the guard and the
   `load_object()` inherit dance are the settled design; only the optional
   6.6 diagnostics polish remains open there).
1. **Real REPL symbol persistence (the "non-restart" REPL).** lpcshell's v1
   restart pattern re-compiles a synthetic object per statement and fakes
   variable persistence textually. The real design: keep one compile
   session open across statements — persistent symbol table/locals, feeding
   each statement's tokens into a fresh `yypush_parse` while retaining
   compiler state. Touches symbol.cc / grammar_rules_decls.cc lifetime
   assumptions; needs its own design pass. (From `repl-push-parser.md`
   Phase 3, explicitly deferred.) NOTE: this is about keeping ONE compile
   context alive across sequential REPL statements — it does not require
   compiler reentrancy/nesting, which stays out of scope.
2. ~~Lift the compile reentrancy guard~~ — **DROPPED** (Phase 6 scope
   correction: the guard is the design; `compile_file()` is never
   legitimately re-entered).
3. ~~Per-instance ring buffer~~ — **DROPPED** (only needed for concurrent/
   nested compiles, which are a non-goal; the file-scope buffer is
   documented as a known scope boundary).
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
   ~~failed trial-expression compiles print their errors before the
   statement-form retry~~ — FIXED by the Phase 8.7 quiet-flag consumer
   (trial diagnostics are captured, never printed). Still open: expression
   results auto-print even for statement-intent input; only
   single-variable declarations are recognized (`int x, y;` is not).
7. **REPL cross-chunk `#if` (optional).** `LexerSession` could allow an
   `#if` spanning REPL statements (the cond stack lives in the session),
   currently disallowed to match old behavior — revisit only if wanted.
8. **`add_input` splice cap.** Single macro-expansion splices are capped at
   ~64KB (`DEFMAX-10`, legacy semantics). Revisit only if a real mudlib
   trips it ("Macro expansion buffer overflow").
