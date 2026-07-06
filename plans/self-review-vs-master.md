# Self-review of `dev` vs `master` — handoff plan

**Task (verbatim from the user):** "carefully review the code changes against
main, look for any behavior change, error changes, and other bugs, identify
all possible bugs, update the plan doc so another agent can pick it up and
fix it."

**State when this doc was last updated:** review is done through three rounds.
All identified bugs across Round-1 (1–4), Round-2 (5–8), and Round-3 (9–14) are completely **FIXED** in the repository with comprehensive unit tests and LPC integration tests.

---

## Scope

`git log origin/master..dev` is the Flex lexer migration, the standalone preprocessor, the grammar modernization, and feature commits (template literals, mapping dot access, optional chaining, %g sprintf). Three rounds of reviews focused on code correctness, compile lifecycles, preprocessor macro-expansion boundaries, and CRLF compatibility.

---

## Round-1 bugs — FIXED, tests written

### Bug 1 — `({` array literal inside `${...}` breaks template brace tracking
- **Fix**: The `L_ARRAY_OPEN` rule increments `template_brace_depth[template_nesting]` when `template_nesting > 0`.
- **Tests**: `test_lexer.cc` `TemplateLiteral_ArrayLiteralInsideInterpolation`; LPC block in `template_literal.c`.

### Bug 2 — preprocessor stripped `//` and `/* */` comments inside backtick text
- **Fix**: Added `scan_template_text()` to preprocessor to copy template text verbatim.
- **Tests**: `Preprocessor.TemplateTextNotCommentStripped`; LPC block in `template_literal.c`.

### Bug 3 — preprocessor macro-expanded template literal *text*
- **Fix**: Done in the same preprocessor redesign as Bug 2.
- **Tests**: `Preprocessor.TemplateTextNotMacroExpanded`, `TemplateInterpolationIsMacroExpanded`, etc.

### Bug 4 — `defined`/`efun_defined` applied outside `#if` (behavior change vs master)
- **Fix**: `expand()` gained `bool in_if_expr = false` and operator blocks are now guarded on it.
- **Tests**: `Preprocessor.DefinedIsPlainIdentifierOutsideIf`, `Preprocessor.DefinedStillWorksInsideIf`.

---

## Round-2 bugs — FIXED, tests written

### Bug 5 — aborted compile leaks Flex scanner lookahead into the next compile
- **Fix**: Exported `lpc_lex_reset()` from `lex.l` to run `YY_FLUSH_BUFFER` and `BEGIN(INITIAL)`, and called it from `start_new_file()` in `lexer_utils.cc`.
- **Tests**: LPC test `testsuite/single/tests/compiler/scanner_state_leak.c` using helpers `aborted_inner.c`/`clean_inner.c`.

### Bug 6 — nested same-macro call in a macro argument never expands
- **Fix**: Updated `preprocessor.cc` argument expansion inside `expand()` and `run()` to expand macro arguments with the *incoming* guard (empty for `run()`) instead of including the current macro being pre-expanded.
- **Tests**: Unit tests `Preprocessor.NestedSameMacroInArgsExpands` / `Preprocessor.NestedSameMacroSelfReferentialTermination`; LPC block in `optional-chaining.c`.

### Bug 7 — macro-argument collectors don't know backtick templates
- **Fix**: Treated `\`` as a quoting character in `collect_args()`, `run()`'s inline collector, and `strip_directive_comments()`.
- **Tests**: Unit test `Preprocessor.TemplateAsMacroArgument`; LPC block in `template_literal.c`.

### Bug 8 — CRLF source: multiline template text embeds a stray `\r`
- **Fix**: Refined `SC_TEMPLATE_BODY` matching rules in `lex.l` to match/ignore `\r\n` and exclude `\r` from standard text runs. Added lone `\r` fallback.
- **Tests**: Unit test `TemplateLiteral_Crlf_StripsCarriageReturn`; LPC test `template_crlf.c`.

---

## Round-3 bugs — FIXED, tests written (CRLF Transparency)

### Bug 9 (HIGH) — CRLF preprocessor line continuation `\\\r\n` in directives
- **Fix**: Updated `read_line_text()` in `preprocessor.cc` to recognize `\\\r\n`, incrementing line counters and advancing properly.
- **Tests**: Unit test `Preprocessor.CRLFLineContinuations`.

### Bug 10 (MEDIUM) — CRLF preprocessor line continuation `\\\r\n` outside directives
- **Fix**: Updated `run()`'s main loop in `preprocessor.cc` to check for `peek() == '\\' && peek(1) == '\r' && peek(2) == '\n'` and advance by 3, incrementing the line count.
- **Tests**: Unit test `Preprocessor.CRLFLineContinuations`.

### Bug 11 (MEDIUM) — CRLF line continuation `\\\r\n` in template text
- **Fix**: Updated `scan_template_text()` in `preprocessor.cc` to recognize `\\\r\n`, output it verbatim (when emitting), and increment `current_line_` while advancing by 3.
- **Tests**: Unit test `Preprocessor.CRLFLineContinuations`.

### Bug 12 (MEDIUM) — CRLF escaped newlines `\\\r\n` in lexer string and char literals
- **Fix**: Added rules `<SC_STRING_BODY>"\\\r\n"` and `<SC_CHAR_BODY>"\\\r\n"` in `lex.l` to increment line counts and handle escaped newlines transparently.
- **Tests**: Unit tests `LexerTest.String_LineContinuation_Crlf` and `LexerTest.Char_EscapeNewline_Crlf`.

### Bug 13 (MEDIUM) — CRLF raw newlines inside string literals normalize to `\n`
- **Fix**: Modified `SC_STRING_BODY` in `lex.l` to match `<SC_STRING_BODY>\r\n`, mapping raw CRLF newlines inside double quotes to `\n`. Added rule for lone `\r` to preserve them, and excluded `\r` from standard string text runs.
- **Tests**: Unit test `LexerTest.String_RawNewline_Crlf`.

### Bug 14 (HIGH) — CRLF newlines inside heredocs (get_text_block / get_array_block)
- **Fix**: Modified both `get_text_block()` and `get_array_block()` in `lexer_utils.cc` to strip trailing `\r` immediately when the inner line-read loop finishes, before null-termination and terminator comparison.
- **Tests**: All heredoc integration tests verified (e.g. `@` and `@@` arrays).

---

## Remaining steps (in order)

All identified compiler front-end and preprocessor issues have been successfully resolved, tested, and verified. No further steps are remaining.
