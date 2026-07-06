// test_lexer.cc — token-level unit tests for the LPC lexer (Phase 2/6)
//
// Tests are written against the yylex() entry point (Flex-generated), so
// they don't care which rule or helper function actually produced a given
// token as the migration to native Flex rules progressed through Phases
// 3-6.
//
// Mechanical, single-shape tokens (single-char operators, punctuation,
// reserved words) are covered by compact table-driven tests below rather
// than one TEST_F per token: Flex matches each of these with a literal
// string pattern and a one-line return, so there's effectively one shared
// failure mode per table, not per row. Test *weight* instead goes to the
// genuinely complex and error-prone corners: string/char escape decoding
// (octal/hex/unicode, including surrogate pairs), scratch-buffer boundary
// crossings, heredoc/comment/directive recovery after malformed input, and
// the lexer-level error paths (lexerror()/yyerror(), which log and return
// rather than longjmp -- see AGENTS.md and compiler.cc's yyerror()).

#include "base/std.h"

#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "base/internal/debugmalloc.h"
#include "base/internal/stralloc.h"
#include "compiler/internal/LexStream.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/grammar_rules.h"    // decl_t / func_block_t — must precede grammar.autogen.h
#include "compiler/internal/grammar.autogen.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/lexer_utils.h"
#include "mainlib.h"
#include "vm/vm.h"

// ---------------------------------------------------------------------------
// Token record
// ---------------------------------------------------------------------------

struct Token {
    int kind;           // Bison token id
    LPC_INT number;     // valid when kind == L_NUMBER / L_PARAMETER
    LPC_FLOAT real;     // valid when kind == L_REAL
    std::string text;   // copy of yytext at token time
    std::string str;    // copy of yylval.string when kind == L_STRING
};

// Leftover lexer state (template nesting, etc.) from the most recent
// Tokenize() call, snapshotted just before the reentrant scanner backing it
// is destroyed. Tests that check for leaked/stuck lexer state (e.g. after a
// deliberately-malformed template literal) read this instead of a global.
static compiler_context_t g_last_lex_ctx;

// ---------------------------------------------------------------------------
// Tokenize() — feed source through the lexer pipeline.
//
// We bypass compile_file()/yyparse() but we still need the compiler's
// mem_block[] initialized (because parseDirective() processes #line
// directives that call add_program_file() → add_to_mem_block()).  We
// replicate just the minimum init that prolog() does, then undo it in
// cleanup. num_parse_error is reset to 0 here, so every test can check it
// directly after a Tokenize() call to see whether that source triggered a
// lexer-level error (lexerror()/yyerror() log and return -- they don't
// longjmp -- so this is always safe to inspect afterward).
// ---------------------------------------------------------------------------

static std::vector<Token> Tokenize(const std::string& source,
                                   const char* filename = "lexer_test") {
    // --- minimal prolog ---
    for (int i = 0; i < NUMAREAS; i++) {
        mem_block[i].block = reinterpret_cast<char *>(
            DMALLOC(START_BLOCK_SIZE, TAG_COMPILER, "Tokenize"));
        mem_block[i].current_size = 0;
        mem_block[i].max_size = START_BLOCK_SIZE;
    }
    memset(string_tags, 0, sizeof(string_tags));
    freed_string = -1;
    num_parse_error = 0;
    // Order must match prolog(): current_file before init_include_path()
    // (init_include_path calls add_slash(current_file) which crashes on null)
    current_file = make_shared_string(filename);
    current_file_id = add_program_file(filename, /*top=*/1);
    init_include_path();

    // --- reentrant scanner setup ---
    compiler_context_t ctx;
    void *scanner = nullptr;
    yylex_init_extra(&ctx, &scanner);

    // --- feed lexer ---
    std::istringstream ss(source);
    auto stream = std::make_unique<IStreamLexStream>(ss);
    start_new_file(std::move(stream), scanner);

    std::vector<Token> toks;
    YYSTYPE yylval;
    for (;;) {
        YYLTYPE yylloc;
        int k = yylex(&yylval, &yylloc, scanner);
        if (k <= 0) break;
        Token t;
        t.kind   = k;
        t.number = (k == L_NUMBER || k == L_PARAMETER) ? yylval.number : 0;
        t.real   = (k == L_REAL)   ? yylval.real   : 0.0;
        t.text   = yyget_text(scanner);
        bool has_string_payload = (k == L_STRING || k == L_TEMPLATE_HEAD ||
                                   k == L_TEMPLATE_MIDDLE || k == L_TEMPLATE_TAIL);
        t.str    = (has_string_payload && yylval.string) ? yylval.string->c_str() : "";
        toks.push_back(t);
    }

    end_new_file();

    g_last_lex_ctx = *yyget_extra(scanner);
    yylex_destroy(scanner);

    // --- minimal epilog ---
    deinit_include_path();
    for (int i = 0; i < NUMAREAS; i++) {
        if (mem_block[i].block) {
            FREE(mem_block[i].block);
            mem_block[i].block = nullptr;
            mem_block[i].current_size = 0;
            mem_block[i].max_size = 0;
        }
    }
    if (current_file) {
        free_string(current_file);
        current_file = nullptr;
    }

    return toks;
}

// ---------------------------------------------------------------------------
// Test fixture — full driver runtime (needed for vm_start / simul_efun)
// ---------------------------------------------------------------------------

class LexerTest : public ::testing::Test {
 public:
    static void SetUpTestSuite() {
        chdir(TESTSUITE_DIR);
        auto* base = init_main("etc/config.test");
        vm_start();
    }
 protected:
    void SetUp() override { clear_state(); }
    void TearDown() override { clear_state(); }
};

// ============================================================================
// 1. Numeric literals — mechanical forms (table) + the genuinely ambiguous
//    ones kept as standalone regression tests.
// ============================================================================

TEST_F(LexerTest, IntegerLiteral_Table) {
    struct Case { const char* src; LPC_INT expect; };
    static const Case cases[] = {
        {"0\n", 0},
        {"42\n", 42},
        {"1000000\n", 1000000},
        {"0xff\n", 0xff},
        {"0X1A\n", 0x1A},
        {"0xDEAD\n", 0xDEAD},
        {"0b1010\n", 10},
        {"0B0001\n", 1},
        {"1_000_000\n", 1000000},
    };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].kind, L_NUMBER) << c.src;
        EXPECT_EQ(t[0].number, c.expect) << c.src;
    }
}

TEST_F(LexerTest, IntDecimal_Negative) {
    // Unary minus is its own token; the lexer never produces negative
    // number literals directly.
    auto t = Tokenize("-99\n");
    ASSERT_EQ(t.size(), 2u);
    EXPECT_EQ(t[0].kind, '-');
    EXPECT_EQ(t[1].kind, L_NUMBER);
    EXPECT_EQ(t[1].number, 99);
}

TEST_F(LexerTest, FloatLiteral_Table) {
    struct Case { const char* src; double expect; };
    static const Case cases[] = {
        {"1.5\n", 1.5},
        {"0.0\n", 0.0},
        {"3.14\n", 3.14},
    };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].kind, L_REAL) << c.src;
        EXPECT_NEAR(t[0].real, c.expect, 1e-9) << c.src;
    }
}

TEST_F(LexerTest, Float_TrailingDot) {
    auto t = Tokenize("1.\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_REAL);
    EXPECT_DOUBLE_EQ(t[0].real, 1.0);
}

// "1..5" must tokenize as NUMBER(1) RANGE NUMBER(5), not REAL(1.0) DOT NUMBER(5):
// a trailing '.' is only part of a float when not immediately followed by
// another '.', which belongs to the range operator instead.
TEST_F(LexerTest, Range_NotConfusedWithTrailingDotFloat) {
    auto t = Tokenize("1..5\n");
    ASSERT_EQ(t.size(), 3u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 1);
    EXPECT_EQ(t[1].kind, L_RANGE);
    EXPECT_EQ(t[2].kind, L_NUMBER);
    EXPECT_EQ(t[2].number, 5);
}

// Same ambiguity, one dot further: "1...2" is NUMBER(1) DOT_DOT_DOT
// NUMBER(2), not NUMBER(1) RANGE DOT NUMBER(2) -- exercises Flex's longest-
// match rule picking "..." over ".." at the same start position.
TEST_F(LexerTest, Range_TripleDotNotConfused) {
    auto t = Tokenize("1...2\n");
    ASSERT_EQ(t.size(), 3u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 1);
    EXPECT_EQ(t[1].kind, L_DOT_DOT_DOT);
    EXPECT_EQ(t[2].kind, L_NUMBER);
    EXPECT_EQ(t[2].number, 2);
}

// ============================================================================
// 2. String literals
// ============================================================================

TEST_F(LexerTest, String_Basic_Table) {
    struct Case { const char* src; const char* expect; };
    static const Case cases[] = {
        {"\"hello\"\n", "hello"},
        {"\"\"\n", ""},
    };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].kind, L_STRING) << c.src;
        EXPECT_EQ(t[0].str, c.expect) << c.src;
    }
}

TEST_F(LexerTest, String_EscapeSequences_Table) {
    struct Case { const char* src; const char* expect; };
    static const Case cases[] = {
        {"\"\\n\"\n", "\n"}, {"\"\\t\"\n", "\t"}, {"\"\\r\"\n", "\r"},
        {"\"\\b\"\n", "\b"}, {"\"\\a\"\n", "\a"}, {"\"\\e\"\n", "\x1b"},
        {"\"\\\\\"\n", "\\"}, {"\"\\\"\"\n", "\""},
    };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].str, c.expect) << c.src;
    }
}

TEST_F(LexerTest, String_OctalEscape) {
    auto t = Tokenize("\"\\101\"\n");  // 0101 octal = 65 = 'A'
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].str, "A");
}

TEST_F(LexerTest, String_HexEscape) {
    auto t = Tokenize("\"\\x41\"\n");  // 0x41 = 'A'
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].str, "A");
}

TEST_F(LexerTest, String_Unicode4) {
    auto t = Tokenize("\"\\u0041\"\n");  // U+0041 = 'A'
    ASSERT_EQ(t.size(), 1u);
    EXPECT_FALSE(t[0].str.empty());
    EXPECT_EQ(static_cast<unsigned char>(t[0].str[0]), 'A');
}

TEST_F(LexerTest, String_Unicode8) {
    auto t = Tokenize("\"\\U00000041\"\n");  // U+0041 = 'A'
    ASSERT_EQ(t.size(), 1u);
    EXPECT_FALSE(t[0].str.empty());
    EXPECT_EQ(static_cast<unsigned char>(t[0].str[0]), 'A');
}

// 😀 is a UTF-16 surrogate pair for U+1F600 (grinning face emoji),
// encoded as 4-byte UTF-8 (F0 9F 98 80). Exercises the lead/trail surrogate
// combination path in parseStringLiteral(), not just a single-codepoint
// escape.
TEST_F(LexerTest, String_UnicodeSurrogatePair) {
    auto t = Tokenize("\"\\uD83D\\uDE00\"\n");
    ASSERT_EQ(t.size(), 1u);
    ASSERT_EQ(t[0].str.size(), 4u);
    EXPECT_EQ(static_cast<unsigned char>(t[0].str[0]), 0xF0);
    EXPECT_EQ(static_cast<unsigned char>(t[0].str[1]), 0x9F);
    EXPECT_EQ(static_cast<unsigned char>(t[0].str[2]), 0x98);
    EXPECT_EQ(static_cast<unsigned char>(t[0].str[3]), 0x80);
}

// A lone trail surrogate with no preceding lead surrogate: reported as an
// error ("expecting surrogate lead, got trail") but recoverable -- the
// malformed escape is dropped and scanning of the rest of the string
// continues rather than aborting the token.
TEST_F(LexerTest, String_UnicodeLoneSurrogateTrail_Recovers) {
    auto t = Tokenize("\"\\uDE00ok\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "ok");
    EXPECT_GT(num_parse_error, 0);
}

// A lead surrogate with no following "\u..." trail: reported as "Missing
// surrogate trail" but likewise recoverable.
TEST_F(LexerTest, String_UnicodeMissingSurrogateTrail_Recovers) {
    auto t = Tokenize("\"\\uD800ok\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "ok");
    EXPECT_GT(num_parse_error, 0);
}

// A backslash immediately followed by a real newline inside a string is a
// line continuation: the newline is swallowed entirely (not copied, not
// even as a literal '\n'), only current_line/total_lines advance.
TEST_F(LexerTest, String_LineContinuation_BackslashNewlineSuppressed) {
    auto t = Tokenize("\"abc\\\ndef\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].str, "abcdef");
}

// parseStringLiteral() copies into a small scratch buffer first (255 bytes
// per its internal chunking), then spills into a second, yytext-based loop
// once that's exhausted. A string long enough to cross that boundary
// exercises the handoff between the two loops and the final
// scratch_large_alloc() reassembly.
TEST_F(LexerTest, String_LongRoundTrip_CrossesScratchBoundary) {
    std::string body(300, 'x');
    auto t = Tokenize("\"" + body + "\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, body);
}

TEST_F(LexerTest, String_UnterminatedEOF_Error) {
    auto t = Tokenize("\"unterminated\n");
    EXPECT_TRUE(t.empty());
    EXPECT_GT(num_parse_error, 0);
}

TEST_F(LexerTest, String_InvalidUtf8Byte_Error) {
    std::string src = "\"";
    src += static_cast<char>(0xFF);  // not a valid UTF-8 lead/continuation byte
    src += "\"\n";
    auto t = Tokenize(src);
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, YYerror);
    EXPECT_GT(num_parse_error, 0);
}

// '\8'/'\9' in a string aren't valid octal digits (deviation from the
// original documented in the migration plan: the hand-written version's
// strtoll() there silently consumed nothing and left an embedded NUL byte
// plus a stray literal digit in the string -- reported as a clean error
// here instead).
TEST_F(LexerTest, String_InvalidOctalDigit_Error) {
    auto t = Tokenize("\"a\\8b\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "ab");
    EXPECT_GT(num_parse_error, 0);
}

// '\u' followed by fewer than 4 hex digits (or none at all): the bare "\u"
// fallback rule fires, reporting an error but leaving the non-hex
// characters that follow to be scanned as ordinary string content --
// documented deviation from the original, which discarded them instead.
TEST_F(LexerTest, String_UnicodeShortEscape_Recovers) {
    auto t = Tokenize("\"a\\u12zz\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "a12zz");
    EXPECT_GT(num_parse_error, 0);
}

TEST_F(LexerTest, String_UnicodeUShortEscape_Recovers) {
    auto t = Tokenize("\"a\\U1234zz\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "a1234zz");
    EXPECT_GT(num_parse_error, 0);
}

// A string past the ~MAXLINE length cap: still returns L_STRING (with
// whatever was accumulated), but logs "String too long" -- matching the
// original's give-up-but-still-return-a-token behavior.
TEST_F(LexerTest, String_TooLong_Error) {
    std::string body(5000, 'x');
    auto t = Tokenize("\"" + body + "\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_GT(num_parse_error, 0);
}

// Regression test for a gap caught during a self-review pass (not present
// in the original, which bounded string scanning via its `while (l--)`
// outer-loop counter regardless of what kind of content that was): the
// "String too long" overflow check used to only be called from the
// ordinary-run and raw-newline rules, so a string built entirely out of
// short escape sequences (never touching either of those two rules) could
// grow far past the ~MAXLINE cap before anything caught it. 10000 "\n"
// escapes accumulate to 10000 output bytes -- comfortably past MAXLINE
// (4096) -- and must still trigger the cap.
//
// Note this string is never actually closed (the overflow fires long
// before the closing quote), so -- in both the original and here -- the
// unconsumed remainder of the malformed input gets left for the top-level
// scanner to trip over afterward (a string of bare backslashes isn't valid
// top-level LPC syntax either), producing a cascade of further "illegal
// character" tokens/errors. That cascade is expected for this kind of
// doubly-malformed input and isn't what this test is checking; only the
// first token (the truncated string itself) and that at least one error
// was reported matter here.
TEST_F(LexerTest, String_TooLong_AllEscapes_Error) {
    std::string body;
    for (int i = 0; i < 10000; i++) body += "\\n";
    auto t = Tokenize("\"" + body + "\"\n");
    ASSERT_FALSE(t.empty());
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_LE(t[0].str.size(), static_cast<size_t>(MAXLINE) + 1);
    EXPECT_GT(num_parse_error, 0);
}

// ============================================================================
// 3. Character literals
// ============================================================================

TEST_F(LexerTest, Char_Simple) {
    auto t = Tokenize("'a'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 'a');
}

TEST_F(LexerTest, Char_EscapeNewline) {
    auto t = Tokenize("'\\n'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, '\n');
}

TEST_F(LexerTest, Char_HexEscape) {
    auto t = Tokenize("'\\x41'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 0x41);
}

TEST_F(LexerTest, Char_OctalEscape) {
    auto t = Tokenize("'\\101'\n");  // 0101 octal = 65 = 'A'
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 65);
}

TEST_F(LexerTest, Char_UnknownEscape) {
    // \z isn't a recognized escape -- falls back to the literal char with a
    // warning (regression test for the native SC_CHAR_BODY catch-all rule).
    auto t = Tokenize("'\\z'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 'z');
}

TEST_F(LexerTest, Char_QuoteEscape) {
    auto t = Tokenize("'\\''\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, '\'');
}

// '\777' (511 decimal) overflows a byte: falls back to 'x' with a warning
// rather than truncating/wrapping.
TEST_F(LexerTest, Char_OctalOverflow_FallsBackToX) {
    auto t = Tokenize("'\\777'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 'x');
}

// '\8' -- '8'/'9' aren't valid octal digits. Documented deviation from the
// pre-Flex hand-written scanner: this
// reports the error directly instead of replicating the original's
// incidental strtoll-consumes-nothing fallthrough. Still a real error.
TEST_F(LexerTest, Char_InvalidOctalDigit_Error) {
    auto t = Tokenize("'\\8'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 0);
    EXPECT_GT(num_parse_error, 0);
}

// '\xFFFF' overflows a byte via hex digits (the hex-escape pattern accepts
// an unbounded digit run) -- same 'x'-fallback-with-warning behavior as the
// octal overflow case, different code path.
TEST_F(LexerTest, Char_HexOverflow_FallsBackToX) {
    auto t = Tokenize("'\\xFFFF'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 'x');
}

// '\x' immediately followed by the closing quote (no hex digits at all)
// falls back to the literal 'x' character with a warning, rather than
// erroring outright.
TEST_F(LexerTest, Char_BareHexEscapeNoDigits) {
    auto t = Tokenize("'\\x'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 'x');
}

// 'ab' -- more than one body character. The first char closes SC_CHAR_BODY,
// then the second fails the required closing quote in SC_CHAR_CLOSE.
TEST_F(LexerTest, Char_TooManyChars_Error) {
    auto t = Tokenize("'ab'\n");
    ASSERT_FALSE(t.empty());
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 0);
    EXPECT_GT(num_parse_error, 0);
}

// EOF before a closing quote is ever seen.
TEST_F(LexerTest, Char_UnterminatedEOF_Error) {
    auto t = Tokenize("'a");
    ASSERT_FALSE(t.empty());
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_GT(num_parse_error, 0);
}

// Regression test for a bug caught during a self-review pass: the
// original always resets yylval.number to 0 when the closing quote isn't
// found, EOF included. A bare "'" (nothing else) gets a synthetic
// trailing newline appended by refill_buffer() (since the source doesn't
// end in one), which SC_CHAR_BODY's [^\\] rule consumes as the (raw) body
// character before genuine EOF is reached in SC_CHAR_CLOSE -- so
// yylval.number must still come back as 0, not the raw '\n' (10) the body
// step set it to.
TEST_F(LexerTest, Char_CloseEofResetsNumberToZero) {
    auto t = Tokenize("'");
    ASSERT_FALSE(t.empty());
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 0);
    EXPECT_GT(num_parse_error, 0);
}

// ============================================================================
// 4. Operators, punctuation, keywords, identifiers — mechanical single-shape
//    tokens, covered by table-driven tests rather than one TEST_F apiece.
// ============================================================================

TEST_F(LexerTest, Operators_Table) {
    struct Case { const char* src; int kind; };
    static const Case cases[] = {
        // Arithmetic / bitwise
        {"+\n", '+'}, {"-\n", '-'}, {"*\n", '*'}, {"/\n", '/'}, {"%\n", '%'},
        {"^\n", '^'}, {"&\n", '&'}, {"|\n", '|'}, {"~\n", '~'},
        // Compound assignment
        {"+=\n", L_ASSIGN}, {"-=\n", L_ASSIGN}, {"*=\n", L_ASSIGN}, {"/=\n", L_ASSIGN},
        {"%=\n", L_ASSIGN}, {"^=\n", L_ASSIGN}, {"&=\n", L_ASSIGN}, {"|=\n", L_ASSIGN},
        {"<<=\n", L_ASSIGN}, {">>=\n", L_ASSIGN}, {"&&=\n", L_ASSIGN}, {"||=\n", L_ASSIGN},
        {"?\?=\n", L_ASSIGN}, {"=\n", L_ASSIGN},
        // Logical / short-circuit
        {"&&\n", L_LAND}, {"||\n", L_LOR}, {"?\?\n", L_QUESTION_QUESTION}, {"!\n", '!'},
        // Comparison
        {"==\n", L_EQ_NE}, {"!=\n", L_EQ_NE}, {"<\n", '<'}, {">\n", L_ORDER},
        {"<=\n", L_ORDER}, {">=\n", L_ORDER},
        // Other multi-char operators
        {"++\n", L_INC_DEC}, {"--\n", L_INC_DEC}, {"->\n", L_ARROW}, {"..\n", L_RANGE},
        {"...\n", L_DOT_DOT_DOT}, {"::\n", L_COLON_COLON}, {"<<\n", L_SHIFT}, {">>\n", L_SHIFT},
    };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].kind, c.kind) << c.src;
    }
}

TEST_F(LexerTest, Punctuation_Table) {
    struct Case { const char* src; int kind; };
    static const Case cases[] = {
        {"(\n", '('}, {")\n", ')'}, {"{\n", '{'}, {"}\n", '}'},
        {"[\n", '['}, {"]\n", ']'}, {";\n", ';'}, {",\n", ','},
        {":\n", ':'}, {"?\n", '?'}, {".\n", '.'},
    };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].kind, c.kind) << c.src;
    }
}

TEST_F(LexerTest, Keywords_Table) {
    struct Case { const char* src; int kind; };
    static const Case cases[] = {
        {"int\n", L_BASIC_TYPE}, {"string\n", L_BASIC_TYPE}, {"float\n", L_BASIC_TYPE},
        {"object\n", L_BASIC_TYPE}, {"mapping\n", L_BASIC_TYPE}, {"mixed\n", L_BASIC_TYPE},
        {"void\n", L_BASIC_TYPE},
        {"if\n", L_IF}, {"else\n", L_ELSE}, {"for\n", L_FOR}, {"foreach\n", L_FOREACH},
        {"while\n", L_WHILE}, {"do\n", L_DO}, {"switch\n", L_SWITCH}, {"case\n", L_CASE},
        {"default\n", L_DEFAULT}, {"break\n", L_BREAK}, {"continue\n", L_CONTINUE},
        {"return\n", L_RETURN}, {"inherit\n", L_INHERIT},
    };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].kind, c.kind) << c.src;
    }
}

TEST_F(LexerTest, Identifiers_Table) {
    struct Case { const char* src; };
    static const Case cases[] = { {"foo\n"}, {"_bar\n"}, {"abc123\n"} };
    for (auto& c : cases) {
        auto t = Tokenize(c.src);
        ASSERT_EQ(t.size(), 1u) << c.src;
        EXPECT_EQ(t[0].kind, L_IDENTIFIER) << c.src;
    }
}

// ============================================================================
// 5. Comments
// ============================================================================

TEST_F(LexerTest, Comment_Block_Ignored) {
    auto t = Tokenize("/* block comment */ 42\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 42);
}

TEST_F(LexerTest, Comment_Block_MultiLine) {
    auto t = Tokenize("/* line 1\n   line 2 */\n99\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 99);
}

TEST_F(LexerTest, Comment_LineComment_Ignored) {
    auto t = Tokenize("// line comment\n42\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 42);
}

TEST_F(LexerTest, Comment_LineComment_DoesNotConsumeNextLine) {
    auto t = Tokenize("// skip\n1\n2\n");
    ASSERT_EQ(t.size(), 2u);
    EXPECT_EQ(t[0].number, 1);
    EXPECT_EQ(t[1].number, 2);
}

// A literal "/*" inside a block comment just warns ("/* found in comment.")
// -- FluffOS doesn't support nested comments, the first "*/" still closes
// it -- but must not error or desync subsequent scanning.
TEST_F(LexerTest, Comment_NestedSlashStarInsideBlock_Warning) {
    auto t = Tokenize("/* outer /* inner */ 42\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 42);
}

// Multiple consecutive '*' immediately before the closing '/' (e.g. "**/")
// must still close the comment -- regression test for the native
// "*"+"/" rule (as opposed to a single "*" "/" pair).
TEST_F(LexerTest, Comment_MultipleStarsBeforeClose) {
    auto t = Tokenize("/** multi-star **/ 42\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 42);
}

TEST_F(LexerTest, Comment_UnterminatedBlock_EOF_Error) {
    auto t = Tokenize("/* unterminated\n");
    EXPECT_TRUE(t.empty());
    EXPECT_GT(num_parse_error, 0);
}

// ============================================================================
// 6. Whitespace handling
// ============================================================================

TEST_F(LexerTest, Whitespace_Table) {
    {
        auto t = Tokenize("   42\n");
        ASSERT_EQ(t.size(), 1u);
        EXPECT_EQ(t[0].number, 42);
    }
    {
        // \r must be silently dropped, not counted as a line/token boundary.
        auto t = Tokenize("1\r\n2\n");
        ASSERT_EQ(t.size(), 2u);
        EXPECT_EQ(t[0].number, 1);
        EXPECT_EQ(t[1].number, 2);
    }
    {
        auto t = Tokenize("1 + 2\n");
        ASSERT_EQ(t.size(), 3u);
        EXPECT_EQ(t[0].number, 1);
        EXPECT_EQ(t[1].kind, '+');
        EXPECT_EQ(t[2].number, 2);
    }
}

// ============================================================================
// 7. Heredocs (@TERM text blocks, @@TERM array blocks)
// ============================================================================

TEST_F(LexerTest, Heredoc_Text_SingleLine) {
    auto t = Tokenize("@END\nhello\nEND\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_NE(t[0].str.find("hello"), std::string::npos);
}

TEST_F(LexerTest, Heredoc_Text_MultiLine) {
    auto t = Tokenize("@STOP\nline1\nline2\nSTOP\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
}

// "@@TERM ... TERM" (Robocoder's array-of-lines form) splices a literal
// "({ ... })" and re-enters the top-level scanner to produce a full array
// expression's worth of tokens, not a single string. Since 9.1 the open
// is the ordinary '(' '{' token pair.
TEST_F(LexerTest, Heredoc_Array) {
    auto t = Tokenize("@@END\nfirst\nsecond\nEND\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, '(');
    EXPECT_EQ(t[1].kind, '{');
    bool found_first = false;
    for (auto& tok : t) {
        if (tok.kind == L_STRING && tok.str == "first") found_first = true;
    }
    EXPECT_TRUE(found_first);
}

// "@" with no valid terminator identifier before the newline: get_terminator()
// reports an empty terminator, which is an error, but scanning recovers and
// resumes with the very next line as ordinary top-level source.
TEST_F(LexerTest, Heredoc_IllegalTerminator_Recovers) {
    auto t = Tokenize("@\n1\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 1);
    EXPECT_GT(num_parse_error, 0);
}

TEST_F(LexerTest, Heredoc_UnterminatedEOF_Error) {
    auto t = Tokenize("@END\nbody with no terminator line\n");
    EXPECT_TRUE(t.empty());
    EXPECT_GT(num_parse_error, 0);
}

// A line that merely starts with the terminator ("ENDING" starts with
// "END") but continues with more alnum/underscore characters must NOT
// close the heredoc -- only a word-bounded terminator match does
// (get_text_block()'s `!uisalnum(...) && ... != '_'` check).
TEST_F(LexerTest, Heredoc_Text_TerminatorSubstringNotMatched) {
    auto t = Tokenize("@END\nENDING\nEND\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_NE(t[0].str.find("ENDING"), std::string::npos);
}

// The terminator can be followed by more content on the same line, as
// long as it's word-bounded (e.g. punctuation) -- the trailing content is
// put back and re-scanned as ordinary source once the heredoc closes.
TEST_F(LexerTest, Heredoc_Text_TrailingContentAfterTerminator) {
    auto t = Tokenize("@END\nhello\nEND;\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_NE(t[0].str.find("hello"), std::string::npos);
    EXPECT_EQ(t[1].kind, ';');
}

// Regression test for a bug caught during a self-review pass (not present
// in the original): when whitespace after the terminator name is followed
// by non-whitespace content -- rather than immediately by a newline -- that
// whitespace must still be skipped before the trailing content becomes the
// first byte of the heredoc body. An earlier version of the native
// SC_HEREDOC_TERM conversion only handled "whitespace immediately followed
// by newline" as a unit; whitespace-then-EOF or whitespace-then-other-
// content fell through to a single-character catch-all that incorrectly
// treated the *whitespace itself* as body content, producing a spurious
// leading space the original never would have.
TEST_F(LexerTest, Heredoc_Text_WhitespaceBeforeTrailingContentSkipped) {
    auto t = Tokenize("@TERM ,x\nTERM\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str.substr(0, 2), ",x");
}

// Same bug, EOF variant: trailing whitespace after the terminator name
// with genuine EOF (no newline at all) must still be an "Illegal
// terminator" error -- not accidentally treated as a valid terminator with
// the whitespace itself pushed back as body content.
TEST_F(LexerTest, Heredoc_TerminatorTrailingWhitespaceThenEOF_Error) {
    auto t = Tokenize("@TERM  ");
    EXPECT_TRUE(t.empty());
    EXPECT_GT(num_parse_error, 0);
}

// ============================================================================
// 8. Pragmas and '#line'/'#' directives
// ============================================================================

TEST_F(LexerTest, Pragma_StrictTypes) {
    // #pragma should be consumed silently; next token is the keyword. Also
    // verifies the actual bitmask effect, not just that scanning continues.
    auto t = Tokenize("#pragma strict_types\nint\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_BASIC_TYPE);
    EXPECT_EQ(pragmas & PRAGMA_STRICT_TYPES, PRAGMA_STRICT_TYPES);
}

// "no_" prefix clears the corresponding bit. PRAGMA_WARNINGS is on by
// default (see DEFAULT_PRAGMAS in local_options), so this actually
// verifies the clear takes effect, not just that the token stream
// continues.
TEST_F(LexerTest, Pragma_NoWarnings_ClearsDefaultBit) {
    Tokenize("#pragma no_warnings\n");
    EXPECT_EQ(pragmas & PRAGMA_WARNINGS, 0);
}

// Regression test for a latent out-of-bounds read fixed while converting
// this to native Flex: handle_pragma() used to do str[strlen(str) - 1]
// unconditionally, underflowing for an empty argument. The original's
// convoluted outp pointer arithmetic happened to never actually call it
// with "" in practice; more direct native routing can (nothing at all
// after "pragma" before the newline).
TEST_F(LexerTest, Pragma_EmptyArgument_NoCrash) {
    auto t = Tokenize("#pragma\n42\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 42);
}

// Same fix, different trigger: an argument that's entirely whitespace
// also used to underflow the trailing-whitespace-strip loop.
TEST_F(LexerTest, Pragma_WhitespaceOnlyArgument_NoCrash) {
    auto t = Tokenize("#pragma   \n42\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 42);
}

// CRLF line ending: the trailing '\r' before the '\n' must be stripped
// from the pragma argument (handled by handle_pragma()'s own trailing-
// whitespace strip, since '\r' is whitespace), not left as part of the
// name being compared against the recognized-pragma table.
TEST_F(LexerTest, Pragma_CRLF_TrailingCarriageReturn) {
    auto t = Tokenize("#pragma strict_types\r\nint\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_BASIC_TYPE);
    EXPECT_EQ(pragmas & PRAGMA_STRICT_TYPES, PRAGMA_STRICT_TYPES);
}

TEST_F(LexerTest, Pragma_Unknown_Recovers) {
    // An unrecognized pragma name just warns and is otherwise ignored.
    auto t = Tokenize("#pragma totally_bogus_option\n42\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, 42);
}

// A '#' NOT at the start of a line isn't a directive at all -- it falls
// through to the generic "illegal character" catch-all instead of being
// parsed as '#pragma'/'#line'.
TEST_F(LexerTest, Directive_HashNotAtLineStart_Illegal) {
    auto t = Tokenize("1 #foo\n");
    ASSERT_EQ(t.size(), 3u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[1].kind, ' ');
    EXPECT_EQ(t[2].kind, L_IDENTIFIER);
}

TEST_F(LexerTest, Directive_Line_UpdatesCurrentLine) {
    Tokenize("#line 100\nint x;\n");
    // "#line 100" sets current_line to 99 (line_num - 1); the newline that
    // follows it and the newline ending "int x;" each advance it by one.
    EXPECT_EQ(current_line, 101);
}

// Tokenize()'s own cleanup unconditionally frees/nulls current_file after
// every call (it owns that lifetime, mirroring prolog()/epilog()), so these
// can only check current_line here -- not current_file's content -- but
// that's still a meaningful check: it confirms the quoted-filename branch
// parses past the filename correctly without corrupting subsequent
// line-number tracking (and doesn't trip ASan on the fname buffer).
TEST_F(LexerTest, Directive_Line_WithQuotedFilename) {
    Tokenize("#line 50 \"other.c\"\nint x;\n");
    EXPECT_EQ(current_line, 51);
}

// NOTE: two directive-handling corners have no dedicated test here because
// Tokenize() always runs the full pipeline (start_new_file() -> the
// standalone preprocessor -> the lexer), and the preprocessor itself
// blocks both before the lexer ever sees them:
//   - The GCC-style "# N [\"file\"]" line marker (no "line" keyword): the
//     preprocessor's own directive-name extraction captures the leading
//     digit run as the (unrecognized) directive name itself rather than
//     leaving it for a "bare digit" fallback, so "# 50 ..." is rejected as
//     an unknown directive before reaching the lexer (a preexisting
//     preprocessor limitation, unrelated to this conversion).
//   - Word-boundary correctness for "pragma"/"line" (e.g. confirming that
//     "#pragmatic" or "#line123" are treated as unrecognized directives,
//     not as "#pragma"/"#line" with a garbled argument): the preprocessor
//     only ever re-emits "#pragma "/"#line " when its own directive-name
//     extraction matched the exact word "pragma"/"line", so anything that
//     would exercise SC_DIRECTIVE_KEYWORD's word-boundary check in lex.l
//     is, by construction, already filtered out upstream.
// Both are still handled correctly in lex.l (kept for grammar
// self-consistency and correctness, same rationale as SC_BLOCK_COMMENT,
// and verified by hand during a self-review pass -- see the migration
// plan), just not exercisable end-to-end through this test harness.

// Multiple spaces between "pragma" and its argument: a minor, documented
// robustness improvement over the original (which only skipped a single
// separating whitespace character, so this used to silently fail to
// recognize the pragma at all).
TEST_F(LexerTest, Pragma_MultipleLeadingSpaces) {
    auto t = Tokenize("#pragma    strict_types\nint\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_BASIC_TYPE);
    EXPECT_EQ(pragmas & PRAGMA_STRICT_TYPES, PRAGMA_STRICT_TYPES);
}

// ============================================================================
// 9. '(' compound tokens & function pointers
// ============================================================================

TEST_F(LexerTest, Paren_ArrayOpen) {
    // Since 9.1 "({" is the ordinary '(' '{' token pair; the grammar
    // pairs them (the closer always was separate '}' ')').
    auto t = Tokenize("({ 1 })\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, '(');
    EXPECT_EQ(t[1].kind, '{');
}

TEST_F(LexerTest, Paren_ArrayOpen_WithWhitespace) {
    // whitespace between '(' and '{' is always allowed (the strict "wombles"
    // dialect that used to forbid this was removed from the driver).
    auto t = Tokenize("(  {\n 1 })\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, '(');
    EXPECT_EQ(t[1].kind, '{');
}

TEST_F(LexerTest, Paren_MappingOpen) {
    auto t = Tokenize("([ 1:2 ])\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, '(');
    EXPECT_EQ(t[1].kind, '[');
}

TEST_F(LexerTest, Paren_ScopeOperator_NotFunctionOpen) {
    // "(::foo" is '(' followed by the "::" scope operator, NOT a
    // function-pointer open -- regression test for the native "(::" rule.
    auto t = Tokenize("(::foo)\n");
    ASSERT_GE(t.size(), 3u);
    EXPECT_EQ(t[0].kind, '(');
    EXPECT_EQ(t[1].kind, L_COLON_COLON);
    EXPECT_EQ(t[2].kind, L_IDENTIFIER);
}

TEST_F(LexerTest, Paren_Bare) {
    auto t = Tokenize("(1)\n");
    ASSERT_GE(t.size(), 1u);
    EXPECT_EQ(t[0].kind, '(');
}

TEST_F(LexerTest, FunctionPointer_AnonymousOpen) {
    auto t = Tokenize("(: $1 :)\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, L_FUNCTION_OPEN);
    EXPECT_EQ(t[1].kind, L_PARAMETER);
    EXPECT_EQ(t[1].number, 0);  // $1 -> parameter index 0
}

TEST_F(LexerTest, FunctionPointer_AnonymousOpen_NoLeadingSpace) {
    // no whitespace between "(:" and the body -- exercises the SC_FUNC_OPEN
    // "anything else, push it back" fallback rather than the whitespace rule
    auto t = Tokenize("(:$1:)\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, L_FUNCTION_OPEN);
    EXPECT_EQ(t[1].kind, L_PARAMETER);
}

// $0 (index < 1): reported as an error, but the lexer still returns
// L_PARAMETER (with the out-of-range value) rather than aborting the token
// stream, so the parser can keep going and report further errors.
TEST_F(LexerTest, FunctionPointer_ParamZero_Error) {
    auto t = Tokenize("(: $0 :)\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, L_FUNCTION_OPEN);
    EXPECT_EQ(t[1].kind, L_PARAMETER);
    EXPECT_EQ(t[1].number, -1);
    EXPECT_GT(num_parse_error, 0);
}

// $256 -> index 255, past the 255-parameter limit.
TEST_F(LexerTest, FunctionPointer_ParamTooLarge_Error) {
    auto t = Tokenize("(: $256 :)\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, L_FUNCTION_OPEN);
    EXPECT_EQ(t[1].kind, L_PARAMETER);
    EXPECT_EQ(t[1].number, 255);
    EXPECT_GT(num_parse_error, 0);
}

TEST_F(LexerTest, FunctionPointer_DollarOutsideContext_Error) {
    auto t = Tokenize("$1\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, YYerror);
    EXPECT_GT(num_parse_error, 0);
}

TEST_F(LexerTest, FunctionPointer_BareDollarOutsideContext_Error) {
    auto t = Tokenize("$\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, YYerror);
    EXPECT_GT(num_parse_error, 0);
}

// ============================================================================
// 10. Multi-token expressions — end-to-end sanity across several rules
// ============================================================================

TEST_F(LexerTest, Expr_IntVarDecl) {
    auto t = Tokenize("int x = 42;\n");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].kind, L_BASIC_TYPE);
    EXPECT_EQ(t[1].kind, L_IDENTIFIER);
    EXPECT_EQ(t[2].kind, L_ASSIGN);
    EXPECT_EQ(t[3].kind, L_NUMBER);
    EXPECT_EQ(t[3].number, 42);
    EXPECT_EQ(t[4].kind, ';');
}

TEST_F(LexerTest, Expr_FunctionCall) {
    auto t = Tokenize("foo(1, 2)\n");
    ASSERT_EQ(t.size(), 6u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, '(');
    EXPECT_EQ(t[2].kind, L_NUMBER);
    EXPECT_EQ(t[3].kind, ',');
    EXPECT_EQ(t[4].kind, L_NUMBER);
    EXPECT_EQ(t[5].kind, ')');
}

// ============================================================================
// 11. Large input — ring-buffer refill stress test
// ============================================================================

TEST_F(LexerTest, LargeFile_RingBufferRefill) {
    std::string s = "int x;\n";
    while (s.size() < 70000) s += "int x;\n";
    auto toks = Tokenize(s);
    ASSERT_FALSE(toks.empty());
    EXPECT_EQ(toks[0].kind, L_BASIC_TYPE);
    // every triple (int, x, ;) must appear
    EXPECT_EQ(toks.size() % 3, 0u);
}

// ============================================================================
// 12. Template literals (`...${expr}...`)
// ============================================================================

// No interpolation at all: returned as a plain L_STRING, not L_TEMPLATE_HEAD/
// TAIL, so it folds into ordinary string concatenation just like "...".
TEST_F(LexerTest, TemplateLiteral_NoInterpolation_IsPlainString) {
    auto t = Tokenize("`hello world`\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "hello world");
}

// The '}' that closes ${...} is consumed by the lexer to resume
// SC_TEMPLATE_BODY -- it's never itself returned as a '}' token (matching
// the grammar: template_literal: L_TEMPLATE_HEAD expr template_parts has no
// '}' in it), so there are 3 tokens here, not 4.
TEST_F(LexerTest, TemplateLiteral_SingleInterpolation_HeadAndTail) {
    auto t = Tokenize("`Hello, ${name}!`\n");
    ASSERT_EQ(t.size(), 3u);
    EXPECT_EQ(t[0].kind, L_TEMPLATE_HEAD);
    EXPECT_EQ(t[0].str, "Hello, ");
    EXPECT_EQ(t[1].kind, L_IDENTIFIER);
    EXPECT_EQ(t[2].kind, L_TEMPLATE_TAIL);
    EXPECT_EQ(t[2].str, "!");
}

// Regression test: the head fragment's string value used to come back
// containing the *tail* fragment's text (scratch_copy()'s small ring buffer
// getting overwritten while the head sat on the parser's value stack through
// the whole interpolated expression -- see the F_TEMPLATE_COERCE commit).
// A nested string literal inside the interpolation is exactly what triggered
// it, so this keeps that specific shape under test at the lexer level.
TEST_F(LexerTest, TemplateLiteral_NestedStringInInterpolation_HeadNotClobbered) {
    auto t = Tokenize("`${strlen(\"hi\")} chars`\n");
    ASSERT_GE(t.size(), 2u);
    EXPECT_EQ(t[0].kind, L_TEMPLATE_HEAD);
    EXPECT_EQ(t[0].str, "");
    EXPECT_EQ(t.back().kind, L_TEMPLATE_TAIL);
    EXPECT_EQ(t.back().str, " chars");
}

TEST_F(LexerTest, TemplateLiteral_MultipleInterpolations_Middle) {
    auto t = Tokenize("`${name} has ${count} apples.`\n");
    // L_TEMPLATE_HEAD("") name L_TEMPLATE_MIDDLE(" has ") count L_TEMPLATE_TAIL(" apples.")
    // -- the closing '}' of each ${...} is consumed by the lexer, not
    // returned as its own token (see the single-interpolation test above).
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].kind, L_TEMPLATE_HEAD);
    EXPECT_EQ(t[0].str, "");
    EXPECT_EQ(t[2].kind, L_TEMPLATE_MIDDLE);
    EXPECT_EQ(t[2].str, " has ");
    EXPECT_EQ(t[4].kind, L_TEMPLATE_TAIL);
    EXPECT_EQ(t[4].str, " apples.");
}

// Escapes specific to template literals: \` and \$ (prevents interpolation).
TEST_F(LexerTest, TemplateLiteral_EscapedBacktickAndDollar) {
    auto t = Tokenize("`a\\`b\\$c`\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "a`b$c");
}

// A bare newline inside a template literal is removed, not preserved
// (unlike a regular string, where it's kept) -- see docs/lpc/types/strings.md.
TEST_F(LexerTest, TemplateLiteral_BareNewlineRemoved) {
    auto t = Tokenize("`line one\nline two`\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "line oneline two");
}

// A real '{'/'}' block inside ${...} (e.g. an anonymous function literal's
// body) must not be mistaken for the '}' that closes the interpolation --
// brace-depth tracking on the plain '{'/'}' rules has to see through it.
TEST_F(LexerTest, TemplateLiteral_BracesInsideInterpolation_NotMistakenForClose) {
    auto t = Tokenize("`${ (: { 1; } :) }`\n");
    // L_TEMPLATE_HEAD("") ... the '{'/'}' pair from the block must come back
    // as ordinary '{'/'}' tokens, with the real closing '}' of ${...} (which
    // is NOT itself returned as a token -- it's consumed to resume
    // SC_TEMPLATE_BODY) landing on a final L_STRING/L_TEMPLATE_TAIL.
    ASSERT_FALSE(t.empty());
    EXPECT_EQ(t[0].kind, L_TEMPLATE_HEAD);
    bool saw_open_brace = false, saw_close_brace = false;
    for (auto& tok : t) {
        if (tok.kind == '{') saw_open_brace = true;
        if (tok.kind == '}') saw_close_brace = true;
    }
    EXPECT_TRUE(saw_open_brace);
    EXPECT_TRUE(saw_close_brace);
    EXPECT_EQ(t.back().kind, L_TEMPLATE_TAIL);
    EXPECT_EQ(t.back().str, "");
    EXPECT_EQ(num_parse_error, 0);
}

// Regression test: an array literal "({ ... })" inside ${...}. Its '{'
// must count toward template brace depth (else the array's '}' is
// misread as the end of the interpolation and the rest of the template
// garbles). Since 9.1 the plain '{' rule fires naturally for it -- the
// old single-token L_ARRAY_OPEN special case in the depth tracking is
// gone.
TEST_F(LexerTest, TemplateLiteral_ArrayLiteralInsideInterpolation) {
    auto t = Tokenize("`${ ({ 1, 2 })[0] } end`\n");
    ASSERT_FALSE(t.empty());
    EXPECT_EQ(t[0].kind, L_TEMPLATE_HEAD);
    bool saw_array_open = false;
    for (size_t i = 0; i + 1 < t.size(); i++) {
        if (t[i].kind == '(' && t[i + 1].kind == '{') saw_array_open = true;
    }
    EXPECT_TRUE(saw_array_open);
    EXPECT_EQ(t.back().kind, L_TEMPLATE_TAIL);
    EXPECT_EQ(t.back().str, " end");
    EXPECT_EQ(num_parse_error, 0);
}

// Nested template literal inside an interpolation: `outer ${`inner ${x}`} end`.
TEST_F(LexerTest, TemplateLiteral_Nested) {
    auto t = Tokenize("`outer ${`inner ${x}`} end`\n");
    ASSERT_FALSE(t.empty());
    EXPECT_EQ(t[0].kind, L_TEMPLATE_HEAD);
    EXPECT_EQ(t[0].str, "outer ");
    // Inner template's own head/tail must show up as distinct tokens, and
    // the outer literal's tail ("...end") must still come back intact and
    // unclobbered after all that nested scanning.
    bool saw_inner_head = false;
    for (auto& tok : t) {
        if (tok.kind == L_TEMPLATE_HEAD && tok.str == "inner ") saw_inner_head = true;
    }
    EXPECT_TRUE(saw_inner_head);
    EXPECT_EQ(t.back().kind, L_TEMPLATE_TAIL);
    EXPECT_EQ(t.back().str, " end");
    EXPECT_EQ(num_parse_error, 0);
}

TEST_F(LexerTest, TemplateLiteral_UnterminatedEOF_Error) {
    auto t = Tokenize("`hello\n");
    EXPECT_GT(num_parse_error, 0);
}

// Regression test for a real out-of-bounds bug caught by UBSan during
// self-review: exceeding MAX_TEMPLATE_NESTING left template_nesting stuck
// one past the array bound (lexerror() doesn't longjmp, so scanning
// continues after the "too deep" error), and the very next '{'/'}'
// anywhere later in the same file wrote out of bounds via
// template_brace_depth[template_nesting]. Deliberately exceeds
// MAX_TEMPLATE_NESTING (16) and then uses a plain brace block afterward in
// the same source, in the same spirit as the heredoc/directive recovery
// tests above -- this needs to not crash under ASan/UBSan, which is the
// whole point of the test.
TEST_F(LexerTest, TemplateLiteral_ExceedsMaxNesting_RecoversWithoutOOB) {
    std::string s;
    for (int i = 0; i < 20; i++) s += "`${";
    s += "x";
    for (int i = 0; i < 20; i++) s += "}`";
    s += "\n{ 1; }\n";  // brace usage after the nesting error, same file
    auto t = Tokenize(s);
    EXPECT_GT(num_parse_error, 0);
}

// Regression test for a second leak found alongside the nesting-limit OOB
// above: STR_CHECK_OVERFLOW() firing while accumulating a template's
// MIDDLE/TAIL fragment (template_is_continuation) bails out via BEGIN
// (INITIAL) + return without ever reaching the normal "template_nesting--"
// on close, so that level leaked for the rest of the compile. Confirmed
// empirically (by temporarily disabling the fix) that this single overflow
// event alone leaves template_nesting == 1 instead of back at 0 -- kept
// deliberately small (one overflow, not many) since a large/repeated
// version of this exhausts the small fixed-size global scratchpad buffer
// that scratch_copy() allocates out of (a separate, pre-existing capacity
// limit of this raw Tokenize() harness, which never frees token string
// values between calls -- not something this test is about).
TEST_F(LexerTest, TemplateLiteral_OverflowDuringMiddleFragment_DoesNotLeakNesting) {
    std::string s = "`${x}";
    s += std::string(5000, 'a');  // > MAXLINE (4096), overflows mid-MIDDLE-fragment
    s += "${y}z`\n";

    auto t = Tokenize(s);
    EXPECT_GT(num_parse_error, 0);
    EXPECT_EQ(g_last_lex_ctx.template_nesting, 0);
}

// ============================================================================
// 13. Mapping dot access / optional chaining tokens
// ============================================================================

TEST_F(LexerTest, OptionalDot_BeforeIdentifier) {
    auto t = Tokenize("m?.key\n");
    ASSERT_EQ(t.size(), 3u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, L_OPTIONAL_DOT);
    EXPECT_EQ(t[2].kind, L_IDENTIFIER);
}

TEST_F(LexerTest, OptionalDot_BeforeBracket) {
    auto t = Tokenize("m?.[1]\n");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, L_OPTIONAL_DOT);
    EXPECT_EQ(t[2].kind, '[');
    EXPECT_EQ(t[3].kind, L_NUMBER);
    EXPECT_EQ(t[4].kind, ']');
}

// Disambiguation: "? .5" (ternary float branch) must NOT be read as "?."
// followed by "5" -- '?' alone, then the float literal ".5".

// "?." is only recognized as L_OPTIONAL_DOT when followed by an
// identifier-start char or '[' (see the lookahead guard in lex.l). Right
// after '?', a '.' followed by a digit can't be the start of any valid
// LPC construct in this dialect either way (leading-dot floats like ".5"
// aren't supported -- ".5" alone always lexes as '.' then L_NUMBER, never
// L_REAL), but the guard must still fall back to plain '?' rather than
// eating the '.' as part of an optional-chaining token, so the '.'/'5' that
// follow get lexed the same way they would anywhere else.
TEST_F(LexerTest, OptionalDot_LookaheadGuard_FallsBackOnNonIdentifierStart) {
    auto t = Tokenize("a?.5\n");
    ASSERT_EQ(t.size(), 4u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, '?');
    EXPECT_EQ(t[2].kind, '.');
    EXPECT_EQ(t[3].kind, L_NUMBER);
    EXPECT_EQ(t[3].number, 5);
}

// A plain ternary with whitespace around '?' is completely unaffected by
// the new "?." rule (no adjacent "?." substring appears at all).
TEST_F(LexerTest, Ternary_StillLexesNormally) {
    auto t = Tokenize("a ? b : c\n");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, '?');
    EXPECT_EQ(t[2].kind, L_IDENTIFIER);
    EXPECT_EQ(t[3].kind, ':');
    EXPECT_EQ(t[4].kind, L_IDENTIFIER);
}

// "??" (nullish coalescing) must still win over "?." + more when there's no
// dot at all, and "??" followed by a dot-like continuation shouldn't merge.
TEST_F(LexerTest, QuestionQuestion_StillLexesNormally) {
    auto t = Tokenize("a ?? b\n");
    ASSERT_EQ(t.size(), 3u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, L_QUESTION_QUESTION);
    EXPECT_EQ(t[2].kind, L_IDENTIFIER);
}

TEST_F(LexerTest, DotOptional_BracketForm) {
    auto t = Tokenize("m.?[1]\n");
    ASSERT_EQ(t.size(), 5u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, L_DOT_OPTIONAL);
    EXPECT_EQ(t[2].kind, '[');
    EXPECT_EQ(t[3].kind, L_NUMBER);
    EXPECT_EQ(t[4].kind, ']');
}

// Plain '.' must still lex correctly when not followed by '?'.
TEST_F(LexerTest, PlainDot_StillLexesNormally) {
    auto t = Tokenize("m.key\n");
    ASSERT_EQ(t.size(), 3u);
    EXPECT_EQ(t[0].kind, L_IDENTIFIER);
    EXPECT_EQ(t[1].kind, '.');
    EXPECT_EQ(t[2].kind, L_IDENTIFIER);
}

// Range operator ".." must not be swallowed by the new ".?"/"?." rules.
TEST_F(LexerTest, RangeDotDot_StillLexesNormally) {
    auto t = Tokenize("a[0..5]\n");
    bool saw_range = false;
    for (auto& tok : t) {
        if (tok.kind == L_RANGE) saw_range = true;
    }
    EXPECT_TRUE(saw_range);
}

// Bug 8: multiline template literal with CRLF line endings should not contain stray \r.
TEST_F(LexerTest, TemplateLiteral_Crlf_StripsCarriageReturn) {
    auto t = Tokenize("`line one\r\nline two`\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "line oneline two");
}

// Bug 12: string literal escaped newlines with CRLF
TEST_F(LexerTest, String_LineContinuation_Crlf) {
    auto t = Tokenize("\"line one\\\r\nline two\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "line oneline two");
}

// Bug 13: string literal raw CRLF newlines should normalize to \n
TEST_F(LexerTest, String_RawNewline_Crlf) {
    auto t = Tokenize("\"line one\r\nline two\"\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_STRING);
    EXPECT_EQ(t[0].str, "line one\nline two");
}

// Bug 12: character literal escaped newlines with CRLF
TEST_F(LexerTest, Char_EscapeNewline_Crlf) {
    auto t = Tokenize("'\\\r\n'\n");
    ASSERT_EQ(t.size(), 1u);
    EXPECT_EQ(t[0].kind, L_NUMBER);
    EXPECT_EQ(t[0].number, '\n');
}
