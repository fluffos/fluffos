#ifndef LEX_STREAM_H
#define LEX_STREAM_H

#include "base/std.h"

#include <memory>
#include <string_view>

// Forward declarations only -- neither type's full definition is needed
// here. compiler_context_t (lex.h) is only ever touched through
// a unique_ptr, with LexTokenStream's ctor/dtor (which need it complete)
// defined out-of-line in lexer_utils.cc, which does include lex.h.
struct LexerSession;
struct compiler_context_t;
union YYSTYPE;
struct YYLTYPE;

// ---------------------------------------------------------------------------
// LexTokenStream — the compiler's token source.
//
// Owns a reentrant Flex scanner (yylex_init_extra/yylex_destroy via RAII)
// and feeds it a source via load(), then hands out tokens one at a time via
// next(). This is the consolidation point for "construct a scanner, point
// it at source text, auto-preprocess it, pull tokens" -- everything
// compile_file() used to wire up inline by hand.
//
// There is no byte-stream abstraction underneath anymore: load() takes the
// complete source TEXT, installed as a native in-memory Flex buffer
// (yy_scan_bytes -- which copies, so the caller's text may be transient),
// exactly the mechanism #include contents and macro splices already use.
// Callers with a file slurp it first; the historical
// LexStream/FileLexStream/StringLexStream read() hierarchy and the
// YY_INPUT refill bridge are gone.
//
// A single instance can be load()ed more than once, reusing the same
// underlying Flex scanner rather than tearing one down and building
// another -- e.g. a REPL can keep one LexTokenStream alive for a whole
// session and call load() once per statement.
//
// Known scope boundary: the lexer state load() resets via
// start_new_file() (the #include metadata stack, expansion frames, etc.
// in lexer_utils.cc) is still file-scope static, not per-instance -- so
// two LexTokenStream instances alive at once would corrupt each other's
// state. Not a problem today: compile_file()'s reentrancy guard already
// prevents concurrent compiles, and only one LexTokenStream exists at a
// time in any code that constructs one.
class LexTokenStream {
 public:
   LexTokenStream();
   ~LexTokenStream();
   LexTokenStream(const LexTokenStream &) = delete;
   LexTokenStream &operator=(const LexTokenStream &) = delete;

   // (Re-)point this scanner at new source text, exactly like starting a
   // fresh top-level compile (scanner/buffer reset, BEGIN(INITIAL), fresh
   // include stack). `source` is auto-preprocessed on demand using
   // `session` if given, or a fresh one-chunk-only session otherwise; it
   // is copied into the scanner's buffer, so it may be transient.
   void load(std::string_view source, std::shared_ptr<LexerSession> session = nullptr);

   // Zero-copy file form: reads fd straight into the arena block the
   // scanner reads in place. Returns false on read error.
   bool load_fd(int fd, std::shared_ptr<LexerSession> session = nullptr);

   // Pull the next token, filling in *yylval_param and (when non-null)
   // *yylloc_param with the token's source span (8.3 Bison locations).
   int next(union YYSTYPE *yylval_param, struct YYLTYPE *yylloc_param = nullptr);

   void *scanner() const { return scanner_; }

 private:
   std::unique_ptr<compiler_context_t> ctx_;
   void *scanner_ = nullptr;
 };

#endif /* end of include guard: LEX_STREAM_H */
