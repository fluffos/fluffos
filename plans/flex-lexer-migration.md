# Flex Lexer Migration Plan

> Migrate `src/compiler/internal/lex.cc` (2,840-line hand-written scanner)
> to a Flex-generated DFA lexer (`lex.l`) in six incremental phases.
> The driver compiles and all tests pass at the end of every phase.

---

## Decision Log

| Question | Decision | Rationale |
|---|---|---|
| Flex vs re2c? | **Flex** | Mirrors existing Bison CMake pattern; `find_package(FLEX)`, `FLEX_TARGET()`, `ADD_FLEX_BISON_DEPENDENCY()` are built-in CMake modules — zero new infrastructure |
| `yytext` size | `%option array` + `#define YYLMAX MAXLINE` (4096 B) | Matches current `char yytext[MAXLINE]` budget; existing scratchpad large-string overflow path unchanged |
| Unicode identifiers | `%option 8bit` for UTF-8 string pass-through | LPC identifiers are ASCII-only (`[a-zA-Z_][a-zA-Z0-9_]*`); `8bit` needed only for string/heredoc literal content |
| Preprocessor integration | Pre-pass unchanged | `LpcPreprocessor` runs before the lexer as today; Flex reads from the resulting `StringLexStream` |

---

## Architecture: Before vs After

```
BEFORE (today):
  LexStream ──read──► start_new_file()
                           │  reads entire stream
                           │  runs LpcPreprocessor (preprocessor.cc)
                           │  feeds StringLexStream → linked_buf_t ring
                           ▼
                      yylex()   [hand-written switch in lex.cc ~770 lines]
                           ▼
                      Bison parser (yyparse)

AFTER (Phase 6):
  LexStream ──read──► start_new_file()           (unchanged)
                           │  reads, preprocesses, fills linked_buf_t
                           ▼
                      yylex()   [Flex-generated DFA in lex.l]
                           │  YY_INPUT → lpc_lex_yy_input() → linked_buf_t
                           ▼
                      Bison parser (yyparse)
```

> **What is never changed during this migration:**
> `linked_buf_t` ring-buffer, `LpcPreprocessor`, `incstate_t` include stack
> (until Phase 5 replaces it with Flex's `<<EOF>>` rule).

---

## Phase Overview

| Phase | Goal | Files Changed | Status |
|---|---|---|---|
| 1 | CMake Flex wiring + `lex.l` skeleton shim | `CMakeLists.txt`, `lex.l` (new), `lex.cc`, `lex.h` | ✅ done |
| 2 | `test_lexer.cc` token-level unit test harness | `test_lexer.cc` (new), `tests/CMakeLists.txt` | ✅ done (121 tests) |
| 3 | Operators, punctuation, whitespace, comments → Flex | `lex.l`, `lex.cc` | ✅ done |
| 4 | Numeric literals, identifiers → Flex natively; string/char literals/`$N` params → Flex-triggered but algorithm untouched (`lex_string.cc` extraction NOT done, see Phase 4 deviation notes) | `lex.l`, `lex.cc` | ✅ done (245 tests) |
| 5 | `YY_INPUT` (already wired in Phase 3, ahead of schedule), `'('`, heredocs, `#pragma`/`#line`, EOF/include-stack → Flex-triggered; `yylex_inner()` and its dispatch loop deleted entirely | `lex.l`, `lex.cc` | ✅ done (245 tests) |
| 6 | Maximize *native* Flex scanning: convert Flex-triggered-but-still-raw-`outp`-reading helpers into genuine Flex DFA rules (start conditions) wherever the risk is manageable, so Flex itself does the character-by-character work instead of a hand-written loop behind a trigger rule | `lex.l`, `lex.cc`/`lex_util.cc`, `lex.h` | ✅ done (254 tests); round 2: comments/strings/directives also converted (74 tests); round 3: heredoc's "@"/"@@"-prefix-and-terminator also converted, dead-code sweep, `lex.cc`→`lex_util.cc` (78 tests); self-review pass (two rounds): 4 real bugs found and fixed (82 tests); only the heredoc *body* match and EOF/include-pop bookkeeping remain non-native, both structurally so; WOMBLES + WARN_TAB configs removed along the way |
| 7 | Rename `lex.cc` → `lex_util.cc` once heredoc's "@"/"@@"-prefix-and-terminator also went native (the planned `lex_helpers.cc`/`lex_string.cc` split turned out unnecessary — see the Phase 7 note); dead-code cleanup; sanitizer-clean build (the default `build/` dir already is one); Flex-absent fallback test | `lex.cc`→`lex_util.cc`, `lex.h`, `lex.l`, `src/CMakeLists.txt` | ✅ mostly done — Flex-absent fallback still open (see note) |

> `YY_INPUT` and the buffer-synchronization machinery (`DEFER_TO_LEGACY_SCANNER`,
> `YY_PENDING_LOOKAHEAD`, the `>=` vs `==` refill-boundary fix) were pulled
> forward into Phase 3, because adding *any* real Flex-matching rule
> immediately requires Flex to actually read through `YY_INPUT` — there's no
> way to add "just the easy operators" without first solving buffer sync.
> This was by far the hardest part of the whole migration; see the Phase 3
> tracking-checklist notes below for what actually went wrong and why.

---

## Phase 1 — Build Infrastructure

### Goal
Wire Flex into CMake. `lex.l` compiles, links, and delegates everything to
the existing `yylex_inner()`. **Zero behavioral change.**

### 1A — Rename `yylex()` → `yylex_inner()` in `lex.cc`

```diff
// lex.cc  line ~1045
-int yylex() {
+// Hand-written implementation. Called by lex.l catch-all during migration.
+// Remove when Phase 5 (yylex_inner catch-all deleted) is complete.
+int yylex_inner() {
```

Update `lex.h`:

```diff
+int yylex_inner();   // remove after Phase 5
-int yylex();         // now generated by Flex
```

### 1B — Create `compiler/internal/lex.l`

```flex
%{
/*
 * lex.l — Flex skeleton for the LPC scanner.
 * Phase 1: catch-all delegates to yylex_inner() in lex.cc.
 *
 * Include order: grammar_rules.h MUST come before grammar.autogen.h
 * because grammar.autogen.h references decl_t / func_block_t from it.
 */
#include "base/std.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"

// Forward declaration: hand-written scanner in lex.cc (phases 1–4 shim)
int yylex_inner();

// YY_INPUT: pull bytes from lex.cc's linked_buf_t ring (active Phase 3+)
#define YY_INPUT(buf, result, max_size) \
    result = lpc_lex_yy_input(buf, max_size)

// Declared in lex.cc; bridges Flex's buffer read to the existing ring-buffer
extern "C" int lpc_lex_yy_input(char *buf, int max_size);
%}

%option noyywrap nounput noinput nodefault
%option never-interactive
%option 8bit
%option array
%define YYLMAX 4096     /* matches MAXLINE in lex.h */

%%

/*
 * Phase 1 catch-all: put the character back and delegate to the legacy
 * hand-written scanner.  As explicit rules are added in Phases 3–5,
 * matching tokens never reach this rule.
 */
[\x00-\xff] {
    yyless(0);
    return yylex_inner();
}

%%
```

### 1C — Add `lpc_lex_yy_input()` to `lex.cc`

```cpp
// Bridge: called by Flex's YY_INPUT macro once Phase 3 rules consume chars.
// During Phases 1-2, this is never reached (yyless(0) bypasses it).
extern "C" int lpc_lex_yy_input(char *buf, int max_size) {
    int n = 0;
    while (n < max_size) {
        unsigned char c = static_cast<unsigned char>(*outp);
        if (c == LEX_EOF) break;
        buf[n++] = static_cast<char>(c);
        ++outp;
        if (outp == last_nl + 1) refill_buffer();
    }
    return n;
}
```

### 1D — CMake changes in `src/CMakeLists.txt`

Insert **after** the `find_package(BISON 3.8)` / `endif()` block (~line 384):

```cmake
# ── Flex (LPC lexer) ──────────────────────────────────────────────────────
find_package(FLEX 2.6)
if (FLEX_FOUND)
  message(STATUS "FLEX: ${FLEX_VERSION}")
  FLEX_TARGET(LpcLexer
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lex.l"
    "${CMAKE_CURRENT_BINARY_DIR}/lex.autogen.cc"
    COMPILE_FLAGS "--nounput --noinput -Ca"
  )
  ADD_FLEX_BISON_DEPENDENCY(LpcLexer Grammar)
  set(GENERATED_LEXER_FILE "${FLEX_LpcLexer_OUTPUTS}")
else()
  message(STATUS "FLEX: not found, using pre-generated source.")
  add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/lex.autogen.cc"
    COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lex.autogen.cc"
            "${CMAKE_CURRENT_BINARY_DIR}/lex.autogen.cc"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lex.autogen.cc"
  )
  set(GENERATED_LEXER_FILE "${CMAKE_CURRENT_BINARY_DIR}/lex.autogen.cc")
endif()
# ─────────────────────────────────────────────────────────────────────────
```

Add to the `SRC` list (alongside `lex.cc` — both compile until Phase 6):

```cmake
  "${GENERATED_LEXER_FILE}"          # Flex-generated lex.autogen.cc
  "compiler/internal/lex.cc"         # hand-written shim (removed Phase 6)
```

Add post-build step to commit the generated file to source tree:

```cmake
if (FLEX_FOUND)
  add_custom_command(TARGET "driver" POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_CURRENT_BINARY_DIR}/lex.autogen.cc"
            "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lex.autogen.cc"
    COMMENT "Copying Flex-generated lex.autogen.cc to source tree")
endif()
```

### Phase 1 Verification

```bash
cmake --build build -j 12
ctest --test-dir build --output-on-failure
cd testsuite && ../build/src/driver etc/config.test -ftest
```

---

## Phase 2 — Test Harness (`test_lexer.cc`)

### Goal
Write the complete lexer unit test file **before** any DFA rules move to
`lex.l`, so every subsequent phase has a regression net at the token level.

### Token coverage (≥ 116 test cases)

| Category | Examples | Count |
|---|---|---|
| Integer decimal | `0`, `42`, `1_000_000` | 4 |
| Integer hex | `0xFF`, `0X1A`, `0xDEAD_BEEF` | 3 |
| Integer binary | `0b1010`, `0B0001_0001` | 2 |
| Float | `1.5`, `0.0`, `3.14` | 3 |
| String basic | `"hello"`, `""` | 2 |
| String escapes | `\n \t \r \b \a \e \\ \"` | 8 |
| String octal/hex/unicode | `"\101"` `"\x41"` `"\u0041"` `"\U00000041"` | 4 |
| Char literal | `'a'` `'\n'` `'\x41'` `'\101'` | 4 |
| Heredoc text `@` | single-line, multi-line, trailing content | 3 |
| Heredoc array `@@` | array block | 3 |
| Operators (single) | `+ - * / % ^ & \| ~ `lex.l`, so every subsequent phase has a regression net at the token level.
 | 10 |
| Operators (compound) | `+= -= *= /= %= ^= &= \|= <<= >>=` | 10 |
| Operators (logic/assign) | `&&= \|\|= && \|\|` | 4 |
| Operators (comparison) | `== != < > <= >=` | 6 |
| Operators (other) | `++ -- -> .. ... :: ?? ??=` | 8 |
| Assignment | `=` | 1 |
| Keywords | `int string float object mapping buffer function void mixed if else for foreach while do switch case default break continue return inherit` | 22 |
| Identifiers | simple, `_leading`, `with123digits` | 3 |
| Comments | `/* block */`, `// line` | 4 |
| Whitespace | space, tab (warn), `\r` ignored, `\n` line count | 4 |
| Pragmas | `#pragma strict_types`, `#pragma no_warnings` | 2 |
| Function pointer | `(:` open | 2 |
| **Total** | | **≥ 116** |

### `Tokenize()` helper

```cpp
struct Token {
    int       kind;    // Bison token (L_NUMBER, L_STRING, L_IDENTIFIER, …)
    LPC_INT   number;  // valid when kind == L_NUMBER
    LPC_FLOAT real;    // valid when kind == L_REAL
    std::string text;  // copy of yytext at token time
};

// Feeds source through start_new_file() + repeated yylex() until -1.
std::vector<Token> Tokenize(const std::string& source);
```

### `tests/CMakeLists.txt` addition

```cmake
add_executable(lexer_tests test_lexer.cc)
target_link_libraries(lexer_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest GTest::Main)
target_compile_definitions(lexer_tests PRIVATE
    -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")
gtest_discover_tests(lexer_tests DISCOVERY_TIMEOUT 60)
```

---

## Phase 3 — Operators, Punctuation, Whitespace, Comments → Flex

Add explicit Flex rules **above** the catch-all. Delete the corresponding
`switch` cases from `yylex_inner()` in `lex.cc`.

```flex
/* ── Whitespace / control ─────────────────────────────────────────── */
[ \f\v\r]    { /* ignore */ }
"\t"         { if (CONFIG_INT(__RC_WARN_TAB__)) yywarn("<TAB>"); }
"\n"         { nexpands = 0; current_line++; total_lines++;
               if (outp == last_nl + 1) refill_buffer(); }

/* ── Comments ─────────────────────────────────────────────────────── */
"/*"         { skip_comment(); }
"//"[^\n]*   { /* line comment consumed; \n left for above rule */ }

/* ── Arithmetic ───────────────────────────────────────────────────── */
"++"   { return L_INC; }
"+="   { yylval.number = F_ADD_EQ;  return L_ASSIGN; }
"+"    { return '+'; }
"--"   { return L_DEC; }
"->"   { return L_ARROW; }
"-="   { yylval.number = F_SUB_EQ;  return L_ASSIGN; }
"-"    { return '-'; }
"*="   { yylval.number = F_MULT_EQ; return L_ASSIGN; }
"*"    { return '*'; }
"/="   { yylval.number = F_DIV_EQ;  return L_ASSIGN; }
"/"    { return '/'; }
"%="   { yylval.number = F_MOD_EQ;  return L_ASSIGN; }
"%"    { return '%'; }

/* ── Bitwise ──────────────────────────────────────────────────────── */
"<<="  { yylval.number = F_LSH_EQ;  return L_ASSIGN; }
"<<"   { return L_LSH; }
">>="  { yylval.number = F_RSH_EQ;  return L_ASSIGN; }
">>"   { return L_RSH; }
"^="   { yylval.number = F_XOR_EQ;  return L_ASSIGN; }
"^"    { return '^'; }
"||="  { yylval.number = F_LOR_EQ;  return L_ASSIGN; }
"||"   { return L_LOR; }
"|="   { yylval.number = F_OR_EQ;   return L_ASSIGN; }
"|"    { return '|'; }
"&&="  { yylval.number = F_LAND_EQ; return L_ASSIGN; }
"&&"   { return L_LAND; }
"&="   { yylval.number = F_AND_EQ;  return L_ASSIGN; }
"&"    { return '&'; }
"~"    { return '~'; }

/* ── Comparison ───────────────────────────────────────────────────── */
"=="   { yylval.number = F_EQ;      return L_EQ; }
"!="   { return L_NE; }
"<="   { yylval.number = F_LE;      return L_ORDER; }
">="   { yylval.number = F_GE;      return L_ORDER; }
"<"    { return '<'; }
">"    { return '>'; }
"!"    { return L_NOT; }

/* ── Nullish coalescing ───────────────────────────────────────────── */
"??="  { yylval.number = F_NULLISH_EQ; return L_ASSIGN; }
"??"   { return L_NULLISH; }

/* ── Range / scope / dot ─────────────────────────────────────────── */
"..."  { return L_DOT_DOT_DOT; }
".."   { return L_RANGE; }
"."    { return L_DOT; }
"::"   { return L_COLON_COLON; }

/* ── Assignment ──────────────────────────────────────────────────── */
"="    { yylval.number = F_ASSIGN; return L_ASSIGN; }

/* ── Punctuation (return ASCII value) ───────────────────────────── */
[(){}\[\];,?:]  { return yytext[0]; }
```

> **Note**: Flex matches the longest rule automatically. `++` beats `+`, `<<=` beats `<<` beats `<`.

### What stays in `yylex_inner()` after Phase 3
Numeric literals · String/char literals · Identifiers & keyword lookup ·
`@`/`@@` heredoc · `#` pragma/line directives · EOF/include-stack pop ·
`(:` function-pointer toggle

---

## Phase 4 — Literals and Identifiers → Flex

### 4A — Numeric literals

```flex
[0-9][0-9_]*  {
    std::string s(yytext, yyleng);
    s.erase(std::remove(s.begin(), s.end(), '_'), s.end());
    char *end; yylval.number = strtoll(s.c_str(), &end, 10);
    return L_NUMBER;
}
[0-9][0-9_]*"."[0-9][0-9_]*  {
    std::string s(yytext, yyleng);
    s.erase(std::remove(s.begin(), s.end(), '_'), s.end());
    char *end; yylval.real = strtod(s.c_str(), &end);
    return L_REAL;
}
"0"[xX][0-9a-fA-F][0-9a-fA-F_]*  {
    std::string s(yytext+2, yyleng-2);
    s.erase(std::remove(s.begin(), s.end(), '_'), s.end());
    char *end; yylval.number = strtoll(s.c_str(), &end, 16);
    return L_NUMBER;
}
"0"[bB][01][01_]*  {
    std::string s(yytext+2, yyleng-2);
    s.erase(std::remove(s.begin(), s.end(), '_'), s.end());
    char *end; yylval.number = strtoll(s.c_str(), &end, 2);
    return L_NUMBER;
}
```

### 4B — String literals (exclusive start condition `SC_STRING`)

```flex
%x SC_STRING

\"                     { BEGIN(SC_STRING); lpc_string_reset(); }
<SC_STRING>{
  \"                   { BEGIN(INITIAL); yylval.string = lpc_string_finish(); return L_STRING; }
  \\n                  { lpc_string_append('\n'); }
  \\t                  { lpc_string_append('\t'); }
  \\r                  { lpc_string_append('\r'); }
  \\b                  { lpc_string_append('\b'); }
  \\a                  { lpc_string_append('\a'); }
  \\e                  { lpc_string_append('\x1b'); }
  \\\"                 { lpc_string_append('"'); }
  \\\\                 { lpc_string_append('\\'); }
  \\[0-7]{1,3}         { lpc_string_append_octal(yytext+1); }
  \\x[0-9a-fA-F]{1,2} { lpc_string_append_hex(yytext+2); }
  \\u[0-9a-fA-F]{4}   { lpc_string_append_unicode_u(yytext+2); }
  \\U[0-9a-fA-F]{8}   { lpc_string_append_unicode_U(yytext+2); }
  \\\n                 { current_line++; total_lines++; }
  \n                   { current_line++; total_lines++; lpc_string_append('\n'); }
  <<EOF>>              { lexerror("End of file in string"); return YYEOF; }
  .                    { lpc_string_append(yytext[0]); }
}
```

String helpers (`lpc_string_reset`, `lpc_string_append`, `lpc_string_finish` variants)
extracted into **`lex_string.cc`** / **`lex_string.h`**.

### 4C — Character literals (start conditions `SC_CHAR`, `SC_CHAR_ESC`)

```flex
%x SC_CHAR SC_CHAR_ESC

\'  { BEGIN(SC_CHAR); yylval.number = 0; }
<SC_CHAR>{
  \\  { BEGIN(SC_CHAR_ESC); }
  \'  { BEGIN(INITIAL); return L_NUMBER; }
  .   { yylval.number = (unsigned char)yytext[0]; }
}
<SC_CHAR_ESC>{
  n              { yylval.number = '\n'; BEGIN(SC_CHAR); }
  t              { yylval.number = '\t'; BEGIN(SC_CHAR); }
  r              { yylval.number = '\r'; BEGIN(SC_CHAR); }
  b              { yylval.number = '\b'; BEGIN(SC_CHAR); }
  a              { yylval.number = '\a'; BEGIN(SC_CHAR); }
  e              { yylval.number = '\x1b'; BEGIN(SC_CHAR); }
  \'             { yylval.number = '\''; BEGIN(SC_CHAR); }
  \\             { yylval.number = '\\'; BEGIN(SC_CHAR); }
  [0-7]{1,3}     { yylval.number = (int)strtol(yytext, nullptr, 8); BEGIN(SC_CHAR); }
  x[0-9a-fA-F]{1,2} { yylval.number = (int)strtol(yytext+1, nullptr, 16); BEGIN(SC_CHAR); }
  .              { yylval.number = (unsigned char)yytext[0]; BEGIN(SC_CHAR); }
}
```

### 4D — Identifiers and keyword lookup

```flex
[a-zA-Z_][a-zA-Z0-9_]*  {
    ident_hash_elem_t *ihe = lookup_ident(yytext);
    if (ihe) {
        if (ihe->token & IHE_RESWORD) {
            if (function_flag) {
                function_flag = 0; add_input(yytext);
                push_function_context(); return L_FUNCTION_OPEN;
            }
            yylval.number = ihe->sem_value;
            return ihe->token & TOKEN_MASK;
        }
        yylval.ihe = ihe;
        return L_DEFINED_NAME;
    }
    if (function_flag) { function_flag = 0; add_input("(:"); }
    yylval.string = scratch_copy(yytext);
    return L_IDENTIFIER;
}
```

---

## Phase 5 — Buffer, Include Stack, Heredoc, Pragma

### 5A — `YY_INPUT` stress test (add to `test_lexer.cc`)

```cpp
TEST_F(LexerTest, LargeFile_RingBufferRefill) {
    std::string s = "int x = 0;\n";
    while (s.size() < 70000) s += "int x = 0;\n";  // > DEFMAX
    auto toks = Tokenize(s);
    ASSERT_FALSE(toks.empty());
}
```

### 5B — Heredoc start conditions

```flex
%x SC_HEREDOC_TEXT SC_HEREDOC_ARRAY
static char s_heredoc_term[MAXLINE];

"@@"[a-zA-Z_][a-zA-Z0-9_]*/[ \t\n] {
    snprintf(s_heredoc_term, sizeof(s_heredoc_term), "%s", yytext+2);
    lpc_heredoc_reset(); BEGIN(SC_HEREDOC_ARRAY);
}
"@"[a-zA-Z_][a-zA-Z0-9_]*/[ \t\n] {
    snprintf(s_heredoc_term, sizeof(s_heredoc_term), "%s", yytext+1);
    lpc_heredoc_reset(); BEGIN(SC_HEREDOC_TEXT);
}
<SC_HEREDOC_TEXT,SC_HEREDOC_ARRAY>{
  ^[a-zA-Z_][a-zA-Z0-9_]*\n {
      if (strncmp(yytext, s_heredoc_term, strlen(s_heredoc_term)) == 0
          && yytext[strlen(s_heredoc_term)] == '\n') {
          if (YY_START == SC_HEREDOC_TEXT) {
              yylval.string = lpc_heredoc_finish_text();
              BEGIN(INITIAL); return L_STRING;
          } else {
              BEGIN(INITIAL); return lpc_heredoc_finish_array();
          }
      }
      lpc_heredoc_append(yytext, yyleng);
  }
  \n  { current_line++; lpc_heredoc_append("\n", 1); }
  .   { lpc_heredoc_append(yytext, 1); }
}
```

### 5C — Pragma and `#line`

```flex
^"#pragma"[\t]+[^\n]*\n{handle_pragma(yytext+8);current_line++;}
^"#line"[\t]+[0-9]+[^\n]*\n{handle_line_directive(yytext);current_line++;}
```

### 5D — EOF / include-stack

```flex
<<EOF>> {
    if (inctop) {
        lpc_pop_include_state();  // extracted from yylex_inner EOF case
    } else {
        return -1;
    }
}
```

After Phase 5: remove catch-all rule from `lex.l`; remove `yylex_inner()` from `lex.cc`.

---

## Phase 6 — Maximize Native Flex Scanning

### Goal
Several Phase 4/5 conversions deliberately kept their *algorithm* in a
hand-written `lex.cc` helper and used Flex only as a *trigger* (match one
character, rewind `outp`, call the helper, flush). That was the right
tradeoff to get `yylex_inner()` eliminated safely and quickly, but it means
Flex's DFA is doing less of the actual scanning than it could. This phase
revisits each remaining trigger-and-delegate helper and asks: can this
become genuine Flex rules (possibly using exclusive start conditions,
`%x`) so Flex itself walks the characters, with no `outp`-touching helper
call at all (or a much smaller one)?

Not every remaining helper is a good candidate — the risk/value tradeoff
varies a lot:

| Helper | Candidate for full native conversion? | Why |
|---|---|---|
| `parseFunctionPointerParam()` (`$N`) | **Yes — trivial** | Pure digit accumulation, no line-counting, no ring-buffer edge cases |
| `parseOpenParen()` (`(`, `({`, `([`, `(:`, `(::`) | **Yes** | Whitespace-skip + single-char dispatch; the `(:` tail still ends in the same `outp`-touching `lpc_lex_resolve_identifier()` call `Phase 4`'s plain-identifier rule already uses, but the *scanning* around it (whitespace skip, identifier match) becomes native |
| `parseCharLiteral()` (`'c'`) | **Yes** | Bounded grammar (one escape or one raw byte, then a closing quote); maps cleanly onto two exclusive start conditions |
| `parseStringLiteral()` (`"..."`) | **Yes — converted in the Phase 6 native-conversion round 2** (see below) | Originally deferred here as "~250 lines, high risk, low marginal value until profiling justifies it." Revisited later in the same session at the user's request to keep pushing native conversion further; turned out tractable using the same per-escape-rule architecture as `parseCharLiteral()`, plus a plain `std::string` accumulator instead of replicating the original's two-phase scratchpad allocator |
| `parseHeredoc()` (`@TERM`/`@@TERM`) | **No** (`skip_comment()`/comments and `parseDirective()`/directives *were* converted — see below) | Terminator is a *runtime* string, not a compile-time pattern — genuinely not expressible as a static Flex regex, unlike everything else in this table. Still true after the round-2 conversions below; this is the one helper in `lex.cc` that structurally can't become native Flex without emulating dynamic pattern matching by hand, which isn't a meaningful win over just keeping it as C++ |
| `parseDirective()` (`#pragma`/`#line`) | **Yes — converted in the Phase 6 native-conversion round 2** (see below) | Originally deferred as a `yytext`-scratch-space risk; turned out to decompose cleanly into keyword-recognition rules (native) handing a captured tail off to ordinary (non-`outp`-touching) C++ parsing, no scratch-space reuse needed |

### Verification
Same as every prior phase: `ctest --test-dir build --output-on-failure`
plus a full mudlib `-ftest` boot, run after *each* individual conversion
before moving to the next.

---

## Phase 7 — Delete `lex.cc`, Final Cleanup — ✅ done, in simplified form

> **Superseded plan, kept for history:** the original idea below was to
> split `lex.cc` into two new files, `lex_helpers.cc` (identifier hash,
> include state, buffer bridge, errors) and `lex_string.cc` (string/heredoc
> buffer accumulator helpers). That two-file split made sense when it was
> written, but by the time this phase was actually reached (in the same
> session as the "Phase 6, round 2" write-up above), `parseStringLiteral()`
> had already been converted to native Flex and deleted entirely — so
> there was no "string buffer accumulator" left to give its own file, only
> the heredoc one (`get_array_block()`/`get_text_block()`), which isn't
> large enough on its own to justify a second file. What actually happened:
> a straight rename, `lex.cc` → `lex_util.cc`, no split. `lex.h` (the public
> API) was left unrenamed on purpose — only the implementation file moved.
> See the "Phase 6, round 2" section above for the heredoc
> "@"/"@@"-prefix-and-terminator conversion that shrank `lex.cc` down to
> the point where a rename (rather than a split) was the right call, and
> the dead-code cleanup (nexpands, expands[]/expand_depth/EXPANDMAX,
> defines_need_freed, deltrail(), lex_breakpoint(), and the
> MLEN/NSIZE/EXPANDMAX/NARGS/MARKS macros in lex.h — all leftovers from the
> pre-Flex, pre-standalone-preprocessor lexer with zero remaining callers)
> that happened alongside it.
>
> What's left in `lex_util.cc` is, per the goal stated for this round:
> global lexer state (`current_line`, `outp`, `pragmas`, the identifier
> hash table, the `linked_buf_t` ring, etc.) and helper functions lex.l
> calls into. Every remaining helper is there for one of two reasons: it's
> structurally unconvertible (heredoc body matching against a
> runtime-supplied terminator; include-stack-pop bookkeeping off Flex's own
> `<<EOF>>`), or it was never "scanning" to begin with (identifier hash
> table maintenance, `handle_pragma()`'s semantic bitmask logic,
> `start_new_file()`/`end_new_file()` setup/teardown). `yylex_inner()` was
> already fully removed back in Phase 5.
>
> Verification for this final push: a *fresh* CMake reconfigure (not just
> an incremental rebuild — renames are exactly the kind of change an
> incremental build can silently paper over) plus a full rebuild, the full
> gtest suite, and a full mudlib `-ftest` boot, all clean. The repo's
> default `build/` directory already had `-fsanitize=address,undefined` in
> its compile options the entire time this work was done (confirmed via
> `cmake -B build` output), so every verification pass in both this
> section and "Phase 6, round 2" above was already ASan-instrumented
> without a separate `build_san` directory ever being needed — and it's
> exactly what caught the `scratch_copy_string()` bug during the string
> literal conversion. The Flex-absent-fallback build
> (`-DFLEX_EXECUTABLE=...`) was attempted but left **inconclusive**: this
> CMake/FindFLEX version doesn't validate the executable at configure time
> (`-DFLEX_EXECUTABLE=/nonexistent` is silently accepted as "found," only
> failing at actual build time with an unhelpful generator-expression
> error), and `find_package(FLEX)` appears to search well-known system
> directories independent of `PATH`, so simply removing `flex` from `PATH`
> didn't hide it either. Getting a clean "Flex genuinely absent" signal
> would need actually uninstalling/relocating the `flex` binary, which
> wasn't done here — this specific checklist item is still open.

### File disposition

| Action | File |
|---|---|
| **DELETE** | `compiler/internal/lex.cc` |
| **NEW** | `compiler/internal/lex_helpers.cc` / `lex_helpers.h` |
| **NEW** | `compiler/internal/lex_string.cc` / `lex_string.h` *(may exist from Phase 4)* |
| **MODIFY** | `src/CMakeLists.txt` — remove `"compiler/internal/lex.cc"` from `SRC` |
| **MODIFY** | `compiler/internal/lex.h` — remove `yylex_inner()` |

### What moves into `lex_helpers.cc`

`init_identifiers()` · `find_or_add_ident()` · `find_or_add_perm_ident()` ·
`lookup_ident()` · `quick_alloc_ident_entry()` · `free_unused_identifiers()` ·
`show_error_context()` · `prepare_logs()` · `lpc_pop_include_state()` ·
`lpc_lex_yy_input()`

### Final file layout

```
src/compiler/internal/
  lex.l               ← Flex source (single source of truth for DFA)
  lex.autogen.cc      ← committed pre-generated fallback
  lex.h               ← unchanged public API
  lex_util.cc         ← ACTUAL: global lexer state + all remaining helpers
                         (identifier hash, include state, buffer bridge,
                         errors, heredoc body matching) -- see the note
                         above for why this replaced the planned
                         lex_helpers.cc/lex_string.cc split
  lexer_utils.cc/h    ← (existing) include paths, predefines
  compiler_utils.cc/h ← (existing) init_instrs, smart_log
  preprocessor.cc/h   ← (existing) standalone preprocessor
  compiler.cc/h       ← (existing) compile_file, prolog/epilog

src/tests/
  test_lexer.cc       ← 78 token-level unit tests (see Phase 2 for the
                         original ≥116 target and Phase 6's test-quality
                         pass for why fewer, higher-signal tests replaced it)
  test_compiler.cc    ← (existing) preprocessor tests
  test_lpc.cc         ← (existing) full-pipeline LPC tests
```

### Phase 6 Verification

```bash
cmake --build build -j 12
ctest --test-dir build --output-on-failure
cmake -B build_san -DENABLE_SANITIZER=ON && cmake --build build_san -j 12
ctest --test-dir build_san --output-on-failure
cmake -B build_noflex -DFLEX_EXECUTABLE=/dev/null && cmake --build build_noflex -j 12
cd testsuite && ../build/src/driver etc/config.test -ftest
```

---

## Risk Mitigation

| Risk | Mitigation |
|---|---|
| Flex absent on CI | Pre-generated `lex.autogen.cc` fallback committed; CI tests both paths |
| `yytext` overflow | `%option array` + `YYLMAX=MAXLINE=4096`; scratchpad overflow path unchanged |
| Conflicting Flex symbols | `%option noyywrap nounput noinput` |
| `yylineno` vs `current_line` | Do NOT use `%option yylineno`; `current_line` remains authoritative |
| `linked_buf_t` + `add_input()` interaction | `lpc_lex_yy_input()` is the sole bridge; `add_input()` stays in `lex_helpers.cc` |
| Grammar include-order constraint | `grammar_rules.h` before `grammar.autogen.h` enforced in `lex.l` prologue |

---

## Tracking Checklist

### Phase 1
- [x] Rename `yylex()` → `yylex_inner()` in `lex.cc`; update `lex.h`
- [x] Create `compiler/internal/lex.l` with catch-all shim
- [x] Add `lpc_lex_yy_input()` bridge to `lex.cc`
- [x] Add `find_package(FLEX)` + `FLEX_TARGET` + fallback to `src/CMakeLists.txt`
- [x] Add `${GENERATED_LEXER_FILE}` to `SRC` list
- [x] Add post-build copy of `lex.autogen.cc` to source tree
- [x] Generate and commit `lex.autogen.cc`
- [x] Verify: build + 121 unit tests + mudlib testsuite

### Phase 2
- [x] Create `src/tests/test_lexer.cc` with `Tokenize()` helper
- [x] Add all ≥116 token test cases (121 tests)
- [x] Add `lexer_tests` to `src/tests/CMakeLists.txt`
- [x] Verify: ≥116 lexer tests pass against `yylex_inner()`

### Phase 3
- [x] Add whitespace / comment / operator rules to `lex.l`
- [x] Delete corresponding cases from `yylex_inner()` in `lex.cc`
- [x] Verify: all test suites pass (242 gtest cases + full mudlib `-ftest` boot)

> **Deviations from the original Phase 3 sketch, found the hard way:**
> - Real `YY_INPUT`/`YY_FLUSH_BUFFER` wiring was required in Phase 3 itself
>   (not deferred to Phase 5) since explicit Flex rules actually invoke
>   Flex's DFA/buffer machinery for the first time.
> - `lpc_lex_yy_input()` supplies **one byte per call** (not a bulk fill).
>   Flex's DFA always peeks one lookahead byte past any match, even
>   unambiguous single-char ones; bulk-filling let `refill_buffer()`
>   relocate/compact the ring buffer mid-fetch, which broke naive
>   pointer-difference rewinds.
> - Added `DEFER_TO_LEGACY_SCANNER()` / `YY_PENDING_LOOKAHEAD()` in `lex.l`
>   to rewind `outp` by exactly the matched text plus any buffered-but-
>   unconsumed lookahead before handing off to `yylex_inner()`.
> - `outp == last_nl + 1` refill triggers reachable from Flex-driven code
>   (`lpc_lex_yy_input()`, `lpc_lex_newline()`) were changed to `>=`: the
>   one-byte DFA lookahead can push `outp` past the exact boundary,
>   silently skipping the `==` check and leaving the ring buffer stale.
> - Two `break;` statements inside `yylex_inner()`'s outer `switch` (the
>   `LEX_EOF` include-stack pop, and the `#line`/`#pragma` directive
>   handler) used to fall through and re-dispatch on the next character
>   using the *same* switch — which no longer has whitespace/operator
>   cases. Both now `return yylex();` to re-enter Flex's top-level scanner
>   instead of looping internally.

### Phase 4
- [x] Add numeric literal rules to `lex.l`
- [x] Add `\"` trigger rule to `lex.l` for string literals (see deviation note;
  `SC_STRING` start-condition / `lex_string.cc` extraction NOT done — deferred)
- [x] Add `'` trigger rule to `lex.l` for char literals (see deviation note;
  `SC_CHAR`/`SC_CHAR_ESC` start-conditions NOT done — deferred, same rationale as strings)
- [x] Add identifier / keyword lookup rule to `lex.l`
- [x] Add `$` trigger rule to `lex.l` for function-pointer `$N` parameters
  (not in original plan scope, but same low-risk shape — bonus item)
- [x] Remove `parseHexIntegerLiteral`, `parseBinaryIntegerLiteral` from `lex.cc`
  (`parseStringLiteral`/`parseCharLiteral`/`parseFunctionPointerParam` kept — see deviation notes)
- [x] Verify: 245/245 gtest cases + full mudlib `-ftest` boot pass (all Phase 4 items)

> **Char literals deviation:** same rationale and shape as the string
> literals deviation directly below. The `case '\''` body (escape handling
> for `\n \t \r \b \a \e \' \" \\`, octal, `\xHH`, unknown-escape warning)
> was extracted verbatim into a standalone `parseCharLiteral()` in `lex.cc`
> (bug-for-bug identical, including a pre-existing `if ((outp = last_nl + 1))`
> assignment-not-comparison in the `\<newline>` branch — left untouched
> rather than "fixed", to avoid an unintended behavior change during a
> mechanical migration) and wired up via an explicit `"'"` Flex rule using
> the same rewind/call/flush shape as `parseStringLiteral()`.

> **Function-pointer `$N` params deviation — had to actually fix something
> during extraction (not just relocate):** `case '$'`'s digit-accumulation
> used the `SAVEC` macro, which expands to `if (...) *yyp++ = c; else {
> lexerror(...); break; }`. The bare `break` only compiled in the original
> because the very first `SAVEC` call sat directly inside the enclosing
> `switch (c = *outp++)` of `yylex_inner()` — the `break` targeted *that*
> switch, silently abandoning the token and letting the outer `for(;;)`
> loop redispatch on the next character. Extracted into a standalone
> `parseFunctionPointerParam()`, that first `SAVEC` call is no longer inside
> any switch/loop at all, so `break` doesn't compile (confirmed: real
> compiler error, not a lint nit). Rewrote the accumulation loop with an
> explicit bounds check that `return`s `yylex()` on overflow instead of
> `break`ing — the correct translation of "abandon this token, resume the
> outer dispatch loop" now that the outer dispatch loop is Flex's, not
> `yylex_inner()`'s. Behavior for well-formed input (the overwhelming
> majority) is unchanged; only the pathological "$" followed by 4000+
> digits case takes a different (but equivalent) path back into the lexer.

> **String literals deviation — kept the algorithm, changed only the
> trigger:** `parseStringLiteral()` is ~250 lines handling UTF-8 validation,
> `\n \t \r \b \a \e \" \\` escapes, octal/hex escapes, `\uXXXX`/`\UXXXXXXXX`
> unicode escapes with surrogate-pair handling, AND a two-phase scratchpad
> allocator (`scr_tail`/`scr_last`/`scratch_end`/`scratch_large_alloc`,
> falling back to accumulating overflow into `yytext` when the current
> scratch chunk is full). Rewriting all of that as native Flex
> start-condition rules (the `SC_STRING` sketch in this plan) is a
> substantial, high-risk undertaking on its own. Given the function already
> discards its `c` parameter and does 100% of its own reading directly from
> `outp` (never touching `yytext` or any Flex state up front — same shape as
> `skip_comment()`), it was left completely unchanged and instead wired up
> the same way `skip_comment()` was in Phase 3: an explicit `\"` Flex rule
> rewinds `outp` by `YY_PENDING_LOOKAHEAD()`, calls `parseStringLiteral('"')`
> unchanged, then flushes Flex's buffer. This gets `"` out of the `.`
> catch-all (matching the phase's intent) with zero algorithmic risk. A true
> `SC_STRING`/`lex_string.cc` rewrite remains open for a future pass if
> ever justified (e.g. by profiling showing the extra `yylex_inner()` hop
> matters).

> **Identifiers deviation — shared-label coupling:** the hand-written
> identifier-accumulation code wasn't only reached from the switch's
> `default:` case; the `'('` case's `"(: name"` function-pointer lookahead
> used `goto parse_identifier` to jump directly into the *same* code with
> `function_flag = 1` pre-set. Moving identifiers to Flex would have left
> that `goto` with no label to jump to. Fix: extracted the
> lookup/reserved-word/function-pointer-disambiguation logic (everything
> from `lookup_ident()` onward) into a standalone `lpc_lex_resolve_identifier()`
> in `lex.cc`, operating on the shared global `yytext`. Both the new
> `[a-zA-Z_][a-zA-Z0-9_]*` Flex rule and the `'('` case's (still hand-written,

> **Identifiers deviation — shared-label coupling:** the hand-written
> identifier-accumulation code wasn't only reached from the switch's
> `default:` case; the `'('` case's `"(: name"` function-pointer lookahead
> used `goto parse_identifier` to jump directly into the *same* code with
> `function_flag = 1` pre-set. Moving identifiers to Flex would have left
> that `goto` with no label to jump to. Fix: extracted the
> lookup/reserved-word/function-pointer-disambiguation logic (everything
> from `lookup_ident()` onward) into a standalone `lpc_lex_resolve_identifier()`
> in `lex.cc`, operating on the shared global `yytext`. Both the new
> `[a-zA-Z_][a-zA-Z0-9_]*` Flex rule and the `'('` case's (still hand-written,
> unmigrated) accumulation loop now call it. `function_flag` is provably
> always 0 on the Flex path (only ever set to 1 synchronously within the
> `'('` case, immediately before it does its own identifier accumulation
> and calls this same function — Flex is never re-entered in between), so
> the `add_input()`/`old_func()` raw-`outp` calls in the function-pointer
> branch are only ever reached from the legacy, non-Flex call site and
> need none of `DEFER_TO_LEGACY_SCANNER`'s pending-lookahead rewind.

> **Numeric literals deviation:** the hand-written scanner special-cased
> `N..M` (backing up out of a tentative float parse when a second `.`
> appeared) so ranges wouldn't be swallowed as `N.` floats. Replicated with
> two float rules instead of one: `[0-9](_?[0-9])*"."(_?[0-9])+` for the
> normal case, and `[0-9](_?[0-9])*"."/[^.]` (trailing context) for a bare
> trailing dot like `1.` that must NOT fire when followed by another `.`.
> Regression test: `LexerTest.Range_NotConfusedWithTrailingDotFloat`.

### Phase 5 — COMPLETE
- [x] Add large-file ring-buffer refill stress test to `test_lexer.cc`
  (done ahead of schedule in Phase 2 — `LexerTest.LargeFile_RingBufferRefill`)
- [x] Add heredoc rule to `lex.l` — done as an explicit `"@"` rule calling
  extracted `parseHeredoc()` (handles both `@TERM ... TERM` text blocks and
  `@@TERM ... TERM` array blocks; original `get_terminator()`/
  `get_array_block()`/`get_text_block()` logic kept verbatim, same
  Flex-triggered/legacy-algorithm shape as strings/chars — no `SC_HEREDOC_*`
  start-conditions needed)
- [x] Add `#pragma` / `#line` rules to `lex.l` — done as `^"#"` (beginning-of-line
  anchor, replacing the hand-written `*(outp-2)=='\n'` check) calling extracted
  `parseDirective()`, which still runs the original pragma/line parsing verbatim
  and ends with `return yylex();` to resume the top-level scanner
- [x] Add `<<EOF>>` include-stack rule — extracted into `parseEofOrIncludePop()`
  (both the include-stack-pop branch and the genuine-EOF `-1` branch)
- [x] Also migrated (ahead of schedule, at user's request to "finish off the
  yylex_inner main loop first"): `'('` compound-token lookahead (`({`, `([`,
  `(:`) extracted into `parseOpenParen()`, wired via an explicit `"("` Flex
  rule. This was explicitly called out in Phase 3 as staying in
  `yylex_inner()` "for now" — it didn't need to stay forever, it just needed
  `lpc_lex_resolve_identifier()` to exist first (Phase 4) so its `(: name`
  branch had something to call instead of `goto parse_identifier`.
- [x] Remove catch-all rule from `lex.l`; remove `yylex_inner()` from `lex.cc`
  — DONE. Once `'@'` and `LEX_EOF` were extracted, `yylex_inner()`'s switch
  had nothing left but `default: return lpc_lex_badlex(c);`, i.e. the
  function had degenerated to "read one char, report it as illegal" — so it
  was deleted outright. The catch-all `.` rule now calls
  `lpc_lex_badlex(yytext[0])` directly (no rewind/flush needed — that
  function only inspects the character value, it never touches `outp`).
  `<<EOF>>` now calls `parseEofOrIncludePop()` directly instead of
  `yylex_inner()`. `lex.cc` shrank from ~2840 to 2541 lines; every remaining
  function in it (`parseOpenParen`, `parseDirective`, `parseHeredoc`,
  `parseStringLiteral`, `parseCharLiteral`, `parseFunctionPointerParam`,
  `parseEofOrIncludePop`, `lpc_lex_badlex`, `skip_comment`,
  `lpc_lex_newline`, `lpc_lex_resolve_identifier`) is a standalone helper
  called directly from an explicit `lex.l` rule — there is no more
  switch-based dispatch loop anywhere in the lexer.
- [x] Verify: 245/245 gtest cases + full mudlib `-ftest` boot pass after
  *each* migration step (`'('`, `'#'`, `'@'`, `LEX_EOF`/`yylex_inner()`
  removal), individually, before moving to the next.

> **Phase 5 finished well ahead of the original plan's shape.** The
> original sketch had Phase 5 adding `SC_HEREDOC_*` start-conditions and a
> `lpc_pop_include_state()` extraction, then still leaving `yylex_inner()`
> itself for Phase 6 to delete. In practice, once Phase 4 built the
> "Flex-triggered rule calling a standalone legacy-algorithm function"
> pattern (proven safe on strings/chars) and Phase 3 built the buffer-sync
> primitives (`YY_PENDING_LOOKAHEAD`, flush-before-vs-after-ordering), every
> remaining `yylex_inner()` case turned out to fit the *same* pattern with
> no new machinery needed — including `'('`, which Phase 3 explicitly
> planned to leave behind permanently. Once every case had its own explicit
> Flex trigger, `yylex_inner()`'s dispatch loop had nothing left to
> dispatch, so removing it was the natural conclusion rather than a
> separate Phase 6 task. Phase 6 is now solely about the remaining
> file-organization goals (splitting `lex.cc` into `lex_helpers.cc`/
> `lex_string.cc`, sanitizer-clean build, Flex-absent fallback test) — see
> below for what's genuinely left.

> **Sequencing bug found and fixed while wiring up `'#'`:** unlike every
> other lex.l-triggered helper (`skip_comment`, `parseStringLiteral`,
> `parseCharLiteral`, `parseFunctionPointerParam`, `parseOpenParen`),
> `parseDirective()` itself calls `yylex()` *recursively* at the end (to
> resume top-level scanning once the directive line is consumed — same
> reason the `LEX_EOF` include-pop branch and the old `case '#':` both did
> `return yylex();` after Phase 3). The established rule shape
> (`outp -= YY_PENDING_LOOKAHEAD(); int result = fn(); YY_FLUSH_BUFFER;
> return result;`) flushes *after* calling the helper — fine when the
> helper doesn't touch Flex, wrong here, because `parseDirective()`'s
> internal `yylex()` call needs the buffer already flushed. Symptom before
> the fix: `SetUpTestSuite()` failed loading `/single/simul_efun` with
> `/tests.h line 0: syntax error, unexpected L_IDENTIFIER` — a `#line`
> marker was processed, but the token immediately after it came out
> corrupted because it was scanned against Flex's stale buffer state. Fix:
> flush *before* calling `parseDirective()`, then `return parseDirective();`
> directly (no separate `result` variable). Any future helper that itself
> calls back into `yylex()`/Flex must use this flush-before, not
> flush-after, ordering.

### Phase 6 — COMPLETE
- [x] Convert `$N` function-pointer params to fully native Flex rules; delete `parseFunctionPointerParam()`
- [x] Convert `'('` compound tokens (`({`, `([`, `(:`, `(::`) to native Flex rules; delete `parseOpenParen()`
- [x] Convert char literals to native Flex start-condition rules; delete `parseCharLiteral()`
- [x] Verify: full gtest suite (254 tests, 9 new) + mudlib `-ftest` boot pass after each conversion
- [x] Add regression tests for previously-untested paths this phase exercises for the first time
  (`(::`/scope-operator disambiguation, `(:`/anonymous function-pointer open, array/mapping-open
  with embedded whitespace, unknown char escape, `\'` escape)

> **Net effect:** `lex.cc` shrank from 2,541 to 2,333 lines (three more
> hand-written helpers deleted: `parseFunctionPointerParam()`,
> `parseOpenParen()`, `parseCharLiteral()`). `lex.l` grew from ~278 to ~473
> lines, all of it genuine Flex rules/start-conditions doing real scanning
> work, not trigger-and-delegate shims. `function_flag` and `lexerror()`
> had to be promoted from file-local `static` to externally-linked (with
> `extern` declarations added to `lex.h`) since native `lex.l` rules now
> call them directly — previously only `lex.cc`-internal helpers needed them.

> **`$N` deviation:** none of substance. Fully native, two rules
> (`"$"[0-9]+` and bare `"$"`), zero `outp` access. The only behavior
> difference from the original is *how* an oversized digit run is detected:
> the original bounded the accumulation loop at `MAXLINE-5` chars one digit
> at a time; the native rule matches the whole digit run in one DFA pass,
> then checks `yyleng - 1 > MAXLINE - 5` after the fact. Same threshold,
> same `lexerror("Line too long")` + abandon-token outcome, just checked
> post-match instead of mid-accumulation.

> **`'('` compound-token deviation:** whitespace between `(` and `{`/`[`/`:`
> is matched by the regex unconditionally (regexes can't see the WOMBLES
> runtime config flag at compile time); when WOMBLES is on *and* whitespace
> was actually consumed, the action rejects the match with `yyless(1)` and
> returns a bare `'('`, falling back to exactly the same token stream the
> original's womble-mode raw-byte check would have produced. The `(::`
> case (scope operator, not function-pointer open) collapses to a single
> `yyless(1); return '(';` regardless of WOMBLES, since both the
> WOMBLES-violation path and the "found a second colon" path in the
> original converge on the same observable outcome (consume only `'('`,
> re-scan the rest). The `(:name` named-function-pointer tail still ends in
> the same `outp`-touching `lpc_lex_resolve_identifier()` call the original
> used (with `function_flag=1`), wrapped in the same rewind/flush pattern
> already established for the plain identifier rule's hypothetical
> function-pointer path — this one function remains the only place this
> whole conversion still touches `outp` directly, and it was already
> documented as out of scope for a full audit (see the Phase 4 "Identifiers
> deviation" note).

> **Char literal deviation — one intentional, one accidental (fixed
> mid-implementation):**
> - *Intentional:* `\8` / `\9` (an octal escape with a non-octal first
>   digit) reports `lexerror("Illegal character constant")` directly,
>   instead of replicating the original's incidental path there (`strtoll()`
>   consuming zero digits, then failing the *next* closing-quote check for
>   the same underlying reason). Same user-visible error, different internal
>   route to it — this corner is deep enough (nobody writes `'\8'`) that
>   bug-for-bug fidelity wasn't worth chasing.
> - *Positive side effect, not a deviation in outcome:* the original's
>   `\<newline>` branch had a documented pre-existing bug (`if ((outp =
>   last_nl + 1))` — assignment where a comparison was clearly intended,
>   kept bug-for-bug through the Phase 4 extraction). That bug is physically
>   impossible to reproduce in the native rule, because Flex's own
>   `YY_INPUT` bridge (`lpc_lex_yy_input()`) already calls `refill_buffer()`
>   for every byte it hands to Flex — including the two bytes matched by
>   `"\\\n"` — so the manual `outp`/`refill_buffer()` dance this branch used
>   to need is simply gone, not fixed-in-place.
> - **Post-Phase-6 follow-up: WOMBLES removed entirely.** The `wombles`
>   runtime config (strict "no whitespace between `(` and `{`/`[`/`:`, or
>   between `}`/`]`/`:` and `)`" dialect) was judged not worth keeping —
>   default-off, low value, and it was the only thing keeping the `'('`
>   compound-token rules above runtime-conditional. Removed from the driver
>   entirely: `__RC_WOMBLES__` renamed to a retired `__RC_INT_30__` slot in
>   `runtime_config.h` (never reused, per that file's own backwards-compat
>   rule -- matches the existing `__RC_INT_1__`/`__RC_INT_18__`/`__RC_INT_22__`/
>   `__RC_INT_23__` placeholders for prior retirements), dropped from
>   `rc.cc`'s `INT_FLAGS[]` table, and a `scan_config_line("wombles : %d\n",
>   tmp, K_WARN_FOUND)` line added to rc.cc's existing "obsolete config
>   lines" block so old config files still setting `wombles` get the
>   standard obsolete-line warning instead of silently doing nothing. The
>   runtime `CONFIG_INT(__RC_WOMBLES__)` checks were deleted outright (not
>   just made unconditional) from `lex.l`'s three `'('` rules, from the five
>   `*(outp - 2) != ...` closing-delimiter checks in
>   `grammar_rules_exprs.cc`/`grammar_rules_types.cc`, and from the
>   `add_predefine("__WOMBLES__", ...)` call in `lexer_utils.cc`. Config
>   docs regenerated (`python3 docs/gen_config_docs.py`); `wombles : 0`
>   dropped from `testsuite/etc/config.test` and `src/Config.example`.
>   Verified with a clean rebuild, 254/254 `lexer_tests` passing, and a full
>   mudlib `-ftest` boot with no new warnings. Net effect: the `'('`
>   compound-token rules in `lex.l` are now unconditionally native with no
>   remaining runtime-config branch.
> - **Second post-Phase-6 follow-up: WARN_TAB removed the same way.** The
>   `warn tab` config (a warning-only `\t` check) was retired using the
>   identical pattern: `__RC_WARN_TAB__` → placeholder `__RC_INT_29__` in
>   `runtime_config.h`, dropped from `rc.cc`'s `INT_FLAGS[]`, an obsolete-line
>   `scan_config_line("warn tab : %d\n", tmp, K_WARN_FOUND)` added, `warn tab`
>   dropped from `testsuite/etc/config.test`/`src/Config.example`, docs
>   regenerated. In `lex.l` this collapsed the standalone `\t` rule into the
>   existing whitespace-ignore rule (`[ \f\v\r]` → `[ \t\f\v\r]`), removing
>   the last `CONFIG_INT()` branch from the whitespace/comment section of the
>   scanner.
> - **Test-file quality pass (`test_lexer.cc`).** Reworked alongside the
>   above: mechanical single-shape tokens (single/double-char operators,
>   punctuation, reserved words, plain identifiers) were consolidated from
>   one `TEST_F` per token into a handful of table-driven tests, since Flex
>   matches each with a literal-string-pattern-and-return with essentially
>   one shared failure mode per table. That freed up room to add real
>   coverage for corners that had none before: WOMBLES-era whitespace
>   handling is gone so those cases were dropped entirely; new tests added
>   for Unicode surrogate-pair decoding (`😀` → 😀) and its two
>   malformed-surrogate recovery paths, the scratch-buffer-boundary crossing
>   in `parseStringLiteral()` (a 300-byte string, past its 255-byte first
>   loop), string line-continuation, `$0`/`$256`/bare-`$`-outside-context
>   function-pointer-parameter errors, char-literal octal/hex overflow and
>   the `\8`/`\9` deviation, heredoc array form (`@@TERM`) and illegal-
>   terminator recovery, nested `/* /* */` comment warnings vs. genuine
>   unterminated-comment/string/heredoc EOF errors, a `#` not at line start,
>   and a `#line` directive's effect on `current_line`. All of these lean on
>   `lexerror()`/`yyerror()` logging and returning rather than longjmp'ing
>   (confirmed via `compiler.cc`'s `yyerror()` — no `setjmp`/`longjmp`
>   anywhere in this path), and on `num_parse_error` being reset to 0 at the
>   top of every `Tokenize()` call, so tests just assert `num_parse_error >
>   0` after a deliberately malformed source rather than needing any
>   exception harness. Net effect: registered `lexer_tests` case count went
>   from 254 down to 67 (fewer, higher-signal `TEST_F`s — several of those
>   still run multiple table rows internally), all passing, with genuinely
>   more code-path coverage than before.
> - *Accidental, caught before landing:* an early draft's comment for the
>   `[^\\]` raw-byte rule used backtick-quoted C-snippets containing literal
>   `{ }` and an adjacent `''` — Flex's brace-matching scanner for action
>   code apparently doesn't fully understand `//` comment content, and
>   misparsed the comment as unbalanced code, corrupting parsing of every
>   following rule ("unrecognized rule" errors at lines with no rule on
>   them). Confirmed by simplifying the comment (removing the embedded
>   braces/quotes) and rebuilding clean. **Lesson for future `.l` edits:**
>   avoid literal `{`/`}`/`'` characters inside comments that live inside a
>   Flex action block; prefer prose over inline code snippets there.
> - Also worth noting: this phase caught a **process bug in verification**,
>   not in the lexer — an earlier "clean build, tests still pass" checkpoint
>   for this exact change was actually silently re-testing a stale binary,
>   because the build command ran from `testsuite/` (a leftover `cd` from
>   the previous mudlib-boot check) where `build/` doesn't exist, and the
>   resulting CMake error didn't match the `grep -i "error:"` filter being
>   used to scan build output. Re-verified from a `rm`-cleaned
>   `lex.autogen.cc` and a directory-confirmed build afterward. Lesson: use
>   absolute paths for build commands in this workflow, or `pwd`-check
>   before trusting a filtered build result.

### Phase 6, round 2 — comments, strings, directives (post-WOMBLES/WARN_TAB)

> Prompted by "continue to look over lex.cc and try to port more native
> parsing to lex.l" after the WOMBLES/WARN_TAB config removals above.
> Revisits the three helpers the original Phase 6 decision table deferred
> ("not this phase"): `skip_comment()`, `parseStringLiteral()`,
> `parseDirective()`. `parseHeredoc()`/`get_terminator()`/`get_array_block()`/
> `get_text_block()` and `parseEofOrIncludePop()` were reassessed too and
> confirmed to stay manual (see the updated decision table above and the
> heredoc/EOF-pop note at the end of this section). Net effect: `lex.cc`
> shrank from 2,333 to 1,777 lines; `lex.l` grew from ~473 to ~739 lines.
> `test_lexer.cc` grew from the 67-test Phase 6 cleanup pass to 74 tests
> (still fewer than the original 254, still more code-path coverage). Full
> gtest suite (195 tests, ASan-instrumented) and a full mudlib `-ftest` boot
> verified after each of the three conversions below, each committed
> separately.
>
> **Comments (`skip_comment()` → `SC_BLOCK_COMMENT`).** Converted first as
> the lowest-risk of the three: bounded grammar, fixed closing delimiter
> (`*/`, unlike a heredoc's runtime terminator). One real subtlety: the
> original's "`/*` found in comment" nested-comment warning fires whenever
> a `*` is encountered that was immediately preceded by `/`, checked byte-
> by-byte regardless of what else was scanned. A naive `[^*\n]+` catch-all
> for ordinary comment text would let Flex's longest-match rule swallow a
> `/` together with whatever preceded it, splitting it away from a
> following `*` and silently breaking that detection — fixed by excluding
> `/` from the catch-all and giving it (and `"*"+"/"`, for runs of stars
> before the close, e.g. `**/`) their own rules. **Discovery that reframed
> the rest of this round:** the standalone preprocessor already strips
> *all* comments — block and line — from its output before the lexer ever
> runs (`LpcPreprocessor::run()`; also directly confirmed by
> `test_compiler.cc`'s existing preprocessor tests). So `skip_comment()`
> was already dead code in the real compile pipeline before this
> conversion touched it, and `SC_BLOCK_COMMENT` is too, after — kept anyway
> for grammar self-consistency (Flex's `nodefault` option, and any
> hypothetical direct/non-preprocessed use of the lexer), and documented
> prominently in lex.l's file header so this isn't rediscovered as a
> surprise later. This discovery motivated actually checking reachability
> for the other two conversions rather than assuming it — strings and
> directives were confirmed genuinely live (see below) before investing in
> them. Side effect: converting away `skip_comment()`'s only caller,
> `cmygetc()`, revealed `cmygetc()` itself was *also* already dead (never
> called from anywhere), along with `skip_line()` (`cmygetc()`'s only other
> caller), the `GETALPHA`/`get_next_char` macros (`cmygetc()`-only), and two
> forward declarations (`refill_on_continuation`, `exgetc`) for functions
> that were never defined at all — all deleted as an adjacent cleanup, ~105
> lines.
>
> **Strings (`parseStringLiteral()` → `SC_STRING_BODY`).** The real prize
> of this round: confirmed genuinely live first (the preprocessor passes
> quoted text through byte-for-byte, only avoiding macro-expansion inside
> it — no stripping, unlike comments), then converted using the same
> per-escape-rule architecture proven on `parseCharLiteral()` in the
> original Phase 6, plus one new piece: a plain `static std::string
> str_accum` accumulator (declared in lex.l's prologue) instead of
> replicating the original's small-scratch/yytext-spillover two-phase
> buffer — that whole dual-buffer design existed purely to avoid a malloc
> for short strings using the pre-existing scratchpad ring machinery
> in-place; with a real growable accumulator there's nothing left for it to
> optimize around. The unbounded octal/hex escape digit runs and the
> UTF-16 surrogate-pair `\u` decoding (matched as one 12-character
> lead+trail unit when well-formed, `char lead_buf[5]`/`trail_buf[5]`
> extracted by fixed `yytext` index since Flex's own longest-match already
> guarantees the 12-char form wins over the 6-char lone-`\u` form whenever
> both apply) were the trickiest parts, but followed directly from the
> char-literal precedent. Three deviations, documented inline and here:
> - *Intentional:* `\8`/`\9` in a string (invalid octal digits) now reports
>   a clean error. The original's `strtoll()` there consumed zero
>   characters and, via an implicit fallthrough nobody could have intended,
>   appended a NUL byte to the string followed by the stray literal digit —
>   an obscure embedded-NUL quirk, not deliberate behavior worth preserving
>   (same reasoning as the char-literal `\8`/`\9` deviation from the
>   original Phase 6).
> - *Intentional:* several of the original's distinct error messages for
>   the different ways a `\u` escape can fail to form a valid surrogate
>   pair (missing trail, wrong digit count, lone trail, etc.) are
>   consolidated into two ("Missing surrogate trail" / "expecting surrogate
>   lead, got trail"). Also, a malformed `\u`/`\U` (fewer than the required
>   4/8 hex digits) now leaves the non-hex characters that follow to be
>   scanned as ordinary string content, where the original silently
>   discarded them — nobody writes malformed surrogate escapes on purpose,
>   and both still report a clear error and recover.
> - *Fixed as a side effect, not preserved:* the original only decoded
>   `\u`/`\U` correctly within the first 255 bytes of a string (its
>   small-scratch loop); past that boundary, its second (yytext-spillover)
>   loop's escape `switch` had no `'u'`/`'U'` cases at all, so those
>   escapes silently fell through to "unknown escape" treatment instead of
>   being decoded — an asymmetry inherent to having two separate,
>   hand-duplicated escape-decoding loops. The single accumulator here has
>   no such split, so `\u`/`\U` decode identically regardless of string
>   length.
>
> Caught by the gtest suite's ASan build before it ever reached
> `testsuite/`: a global-buffer-overflow inside `scratch_copy_string()`,
> crashing on the very first string literal compiled during
> `LexerTest::SetUpTestSuite()` (i.e. real mudlib code, not a test-only
> corner). Root cause: `scratch_copy_string()` turned out not to be a
> generic "copy this string into scratchpad memory" function at all — it
> **re-decodes backslash escapes itself** and unconditionally skips the
> first input byte (`s++` at the top), because it exists specifically for
> heredocs, whose intermediate representation (`get_text_block()`) is raw
> text with escape sequences it *added itself* (e.g. literal `\n` for each
> embedded newline), not an already-decoded string. Passing it an
> already-decoded `str_accum` corrupted short/empty strings immediately.
> Fixed by switching to `scratch_copy()` — the actual "copy this string
> as-is" allocator, the same one `lpc_lex_resolve_identifier()` already
> uses for plain identifiers. **Lesson:** a scratchpad helper's name isn't
> enough to infer its exact contract when there are several similarly-named
> ones (`scratch_copy` vs `scratch_copy_string` vs `scratch_large_alloc`)
> — read the implementation, not just the signature, before reusing an
> existing allocator in a new context. The ASan-instrumented `lexer_tests`
> binary catching this at `SetUpTestSuite()` time (before a single `TEST_F`
> even ran) rather than needing a targeted mudlib-boot repro is itself
> worth noting as a reason to always run the gtest suite (not just the
> mudlib boot) after touching allocation code here.
>
> **Directives (`parseDirective()` → `SC_DIRECTIVE_PRAGMA`/
> `SC_DIRECTIVE_LINE`/`SC_DIRECTIVE_SKIP`).** Also confirmed genuinely live
> first: unlike `#include`/`#define`/etc. (fully consumed by the
> preprocessor), it recognizes `#pragma`/`#line` by name but **re-emits
> them verbatim** into its output, and synthesizes its own `#line N
> "file"` markers around every `#include` — so this path is exercised on
> any file with an `#include`, which is most mudlib source. Converted by
> splitting into native keyword-recognition rules (`^"#"[ \t]*"pragma"`,
> `^"#"[ \t]*"line"[ \t]*`, plus a bare `^"#"` fallback for an unrecognized
> directive keyword) that hand a captured tail off to ordinary, non-`outp`-
> touching C++ parsing — sidestepping the original concern about reusing
> `yytext` as scratch space, since a plain local pointer walk over an
> already-captured, Flex-owned `yytext` needs no such care. `handle_pragma()`
> promoted from `static` to externally-linked (same pattern as
> `function_flag`/`lexerror()` from the original Phase 6), since it's now
> called from lex.l's `SC_DIRECTIVE_PRAGMA` rule in a separate translation
> unit. While touching it, fixed a latent out-of-bounds read
> (`str[strlen(str) - 1]` on an empty string) and bounded its
> trailing-whitespace-strip loop (same underflow risk for an
> all-whitespace argument) — the original's convoluted `outp` pointer
> arithmetic happened to never actually call it with an empty string in
> practice, but this more direct routing can (`"#pragma"` with nothing
> after it on the line, immediately followed by EOF or a bare newline).
> Two more deviations:
> - *Intentional, documented as a robustness improvement:* leading
>   whitespace after `pragma` is now skipped in full, not just a single
>   character. The original only skipped one whitespace char between
>   `"pragma"` and its argument (`char *sp = outp;` captured right after
>   the loop that read the `"pragma"` token consumed exactly one
>   terminating space), so `"#pragma  strict_types"` (two spaces) silently
>   failed to recognize the pragma at all in the original. Same for
>   `"#line5"` (no separating space) now parsing as a valid line directive
>   where the original's token-boundary scan (which reads a whitespace-
>   delimited word) treated the whole thing as one unrecognized token.
> - *Discovered, not a regression:* the keyword-less GCC-style `"# N
>   [\"file\"]"` line marker form — which the original hand-written
>   `parseDirective()` did support, and which this conversion's
>   `^"#"[ \t]+/[0-9]` rule still supports for grammar completeness — turns
>   out to be **unreachable end-to-end** through the real pipeline: the
>   standalone preprocessor's own directive-name extraction captures the
>   leading digit run as an (unrecognized) directive name rather than
>   leaving it empty for a bare-digit fallback, so the preprocessor itself
>   rejects `"# 50 ..."` with "Unknown preprocessor directive: #50" before
>   the lexer ever sees it. A preexisting preprocessor limitation, unrelated
>   to this conversion; not fixed here (out of scope), just documented —
>   there's a `test_lexer.cc` comment at the spot where a test for this
>   would otherwise go, explaining why there isn't one.
>
> **Why `parseHeredoc()`'s *body* matching and `parseEofOrIncludePop()`
> still aren't converted (updated below -- the `@`/`@@`-prefix-and-
> terminator part of `parseHeredoc()` WAS converted in a follow-up pass;
> see "Phase 6, round 3" further down).** A heredoc's closing terminator
> (`@TERM ... TERM`) is supplied by the LPC source itself at compile time —
> Flex patterns are compiled once, ahead of time, into a fixed DFA, so
> there's no way to match "whatever identifier followed the opening `@`"
> as a closing delimiter without hand-rolling dynamic pattern matching
> inside a rule action, at which point Flex isn't buying anything over
> just keeping `get_array_block()`/`get_text_block()`'s body-scanning loops
> as C++. This is structurally different from every other helper in this
> file (including the ones converted in this round), which all had *fixed*
> closing delimiters or bounded grammars. `parseEofOrIncludePop()` isn't
> really "scanning" at all — it's include-stack bookkeeping (popping a
> saved buffer position, restoring `current_line`/`current_file`)
> triggered by Flex's own `<<EOF>>` mechanism, with no character-level
> grammar to native-ify.

### Phase 6, round 3 — heredoc terminator, dead-code sweep, `lex.cc` → `lex_util.cc`

> Prompted by "continue to look at lex.cc, move lexing logic as much as
> possible to lexer.l, leaving only global state and helper, which all
> should be moved to lex_util.cc, and don't stop until you reach that
> goal." Picks up exactly where "round 2" left off: reassessed whether
> *any* part of `parseHeredoc()` could still go native (round 2 had
> written off the whole function), found that the "@"/"@@" prefix and
> terminator *identifier* genuinely could (only the body-vs-terminator
> matching is structurally stuck in C++), converted that, then did the
> file-level consolidation the instruction asked for.
>
> **Heredoc "@"/"@@" prefix + terminator (`get_terminator()` → native
> `SC_HEREDOC_TERM`).** `get_terminator()`'s own grammar turned out to be
> exactly the same shape as everything else converted in round 2: a fixed,
> bounded pattern (an alnum/underscore identifier, then optional horizontal
> whitespace, then a required newline or EOF) — nothing about *reading the
> terminator name* is dynamic, only *matching the heredoc body against it*
> is. Converted using the same architecture as `SC_FUNC_OPEN`: native rules
> recognize and capture the prefix and identifier into a
> `static std::string heredoc_terminator` (cleared by the trigger rule, so
> an empty terminator like `"@\n"` is still caught even though the
> identifier rule then never fires for it), then hand off to
> `parseHeredoc()` — whose signature changed from taking no arguments and
> reading `"@"`/`"@@"` plus the terminator itself via `outp`, to
> `parseHeredoc(const char *terminator, int is_array)`. `get_terminator()`
> itself is deleted; `get_array_block()`/`get_text_block()` (the genuinely
> unconvertible body-matching loops) are untouched. One deliberate,
> documented simplification: the original's exact sub-case distinction
> between "trailing whitespace only, then EOF" vs. "trailing garbage, then
> EOF" collapses into one `<SC_HEREDOC_TERM><<EOF>>` rule reporting
> "Illegal terminator" either way — matches the original's *outcome*
> (EOF anywhere before the trailing newline is always illegal, even past a
> well-formed terminator identifier) without replicating which of several
> equivalent code paths got there. Verified against the mudlib's own
> CRLF-heredoc test (`at_block_crlf.c`, which specifically exercises the
> `[ \t\v\f\r]*\n` trailing-whitespace-then-newline pattern) and its
> unterminated-heredoc negative test (`bad_at_block.c`), both unchanged in
> behavior.
>
> **Dead-code sweep.** With `get_terminator()` gone, did a full pass over
> what remained in `lex.cc` looking for anything else orphaned by this
> session's cumulative deletions (`skip_comment()`'s removal in round 1 had
> already revealed `cmygetc()`/`skip_line()`/two dangling macros/two
> never-defined forward declarations as dead — see round 1 above). Found
> four more, all leftovers from the pre-Flex, pre-standalone-preprocessor
> lexer with zero remaining callers or readers: `nexpands` (written in
> three places, read nowhere — the *reachable* macro-expansion-depth guard
> now lives entirely in the standalone preprocessor, `preprocessor.cc`,
> which has its own unrelated state), `expands[]`/`expand_depth` plus the
> `EXPANDMAX` macro that sized them, `defines_need_freed` (written once at
> declaration, never read or reassigned again), `deltrail()` and
> `lex_breakpoint()` (both fully unreachable — no callers at all; the
> `"#breakpoint"` directive `lex_breakpoint()`'s own comment refers to
> isn't implemented by any of this session's directive-handling rules,
> native or otherwise). Also dropped `MLEN`/`NSIZE`/`NARGS`/`MARKS` from
> `lex.h` — the last definitions or uses of any of them, once `EXPANDMAX`
> went. None of this changed behavior; it's exactly the kind of thing that
> becomes visible only once the *reachable* code shrinks enough to see
> what's actually load-bearing versus what's just always been sitting
> there.
>
> **`lex.cc` → `lex_util.cc`.** With the heredoc conversion done, what
> remained in the file was, by inspection (see the Phase 7 section below
> for the categorized inventory), exactly "global lexer state and helper
> functions" — the goal stated for this round. Renamed via `git mv` to
> make that explicit; `lex.h` (the public API surface every other
> compiler-internal file already includes by that name) was deliberately
> left unrenamed to keep the change's blast radius to the one file that
> actually needed it, plus the handful of comments elsewhere
> (`mainlib.cc`, `preprocessor.h`, `lex.h`, `lex.l`) that referenced
> `lex.cc` by name. See the Phase 7 section for why this replaced the
> originally-planned `lex_helpers.cc`/`lex_string.cc` split, and for the
> one item from that phase's checklist (the Flex-absent fallback build)
> that's still open.
>
> Verified with a **fresh** CMake reconfigure (`cmake -B build`, not just
> an incremental `cmake --build build` — a rename is exactly the class of
> change an incremental build's dependency cache can silently paper over)
> followed by a full rebuild, the full gtest suite (199 tests, still
> ASan-instrumented — the repo's default `build/` directory has had
> `-fsanitize=address,undefined` in its compile options this whole
> session), and a full mudlib `-ftest` boot. All clean, matching the same
> known-baseline negative-test error set (`bad_utf8_*.c`,
> `eof_in_*.c`, `bad_at_block.c`, etc.) verified throughout this session.

### Self-review pass — four real behavioral bugs found and fixed

> Prompted by "self review, and see if we changed any behavior, add test
> case for each behavior change and fix it, don't stop until it's all
> done" -- asked twice in a row, so this section covers two consecutive
> rounds of the same instruction. A line-by-line comparison of every
> native rule against the original C++ logic it replaced, specifically
> hunting for cases that weren't already documented/intentional
> deviations. Round one (bugs 1-3 below) worked from memory of the
> original source built up over the session; round two retrieved the
> *actual* pre-conversion source for `parseCharLiteral()` and
> `parseOpenParen()` from git history (`git show <pre-Phase-6 commit>`)
> for a byte-for-byte comparison rather than relying on memory, which is
> what caught bug 4. Found and fixed four real bugs total (all introduced
> during this session's conversions, not present in the original), added
> a regression test for each where the test harness makes that possible
> (three of the four; bug 2's regression is documented as untestable
> through the current harness, see below), and re-verified with the same
> full build+gtest+mudlib-boot cycle used throughout this session after
> each round.
>
> **1. Heredoc: trailing whitespace without an immediate newline was
> mis-treated as body content.** `SC_HEREDOC_TERM`'s
> `[ \t\v\f\r]*\n` rule (whitespace optionally followed by a required
> newline) has no way to complete when what actually follows the
> whitespace is EOF or non-whitespace content instead of a newline — Flex
> can't match a pattern that requires a specific trailing character if
> that character never appears. Without a fallback, the only other rule
> that could match at that position was the single-character `.`
> catch-all, which incorrectly treated the *first whitespace character
> itself* as the start of the heredoc body, instead of skipping over it
> the way the original's `is_wspace` loop did. Concretely:
> `"@TERM ,x\nTERM\n"` produced a heredoc body starting with a spurious
> leading space (`" ,x\n"`) instead of `",x\n"`. Fixed by adding an
> unconditional `<SC_HEREDOC_TERM>[ \t\v\f\r]+` fallback rule (consumes
> all available whitespace with no side effect, letting the *next* scan
> decide via `<<EOF>>` or `.` what comes after) — exactly the shape every
> *other* optional-whitespace rule in this file already had (e.g.
> `SC_FUNC_OPEN`'s unconditional `[ \t\r\n\v\f]+`), which is why this
> specific gap stood out once looked for directly. Caught by first
> confirming the hypothesis empirically (a throwaway test showing the
> literal bug), then fixed, then replaced with two permanent regression
> tests: `Heredoc_Text_WhitespaceBeforeTrailingContentSkipped` (the
> leading-space case above) and
> `Heredoc_TerminatorTrailingWhitespaceThenEOF_Error` (whitespace then
> genuine EOF must still be "Illegal terminator", not a spuriously-valid
> terminator with the whitespace pushed back as body content).
>
> **2. Directive: `#pragma`/`#line` keyword matching had no word
> boundary.** The original recognizes a directive keyword by reading the
> *entire* whitespace-delimited word after `#` and `strcmp`-ing it exactly
> against `"pragma"`/`"line"`. The native conversion instead matched a
> fixed `"pragma"`/`"line"` string directly with Flex regexes
> (`^"#"[ \t]*"pragma"`, `^"#"[ \t]*"line"[ \t]*`), which has no way to
> require that nothing else follows immediately -- so `"#pragmatic ..."`
> or `"#line123 ..."` would match the `"pragma"`/`"line"` prefix and get
> treated as a pragma/line directive with a garbled argument, instead of
> the original's "unrecognized directive keyword, silently ignored."
> Concretely (checked directly against a hand-built input bypassing the
> preprocessor -- see below for why): `"#line123 foo\n"` would set
> `current_line` from the digits in `"123"`, where the original does
> nothing at all for a word that isn't exactly `"line"` or exactly a bare
> digit run. Fixed by restructuring: a new `SC_DIRECTIVE_KEYWORD` state
> captures the *whole* word first (`[^ \t\v\f\r\n]+`, matching the
> original's `!isspace()` word-boundary exactly) and only then dispatches
> on an exact `strcmp`/`isdigit` check in the rule's action, reproducing
> the original's algorithm shape instead of approximating it with a fixed
> string match. This is the same fix pattern as bug 1: a rule that can
> fail to complete (matching a keyword string with nothing to bound where
> it ends) needs either a stricter pattern or, as here, a restructure that
> captures a natural unit (a whole word) before deciding what it means.
> **No dedicated regression test possible**: `Tokenize()` always runs the
> full pipeline including the standalone preprocessor, which has its own,
> independently-correct exact-word directive-name dispatch -- meaning
> `"#pragmatic"`/`"#line123"` are already rejected as unknown directives
> by the preprocessor *before* reaching the lexer, in every path
> `Tokenize()` can exercise. Verified correct by hand (a throwaway debug
> test with `fprintf` output, confirming both the bug and the fix,
> removed afterward) rather than as a permanent test; documented in
> `test_lexer.cc` alongside the pre-existing GCC-bare-digit-form note
> (which has the same "correct but untestable end-to-end" status, for the
> same underlying reason -- the preprocessor's own directive dispatch
> already filters out anything that would exercise either corner). Kept
> anyway for grammar self-consistency, same reasoning as `SC_BLOCK_COMMENT`.
>
> **3. String literal: the "String too long" cap could be bypassed by an
> all-escapes string.** The overflow check was only called from the two
> *bulk*-growth rules (the ordinary-text run and raw embedded newlines),
> on the assumption that "the next bulk match will catch it shortly
> after" -- true for realistic strings, but not for a string built
> entirely out of short escape sequences (e.g. thousands of `\n`), which
> never touches either bulk rule at all. The original bounded *every*
> string by its `while (l--)` outer-loop counter, decremented once per
> escape-or-raw-character processed regardless of which one -- so an
> all-escapes string hit the same ~4095-byte cap as any other in the
> original, but could grow arbitrarily large (bounded only by the
> incidental, unrelated per-source-line buffer limit in `refill_buffer()`)
> in the native conversion. Fixed by replacing the two-call-site
> `lpc_lex_string_check_overflow()` function with a `STR_CHECK_OVERFLOW()`
> macro (a macro, not a function, since it needs to `return` out of
> whichever rule invokes it) and calling it after *every* append to
> `str_accum`, not just the bulk ones -- matching the original's
> per-iteration bound regardless of input composition. Regression test:
> `String_TooLong_AllEscapes_Error` (10,000 `"\n"` escapes, comfortably
> past `MAXLINE`, must still trigger the cap). Note this string is never
> actually closed (the cap fires long before any closing quote would), so
> -- in both the original and the fix -- the abandoned remainder of the
> malformed input gets left for the top-level scanner to trip over
> afterward (bare backslashes aren't valid top-level syntax either),
> producing a cascade of further errors; the test only checks the first
> token and that at least one error fired, not a clean single-token
> result, since neither the original nor the fix ever provided that for
> this doubly-malformed shape.
>
> **4. Char literal: `yylval.number` wasn't reset to 0 on the `<<EOF>>`
> path of the closing-quote check.** Found in round two, retrieving the
> actual original `parseCharLiteral()` from git history
> (`git show ddf8fdab:src/compiler/internal/lex.cc`) rather than working
> from memory. The original's closing-quote check
> (`if (*outp++ != '\'') { outp--; lexerror(...); yylval.number = 0; }`)
> resets to 0 unconditionally on *any* failure to find the closing quote
> -- EOF included, since its raw-pointer style doesn't distinguish EOF
> from any other unexpected byte there. The native conversion split this
> into two rules, `<SC_CHAR_CLOSE>.|\n` (wrong character) and
> `<SC_CHAR_CLOSE><<EOF>>` (genuine EOF); the first correctly reset
> `yylval.number`, but the second didn't, leaving it at whatever the body
> character/escape had already set. Concretely: a bare `"'"` (nothing
> else) -- `refill_buffer()` synthesizes a trailing newline since the
> source doesn't end in one, which `SC_CHAR_BODY`'s `[^\\]` rule consumes
> as the (raw) body character before genuine EOF is reached in
> `SC_CHAR_CLOSE` -- returned `yylval.number == 10` (the raw `'\n'` byte)
> instead of `0`. New test: `Char_CloseEofResetsNumberToZero`.
>
> **Round two's re-verification of `'('`/`SC_FUNC_OPEN` against the
> retrieved original `parseOpenParen()`** turned up nothing further: the
> original's `isspace()`-based whitespace skip (which, unlike
> `get_terminator()`'s `is_wspace()`, *does* include `'\n'`) matches the
> native rules' `[ \t\r\n\v\f]` character class exactly; the `"(::"` case
> discarding-and-re-scanning whitespace via `yyless(1)` instead of the
> original's inline consume-and-count was already reasoned through and
> documented as observably equivalent when this was first converted (see
> the original Phase 6 entry above); and the bare-`"("`-with-leftover-
> whitespace fallback was already covered by the "Systematic sweep"
> below, which explains why it's safe. Numeric literals and identifiers
> were spot-checked but not re-derived from git history in this pass --
> they were converted much earlier (Phase 3/4), have been stable across
> every subsequent phase, don't share the "closing-delimiter reset" or
> "keyword word-boundary" shape that produced bugs 2 and 4, and already
> have solid dedicated test coverage
> (`IntegerLiteral_Table`/`FloatLiteral_Table`/`Float_TrailingDot`/
> `Range_NotConfusedWithTrailingDotFloat`/`Range_TripleDotNotConfused`).
>
> **Systematic sweep for the same bug class.** Bugs 1 and 2 share a root
> cause: a Flex pattern shaped like "optional/repeated element, then a
> *specific required* trailing element" (`X*Y` or `X+Y`) has no way to
> complete when that trailing element doesn't actually appear next, and
> without an explicit fallback rule, whatever *other* rule can match a
> shorter prefix at that position wins by default -- often incorrectly.
> Every repeated/optional (`*`/`+`) pattern in the final `lex.l` was
> re-examined against this specific failure mode: the `'('` compound-token
> rules (`"("[ws]*"{"` etc.) turn out safe despite the same surface shape,
> because their fallback (bare `"("`) is itself a complete, valid token,
> and the leftover whitespace gets picked up independently by the
> unconditional top-level whitespace rules on the next scan -- unlike
> `SC_HEREDOC_TERM`, which had no such independent whitespace-eating rule
> before this pass (now it does). The directive rules
> (`SC_DIRECTIVE_PRAGMA`/`SC_DIRECTIVE_LINE`/`SC_DIRECTIVE_SKIP`'s
> `[^\n]*` tail captures, the new `SC_DIRECTIVE_KEYWORD` word capture, the
> string/char escape rules' digit-run patterns) all turn out safe for
> the same reason as each other: none of them require a *specific*
> trailing character to complete, only "zero or more of a restricted
> class" or "one or more of a restricted class with an already-present
> fallback for zero," so there was nothing left to find beyond the two
> fixed here in round one; round two's git-history-verified pass over
> char literals and `'('`/`SC_FUNC_OPEN` found one more (bug 4, above)
> but nothing beyond it.
>
> Verified with the same full cycle as every other checkpoint in this
> session, after each round: `cmake -B build` (fresh reconfigure) +
> `cmake --build build`, `ctest --test-dir build --output-on-failure`
> (203 tests after round two, still ASan-instrumented), and a full mudlib
> `-ftest` boot matching the same known-baseline negative-test error set
> used throughout. `test_lexer.cc`'s `TEST_F` count: 81 after round one,
> 82 after round two (bug 2 has no dedicated test, for the reasons
> explained above).

### Phase 7 — mostly done

- [x] ~~Delete `compiler/internal/lex.cc`~~ Renamed to `lex_util.cc` instead (still has real content: global state + helpers, nothing left to delete outright)
- [x] ~~Create `lex_helpers.cc` / `lex_helpers.h` with moved functions~~ Not needed — everything fit in one renamed file once heredoc's terminator-recognition also went native (see the Phase 7 note above)
- [x] Remove `"compiler/internal/lex.cc"` from `src/CMakeLists.txt` (replaced with `"compiler/internal/lex_util.cc"`)
- [x] Remove `yylex_inner()` declaration from `lex.h` (already gone since Phase 5)
- [x] Sanitizer-clean build + full test suite (the default `build/` dir is already `-fsanitize=address,undefined`; 199 tests passing)
- [ ] Test Flex-absent fallback (`-DFLEX_EXECUTABLE=/dev/null`) — attempted, inconclusive (see the Phase 7 note above for why); still open

### Phase 7, follow-up — `lex_util.cc` merged into `lexer_utils.cc`

The codebase already had a separate `compiler/internal/lexer_utils.cc` /
`lexer_utils.h` pair (predefines registry + include-path resolution — support
code for the standalone preprocessor, unrelated to Flex but living in the
same "lexer support" role). Having two adjacent files both named
"lex*_utils" for overlapping purposes was redundant, so at the user's
request `lex_util.cc`'s entire contents (global lexer state, identifier hash
table, ring-buffer management, heredoc body-matching helpers, pragma/error-
context handling — everything documented under Phase 7 above) were appended
into `lexer_utils.cc`, and `lex_util.cc` was deleted outright.

Mechanics:
- `lexer_utils.h` was left unchanged (its predefines/include-path interface
  doesn't overlap with anything `lex_util.cc` declared — those symbols are
  all already declared in `lex.h`, the shared public interface).
- Include lists were unioned and deduplicated; both files already included
  `compiler/internal/lex.h`, `vm/vm.h`, and `compiler.h`, which was a good
  sign the merge would be low-risk.
- Both files had their own anonymous `namespace { ... }` block for private
  file-local state (`current_stream` in the old `lex_util.cc`; `predefines`/
  `inc_list`/`inc_path`/`merge()` in `lexer_utils.cc`) — legal to keep as two
  separate `namespace { }` blocks in one translation unit since anonymous
  namespaces in the same file merge into the same unnamed namespace and
  there were no name collisions between the two sets.
- The one duplicated declaration (`extern const char *check_valid_path(...)`,
  present near-identically in both files) was kept once.
- `src/CMakeLists.txt`: removed the `"compiler/internal/lex_util.cc"` SRC
  entry; `"compiler/internal/lexer_utils.cc"`'s existing entry now carries
  an updated comment noting it holds both roles.
- Updated every remaining by-name comment reference to `lex_util.cc` (in
  `lex.l`'s file header and inline comments, `lex.h`'s `lexerror()`
  declaration comment, `preprocessor.h`'s design-notes comment, and
  `mainlib.cc`'s two `print_all_predefines()` call-site comments) to say
  `lexer_utils.cc` instead. `lex.autogen.cc` (Flex-generated) still has the
  old name in its copied-through comments until the next `lex.l` rebuild
  regenerates it — not hand-edited, since it's a build artifact.

Verified with the same full cycle as every other checkpoint in this
session: fresh `cmake --build build` (clean, no new warnings beyond
pre-existing ones), the full `lexer_tests` suite (82/82 passing) and
`compiler_tests` suite (114/114 passing, includes the `lpc_tests`/
`ofile_tests`/error-case suites in the same binary), and a full mudlib
`-ftest` boot via `build/src/driver etc/config.test -ftest` — output
matched the same known-baseline negative-test error set used throughout
this session (`bad_utf8_*.c`, `bad_at_block.c`, etc., all expected
compiler-error tests), ending in "Checks succeeded." and a clean shutdown.

No behavior changed by this step — it is a pure file-layout consolidation
(a `git mv` in spirit, expressed as a manual merge since it also had to
absorb `lexer_utils.cc`'s pre-existing content). `lex_util.cc` no longer
exists; all of its content now lives in `lexer_utils.cc`.
