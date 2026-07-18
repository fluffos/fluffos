// test_compiler.cc — unit tests for the LPC compiler front-end's
// preprocessing behavior (#define/#undef, conditionals, #include, macro
// expansion, __LINE__/__FILE__, directives), which lives in the lexer's
// single scan (lexer_rules_pp.cc + lexer.l's directive rule).
//
// These tests drive the REAL lexer end-to-end: TokenizeSession() feeds a
// source string through start_new_file()+yylex(), and
// the LpcPreprocessor shim below reconstructs comparable text from the
// resulting token stream (whitespace-normalized comparison), preserving the
// original text-oriented test bodies while asserting through actual
// tokenization -- a strictly stronger check than the old standalone
// preprocessor's string-in/string-out tests. No VM is booted:
// compiler_vm_context stays null, so error paths skip master applies.

#include "base/std.h"

#include <gtest/gtest.h>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#endif

#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "base/internal/debugmalloc.h"
#include "base/internal/stralloc.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/grammar_rules.h"
#include "compiler/internal/grammar.autogen.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/lexer_utils.h"
#include "compiler/internal/lexer_rules_pp.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/stage_output.h"
#include "mainlib.h"
#include "vm/vm.h"

// ---------------------------------------------------------------------------
// Scratchpad arena (scratchpad.cc): direct unit coverage of the monotonic
// bump allocator behind the compiler's transient strings and the parser's
// value stack. Each test starts from a clean arena; scratch_destroy() is
// the bulk reset.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Scratchpad arena (scratchpad.cc): direct unit coverage of the monotonic
// bump allocator behind the compiler's transient strings and the parser's
// value stack. Each test starts from a clean arena; scratch_destroy() is
// the bulk reset.
// ---------------------------------------------------------------------------
TEST(Scratchpad, NewStringRoundTrip) {
  scratch_destroy();
  ScratchString* a = scratch_new_string("hello");
  ScratchString* b = scratch_new_string("");
  ScratchString* c = scratch_new_string("a longer string with spaces");
  EXPECT_EQ(*a, "hello");
  EXPECT_TRUE(b->empty());
  EXPECT_EQ(*c, "a longer string with spaces");
  // Distinct stable objects: later allocations never disturb earlier ones.
  EXPECT_EQ(*a, "hello");
  scratch_destroy();
}

TEST(Scratchpad, LargeStringNoCap) {
  // The historical arena capped inline strings at 255 bytes and malloc'd
  // the rest; this arena has no cap. Well past a single 64K chunk, so an
  // exact-fit overflow chunk is exercised.
  scratch_destroy();
  std::string big(200000, 'x');
  ScratchString* p = scratch_new_string(big);
  EXPECT_EQ(p->size(), 200000u);
  EXPECT_EQ((*p)[0], 'x');
  EXPECT_EQ((*p)[199999], 'x');
  scratch_destroy();
}

TEST(Scratchpad, DestroyResetsToPersistentBase) {
  // scratch_destroy frees overflow chunks and RESETS the persistent base
  // chunk: the first allocation after each destroy lands at the same
  // address -- back-to-back small compiles reuse the block, no malloc.
  scratch_destroy();
  void* first = scratch_raw_allocate(64, 1);
  scratch_destroy();
  void* again = scratch_raw_allocate(64, 1);
  EXPECT_EQ(first, again);
  scratch_destroy();
}

TEST(Scratchpad, ScratchStringIsArenaBacked) {
  // A std::basic_string over ScratchAllocator: grows, concatenates, and
  // destructs like std::string, but its storage is arena memory (bulk
  // freed at scratch_destroy). ASan would catch any mismanagement.
  scratch_destroy();
  {
    ScratchString s("foo");
    s += "bar";
    s += ScratchString("baz");
    for (int i = 0; i < 1000; ++i) s.push_back('x');  // growth churn
    EXPECT_EQ(s.substr(0, 9), "foobarbaz");
    EXPECT_EQ(s.size(), 9u + 1000u);
    // Materialize a value-stack token from the accumulated text.
    ScratchString* tok = scratch_new_string(s);
    EXPECT_EQ(*tok, s);
  }  // ScratchString destructs here -> no-op deallocate into the arena
  scratch_destroy();
}

TEST(Scratchpad, ScratchVectorIsArenaBacked) {
  scratch_destroy();
  {
    ScratchVector<int> v;
    for (int i = 0; i < 5000; ++i) v.push_back(i);
    EXPECT_EQ(v.size(), 5000u);
    EXPECT_EQ(v[4999], 4999);
    ScratchVector<ScratchString> names;
    for (int i = 0; i < 100; ++i) names.emplace_back("name");
    EXPECT_EQ(names.size(), 100u);
    EXPECT_EQ(names[99], "name");
  }
  scratch_destroy();
}

TEST(Scratchpad, PlacedStringGrowthStaysOnArena) {
  // scratch_new_string placement-news the OBJECT into the arena; every
  // later MODIFICATION (+=, insert, growth past SSO) must draw its new
  // buffer from the arena too. Proof: the arena's cycle-byte counter
  // advances by at least the new capacity when a placed string grows.
  scratch_destroy();
  ScratchString* tok = scratch_new_string("x");
  std::size_t before = scratch_stats().cycle_bytes;
  for (int i = 0; i < 100; i++) *tok += "0123456789abcdef";  // 1.6KB growth
  std::size_t after = scratch_stats().cycle_bytes;
  EXPECT_GE(after - before, tok->size());  // growth came from the arena
  EXPECT_EQ(tok->substr(0, 17), "x0123456789abcdef");
  scratch_destroy();
}

TEST(Scratchpad, TinyChunkWorstCase) {
  // Force pathological 400-byte chunks: every ScratchString growth past
  // 400B goes oversize, advances happen constantly, and the retained
  // array cycles. Correctness must hold (ASan guards the rest).
  scratch_set_chunk_size_for_testing(400);
  for (int round = 0; round < 3; round++) {
    {
      // Everything arena-backed must leave scope BEFORE the reset:
      // with 400B chunks these land in OVERFLOW chunks that
      // scratch_destroy actually frees, so even a destructor
      // touching them afterwards is use-after-free (ASan-proven --
      // this brace block is the documented discipline).
      std::vector<ScratchString*> toks;
      for (int i = 0; i < 200; i++) {
        toks.push_back(scratch_new_string("token_" + std::to_string(i)));
      }
      ScratchString big;
      for (int i = 0; i < 500; i++) big += "0123456789abcdef";  // 8KB >> chunk
      EXPECT_EQ(big.size(), 8000u);
      for (int i = 0; i < 200; i++) {
        EXPECT_EQ(std::string_view(*toks[i]), "token_" + std::to_string(i));
      }
      ScratchVector<ScratchString> v;
      for (int i = 0; i < 64; i++) v.emplace_back("arg");
      EXPECT_EQ(v.size(), 64u);
    }
    scratch_destroy();
  }
  ScratchStats st = scratch_stats();
  EXPECT_GT(st.chunk_mallocs, 0u);                  // the worst case actually spilled
  scratch_set_chunk_size_for_testing(1024 * 1024);  // restore production
}

TEST(Scratchpad, StaleObjectResetAfterDestroyIsSafe) {
  // The scanner context holds ScratchStrings across compiles; after
  // scratch_destroy their buffers are gone. The documented recovery --
  // assign a fresh empty string before any use -- must be safe (no-op
  // deallocate) and leave the object fully usable.
  scratch_destroy();
  static ScratchString persistent;  // simulates compiler_context_t member
  persistent = ScratchString();
  persistent.reserve(100);
  persistent += "first compile text that exceeds sso for sure 0123456789";
  scratch_destroy();             // arena reset; `persistent` now stale
  persistent = ScratchString();  // the documented re-initialization
  persistent += "second";
  EXPECT_EQ(persistent, "second");
  persistent = ScratchString();
  scratch_destroy();
}

struct Token {
  int kind;
  LPC_INT number;
  LPC_FLOAT real;
  std::string text;
  std::string str;
  bool is_char_literal = false;
  bool is_template = false;
};

#include "base/internal/rc.h"

// Shared across the lightweight tokenizer harness AND the full-boot
// compile-entry tests: whichever initializes first wins, and the other
// must NOT re-run init_strings (a second call orphans the first 64KB
// string table -- a real LeakSanitizer catch from CI when one test mixed
// both environments in one process).
static bool g_test_env_inited = false;

static std::vector<Token> TokenizeSession(bool keep_macros, const std::string& source,
                                          const char* filename = "test") {
  if (!g_test_env_inited) {
    chdir(TESTSUITE_DIR);
    config_init();
    init_strings();
    init_identifiers();
    g_test_env_inited = true;
  }

  for (int i = 0; i < NUMAREAS; i++) {
    mem_block[i].block =
        reinterpret_cast<char*>(DMALLOC(START_BLOCK_SIZE, TAG_COMPILER, "Tokenize"));
    mem_block[i].current_size = 0;
    mem_block[i].max_size = START_BLOCK_SIZE;
  }
  memset(string_tags, 0, sizeof(string_tags));
  freed_string = -1;
  num_parse_error = 0;

  ScratchString norm_file = normalize_filename(filename);
  current_file = make_shared_string(norm_file.c_str());
  current_file_id = add_program_file(norm_file.c_str(), /*top=*/1);

  compiler_context_t ctx;
  void* scanner = nullptr;
  yylex_init_extra(&ctx, &scanner);

  start_new_file(source, scanner, keep_macros);

  std::vector<Token> toks;
  YYSTYPE yylval;
  for (;;) {
    YYLTYPE yylloc;
    int k = yylex(&yylval, &yylloc, scanner);
    if (k <= 0) break;
    Token t;
    t.kind = k;
    t.number = (k == L_NUMBER || k == L_PARAMETER) ? yylval.number : 0;
    t.real = (k == L_REAL) ? yylval.real : 0.0;
    t.text = yyget_text(scanner);
    bool has_string_payload =
        (k == L_STRING || k == L_TEMPLATE_HEAD || k == L_TEMPLATE_MIDDLE || k == L_TEMPLATE_TAIL);
    t.str = (has_string_payload && yylval.string) ? yylval.string->c_str() : "";
    t.is_char_literal = (k == L_NUMBER && ctx.is_char_literal);
    ctx.is_char_literal = false;
    t.is_template = ctx.is_template;
    ctx.is_template = false;
    toks.push_back(std::move(t));
  }

  // Ownership discipline (see lpc_lex_scanner_destroyed): whoever
  // destroys a scanner clears the active pointer, or the NEXT compile's
  // very first current_line read dereferences the destroyed scanner's
  // guts -- an order-dependent use-after-free that intermittently
  // segfaulted CompileEntry tests running after any tokenizer test.
  lpc_lex_scanner_destroyed(scanner);
  yylex_destroy(scanner);
  // Symmetric with start_new_file() above: that call arms lexer_utils.cc's
  // file-local main_filename exactly once per top-level compile (a
  // make_shared_string() ref), and only end_new_file() ever disarms it.
  // Skipping this call here (as this function did before) leaves
  // main_filename permanently pinned to THIS session's filename for the
  // rest of the process -- harmless-looking on its own, but a real bug
  // when a later CompileEntry test's ensure_compile_env() then
  // re-initializes the shared-string table (see the g_test_env_inited
  // comment above): the pin still points at a live, valid block, but one
  // that's no longer reachable from the fresh table, so the eventual
  // free_string() of a REAL compile's own main_filename trips "free_string
  // called on non-shared string" against this session's stale pin.
  end_new_file();
  free_string(const_cast<char*>(current_file));
  current_file = nullptr;
  for (int i = 0; i < NUMAREAS; i++) {
    FREE(mem_block[i].block);
  }
  return toks;
}

static std::vector<Token> Tokenize(const std::string& source, const char* filename = "test") {
  return TokenizeSession(false, source, filename);
}

static std::string normalize_whitespace(const std::string& s) {
  std::string res;
  bool in_string = false;
  bool in_char = false;
  for (size_t i = 0; i < s.size(); ++i) {
    if (in_string) {
      if (s[i] == '\\' && i + 1 < s.size()) {
        res += s[i];
        res += s[i + 1];
        i++;
      } else if (s[i] == '"') {
        res += s[i];
        in_string = false;
      } else {
        res += s[i];
      }
    } else if (in_char) {
      if (s[i] == '\\' && i + 1 < s.size()) {
        res += s[i];
        res += s[i + 1];
        i++;
      } else if (s[i] == '\'') {
        res += s[i];
        in_char = false;
      } else {
        res += s[i];
      }
    } else {
      if (s[i] == '"') {
        res += s[i];
        in_string = true;
      } else if (s[i] == '\'') {
        res += s[i];
        in_char = true;
      } else if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
        // skip
      } else {
        res += s[i];
      }
    }
  }
  return res;
}

struct NormalizedString {
  std::string val;

  bool operator==(const NormalizedString& other) const {
    return normalize_whitespace(val) == normalize_whitespace(other.val);
  }
  bool operator==(const std::string& other) const {
    return normalize_whitespace(val) == normalize_whitespace(other);
  }
  bool operator==(const char* other) const {
    return normalize_whitespace(val) == normalize_whitespace(other);
  }
  operator std::string() const { return val; }
};

inline std::ostream& operator<<(std::ostream& os, const NormalizedString& ns) {
  return os << ns.val;
}

class LpcPreprocessor {
 public:
  bool first_chunk_ = true;  // chunk 2+ keeps the user macro table
  std::vector<std::string> errors_;

  static std::shared_ptr<LpcPreprocessor> make_session() {
    return std::make_shared<LpcPreprocessor>();
  }

  NormalizedString preprocess_next(const std::string& src, const char* filename = "test") {
    num_parse_error = 0;
    auto tokens = TokenizeSession(!first_chunk_, src, filename);
    first_chunk_ = false;
    errors_.clear();
    if (num_parse_error > 0) {
      errors_.push_back("error");
    }

    std::string result;
    for (size_t i = 0; i < tokens.size(); ++i) {
      if (i > 0) result += " ";
      const auto& t = tokens[i];
      if (t.kind == L_STRING) {
        if (t.is_template) {
          result += "`" + t.str + "`";
        } else {
          result += stringize(t.str);
        }
      } else if (t.kind == L_TEMPLATE_HEAD) {
        result += "`" + t.str + "${";
      } else if (t.kind == L_TEMPLATE_MIDDLE) {
        result += "}" + t.str + "${";
      } else if (t.kind == L_TEMPLATE_TAIL) {
        result += "}" + t.str + "`";
      } else if (t.kind == L_NUMBER) {
        if (t.is_char_literal) {
          if (t.number == '\n')
            result += "'\\n'";
          else if (t.number == '\t')
            result += "'\\t'";
          else if (t.number == '\r')
            result += "'\\r'";
          else if (t.number == '\b')
            result += "'\\b'";
          else if (t.number == '\'')
            result += "'\\''";
          else if (t.number == '\\')
            result += "'\\\\'";
          else
            result += "'" + std::string(1, (char)t.number) + "'";
        } else {
          result += std::to_string(t.number);
        }
      } else if (t.kind == L_REAL) {
        result += std::to_string(t.real);
      } else {
        result += t.text;
      }
    }
    return NormalizedString{std::move(result)};
  }

  const std::vector<std::string>& errors() const { return errors_; }
};

static NormalizedString pp(const std::string& src, const char* filename = "test") {
  static bool predefs_added = false;
  if (!predefs_added) {
    add_predefine("FLUFFOS", -1, "");
    add_predefine("MUDOS", -1, "");
    add_predefine("__PACKAGE_TRIM__", -1, "");
    add_predefine("SIZEOFINT", -1, "8");
    predefs_added = true;
  }
  auto p = LpcPreprocessor::make_session();
  return p->preprocess_next(src, filename);
}

// ---------------------------------------------------------------------------
// 1. Basic passthrough
// ---------------------------------------------------------------------------

TEST(Preprocessor, PassthroughPlainCode) {
  const std::string src = R"(int x;
void foo() {}
)";
  EXPECT_EQ(pp(src), src);
}

TEST(Preprocessor, PassthroughEmptyInput) { EXPECT_EQ(pp(""), ""); }

// ---------------------------------------------------------------------------
// 2. Object-like macros
// ---------------------------------------------------------------------------

TEST(Preprocessor, ObjectMacroSimple) {
  EXPECT_EQ(pp(R"(
#define FOO 42
int x = FOO;
)"),
            R"(

int x = 42;
)");
}

TEST(Preprocessor, ObjectMacroMultiToken) {
  EXPECT_EQ(pp(R"(
#define MAX 100
#define MIN 0
int a = MAX;
int b = MIN;
)"),
            R"(


int a = 100;
int b = 0;
)");
}

TEST(Preprocessor, ObjectMacroInExpression) {
  EXPECT_EQ(pp(R"(
#define SIZE 8
char buf[SIZE];
)"),
            R"(

char buf[8];
)");
}

TEST(Preprocessor, ObjectMacroUndef) {
  EXPECT_EQ(pp(R"(
#define X 1
#undef X
int v = X;
)"),
            R"(


int v = X;
)");
}

TEST(Preprocessor, ObjectMacroChained) {
  EXPECT_EQ(pp(R"(
#define B 99
#define A B
int x = A;
)"),
            R"(


int x = 99;
)");
}

TEST(Preprocessor, ObjectMacroRecursionGuard) {
  // Self-referencing macro must not expand infinitely
  EXPECT_EQ(pp(R"(
#define X X
int v = X;
)"),
            R"(

int v = X;
)");
}

// ---------------------------------------------------------------------------
// 2b. Self-reference "blue paint" -- per-OCCURRENCE, counted at expansion
// time into compiler_context_t::pending_plain and consumed at rescan (this
// replaced the "\x1e<name>" end-of-splice sentinel token; see lexer.h's
// pending_plain comment). These pin the count bookkeeping specifically:
// exactly the guarded literal occurrences stay plain, nothing more (a later
// fresh use of the same macro expands again) and nothing less (every
// guarded occurrence in one expansion is skipped, however many).
// ---------------------------------------------------------------------------

TEST(Preprocessor, BluePaintDoesNotStickAcrossUses) {
  // Each use paints only its own literal occurrence; the next use of the
  // same macro must expand again (a leaked count would leave `y = A;`).
  EXPECT_EQ(pp(R"(
#define A A + 1
int x = A;
int y = A;
)"),
            "int x = A + 1; int y = A + 1;");
}

TEST(Preprocessor, BluePaintCountsMultipleOccurrencesInOneBody) {
  // One expansion leaving TWO guarded literals must skip exactly two
  // occurrences at rescan -- an under-count would re-expand and an
  // over-count would suppress the later fresh use.
  EXPECT_EQ(pp(R"(
#define D D + D
int x = D;
int y = D;
)"),
            "int x = D + D; int y = D + D;");
}

TEST(Preprocessor, BluePaintMutualRecursionTerminates) {
  // X -> Y -> X(guarded): the inner X is left literal by the text-level
  // guard and must stay a plain identifier at rescan.
  EXPECT_EQ(pp(R"(
#define X Y
#define Y X
int a = X;
int b = Y;
)"),
            "int a = X; int b = Y;");
}

TEST(Preprocessor, BluePaintFunctionLikeSelfRefIgnoresFollowingParens) {
  // A PAINTED function-like name must stay plain even though its literal
  // occurrence is immediately followed by '(' at rescan -- the paint
  // sticks to the occurrence (C semantics), it isn't re-evaluated
  // against what follows in the stream.
  EXPECT_EQ(pp("#define F(x) F(x + 1)\nint v = F(2);"), "int v = F(2 + 1);");
}

TEST(Preprocessor, CrlfMultiLineDefineAndUse) {
  // Windows CI checks the tree out with CRLF (autocrlf): a multi-line
  // #define whose continuation ends "\\\r\n" must fold exactly like
  // the LF form, and the expanded body must not carry stray '\r'
  // bytes. Mirrors testsuite/include/tests.h's OUTPUT macro, whose
  // continuation leaking as top-level code was the July-1 Windows CI
  // failure signature.
  EXPECT_EQ(pp("#define SAFE(x) do {x} while(0)\r\n"
               "#define OUTPUT(x) SAFE(write(x); \\\r\n"
               "  if(!this_player()) { shutdown(-1); })\r\n"
               "OUTPUT(\"hi\")\r\n"),
            "do { write(\"hi\"); if (!this_player()) { shutdown(-1); } } while (0)");
}

TEST(Preprocessor, MultiLineMacroArgKeepsLineCount) {
  // A macro argument spanning a newline: the newline is consumed once
  // from the raw stream by the argument collector AND a copy of it can
  // reappear in the spliced expansion -- __LINE__ afterward must still
  // be exact (no double count).
  EXPECT_EQ(pp("#define ID(x) x\nint a = ID(1 +\n2);\nint l = __LINE__;\n"),
            "int a = 1 + 2; int l = 4;");
}

TEST(Preprocessor, DroppedMultiLineArgKeepsLineCount) {
  // The parameter is unused, so the argument text (and its newline)
  // never reappears in the expansion -- __LINE__ must still be exact
  // (no under-count either).
  EXPECT_EQ(pp("#define EAT(x)\nint a = 1; EAT(foo\nbar); int l = __LINE__;\n"),
            "int a = 1; ; int l = 3;");
}

TEST(Preprocessor, AliasToFunctionLikeExpandsWithStreamArgs) {
  // UNPAINTED function-like name left literal only because no '('
  // followed it in the expansion text itself: when the '(' turns out to
  // follow in the real input stream, the rescan expands it there -- C
  // behavior. (Under the old end-of-splice sentinel design the sentinel
  // byte sat between the spliced "F" and the stream's "(3)", silently
  // blocking this expansion; the per-occurrence counts have no such
  // in-band artifact.)
  EXPECT_EQ(pp(R"(
#define F(x) ((x) * 2)
#define APPLY F
int v = APPLY(3);
)"),
            "int v = ((3) * 2);");
}

// ---------------------------------------------------------------------------
// 3. Function-like macros
// ---------------------------------------------------------------------------

TEST(Preprocessor, FunctionMacroOneArg) {
  EXPECT_EQ(pp(R"(
#define DOUBLE(x) x + x
int v = DOUBLE(5);
)"),
            R"(

int v = 5 + 5;
)");
}

TEST(Preprocessor, FunctionMacroTwoArgs) {
  EXPECT_EQ(pp(R"(
#define ADD(a, b) a + b
int v = ADD(3, 4);
)"),
            R"(

int v = 3 + 4;
)");
}

TEST(Preprocessor, FunctionMacroNoArgs) {
  EXPECT_EQ(pp(R"(
#define ZERO() 0
int v = ZERO();
)"),
            R"(

int v = 0;
)");
}

TEST(Preprocessor, FunctionMacroWithExpression) {
  EXPECT_EQ(pp(R"(
#define MAX(a, b) ((a) > (b) ? (a) : (b))
int v = MAX(x, y);
)"),
            R"(

int v = ((x) > (y) ? (x) : (y));
)");
}

TEST(Preprocessor, FunctionMacroNestedCalls) {
  EXPECT_EQ(pp(R"(
#define SQ(x) ((x) * (x))
#define CUBE(x) ((x) * SQ(x))
int v = CUBE(3);
)"),
            R"(


int v = ((3) * ((3) * (3)));
)");
}

TEST(Preprocessor, FunctionMacroWithoutCallPassthrough) {
  // Not followed by '(' → leave unexpanded
  EXPECT_EQ(pp(R"(
#define FN(x) x
int FN;
)"),
            R"(

int FN;
)");
}

TEST(Preprocessor, FunctionMacroArgWithNestedParens) {
  EXPECT_EQ(pp(R"(
#define MAX(a, b) ((a) > (b) ? (a) : (b))
int v = MAX(f(1, 2), g(3, 4));
)"),
            R"(

int v = ((f(1, 2)) > (g(3, 4)) ? (f(1, 2)) : (g(3, 4)));
)");
}

// ---------------------------------------------------------------------------
// 4. Comment stripping
// ---------------------------------------------------------------------------

TEST(Preprocessor, LineCommentStripped) {
  // Line comment removed; the space before '//' is preserved in the output
  EXPECT_EQ(pp(R"(int x; // comment
int y;
)"),
            "int x; \nint y;\n");
}

TEST(Preprocessor, BlockCommentCollapsedToSpace) {
  // Block comment is replaced by a single space; surrounding spaces are also emitted
  EXPECT_EQ(pp(R"(int x; /* a comment */ int y;
)"),
            R"(int x;   int y;
)");
}

TEST(Preprocessor, BlockCommentMultiline) {
  // Newline inside block comment is emitted to preserve line numbers
  EXPECT_EQ(pp(R"(int x;
/* line1
   line2 */
int y;
)"),
            "int x;\n \n\nint y;\n");
}

// A block comment opened on a directive line may close on a LATER line
// (#1236): the directive rule's capture stops at the physical newline, so
// lpc_lex_complete_directive() must pull the comment's remaining lines --
// they are whitespace, not code, and they don't end the directive.

TEST(Preprocessor, DefineWithCommentSpanningLines) {
  // The exact #1236 repro shape: continuation line must not be tokenized.
  EXPECT_EQ(pp("#define WARNING_LEVEL 1 /* Change this to higher values to\n"
               "                           show more warnings. */\n"
               "int x = WARNING_LEVEL;\n"),
            "int x = 1;");
}

TEST(Preprocessor, DefineCommentTailStillDirective) {
  // Text after the close on the final line still belongs to the body (C
  // semantics: the whole comment reads as one space) -- and the tail may
  // open ANOTHER spanning comment.
  EXPECT_EQ(pp("#define V 10 /* spans\none line */ + 5\nint x = V;\n"), "int x = 10 + 5;");
  EXPECT_EQ(pp("#define W 1 /* one\n*/ + 2 /* two\n*/ + 3\nint y = W;\n"), "int y = 1 + 2 + 3;");
}

TEST(Preprocessor, DefineCommentSpanKeepsLineCount) {
  // Physical lines consumed for the comment are counted exactly once.
  EXPECT_EQ(pp("#define W 1 /* a\nb\nc */\nint l = __LINE__;\n"), "int l = 4;");
}

TEST(Preprocessor, IfWithCommentSpanningLines) {
  // On a live #if the comment is whitespace in the expression; on a dead
  // one the branch below the comment's close is still skipped.
  EXPECT_EQ(pp("#if 1 /* live,\nstill comment */\nint x = 1;\n#endif\n"), "int x = 1;");
  EXPECT_EQ(pp("#if 0 /* dead,\nstill comment */\nnot code at all\n#endif\nint y = 2;\n"),
            "int y = 2;");
}

TEST(Preprocessor, DirectiveStringDoesNotOpenComment) {
  // A '/*' inside a string literal on the directive line is body text.
  EXPECT_EQ(pp("#define S \"a/*b\"\nstring s = S;\n"), "string s = \"a/*b\";");
}

TEST(Preprocessor, DirectiveCommentUnterminatedAtEofErrors) {
  // The pull must stop at EOF with a diagnostic, not spin.
  auto p = LpcPreprocessor::make_session();
  p->preprocess_next("#define X 1 /* never closed\n", "test");
  EXPECT_FALSE(p->errors().empty());
}

// Comments are whitespace (C translation phase 3) and must be stripped
// from a directive's payload BEFORE it is parsed (#1240): a '//' tail
// captured into a macro body comments out the rest of whatever spliced
// text the macro later expands into, and a trailing comment on
// #undef/#ifdef corrupts the looked-up name.

TEST(Preprocessor, DefineLineCommentNotInBody) {
  // The #1240 repro shape: CREDITS expanding inside MIN's spliced body
  // must not swallow the rest of the expression.
  EXPECT_EQ(pp("#define CREDITS \"credits\" // Current number of credits\n"
               "#define MIN(x,y) ((x) < (y) ? (x) : (y))\n"
               "int f() { return MIN(c, m[e][CREDITS]); }\n"),
            "int f() { return ((c) < (m[e][\"credits\"]) ? (c) : (m[e][\"credits\"])); }");
}

TEST(Preprocessor, DefineBlockCommentInBodyIsOneSpace) {
  // The comment folds to ONE space, so "-/*c*/-1" stays two '-' tokens;
  // pasting them into '--' would fail the #if evaluation and flip the
  // branch.
  EXPECT_EQ(pp("#define TWO 1 -/*c*/-1\n#if TWO == 2\nint spaced;\n#else\nint pasted;\n#endif\n"),
            "int spaced;");
}

TEST(Preprocessor, DefineParamListComment) {
  // A comment inside a function-like parameter list is whitespace too.
  EXPECT_EQ(pp("#define F(a, /* second */ b) a + b\nint x = F(1, 2);\n"), "int x = 1 + 2;");
}

TEST(Preprocessor, UndefWithTrailingComment) {
  // "#undef GONE // bye" must erase GONE, not look up "GONE // bye".
  EXPECT_EQ(pp("#define GONE 5\n#undef GONE // bye\nint GONE = 6;\nint x = GONE;\n"),
            "int GONE = 6; int x = GONE;");
}

TEST(Preprocessor, IfdefWithTrailingComment) {
  EXPECT_EQ(pp("#define FEAT 1\n#ifdef FEAT // enabled\nint on;\n#endif\n"), "int on;");
  EXPECT_EQ(pp("#ifndef FEAT /* not defined */\nint off;\n#endif\n"), "int off;");
}

// ---------------------------------------------------------------------------
// 5. Conditional compilation
// ---------------------------------------------------------------------------

TEST(Preprocessor, IfdefDefined) {
  EXPECT_EQ(pp(R"(
#define FEATURE
#ifdef FEATURE
int enabled;
#endif
)"),
            R"(


int enabled;

)");
}

TEST(Preprocessor, IfdefUndefined) {
  EXPECT_EQ(pp(R"(
#ifdef FEATURE
int enabled;
#endif
)"),
            R"(

)");
}

TEST(Preprocessor, IfndefUndefined) {
  EXPECT_EQ(pp(R"(
#ifndef FEATURE
int fallback;
#endif
)"),
            R"(

int fallback;

)");
}

TEST(Preprocessor, IfndefDefined) {
  EXPECT_EQ(pp(R"(
#define FEATURE
#ifndef FEATURE
int fallback;
#endif
)"),
            R"(


)");
}

TEST(Preprocessor, IfElseTrueBranch) {
  EXPECT_EQ(pp(R"(
#define X 1
#ifdef X
int a;
#else
int b;
#endif
)"),
            R"(


int a;

)");
}

TEST(Preprocessor, IfElseFalseBranch) {
  EXPECT_EQ(pp(R"(
#ifdef X
int a;
#else
int b;
#endif
)"),
            R"(

int b;

)");
}

TEST(Preprocessor, IfElifElse) {
  EXPECT_EQ(pp(R"(
#define LEVEL 2
#if LEVEL == 1
int l1;
#elif LEVEL == 2
int l2;
#else
int lx;
#endif
)"),
            R"(


int l2;

)");
}

TEST(Preprocessor, NestedIfdef) {
  EXPECT_EQ(pp(R"(
#define A
#ifdef A
  #ifdef B
  int ab;
  #else
  int a_only;
  #endif
#endif
)"),
            "\n\n\n  \n  int a_only;\n  \n\n");
}

// ---------------------------------------------------------------------------
// 6. #if expression evaluation — parameterized
// ---------------------------------------------------------------------------

struct IfExprCase {
  const char* name;
  const char* expr;
  bool expected_true;
};

inline void PrintTo(const IfExprCase& ie, ::std::ostream* os) {
  *os << ie.name << " (" << ie.expr << " => " << (ie.expected_true ? "true" : "false") << ")";
}

class IfExprEvalTest : public ::testing::TestWithParam<IfExprCase> {};

TEST_P(IfExprEvalTest, Evaluates) {
  const auto& c = GetParam();
  std::string src = std::string("\n#if ") + c.expr + "\nint x;\n#endif\n";
  if (c.expected_true)
    EXPECT_EQ(pp(src), "\n\nint x;\n\n") << "expr: " << c.expr;
  else
    EXPECT_EQ(pp(src), "\n\n") << "expr: " << c.expr;
}

INSTANTIATE_TEST_SUITE_P(
    IfExpr, IfExprEvalTest,
    ::testing::Values(
        // basic constants
        IfExprCase{"TrueConst", "1", true}, IfExprCase{"FalseConst", "0", false},
        // arithmetic & comparison
        IfExprCase{"ArithEq", "2+2==4", true}, IfExprCase{"ArithNe", "2!=3", true},
        IfExprCase{"ArithNeFalse", "3!=3", false}, IfExprCase{"CmpLt", "2<3", true},
        IfExprCase{"CmpLtFalse", "3<2", false}, IfExprCase{"CmpGt", "3>2", true},
        IfExprCase{"CmpGtFalse", "2>3", false}, IfExprCase{"CmpLe", "2<=2", true},
        IfExprCase{"CmpLeFalse", "3<=2", false}, IfExprCase{"CmpGe", "3>=3", true},
        IfExprCase{"CmpGeFalse", "2>=3", false},
        // logical
        IfExprCase{"LogAndTrue", "1&&1", true}, IfExprCase{"LogAndFalse", "1&&0", false},
        IfExprCase{"LogOrTrue", "0||1", true}, IfExprCase{"LogOrFalse", "0||0", false},
        IfExprCase{"LogNotFalse", "!0", true}, IfExprCase{"LogNotTrue", "!1", false},
        // bitwise
        IfExprCase{"BitAnd", "3&2", true}, IfExprCase{"BitOr", "1|2", true},
        IfExprCase{"BitXor", "3^1", true}, IfExprCase{"BitNot", "~0==-1", true},
        // shift
        IfExprCase{"Shl", "1<<2==4", true}, IfExprCase{"Shr", "8>>2==2", true},
        // unary
        IfExprCase{"UnaryMinus", "-1<0", true}, IfExprCase{"UnaryPlus", "+1>0", true},
        // integer literal forms
        IfExprCase{"HexLit", "0xFF==255", true}, IfExprCase{"HexLit2", "0x10==16", true},
        // 7.4 token-based evaluation: #if literals follow LPC's OWN number
        // grammar now (the old char walker used C's strtol rules). LPC has no
        // octal literals -- a leading zero is just decimal, matching what the
        // same spelling means in code proper -- and no U/L suffixes.
        IfExprCase{"LeadingZeroDecimal", "010==10", true},
        IfExprCase{"NoIntSuffix", "100==100", true},
        // char literals
        IfExprCase{"CharLit", "'A'==65", true}, IfExprCase{"CharEscN", "'\\n'==10", true},
        IfExprCase{"CharEscE", "'\\e'==27", true}, IfExprCase{"CharEscHex", "'\\x41'==65", true},
        IfExprCase{"CharEscOctal", "'\\101'==65", true},
        // ternary
        IfExprCase{"TernaryFalse", "0?0:1", true}, IfExprCase{"TernaryTrue", "1?0:1", false},
        // efun_defined
        IfExprCase{"EfunDefinedKnown", "efun_defined(write)", true},
        IfExprCase{"EfunDefinedUnknown", "efun_defined(not_a_real_efun_xyz)", false}),
    [](const ::testing::TestParamInfo<IfExprCase>& i) { return i.param.name; });

// ---------------------------------------------------------------------------
// 6b. #if with macro expansion (needs #define, can't use simple parameterized)
// ---------------------------------------------------------------------------

TEST(Preprocessor, IfExprMacroExpansion) {
  EXPECT_EQ(pp(R"(
#define VER 3
#if VER >= 2
int new_api;
#endif
)"),
            R"(


int new_api;

)");
}

TEST(Preprocessor, IfDefinedWithParens) {
  EXPECT_EQ(pp(R"(
#define FOO
#if defined(FOO)
int yes;
#endif
)"),
            R"(


int yes;

)");
  EXPECT_EQ(pp(R"(
#if defined(NOT_DEFINED_MACRO)
int no;
#endif
)"),
            "\n\n");
}

TEST(Preprocessor, IfDefinedWithoutParens) {
  EXPECT_EQ(pp(R"(
#define FOO
#if defined FOO
int yes;
#endif
)"),
            R"(


int yes;

)");
}

// ---------------------------------------------------------------------------
// 7. Line continuation
// ---------------------------------------------------------------------------

TEST(Preprocessor, LineContinuationInDefine) {
  EXPECT_EQ(pp(R"(
#define LONG \
    100
int v = LONG;
)"),
            R"(

int v = 100;
)");
}

// ---------------------------------------------------------------------------
// 8. String / char literals: macros must not expand inside them
// ---------------------------------------------------------------------------

TEST(Preprocessor, MacroNotExpandedInStringLiteral) {
  EXPECT_EQ(pp(R"(
#define FOO 42
string s = "FOO is FOO";
)"),
            R"(

string s = "FOO is FOO";
)");
}

TEST(Preprocessor, MacroNotExpandedInCharLiteral) {
  EXPECT_EQ(pp(R"(
#define A 65
int c = 'A';
)"),
            R"(

int c = 'A';
)");
}

// ---------------------------------------------------------------------------
// 9. #include handling
// ---------------------------------------------------------------------------

TEST(Preprocessor, IncludeHandled) {
  // The preprocessor must handle #include directives, not silently pass them through.
  // In the test environment no include paths are configured, so the include fails
  // and an error is reported — verify the directive was consumed (not passed through).
  auto p = LpcPreprocessor::make_session();
  std::string result = p->preprocess_next(R"(
#include <stdio.h>
)",
                                          "test");
  EXPECT_FALSE(p->errors().empty());
  // The raw #include directive must not appear as a bare token in the output
  EXPECT_EQ(result.find("\n#include"), std::string::npos);
}

// ---------------------------------------------------------------------------
// 10. Multiple directives interacting
// ---------------------------------------------------------------------------

TEST(Preprocessor, DefineInsideElseBranch) {
  // PLATFORM not defined → takes #else branch, SIZE=32
  EXPECT_EQ(pp(R"(
#ifdef PLATFORM
#define SIZE 64
#else
#define SIZE 32
#endif
int arr[SIZE];
)"),
            R"(



int arr[32];
)");
}

TEST(Preprocessor, UndefAndRedefine) {
  // X is undefined before redefinition → no warning
  EXPECT_EQ(pp(R"(
#define X 1
#undef X
#define X 2
int v = X;
)"),
            R"(



int v = 2;
)");
}

TEST(Preprocessor, LineCountPreserved) {
  // Each directive line becomes a blank line, preserving line count
  EXPECT_EQ(pp(R"(line1
#define FOO bar
line3
FOO;
)"),
            R"(line1

line3
bar;
)");
}

// ---------------------------------------------------------------------------
// 11. Advanced Features: Stringizing (#) and Token Pasting (##)
// ---------------------------------------------------------------------------

TEST(Preprocessor, StringizingSimple) {
  EXPECT_EQ(pp(R"(
#define STR(x) #x
string s = STR(hello world);
)"),
            R"(

string s = "hello world";
)");
}

TEST(Preprocessor, StringizingWithQuotes) {
  // Stringizing escapes embedded quotes and backslashes
  EXPECT_EQ(pp(R"(
#define STR(x) #x
string s = STR("hello" \world);
)"),
            R"(

string s = "\"hello\" \\world";
)");
}

TEST(Preprocessor, TokenPastingSimple) {
  EXPECT_EQ(pp(R"(
#define GLUE(a, b) a ## b
int GLUE(var, 1) = 42;
)"),
            R"(

int var1 = 42;
)");
}

TEST(Preprocessor, TokenPastingWithSpaces) {
  EXPECT_EQ(pp(R"(
#define GLUE(a, b) a   ##   b
int GLUE(foo, bar) = 100;
)"),
            R"(

int foobar = 100;
)");
}

// ---------------------------------------------------------------------------
// 12. Predefined Macros: __FILE__ and __LINE__
// ---------------------------------------------------------------------------

TEST(Preprocessor, PredefinedFileAndLine) {
  EXPECT_EQ(pp(R"(
string f = __FILE__;
int l = __LINE__;
)",
               "my_source_file.c"),
            R"(
string f = "/my_source_file.c";
int l = 3;
)");
}

// ---------------------------------------------------------------------------
// 13. Ported Built-in Macros
// ---------------------------------------------------------------------------

TEST(Preprocessor, PortedBuiltinMacros) {
  EXPECT_EQ(pp(R"(
#ifdef FLUFFOS
int fluffos_defined = 1;
#endif
#ifdef MUDOS
int mudos_defined = 1;
#endif
#ifdef __PACKAGE_TRIM__
int package_trim_defined = 1;
#endif
#if SIZEOFINT == 8 || SIZEOFINT == 4
int sizeof_int_ok = 1;
#endif
)"),
            R"(

int fluffos_defined = 1;


int mudos_defined = 1;


int package_trim_defined = 1;


int sizeof_int_ok = 1;

)");
}

TEST(Preprocessor, HereDocNoMacroExpansion) {
  // Heredoc bodies are raw-captured by parseHeredoc() without ever going
  // through identifier resolution, so FOO must NOT expand inside either
  // form: the @END form yields one string token containing "FOO", the
  // @@END2 array-of-lines form a "FOO" line string. Nothing anywhere in
  // the token stream may carry the macro's 42.
  NormalizedString out = pp(R"(
#define FOO 42
string x = @END
FOO
END;
string *y = @@END2
FOO
END2;
)");
  EXPECT_NE(out.val.find("\"FOO"), std::string::npos);
  EXPECT_EQ(out.val.find("42"), std::string::npos);
}

TEST(Preprocessor, HeredocTextFormKeepsLineCount) {
  // parseHeredoc() counts each body line as it reads it through Flex,
  // and a LONE terminator line's newline is consumed and counted inside
  // the helper itself -- __LINE__ afterward must be exact.
  EXPECT_EQ(pp("string x = @END\na\nb\nEND\n;\nint l = __LINE__;\n"),
            "string x = \"a\nb\n\" ; int l = 6;");
}

TEST(Preprocessor, HeredocArrayFormKeepsLineCount) {
  // Array form: body lines are counted at read time and the spliced
  // `"a", "b", })` rescan text deliberately carries NO newlines (the
  // legacy get_array_block joined elements with ",\n", making the rescan
  // count every body line a second time). Also covers the
  // trailing-content terminator path: the ";" after END is pushed back
  // along with the line's newline, which is then counted exactly once at
  // rescan. Since 9.1 the spliced literal is ordinary '(' '{' tokens.
  EXPECT_EQ(pp("string *y = @@END\na\nb\nEND;\nint l = __LINE__;\n"),
            "string *y = ({ \"a\", \"b\", }); int l = 5;");
}

// ---------------------------------------------------------------------------
// 14. Error-generating directives — parameterized
// ---------------------------------------------------------------------------

struct ErrorCase {
  const char* name;
  const char* src;
};

inline void PrintTo(const ErrorCase& ec, ::std::ostream* os) { *os << ec.name; }

class ErrorCaseTest : public ::testing::TestWithParam<ErrorCase> {};

TEST_P(ErrorCaseTest, ReportsError) {
  static bool predefs_added = false;
  if (!predefs_added) {
    add_predefine("FLUFFOS", -1, "");
    predefs_added = true;
  }
  auto p = LpcPreprocessor::make_session();
  p->preprocess_next(GetParam().src, "test");
  EXPECT_FALSE(p->errors().empty()) << "src: " << GetParam().src;
}

INSTANTIATE_TEST_SUITE_P(
    Errors, ErrorCaseTest,
    ::testing::Values(ErrorCase{"ErrorDirective", "#error something went wrong\n"},
                      // (WarnDirective removed: #warn is a real warning now, not an error
                      // -- see WarnDirectiveIsNonFatalWarning.)
                      ErrorCase{"UnknownDirective", "#frobnicate foo\n"},
                      ErrorCase{"HashHashAtStart", "#define FOO ## x\nFOO\n"},
                      ErrorCase{"HashHashAtEnd", "#define FOO x ##\nFOO\n"},
                      ErrorCase{"MismatchedParen", "#if (1 + 2\nint ok;\n#endif\n"},
                      ErrorCase{"DivByZero", "#if 1 / 0\nint x;\n#endif\n"},
                      ErrorCase{"ModByZero", "#if 5 % 0\nint x;\n#endif\n"},
                      ErrorCase{"UndefPredefined", "#undef FLUFFOS\n"},
                      ErrorCase{"MissingEndif", "#ifdef FOO\nint x;\n"},
                      ErrorCase{"UnexpectedElse", "int x;\n#else\nint y;\n#endif\n"},
                      ErrorCase{"UnexpectedEndif", "int x;\n#endif\n"},
                      // (RedefDifferentBody removed: redefining a macro is a non-fatal
                      // warning now, not an error -- see RedefinitionIsAllowedWithWarning.)
                      ErrorCase{"EmptyIfExpr", "#if\nint x;\n#endif\n"},
                      ErrorCase{"TernaryMissingColon", "#if 1 ? 2\nint x;\n#endif\n"}),
    [](const ::testing::TestParamInfo<ErrorCase>& i) { return i.param.name; });

// ---------------------------------------------------------------------------
// 14a-bis. Structured diagnostics (Phase 6.2): yyerror()/yywarn() also
// capture a Diagnostic into compiler_diags -- position, severity, and
// provenance notes (live #include stack; compile-session chain) recorded
// at the moment of the report. Default text output is unchanged; these
// assert the structured record and its clang-style rendering.
// ---------------------------------------------------------------------------

TEST(Diagnostics, CapturesErrorPositionAndMessage) {
  pp("int a;\n#error boom here\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_FALSE(d.is_warning);
  EXPECT_EQ(d.line, 2);
  EXPECT_NE(d.message.find("boom here"), std::string::npos);
  EXPECT_FALSE(d.file.empty());
  // Top-level error: no include stack, no compile chain.
  EXPECT_TRUE(d.notes.empty());
  // Rendering: "file:2: error: ..." on one line.
  std::string r = render_diagnostic(d);
  EXPECT_NE(r.find(":2: error:"), std::string::npos);
  EXPECT_NE(r.find("boom here"), std::string::npos);
}

TEST(Diagnostics, ClearedPerChunk) {
  pp("#error first\n");
  ASSERT_FALSE(compiler_diags.empty());
  pp("int clean = 1;\n");
  EXPECT_TRUE(compiler_diags.empty());
}

TEST(Diagnostics, WarningSeverityCaptured) {
  // An unknown \ escape in a string reports through yywarn().
  pp("string s = \"a\\q\";\n");
  ASSERT_FALSE(compiler_diags.empty());
  EXPECT_TRUE(compiler_diags.back().is_warning);
}

TEST(Preprocessor, IncludeWithTrailingComment) {
  // Real mudlibs write `#include "x.h" /* why */` -- everything after
  // the CLOSING delimiter is ignored. Regression: the parser assumed
  // the closing delimiter was the last character of the line and
  // swallowed the comment into the filename (real-mud ftpd.c failure).
  std::string inc_path = std::string(TESTSUITE_DIR) + "/zz_inc_trail.h";
  {
    std::ofstream inc(inc_path);
    inc << "#define FROM_TRAIL 7\n";
  }
  NormalizedString out =
      pp("#include \"zz_inc_trail.h\" /* gets mud info */\nint x = FROM_TRAIL;\n");
  unlink(inc_path.c_str());
  EXPECT_EQ(num_parse_error, 0);
  EXPECT_EQ(out, "int x = 7;");
}

TEST(Preprocessor, IncludeAngleTrailingCommentParsesCleanName) {
  // Angle form with a trailing comment: no include path is configured
  // here so resolution fails, but the reported name must be EXACTLY the
  // bracketed text -- not the comment-swallowed tail.
  pp("#include <zz_no_such.h> /* tmi-2 socket defines */\nint y;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_NE(d.message.find("zz_no_such.h"), std::string::npos) << d.message;
  EXPECT_EQ(d.message.find("/*"), std::string::npos) << d.message;
  EXPECT_EQ(d.message.find("tmi-2"), std::string::npos) << d.message;
}

TEST(Preprocessor, IncludeLineCommentTail) {
  std::string inc_path = std::string(TESTSUITE_DIR) + "/zz_inc_tail2.h";
  {
    std::ofstream inc(inc_path);
    inc << "#define TAIL2 9\n";
  }
  NormalizedString out =
      pp("#include \"zz_inc_tail2.h\" // trailing line comment\nint z = TAIL2;\n");
  unlink(inc_path.c_str());
  EXPECT_EQ(num_parse_error, 0);
  EXPECT_EQ(out, "int z = 9;");
}

TEST(Diagnostics, IncludeProvenanceNote) {
  // An error INSIDE an included file must carry the "in file included
  // from" note naming the includer and the #include line. The include
  // target is created on disk next to the (testsuite-cwd-relative)
  // including file so quoted-include resolution finds it.
  // Absolute path under the testsuite dir: TokenizeSession chdir()s
  // there on first use, but under ctest this test may run in its own
  // process where that hasn't happened yet when the file is created.
  std::string inc_path = std::string(TESTSUITE_DIR) + "/zz_diag_include_test.h";
  {
    std::ofstream inc(inc_path);
    inc << "#error exploded inside include\n";
  }
  pp("int a;\nint b;\n#include \"zz_diag_include_test.h\"\n");
  unlink(inc_path.c_str());
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_NE(d.message.find("exploded inside include"), std::string::npos);
  EXPECT_EQ(d.line, 1);  // line 1 of the included file
  EXPECT_NE(d.file.find("zz_diag_include_test.h"), std::string::npos);
  // The includer is the chunk file ("test"), at the #include's line (3).
  ASSERT_FALSE(d.included_from.empty());
  EXPECT_EQ(d.included_from[0].second, 3);
  // clang convention: the include chain renders BEFORE the main line.
  std::string r = render_diagnostic(d);
  EXPECT_NE(r.find("In file included from "), std::string::npos);
  EXPECT_NE(r.find(":3:\n"), std::string::npos);
  EXPECT_LT(r.find("In file included from"), r.find("error:"));
}

TEST(Diagnostics, ExpansionChainNote) {
  // A lexical error INSIDE spliced expansion text must carry the
  // "during expansion of macro 'X' (defined at ...)" note -- the frame
  // is tracked by byte-extent accounting on the splice, live exactly
  // while the splice is being rescanned. '\x01' is an illegal character,
  // guaranteed to error at scan time.
  pp("#define BAD \x01\nint x = BAD;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  ASSERT_FALSE(d.expansions.empty());
  EXPECT_EQ(d.expansions[0].macro_name, "BAD");
  EXPECT_EQ(d.expansions[0].def_line, 1);  // defined at line 1
  std::string r = render_diagnostic(d);
  // clang shape: a LOCATED note line ("/file:line[:col]: note: ...").
  EXPECT_NE(r.find(": note: expanded from macro 'BAD'"), std::string::npos);
}

TEST(Diagnostics, NestedExpansionChainNotes) {
  // OUTER's body references INNER; the bad byte lives in INNER's text.
  // Both frames must be reported, innermost first.
  pp("#define INNER \x01\n#define OUTER INNER\nint x = OUTER;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  // Rescan-driven expansion (7.4): each nested reference gets its own
  // buffer + frame, so BOTH sites report, innermost first -- the full
  // clang-style chain the flattened textual design couldn't produce.
  ASSERT_GE(d.expansions.size(), 2u);
  EXPECT_EQ(d.expansions[0].macro_name, "INNER");
  EXPECT_EQ(d.expansions[1].macro_name, "OUTER");
}

TEST(Diagnostics, ExpansionInsideIncludeCombined) {
  // Error in a macro used inside an included file: BOTH provenance axes
  // must be present -- the include chain prefix and the expansion note.
  std::string inc_path = std::string(TESTSUITE_DIR) + "/zz_diag_combo_test.h";
  {
    std::ofstream inc(inc_path);
    inc << "#define KABOOM \x01\n";
    inc << "int v = KABOOM;\n";
  }
  pp("#include \"zz_diag_combo_test.h\"\n");
  unlink(inc_path.c_str());
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  ASSERT_FALSE(d.included_from.empty());
  ASSERT_FALSE(d.expansions.empty());
  EXPECT_EQ(d.expansions[0].macro_name, "KABOOM");
  std::string r = render_diagnostic(d);
  EXPECT_LT(r.find("In file included from"), r.find("error:"));
  EXPECT_NE(r.find(": note: expanded from macro 'KABOOM'"), std::string::npos);
}

TEST(Diagnostics, ColumnAndSnippetCaptured) {
  // A scan-level error must carry the diagnosed token's 1-based START
  // column and the physical line's text; the renderer emits
  // "file:line:col:" plus an indented snippet with a caret under the
  // column. '\x01' is an illegal character at column 9 of line 2.
  pp("int a;\nint bb = \x01;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_EQ(d.line, 2);
  EXPECT_EQ(d.column, 10);
  EXPECT_NE(d.snippet.find("int bb ="), std::string::npos);
  std::string r = render_diagnostic(d);
  EXPECT_NE(r.find(":2:10: error:"), std::string::npos);
  // Caret line: two-space indent + (column-1) spaces + '^'.
  // clang gutter: "      | " prefix, caret at column 10.
  EXPECT_NE(r.find("\n      | " + std::string(9, ' ') + "^"), std::string::npos);
}

TEST(Diagnostics, ExpansionErrorAttributesInvocationColumn) {
  // An error INSIDE an expansion attributes line/column/snippet to the
  // OUTERMOST invocation site (clang's "expansion location"): the
  // column is the macro NAME's start, the snippet is the invocation
  // line, and the expansion itself is the note's job.
  pp("#define BAD \x01\nint xx = BAD;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_EQ(d.line, 2);
  EXPECT_EQ(d.column, 10);  // 'B' of BAD on line 2
  EXPECT_NE(d.snippet.find("int xx = BAD;"), std::string::npos);
  ASSERT_FALSE(d.expansions.empty());
}

TEST(Diagnostics, DirectiveErrorsCarryNoColumn) {
  // Whole-line constructs (#error and friends) attribute by line only:
  // column stays 0 and no snippet/caret block renders.
  pp("#error boom\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_EQ(d.column, 0);
  EXPECT_TRUE(d.snippet.empty());
}

TEST(Diagnostics, ErrorTextWithPercentIsNotAFormatString) {
  // #error / #warn / macro-name text is arbitrary source; a '%' in it
  // must reach the diagnostic literally, never as a printf conversion
  // (CodeQL cpp/tainted-format-string; ASan-confirmed crash on
  // `#error %s%n` before lexerror() started %-quoting).
  pp("#error boom %s%s%n percent\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_NE(d.message.find("%s%s%n"), std::string::npos);
}

TEST(Diagnostics, UnknownEscapeCarriesFixIt) {
  // The unknown-escape warning suggests dropping the backslash: a
  // fix-it spanning the two-character escape, replacement = the bare
  // character, rendered under the caret.
  pp("string s = \"a\\q\";\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_TRUE(d.is_warning);
  ASSERT_EQ(d.fixits.size(), 1u);
  EXPECT_EQ(d.fixits[0].replacement, "q");
  EXPECT_EQ(d.fixits[0].col_end, d.fixits[0].col_start + 2);
  std::string r = render_diagnostic(d);
  // The replacement line renders after the caret line.
  EXPECT_NE(r.find("^"), std::string::npos);
  EXPECT_GT(r.rfind("q"), r.find("^"));
}

TEST(Diagnostics, UnknownPragmaWarnsAndIsIgnored) {
  pp("#pragma strict_typs\nint x;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_TRUE(d.is_warning);
  EXPECT_NE(d.message.find("Unknown #pragma"), std::string::npos);
}

TEST(Diagnostics, SuppressedWarningDoesNotLeakContext) {
  // A producer queues its note/fix-it BEFORE yywarn(); when warnings
  // are pragma-disabled the report is suppressed -- its queued context
  // must be discarded with it, not attached to the next (unrelated)
  // diagnostic.
  pp("#pragma no_warnings\nstring s = \"a\\q\";\n#pragma warnings\nint x = \x01;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_FALSE(d.is_warning);  // the illegal-character error
  EXPECT_TRUE(d.fixits.empty());
  for (const auto& n : d.notes) {
    EXPECT_EQ(n.find("did you mean"), std::string::npos) << n;
  }
}

TEST(Diagnostics, FloatInIfExprErrors) {
  // #if literals follow LPC's number grammar, but a float can still be
  // SPELLED -- it must produce a clear error, not silently evaluate
  // as zero.
  pp("#if 1.5\nint x;\n#endif\n");
  ASSERT_FALSE(compiler_diags.empty());
  bool found = false;
  for (const auto& d : compiler_diags) {
    if (d.message.find("floating point") != std::string::npos) found = true;
  }
  EXPECT_TRUE(found);
}

TEST(Diagnostics, RedefinitionIsAllowedWithWarning) {
  // Redefining an LPC macro with a different body is ALLOWED: it must
  // be a non-fatal WARNING (not an error that fails the compile) and
  // the new definition must win. Regression: it used to go through
  // lexerror() and fail compilation.
  NormalizedString out = pp("#define FOO 1\n#define FOO 2\nint x = FOO;\n");
  EXPECT_EQ(num_parse_error, 0);  // did NOT fail the compile
  EXPECT_EQ(out, "int x = 2;");   // the redefinition took effect
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_TRUE(d.is_warning);
  EXPECT_NE(d.message.find("Macro 'FOO' redefined"), std::string::npos);
  ASSERT_FALSE(d.notes.empty());
  EXPECT_NE(d.notes[0].find("previous definition of 'FOO' was at "), std::string::npos);
  EXPECT_NE(d.notes[0].find(":1"), std::string::npos);
}

TEST(Diagnostics, WarnDirectiveIsNonFatalWarning) {
  // #warn matches its name: a WARNING that does not fail the compile
  // (it went through lexerror() before, failing compilation like
  // #error). The payload is carried in the message.
  NormalizedString out = pp("#warn deprecated header\nint x = 1;\n");
  EXPECT_EQ(num_parse_error, 0);  // did NOT fail the compile
  EXPECT_EQ(out, "int x = 1;");   // input scanned normally
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_TRUE(d.is_warning);
  EXPECT_NE(d.message.find("#warn deprecated header"), std::string::npos);
}

TEST(Diagnostics, IdenticalRedefinitionIsSilent) {
  // Redefining with the IDENTICAL body is not even a warning.
  pp("#define FOO 1\n#define FOO 1\n");
  for (const auto& d : compiler_diags) {
    EXPECT_EQ(d.message.find("redefined"), std::string::npos);
  }
}

TEST(Diagnostics, NoStaleExpansionNoteAfterSpliceConsumed) {
  // The frame must POP once the splice is fully consumed: an error on a
  // later line, AFTER a successful macro use, must carry no expansion
  // note (this pins the extent accounting's debit/pop precision).
  pp("#define OK 42\nint a = OK;\nint b = \x01;\n");
  ASSERT_FALSE(compiler_diags.empty());
  const Diagnostic& d = compiler_diags.back();
  EXPECT_TRUE(d.expansions.empty());
  EXPECT_EQ(d.line, 3);
}

// ---------------------------------------------------------------------------
// 14b. Error in dead block must be suppressed
// ---------------------------------------------------------------------------

TEST(Preprocessor, ErrorDirectiveInDeadBlockIgnored) {
  auto p = LpcPreprocessor::make_session();
  p->preprocess_next(R"(
#ifdef NOT_DEFINED
#error should not fire
#endif
)",
                     "test");
  EXPECT_TRUE(p->errors().empty());
}

// ---------------------------------------------------------------------------
// 15. ## in a dead conditional block must not corrupt prior output
// ---------------------------------------------------------------------------

TEST(Preprocessor, TokenPasteInDeadBlockNoCorruption) {
  // The ## in the dead block must not strip the trailing spaces from out_.
  // "int x /* c */" emits "int x " (space from comment replacement).
  std::string src = R"(int x /* c */
#if 0
##
#endif
int y;
)";
  EXPECT_EQ(pp(src), R"(int x  

int y;
)");
}

// ---------------------------------------------------------------------------
// 16. ## inside a string literal in a macro body must be preserved
// ---------------------------------------------------------------------------

TEST(Preprocessor, TokenPasteInsideStringLiteralPreserved) {
  // Object-like macro: ## inside a string literal is always preserved
  EXPECT_EQ(pp(R"(
#define FOO "a##b"
string s = FOO;
)"),
            R"(

string s = "a##b";
)");

  // Function-like macro: substitute()'s second pass must skip ## inside string literals
  EXPECT_EQ(pp(R"(
#define WRAP(x) "a##b"
string s = WRAP(1);
)"),
            R"(

string s = "a##b";
)");
}

// ---------------------------------------------------------------------------
// 17. Predefined __DIR__
// ---------------------------------------------------------------------------

TEST(Preprocessor, PredefinedDir) {
  EXPECT_EQ(pp("string d = __DIR__;", "mydir/myfile.c"), "string d = \"/mydir/\";");
}

TEST(Preprocessor, IfdefDir) {
  EXPECT_EQ(pp(R"(
#ifdef __DIR__
int dir_defined;
#endif
)"),
            R"(

int dir_defined;

)");
}

// ---------------------------------------------------------------------------
// 18. Multiple #elif chains
// ---------------------------------------------------------------------------

TEST(Preprocessor, MultipleElif) {
  EXPECT_EQ(pp(R"(
#define LEVEL 3
#if LEVEL == 1
int l1;
#elif LEVEL == 2
int l2;
#elif LEVEL == 3
int l3;
#else
int lx;
#endif
)"),
            R"(


int l3;

)");
}

// ---------------------------------------------------------------------------
// 19. Three-level nested conditionals
// ---------------------------------------------------------------------------

TEST(Preprocessor, ThreeLevelNestedConditionals) {
  // A and B defined, C not. Body: abc dead, ab_only emitted (with indentation).
  EXPECT_EQ(pp(R"(
#define A
#define B
#ifdef A
  #ifdef B
    #ifdef C
    int abc;
    #else
    int ab_only;
    #endif
  #endif
#endif
)"),
            "\n\n\n\n  \n    \n    int ab_only;\n    \n  \n\n");
}

// ---------------------------------------------------------------------------
// 20. #ifdef on built-in special macros (__LINE__, __FILE__, __DIR__)
// ---------------------------------------------------------------------------

TEST(Preprocessor, IfdefBuiltinSpecials) {
  EXPECT_EQ(pp(R"(
#ifdef __LINE__
int line_defined;
#endif
)"),
            R"(

int line_defined;

)");
  EXPECT_EQ(pp(R"(
#ifdef __FILE__
int file_defined;
#endif
)"),
            R"(

int file_defined;

)");
  EXPECT_EQ(pp(R"(
#ifdef __DIR__
int dir_defined2;
#endif
)"),
            R"(

int dir_defined2;

)");
}

// ---------------------------------------------------------------------------
// 21. #if with division / modulo by zero (also checks content not emitted)
// ---------------------------------------------------------------------------

TEST(Preprocessor, IfExprDivisionByZero) {
  auto p1 = LpcPreprocessor::make_session();
  std::string r1 = p1->preprocess_next(R"(
#if 1 / 0
int div_zero;
#endif
)",
                                       "test");
  EXPECT_FALSE(p1->errors().empty());
  EXPECT_EQ(r1.find("int div_zero;"), std::string::npos);

  auto p2 = LpcPreprocessor::make_session();
  std::string r2 = p2->preprocess_next(R"(
#if 5 % 0
int mod_zero;
#endif
)",
                                       "test");
  EXPECT_FALSE(p2->errors().empty());
  EXPECT_EQ(r2.find("int mod_zero;"), std::string::npos);
}

// ---------------------------------------------------------------------------
// 22. #pragma passthrough
// ---------------------------------------------------------------------------

TEST(Preprocessor, PragmaPassthrough) {
  // #pragma is consumed entirely by the directive dispatch (nothing
  // reaches the token stream) and its effect lands on the `pragmas`
  // flags at exactly that point in the scan.
  EXPECT_EQ(pp(R"(int a;
#pragma strict_types
int b;
)"),
            "int a; int b;");
  EXPECT_TRUE(pragmas & PRAGMA_STRICT_TYPES);
}

// ---------------------------------------------------------------------------
// 23. #line directive
// ---------------------------------------------------------------------------

TEST(Preprocessor, LineDirectiveUpdatesLineNumber) {
  // #line is consumed entirely by the directive dispatch; the line right
  // after `#line 10` reports itself as line 10 via __LINE__.
  EXPECT_EQ(pp(R"(
#line 10
int l = __LINE__;
)"),
            "int l = 10;");
}

// ---------------------------------------------------------------------------
// 24. #echo and #breakpoint don't error
// ---------------------------------------------------------------------------

TEST(Preprocessor, EchoDoesNotError) {
  auto p = LpcPreprocessor::make_session();
  p->preprocess_next("#echo hello world\n", "test");
  EXPECT_TRUE(p->errors().empty());
}

TEST(Preprocessor, BreakpointDoesNotError) {
  auto p = LpcPreprocessor::make_session();
  p->preprocess_next("#breakpoint\n", "test");
  EXPECT_TRUE(p->errors().empty());
}

// ---------------------------------------------------------------------------
// 25. CRLF line endings are handled transparently
// ---------------------------------------------------------------------------

TEST(Preprocessor, CRLFLineEndings) {
  // \r\n in directives and code must tokenize the same as \n alone
  EXPECT_EQ(pp("#define FOO 42\r\nint v = FOO;\r\n"), "int v = 42;");
  auto p = LpcPreprocessor::make_session();
  NormalizedString out = p->preprocess_next("#define X 1\r\nint v = X;\r\n", "test");
  EXPECT_TRUE(p->errors().empty());
  EXPECT_EQ(out, "int v = 1;");
}

// ---------------------------------------------------------------------------
// 26. #undef of undefined macro is silent (no error)
// ---------------------------------------------------------------------------

TEST(Preprocessor, UndefUndefinedIsSilent) {
  auto p = LpcPreprocessor::make_session();
  p->preprocess_next("#undef NOT_DEFINED_AT_ALL\n", "test");
  EXPECT_TRUE(p->errors().empty());
}

// ---------------------------------------------------------------------------
// 27. Redefine with identical body is silent (no warning)
// ---------------------------------------------------------------------------

TEST(Preprocessor, RedefineIdenticalBodyNoWarning) {
  auto p = LpcPreprocessor::make_session();
  p->preprocess_next("#define FOO 1\n#define FOO 1\n", "test");
  EXPECT_TRUE(p->errors().empty());
}

// ---------------------------------------------------------------------------
// 28. Template literals (backtick strings) — found in self-review vs master
// ---------------------------------------------------------------------------
// The preprocessor must treat a template literal's *text* portions as
// protected string content (no comment stripping, no macro expansion) while
// still fully processing the code inside ${...} interpolations, mirroring
// the pre-standalone-preprocessor driver where the template scanner read
// text bytes directly but returned to the expanding scanner for ${expr}.

TEST(Preprocessor, TemplateTextNotCommentStripped) {
  // '//' inside backtick text used to be eaten as a line comment,
  // swallowing the closing backtick and the rest of the line.
  EXPECT_EQ(pp("string s = `http://x.com/ ok`;"), "string s = `http://x.com/ ok`;");
  // '/*' inside backtick text must not open a block comment.
  EXPECT_EQ(pp("string s = `not /* a comment */ here`;"), "string s = `not /* a comment */ here`;");
}

TEST(Preprocessor, TemplateTextNotMacroExpanded) {
  EXPECT_EQ(pp("#define GREET \"hi\"\nstring s = `say GREET`;"), "\nstring s = `say GREET`;");
}

TEST(Preprocessor, TemplateInterpolationIsMacroExpanded) {
  EXPECT_EQ(pp("#define N 42\nstring s = `n=${N}!`;"), "\nstring s = `n=${42}!`;");
  // Function-like macro in the interpolation.
  EXPECT_EQ(pp("#define D(x) ((x)*2)\nstring s = `${D(3)}`;"), "\nstring s = `${((3)*2)}`;");
}

TEST(Preprocessor, TemplateInterpolationBraceTracking) {
  // Braces inside the interpolated expression (array literal "({ })")
  // must not be mistaken for the '}' that closes the interpolation:
  // the text after it stays protected text.
  EXPECT_EQ(pp("#define Z 9\nstring s = `${ ({ 1, Z })[0] } Z`;"),
            "\nstring s = `${ ({ 1, 9 })[0] } Z`;");
}

TEST(Preprocessor, TemplateNestedInInterpolation) {
  // A nested template inside ${...}: its own text is protected, its own
  // interpolation expands, and the outer text after it stays protected.
  EXPECT_EQ(pp("#define W 5\nstring s = `a W ${ `b W ${W}` } c W`;"),
            "\nstring s = `a W ${ `b W ${5}` } c W`;");
}

TEST(Preprocessor, TemplateEscapesPassThrough) {
  // \` and \$ decode to literal '`'/'$' inside the template text, the
  // escaped backtick must not terminate the template, and -- the point of
  // this test -- the X inside the template's literal text must NOT be
  // macro-expanded (the escaped \${X} is text, not an interpolation).
  EXPECT_EQ(pp("#define X 1\nstring s = `a\\`X\\${X}`;"), "string s = `a`X${X}`;");
}

TEST(Preprocessor, TemplateMultilineTracksLineNumbers) {
  // Newlines inside template text must keep __LINE__ accurate afterward.
  EXPECT_EQ(pp("string s = `a\nb`;\nint l = __LINE__;"), "string s = `a\nb`;\nint l = 3;");
}

// ---------------------------------------------------------------------------
// 29. defined()/efun_defined() are #if-only operators — self-review vs master
// ---------------------------------------------------------------------------
// The old in-lexer preprocessor only recognized defined()/efun_defined()
// while evaluating #if/#elif conditions. The standalone preprocessor
// applied them during *all* macro expansion, so a bare `defined` inside an
// ordinary macro argument (e.g. ASSERT_EQ(x, m?.defined)) was silently
// rewritten to 0/1.

TEST(Preprocessor, DefinedIsPlainIdentifierOutsideIf) {
  EXPECT_EQ(pp("#define ID(x) x\nint defined = 1; int y = ID(defined);"),
            "\nint defined = 1; int y = defined;");
  EXPECT_EQ(pp("#define ID(x) x\nint z = ID(efun_defined(write));"),
            "\nint z = efun_defined(write);");
}

TEST(Preprocessor, DefinedStillWorksInsideIf) {
  EXPECT_EQ(pp("#define FOO 1\n#if defined(FOO) && efun_defined(write)\nint ok;\n#endif\n"),
            "\n\nint ok;\n\n");
}

// Bug 6: Nested call of the same macro in its own argument list must expand fully
TEST(Preprocessor, NestedSameMacroInArgsExpands) {
  EXPECT_EQ(pp("#define SECOND(a, b) (b)\nint v = SECOND(1, SECOND(2, 3));"), "\nint v = ((3));");
}

TEST(Preprocessor, NestedSameMacroSelfReferentialTermination) {
  EXPECT_EQ(pp("#define LOOP(x) LOOP(x)\nLOOP(1)"), "\nLOOP(1)");
}

// Bug 7: Template literals as macro arguments do not get commas split
TEST(Preprocessor, TemplateAsMacroArgument) {
  EXPECT_EQ(pp("#define ID(x) x\nstring s = ID(`a,b`);"), "\nstring s = `a,b`;");
}

// CRLF line continuation checks (Bugs 9, 10, 11)
TEST(Preprocessor, CRLFLineContinuations) {
  // Escaped newline in CRLF format inside a macro definition
  EXPECT_EQ(pp("#define FOO \\\r\n42\r\nint v = FOO;\r\n"), "\nint v = 42;\r\n");
  // Escaped newline in CRLF format outside macro
  EXPECT_EQ(pp("int v = \\\r\n42;\r\n"), "int v = 42;\r\n");
  // Escaped newline inside template literal: spliced away by the lexer's
  // own CRLF continuation rule inside SC_TEMPLATE_BODY.
  EXPECT_EQ(pp("string s = `abc\\\r\ndef`;"), "string s = `abcdef`;");
}

// ---------------------------------------------------------------------------
// 30. Session mode (make_session()/preprocess_next()) -- REPL groundwork.
// A fresh LpcPreprocessor per line would lose #define state between lines;
// session mode keeps one Impl alive so macros persist across calls.
// ---------------------------------------------------------------------------

TEST(PreprocessorSession, MacroDefinedInEarlierChunkExpandsInLater) {
  auto session = LpcPreprocessor::make_session();
  session->preprocess_next("#define X 42\n");
  EXPECT_EQ(session->preprocess_next("int y = X;\n"), "int y = 42;\n");
}

TEST(PreprocessorSession, RedefiningAcrossChunksUpdatesValue) {
  auto session = LpcPreprocessor::make_session();
  session->preprocess_next("#define X 1\n");
  EXPECT_EQ(session->preprocess_next("int a = X;\n"), "int a = 1;\n");
  session->preprocess_next("#undef X\n#define X 2\n");
  EXPECT_EQ(session->preprocess_next("int b = X;\n"), "int b = 2;\n");
}

TEST(PreprocessorSession, IndependentSessionsDoNotShareMacros) {
  auto s1 = LpcPreprocessor::make_session();
  auto s2 = LpcPreprocessor::make_session();
  s1->preprocess_next("#define ONLY_S1 1\n");
  EXPECT_EQ(s2->preprocess_next("int y = ONLY_S1;\n"), "int y = ONLY_S1;\n");
}

TEST(PreprocessorSession, IfEndifSelfContainedWithinOneChunkWorks) {
  auto session = LpcPreprocessor::make_session();
  session->preprocess_next("#define DEBUG 1\n");
  EXPECT_EQ(session->preprocess_next("#if DEBUG\nint ok;\n#endif\n"), "\nint ok;\n\n");
}

TEST(PreprocessorSession, UnterminatedIfWithinOneChunkErrorsLikeEOF) {
  auto session = LpcPreprocessor::make_session();
  session->preprocess_next("#if 1\nint ok;\n");
  EXPECT_FALSE(session->errors().empty());
}

TEST(PreprocessorSession, ErrorsResetPerChunk) {
  auto session = LpcPreprocessor::make_session();
  session->preprocess_next("#unknown_directive\n");
  EXPECT_FALSE(session->errors().empty());
  session->preprocess_next("int ok;\n");
  EXPECT_TRUE(session->errors().empty());
}

// ---------------------------------------------------------------------------
// compile_file entry points and abort recovery (raw-scanner era).
// ---------------------------------------------------------------------------
// Full driver boot (once): compile_file's codegen path pushes onto the
// eval stack (e.g. the valid-override master apply), so these end-to-end
// entry tests run against a real booted driver -- same pattern as
// bench_compile. They live at the END of the file so the lightweight
// tokenizer-harness tests keep their minimal environment.
static void ensure_compile_env() {
  static bool booted = false;
  if (!booted) {
    chdir(TESTSUITE_DIR);
    init_main("etc/config.test");
    vm_start();
    current_object = master_ob;
    booted = true;
    g_test_env_inited = true;  // the boot covered the harness's init
  }
}

// Portable source-on-an-fd helper: tmpfile() works on every CI platform
// (no /tmp, no mkstemp, auto-deleted).
static FILE* source_as_file(const char* src) {
  // coverity[secure_temp] - tmpfile() opens O_EXCL and unlinks immediately;
  // there is no pathname window to exploit. Kept for Windows-CI portability.
  FILE* f = tmpfile();
  EXPECT_NE(f, nullptr);
  EXPECT_EQ(fwrite(src, 1, strlen(src), f), strlen(src));
  rewind(f);
  return f;
}

TEST(CompileEntry, CompileFileFdSuccess) {
  ensure_compile_env();
  // The zero-copy fd path end to end: a real file, compiled by fd.
  FILE* f = source_as_file("int f() { return 40 + 2; }\n");
  program_t* prog = compile_file_fd(fileno(f), "/fd_success");
  fclose(f);
  ASSERT_NE(prog, nullptr);
  deallocate_program(prog);
}

TEST(CompileEntry, CompileFileFdBadFdThenViewStillWorks) {
  ensure_compile_env();
  // A failing fd load must not poison the next compile (the fd flag is
  // consumed-and-cleared inside prolog, unwind-safe) and must not hang
  // (the parse loop is skipped when the source failed to load).
  program_t* bad = compile_file_fd(-1, "/fd_bad");
  EXPECT_EQ(bad, nullptr);
  program_t* good = compile_file("int g() { return 7; }\n", "/after_bad_fd");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}

TEST(CompileEntry, FatalAbortThenRecompile) {
  ensure_compile_env();
  // A token larger than YYLMAX aborts the compile through the fatal
  // path (exception unwind through clean_parser). The next compile must
  // succeed -- pins the teardown ordering: buffers are deleted by the
  // unwind's clean_parser BEFORE the scanner is destroyed (a destroyed-
  // scanner touch here was a real use-after-free caught in review).
  std::string huge = "int ";
  huge.append(70000, 'a');  // one identifier > YYLMAX (65536)
  huge += ";\n";
  // The fatal path unwinds via error(); give it the catch frame
  // load_object provides in production.
  error_context_t econ{};
  save_context(&econ);
  program_t* bad = nullptr;
  try {
    bad = compile_file(huge, "/fatal_abort");
  } catch (...) {
    restore_context(&econ);
  }
  pop_context(&econ);
  EXPECT_EQ(bad, nullptr);
  program_t* good = compile_file("int h() { return 1; }\n", "/after_fatal");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}

TEST(CompileEntry, InheritInsideIncludeAbortsCleanly) {
  ensure_compile_env();
  // Real-mud shape: `inherit` INSIDE an included file. The compile
  // aborts via YYACCEPT with the include buffer still LIVE; the
  // error-path cleanup must tear buffers down BEFORE freeing the
  // mem_block areas, because popping an include performs accounting
  // writes into them (heap-use-after-free on a live mud before the
  // ordering fix).
  std::string inc_path = std::string(TESTSUITE_DIR) + "/zz_inh_abort.h";
  {
    std::ofstream inc(inc_path);
    inc << "inherit \"/zz_no_such_parent\";\n";
  }
  program_t* prog =
      compile_file("#include \"zz_inh_abort.h\"\nint after() { return 1; }\n", "/inh_abort");
  unlink(inc_path.c_str());
  EXPECT_EQ(prog, nullptr);  // abandoned for the inherit reload
  if (inherit_file) {        // consume the handoff like load_object does
    FREE(inherit_file);
    inherit_file = nullptr;
  }
  // The next compile must be clean.
  program_t* good = compile_file("int ok() { return 2; }\n", "/after_inh_abort");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}

TEST(CompileEntry, FatalInsideIncludeAbortsCleanly) {
  ensure_compile_env();
  // The fatal-abort variant WITH a live include buffer: the exception
  // unwind's clean_parser must survive popping it.
  std::string inc_path = std::string(TESTSUITE_DIR) + "/zz_fatal_inc.h";
  {
    std::ofstream inc(inc_path);
    std::string huge = "int ";
    huge.append(70000, 'b');  // one identifier > YYLMAX
    huge += ";\n";
    inc << huge;
  }
  error_context_t econ{};
  save_context(&econ);
  program_t* bad = nullptr;
  try {
    bad = compile_file("#include \"zz_fatal_inc.h\"\nint x;\n", "/fatal_inc");
  } catch (...) {
    restore_context(&econ);
  }
  pop_context(&econ);
  unlink(inc_path.c_str());
  EXPECT_EQ(bad, nullptr);
  program_t* good = compile_file("int ok2() { return 3; }\n", "/after_fatal_inc");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}

TEST(CompileEntry, FdGrowthPathViaPipe) {
  ensure_compile_env();
  // fstat on a pipe reports size 0, forcing the slurp's doubling-growth
  // branch. ~15KB fits inside the default pipe buffer (64KB on every CI
  // platform), so no reader process is needed -- portable, no fork.
  int fds[2];
#ifdef _WIN32
  ASSERT_EQ(_pipe(fds, 1 << 16, _O_BINARY), 0);
#else
  ASSERT_EQ(pipe(fds), 0);
#endif
  std::string src = "int pipe_fn() { return 0";
  for (int i = 0; i < 3000; i++) src += " + 1";  // ~15KB
  src += "; }\n";
  size_t off = 0;
  while (off < src.size()) {
    ssize_t n = write(fds[1], src.data() + off, src.size() - off);
    ASSERT_GT(n, 0);
    off += static_cast<size_t>(n);
  }
  close(fds[1]);
  program_t* prog = compile_file_fd(fds[0], "/pipe_growth");
  close(fds[0]);
  ASSERT_NE(prog, nullptr);
  deallocate_program(prog);
}

TEST(CompileEntry, IncludeEmptyFileAndNoTrailingNewline) {
  ensure_compile_env();
  // Empty include: slurp guarantees a newline, nothing leaks through.
  // No-final-newline include ENDING IN A DIRECTIVE: the guaranteed
  // newline is what terminates that directive.
  std::string p1 = std::string(TESTSUITE_DIR) + "/zz_empty.h";
  std::string p2 = std::string(TESTSUITE_DIR) + "/zz_nonl.h";
  {
    std::ofstream f(p1);
  }  // 0 bytes
  {
    std::ofstream f(p2);
    f << "#define NONL_VALUE 5";
  }  // no trailing \n
  NormalizedString out =
      pp("#include \"zz_empty.h\"\n#include \"zz_nonl.h\"\nint v = NONL_VALUE;\n");
  unlink(p1.c_str());
  unlink(p2.c_str());
  EXPECT_EQ(num_parse_error, 0);
  EXPECT_EQ(out, "int v = 5;");
}

TEST(CompileEntry, FatalInsideIfExpressionRecovers) {
  ensure_compile_env();
  // Fatal abort while the #if TOKEN-EXPRESSION buffer is live: the
  // unwind must pop the IF_EXPR buffer safely and the evaluator's
  // expansion-suppression flag must not strand into the next compile
  // (macros must still expand afterwards).
  std::string huge = "#if 1 + ";
  huge.append(70000, 'c');  // one identifier > YYLMAX inside the #if
  huge += "\nint a;\n#endif\n";
  error_context_t econ{};
  save_context(&econ);
  program_t* bad = nullptr;
  try {
    bad = compile_file(huge, "/fatal_ifexpr");
  } catch (...) {
    restore_context(&econ);
  }
  pop_context(&econ);
  EXPECT_EQ(bad, nullptr);
  // Next compile: a macro MUST expand (suppress_expansion not stranded).
  program_t* good = compile_file("#define SEVEN 7\nint ok3() { return SEVEN; }\n", "/after_ifexpr");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}

TEST(CompileEntry, UnterminatedTextBlockAndTemplateRecover) {
  ensure_compile_env();
  // EOF inside a text block and inside a template literal: both must
  // produce a clean error and leave the compiler reusable.
  program_t* tb = compile_file("string f() { return @END\nnever terminated\n", "/unterm_tb");
  EXPECT_EQ(tb, nullptr);
  program_t* tp = compile_file("string g() { return `open ${1 + \n", "/unterm_tpl");
  EXPECT_EQ(tp, nullptr);
  program_t* good = compile_file("int ok4() { return 4; }\n", "/after_unterm");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}

TEST(CompileEntry, IncludeDepthLimitCleanError) {
  ensure_compile_env();
  // A self-including header hits MAX_INCLUDE_DEPTH: must be a clean
  // compile error (bounded, no crash), and the next compile clean.
  std::string p = std::string(TESTSUITE_DIR) + "/zz_selfinc.h";
  {
    std::ofstream f(p);
    f << "#include \"zz_selfinc.h\"\n";
  }
  program_t* bad = compile_file("#include \"zz_selfinc.h\"\nint x;\n", "/deep_inc");
  unlink(p.c_str());
  EXPECT_EQ(bad, nullptr);
  program_t* good = compile_file("int ok5() { return 5; }\n", "/after_deep");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}

// ---------------------------------------------------------------------------
// Staged outputs (stage_output.cc): pp/tokens forms through the real
// lexer+preprocessor, no parser.
// ---------------------------------------------------------------------------
static std::string run_stage_dump(const char* src, bool pp_form) {
  ensure_compile_env();
  FILE* in = source_as_file(src);
  // coverity[secure_temp] - see source_as_file.
  FILE* out = tmpfile();  // portable capture (open_memstream is not)
  EXPECT_NE(out, nullptr);
  bool ok = lpc_dump_stage_tokens(fileno(in), "/stage_test", pp_form, out);
  fclose(in);
  EXPECT_TRUE(ok);
  std::string result;
  rewind(out);
  char chunk[4096];
  size_t n;
  while ((n = fread(chunk, 1, sizeof(chunk), out)) > 0) result.append(chunk, n);
  fclose(out);
  return result;
}

TEST(StageOutput, PpFormExpandsMacros) {
  std::string out =
      run_stage_dump("#define TWICE(x) ((x) + (x))\nint f() { return TWICE(21); }\n", true);
  EXPECT_NE(out.find("( ( 21 ) + ( 21 ) )"), std::string::npos) << out;
  EXPECT_EQ(out.find("TWICE"), std::string::npos) << out;    // macro consumed
  EXPECT_EQ(out.find("#define"), std::string::npos) << out;  // directive applied
}

TEST(StageOutput, TokensFormOnePerLineWithPositions) {
  std::string out = run_stage_dump("int f() { return \"hi\"; }\n", false);
  // Every line is `line:col kind spelling`; the string payload re-quoted.
  EXPECT_NE(out.find("\"hi\""), std::string::npos) << out;
  int lines = 0;
  for (char c : out) lines += (c == '\n');
  EXPECT_GE(lines, 9) << out;  // int f ( ) { return "hi" ; }
}

TEST(StageOutput, LoadFailureReturnsFalseAndCleansUp) {
  ensure_compile_env();
  // coverity[secure_temp] - see source_as_file.
  FILE* out = tmpfile();
  ASSERT_NE(out, nullptr);
  EXPECT_FALSE(lpc_dump_stage_tokens(-1, "/nope", true, out));
  fclose(out);
  // The environment must be reusable afterwards (ASan guards leaks).
  program_t* good = compile_file("int k() { return 3; }\n", "/after_stage_fail");
  ASSERT_NE(good, nullptr);
  deallocate_program(good);
}
