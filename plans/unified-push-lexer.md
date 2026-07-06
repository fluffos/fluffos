# Design Plan — Unified Push-Driven Lexer (Merge Preprocessor into Lexer)

Goal: collapse the separate preprocess-then-lex pipeline into **one scan**. The
parser's push loop asks `LexTokenStream::next()` for a token; producing that
token does only the work that token needs — reading at most a few lines of
input, resolving one macro, or lazily opening an `#include` file mid-stream.
No whole-buffer pre-pass anywhere. `#include` is handled by a stack of input
sources; every other `#` directive is handled inline, at its position, as part
of scanning.

This is deliberately a return to the original MudOS/FluffOS architecture (the
pre-standalone-preprocessor lexer did exactly this — `add_input()`'s error
message still says "Macro expansion buffer overflow", a fossil of it), but
reentrant, testable, and organized per the current `lexer_rules.cc` style.

---

## 1. Current Interaction (as-is)

```
compile_file()                                   [compiler.cc]
  └─ LexTokenStream.load(raw_stream)             [LexStream.h / lexer_utils.cc]
       └─ start_new_file()
            └─ current_stream = PreprocessingLexStream(raw, session)
                 │  first read() → LpcPreprocessor::Impl::run():
                 │    • slurps the ENTIRE raw file into a string
                 │    • walks every byte: comments, strings, templates,
                 │      #define/#if/#include (recursive sub-Impl per include),
                 │      macro expansion, #pragma/#line → sentinel markers
                 │    • returns one big preprocessed string
                 └─ ring buffer (refill_buffer/outp) pulls from that string
                      └─ Flex scanner (lex.l) re-scans every byte AGAIN
                           └─ tokens → yypush_parse loop
```

Problems this plan eliminates:

1. **Every byte is scanned twice.** The preprocessor must fully understand
   strings, char literals, template literals, and comments (to avoid expanding
   macros inside them) — then Flex scans the identical constructs again. Two
   implementations of the same tricky quoting rules, kept in sync by hand
   (the preprocessor's `scan_template_text()`/`template_brace_stack_` is a
   parallel copy of lex.l's `SC_TEMPLATE_BODY` + brace-depth machinery).
2. **Two line counters.** `Impl::current_line_` (preprocessor) and the global
   `current_line` (lexer) count the same newlines independently. This is not a
   theoretical smell: it's the root cause of the `#pragma`/`#line` sentinel-
   marker workaround, and of the empirically-confirmed corruption when direct
   preprocessor state mutation was attempted (current_line 162-vs-101 bug —
   see plans/repl-push-parser.md).
3. **First-token latency is O(entire file + all includes).** Everything is
   expanded before the parser sees token one. For the REPL this means each
   statement still pays a full-chunk pre-pass.
4. **Dormant duplicate machinery.** The lexer still carries a complete,
   battle-tested `#include` stack (`incstate_t`/`inctop`, the `<<EOF>>` →
   `parseEofOrIncludePop()` pop path, `refill_buffer()`'s include-aware
   branch) that is now dead code — the push side was deleted when the
   preprocessor took over `#include` and re-implemented it as recursion.

## 2. Target Architecture

```
yypush_parse loop                                 [compiler.cc — unchanged]
  └─ LexTokenStream::next()                       [unchanged interface]
       └─ yylex (Flex, reentrant)                 [lex.l — gains directive rules]
            └─ YY_INPUT ← ring buffer ← INPUT-SOURCE STACK
                                          [top]  macro-expansion splice (add_input)
                                                 #include file (incstate push)
                                                 #include file (nested)
                                          [base] main file / REPL string chunk
```

One scanner. One line counter. One pass. Directives are position-correct by
construction because scan order **is** token order — the entire class of
"preprocessor state vs lexer position" bugs (sentinel markers, the
current_line corruption) ceases to exist structurally.

`LexStream` stays the dumb byte interface. `PreprocessingLexStream` is deleted.
`LpcPreprocessor` (the engine) is deleted; its self-contained parts are ported,
not rewritten (see §3). The `LexTokenStream::load(stream, session)` surface is
kept — `session` becomes the macro-table/conditional-stack session object, so
the REPL's `#define`-persistence works exactly as today.

### What "push-driven, one token at a time" means concretely

- `next()` triggers at most: a few ring-buffer refills (bounded by MAXLINE),
  one directive-line's processing, or one macro expansion splice.
- `#include` no longer expands anything eagerly: the directive pushes the
  opened file onto the input stack and scanning simply continues from it;
  hitting its EOF pops back — the *existing* `parseEofOrIncludePop()` path,
  resurrected from dormancy rather than written new.
- The REPL feeds a statement chunk and pulls tokens; a future interactive
  driver can even pause between tokens (the parser side is already push-only).

## 3. Component-by-Component Disposition

| Preprocessor piece (preprocessor.cc) | Disposition |
|---|---|
| `PpMacro` + `defines_` table | **Port** → `LpcMacroTable` in a new `lexer_macro.{h,cc}` (or into `lexer_rules.cc`), owned by the session object. Predefines registry already lives lexer-side (`lexer_utils.cc`) and plugs in unchanged. |
| `handle_define()` / `#undef` | **Port near-verbatim.** Input is one captured directive line of text — same in both worlds. |
| `IfExprParser` | **Port verbatim.** Fully self-contained class, already unit-tested by 40+ parameterized tests. |
| `#if/#ifdef/#ifndef/#elif/#else/#endif` state (`cond_stack_`) | **Port** the `{emitting, had_true}` stack into the session. False branches are skipped by a new lexer skip mode (§4.3), not tokenized — dead code may be syntactically invalid and must stay un-lexed. |
| `#include` (recursive sub-`Impl`) | **Replace with the stack**: `inc_open()` + push an `incstate_t`; EOF pops. Push side resurrected; pop side already exists. |
| Macro expansion (`expand()`, guard sets) | **Re-site to identifier-resolution time** (§4.4): `lpc_lex_resolve_identifier()` consults the macro table before the identifier hash; expansion text is spliced into the input via `add_input()` (its original purpose). Recursion guard = active-expansion name stack in `compiler_context_t`. |
| `substitute()` (params, `#` stringize, `##` paste) | **Port verbatim** — pure text-to-text, used at expansion time. |
| Function-like macro argument collection (paren/quote-aware raw capture) | **Port** as a small raw-input scanner used by the expansion path (reads via `outp`, same as heredocs do). |
| `__LINE__`/`__FILE__`/`__DIR__` | **Simplify**: expand from the live `current_line`/`current_file` at resolve time. No shadow counter to sync. |
| Comment stripping | **Delete.** lex.l's comment rules exist and become load-bearing again (they're currently documented as "essentially never reached"). |
| Template-literal tracking (`scan_template_text`, `template_brace_stack_`) | **Delete outright.** Only existed so the text pass wouldn't macro-expand inside templates. In the merged design expansion happens at identifier resolution, and `SC_TEMPLATE_BODY`/`SC_STRING_BODY` never call resolve-identifier — "no expansion inside strings/templates" falls out for free. |
| Line splicing (backslash-newline) | **Move to lexer**: top-level and in-directive-capture handling (§6, risk 1). |
| `#pragma`/`#line` sentinel markers (`LPC_PRAGMA_MARKER`/`LPC_LINE_MARKER`) | **Delete.** Directives are now applied by the one-and-only scanner at their natural position. `handle_pragma()`/`lpc_lex_handle_line_directive()` are called straight from the directive dispatch. |
| `#error/#warn/#echo/#breakpoint` | One-line handlers in the directive dispatch. |
| `errors()` reporting | Directive errors go through `yyerror()`/`lexerror()` directly — correct file/line for free; the errors-vector plumbing in `PreprocessingLexStream::read()` dies with it. |

## 4. How the Merged Scanner Works

### 4.1 Directive capture (lex.l)

One anchored rule captures the *whole logical directive line* (with
backslash-continuation folded) and hands it to a dispatcher in
`lexer_rules.cc`:

```
^"#" → SC_DIRECTIVE: accumulate to end-of-line (honoring \-continuations,
        stripping // and /* */ comments in the directive text — port
        strip_directive_comments()), then:
        lpc_lex_dispatch_directive(session, text) → one of:
          DEFINE / UNDEF          → mutate macro table
          IF / IFDEF / IFNDEF     → eval (IfExprParser), push cond state;
                                     if now not emitting → BEGIN(SC_COND_SKIP)
          ELIF / ELSE / ENDIF     → adjust cond state; maybe BEGIN(SC_COND_SKIP)
          INCLUDE                 → push include (§4.2)
          PRAGMA / LINE / ERROR / WARN / ECHO / BREAKPOINT → apply now
```

This replaces both the preprocessor's `#`-handling *and* the old 4-state
`SC_DIRECTIVE_*` machinery in one shot — there is exactly one directive parser
in the codebase afterward.

### 4.2 `#include` — the stack (resurrection, not invention)

On `INCLUDE`: resolve via existing `inc_open()`; allocate an `incstate_t`
saving `{stream, outp, line, file, file_id, last_nl}`; chain it onto `inctop`;
swap `current_stream` to the included file's `FileLexStream`; flush Flex's
buffer (same `outp -= YY_PENDING_LOOKAHEAD(); YY_FLUSH_BUFFER` dance the
heredoc rules already do); continue scanning. The `<<EOF>>` rule's existing
`parseEofOrIncludePop()` already restores the outer state and resumes — it
needs only minor updates (it currently assumes the pre-preprocessor world).
Depth cap (32, as today) enforced at push time.

### 4.3 Conditional skip mode

`SC_COND_SKIP` (exclusive): consumes input line-by-line **without tokenizing**,
tracking only: string/comment awareness is *not* needed line-wise (matching the
current preprocessor, which also processes dead branches line-by-line only for
nested directives), plus `^#if*` (depth++), `^#endif` (depth--/exit),
`^#elif`/`^#else` at depth 0 (re-evaluate via the same dispatcher). This
matches the legacy lexer's skip loop and keeps syntactically-invalid dead code
harmless.

### 4.4 Macro expansion at identifier resolution

In `lpc_lex_resolve_identifier()` (lexer_utils.cc), before the
`lookup_ident()` call:

```
if (session->macros.lookup(yytext) && !in_active_expansion(yytext)):
    object-like  → add_input(substituted_body); return yylex(...)   // rescan
    function-like:
        next non-ws char '(' ? → raw-capture args (paren/quote aware, via outp)
                                 → add_input(substitute(body, params, args))
                                 → return yylex(...)
        else → fall through (macro name without call = plain identifier,
               same as today)
```

`add_input()` splices into the ring buffer ahead of the current position —
the exact mechanism `(: name` handling and heredoc arrays already use, and
what it was originally built for. Rescanning spliced text gives recursive
expansion; the guard stack (names currently being expanded, in
`compiler_context_t`) gives the standard self-reference termination the
current tests pin (`NestedSameMacroSelfReferentialTermination` etc.).

Growth note: `add_input` caps at `DEFMAX-10` (~64KB) per splice. Keep the cap
initially (matches legacy semantics; the current preprocessor has no
comparable single-expansion limit but 64KB per expansion step is generous),
revisit only if a real mudlib trips it.

### 4.5 Session object (REPL persistence)

```cpp
struct LexerSession {              // owned via shared_ptr, as today
  LpcMacroTable macros;            // #define/#undef state — persists
  std::vector<CondState> conds;    // must be empty at chunk end ("missing #endif")
};
```
`LexTokenStream::load(stream, session)` keeps its exact signature;
`start_new_file()` stores the session where the directive dispatcher can reach
it (alongside `current_stream`). Everything the REPL relies on today
(`#define` persisting across statements) transfers; as a bonus, `#if` spanning
REPL statements becomes *possible* later (the cond stack lives in the
session), though we keep requiring self-contained chunks initially to match
current behavior.

## 5. Migration Stages (each stage: build + full ctest + driver-autotest green)

**Stage 1 — Port the pure parts, standalone.**
`IfExprParser`, `PpMacro`→`LpcMacroTable`, `handle_define`, `substitute`,
`strip_directive_comments` move into `lexer_macro.{h,cc}` as free-standing,
unit-testable code. New gtest file exercises them directly (the existing
`IfExpr/*` and define/paste/stringize tests port with trivial harness
changes). preprocessor.cc temporarily #includes/uses the ported versions so
there is exactly one copy — proves equivalence while the old pipeline still
runs everything.

**Stage 2 — The cutover.** (Necessarily one atomic step: the preprocessor
consumes every `#` line today, so new lexer rules can't be exercised while it
runs. Stage 1's thoroughness is the mitigation.)
- lex.l: directive capture + dispatch (§4.1), `SC_COND_SKIP` (§4.3),
  top-level backslash-newline rule.
- Resurrect the `#include` push (§4.2); update `parseEofOrIncludePop()`.
- Macro expansion in `lpc_lex_resolve_identifier()` (§4.4).
- `start_new_file()` wires the raw stream directly; `PreprocessingLexStream`
  and the sentinel markers die; `#pragma`/`#line` applied from dispatch.
- Session object replaces `LpcPreprocessor` in `load()`'s signature.

**Stage 3 — Delete & migrate tests.**
Delete `preprocessor.cc`/`.h` engine remains. The ~78 `Preprocessor.*` tests
assert on preprocessed *text*, which no longer exists: migrate the semantic
ones to token-level assertions via test_lexer.cc's `Tokenize()` harness
(`"#define X 42\nint v = X;"` → expect `L_NUMBER(42)` in the token stream —
arguably a *stronger* assertion than string equality), keep the
`IfExprParser`/define/substitute unit tests from Stage 1 as-is, drop the
handful that test pure text-formatting artifacts (marker emission, exact
newline placement) that have no observable behavior anymore.

**Stage 4 — Docs & cleanup.**
README.md (compiler/internal), plans/repl-push-parser.md cross-reference,
delete dead `#pragma`/`#line` marker definitions, `lpcshell` re-verify
(it exercises the REPL session path end-to-end).

## 6. Risks & Open Questions

1. **Backslash-newline outside directives.** Today the preprocessor splices
   continuations everywhere. Grep the testsuite for non-directive
   continuations; add a top-level lex.l rule (`\\(\r)?\n` → count line, emit
   nothing). Strings already handle their own.
2. **`#define` bodies spanning huge expansions** — `add_input` cap (§4.4).
3. **Directive text comments**: `#define X 1 /* c */` must strip the comment
   from the captured line before parsing (ported helper covers it); a `/*`
   *opening* in a directive and closing lines later is currently... (verify
   preprocessor behavior; match it).
4. **`__LINE__` off-by-ones**: the preprocessor's `current_line_` had its own
   "-1 because the newline will increment" convention. Expanding from live
   `current_line` needs the same fence-post check against existing tests
   (`PredefinedFileAndLine`, `LineCountPreserved`).
5. **CRLF**: preprocessor handles `\r\n` in continuations/directives; captured
   directive lines must too (tests exist: `CRLFLineContinuations` etc.).
6. **Error volume**: directive errors now flow through `yyerror()` mid-scan
   — same as other lex errors; verify `driver-autotest`'s expected-error
   compiler tests (`single/tests/compiler/fail/*`) byte-match where they
   assert messages.
7. **`#include` inside a false `#if`** must not open the file (skip mode
   handles it — dispatcher consults `emitting()` first, as today).
8. **Heredocs**: bodies must remain expansion-free (`HereDocNoMacroExpansion`
   test) — automatic, since heredoc bodies are raw-captured without identifier
   resolution.

## 7. What This Buys (recap)

- Single scan; every byte read once. Strings/templates/comments have exactly
  one implementation of their quoting rules.
- Single line counter — the sentinel-marker workaround and the entire
  "preprocessor state vs lexer position" bug class are eliminated
  structurally, not patched.
- True one-token-at-a-time incrementality end-to-end (parser is already
  push-only; now the input side matches).
- `#include` cost paid lazily at the include site; first token available
  after reading one line.
- Net code deletion: preprocessor's ~600 lines of scanning/template/comment
  logic die; its ~400 lines of genuinely-unique logic (macro table, if-expr
  eval, substitution) move; the lexer's dormant include machinery goes back
  to work instead of being dead weight.

---

## 8. Implementation Status — DONE (all stages landed, all tests green)

Landed as one working-tree change set (Stages 1–3 collapsed together in
practice; the "temporarily share ported code with the old engine" step was
skipped in favor of cutting over directly and driving the result green).
Verified: full 246-test unit suite passes, full LPC testsuite
(`driver-autotest`) passes with zero failures, `lpcshell` verified
interactively (expressions, multi-line continuation, `#define`-using
statements, variable persistence).

**Where things landed** (per direction, preprocessing logic lives in
`lexer_rules_pp.cc`):

- `lexer_rules_pp.{h,cc}` — `PpMacro` + `LpcMacroTable` (a plain
  `unordered_map` alias; the wrapper class draft was deleted as an
  unnecessary abstraction), `LexerSession` (macro table + `CondState`
  conditional stack + builtin-macro seeding from the predefines registry),
  `IfExprParser` (ported verbatim), `substitute()`/`collect_args()`/
  `strip_directive_comments()`/`stringize()`/`trim()`, textual expansion
  `lpc_lex_expand_string()` with guard lists and `#if`-only
  `defined()`/`efun_defined()`, `lpc_lex_builtin_macro()`
  (`__LINE__`/`__FILE__`/`__DIR__` from the LIVE scan position, filenames
  normalized to leading-`/` form — a real bug found by the testsuite's
  `function_exists.c`: the first cut used raw `current_file` and dropped
  the `/`), the directive dispatcher `lpc_lex_dispatch_directive()`, and
  the `SC_COND_SKIP` classifier `lpc_lex_classify_skip_directive()`
  (classification in C++ on the whole captured line, not per-keyword Flex
  patterns — avoids the `#if(x)`-vs-`#ifdef` word-boundary fragility).
- `lex.l` — ONE anchored directive rule owning the whole logical `#` line
  (continuations folded by the pattern itself). Critical ordering, learned
  from a real corruption: the rule rewinds `outp` past Flex's prefetch and
  consumes + counts the terminating newline BEFORE dispatching, so an
  `#include` push records the exact parent resume point with `outp[-1]`
  being the newline `refill_buffer()`'s include branch reuses as its
  sentinel; the rule flushes Flex's buffer after dispatch in all cases.
  Plus `SC_COND_SKIP` (single-rule + classifier), the top-level
  backslash-newline splice rule, and the `\x1e<name>` sentinel rule that
  pops the active-expansion guard.
- `lexer_utils.cc` — macro expansion at identifier-resolution time in
  `lpc_lex_resolve_identifier()` (object-like: expand body + splice via
  `add_input()` + rescan; function-like: raw-capture args from `outp` with
  save/restore when no `(` follows), `lpc_lex_handle_include()` (the
  resurrected legacy push, byte-for-byte matching the legacy line
  bookkeeping — `save_file_info`/`current_line_base` — that the untouched
  legacy pop still assumes, ending with `refill_buffer()`), and
  `start_new_file()` delegating `__GLOBAL_INCLUDE_FILE__` to that same
  push exactly like legacy `handle_include(gifile, 1)` did.
- **Deleted outright**: `preprocessor.cc`/`preprocessor.h` (the whole
  standalone engine), `PreprocessingLexStream`, the `#pragma`/`#line`
  sentinel markers (`LPC_PRAGMA_MARKER`/`LPC_LINE_MARKER`) — directives now
  apply at their scan position, which the single-scan design makes correct
  by construction. `LexTokenStream::load()`'s session parameter is now
  `std::shared_ptr<LexerSession>`.

**Additions beyond the original plan:**

- **`vm_context_t`** (per direction): `compile_file()` takes a
  `vm_context` parameter (default `&g_driver_vm_context`); the global
  `compiler_vm_context` is null for unit tests driving lexer/compiler
  pieces without a booted VM, and gates every VM interaction on the
  compile path — `smart_log()`'s master-apply reporting, `yyerror()`'s
  mudlib-stats recording (`add_errors_for_file` → eval-stack pushes:
  the exact null-`sp` UB the tests were hitting), and
  `init_include_path()`'s `APPLY_GET_INCLUDE_PATH` query (falls back to
  the config include list).
- **Top-level `##` was implemented, then removed as invalid** (per
  direction): `##` exists only inside macro bodies (resolved by
  `substitute()`); the ancient `X1##X2`-at-top-level line in
  `testsuite/single/tests/compiler/succeed.c` was rewritten to proper
  macro-body pasting (`PASTE(a,b) a##b`).
- **Session recovery**: "Missing #endif" at EOF clears the session's
  conditional stack — otherwise a reused (REPL) session would silently
  skip all subsequent input forever.
- `#line`'s handler drops the old `-1` convention — the directive rule
  counts the terminating newline before dispatch now (fence-post risk #4,
  caught by `LineDirectiveUpdatesLineNumber`).

**Test migration** (Stage 3): `test_compiler.cc`'s `pp()` harness now
drives the REAL lexer end-to-end and reconstructs comparable text from the
token stream (whitespace-normalized comparison) — the ~78 existing
`Preprocessor.*` tests kept their bodies nearly verbatim but now assert
through actual tokenization, a strictly stronger check. A handful of
expectations changed from text-world to token-world (heredoc/template
escapes now decoded by the lexer, `#pragma` asserted via the `pragmas`
flags, `#line` via `__LINE__`).

**Known deltas vs the old standalone preprocessor** (accepted, legacy-parity
or better):

- An unbalanced `#if` opened inside an `#include`d file leaks into the
  parent (one shared conditional stack, reported at top-level EOF) — the
  legacy scanner behaved this way too; the standalone preprocessor's
  per-include `Impl` caught it per-file. A conds-depth snapshot in
  `incstate_t` could restore per-file checking later if wanted.
- Directive error text position: errors report at the line after the
  directive's newline (consumed before dispatch); cosmetic.
