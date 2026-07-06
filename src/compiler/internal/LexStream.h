#ifndef LEX_STREAM_H
#define LEX_STREAM_H

#include "base/std.h"

#include <cinttypes>
#include <cstddef>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

// ---------------------------------------------------------------------------
// Byte-level streams: LexStream is a plain read()/close() interface with no
// knowledge of LPC, preprocessing, or tokens. Every concrete stream the
// compiler ever reads from -- a real file, an in-memory string, another
// std::istream, or a preprocessing stage wrapping one of those -- is one of
// these. Kept deliberately dumb: adding preprocessing or tokenization to
// what LexStream itself means would make every implementation here (and
// every ring-buffer/YY_INPUT caller of read()) responsible for a much wider
// contract than "hand me the next `size` bytes."
// ---------------------------------------------------------------------------

class LexStream {
 public:
  LexStream() = default;
  virtual ~LexStream() = default;

  virtual size_t read(char* buffer, size_t size) = 0;
  virtual void close() = 0;
};

class FileLexStream : public LexStream {
 public:
  FileLexStream(int fd) : fd_(fd) {}
  ~FileLexStream() override {}

  size_t read(char* buffer, size_t size) override { return ::read(fd_, buffer, size); }
  void close() override {
    ::close(fd_);
    fd_ = 0;
  };

 private:
  int fd_;
};

class IStreamLexStream : public LexStream {
 public:
  IStreamLexStream(std::istream& is) : is_(is) {}
  ~IStreamLexStream() override {}

  size_t read(char* buffer, size_t size) override { return is_.readsome(buffer, size); }
  void close() override { is_.clear(); };

 private:
  std::istream& is_;
};

class StringLexStream : public LexStream {
 public:
  StringLexStream(std::string str) : str_(std::move(str)), stream_(str_) {}
  ~StringLexStream() override {}

  size_t read(char* buffer, size_t size) override {
    stream_.read(buffer, size);
    return stream_.gcount();
  }
  void close() override {
    stream_.clear();
  }

 private:
  std::string str_;
  std::istringstream stream_;
};

// Forward declarations only -- neither type's full definition is needed
// here. compiler_context_t (lex.h) is only ever touched through
// a unique_ptr, with LexTokenStream's ctor/dtor (which need it complete)
// defined out-of-line in lexer_utils.cc, which does include lex.h.
struct LexerSession;
struct compiler_context_t;
union YYSTYPE;

// ---------------------------------------------------------------------------
// LexTokenStream — a token-level abstraction built on top of the above, not
// a LexStream itself (it doesn't implement read()/close(); "the next unit"
// it hands out is a parser token, not a byte range). Declared here anyway,
// next to what it wraps, since it's the other half of "what does the
// compiler pull its input through."
//
// Owns a reentrant Flex scanner (yylex_init_extra/yylex_destroy via RAII)
// and feeds it a source via load(), then hands out tokens one at a time via
// next(). This is the consolidation point for "construct a scanner, point
// it at a file/string, auto-preprocess it, pull tokens" -- everything
// compile_file() used to wire up inline by hand.
//
// A single instance can be load()ed more than once, reusing the same
// underlying Flex scanner (and its allocated buffers) rather than tearing
// one down and building another -- e.g. a REPL can keep one LexTokenStream
// alive for a whole session and call load() once per statement, instead of
// paying scanner construction/destruction on every single line.
//
// Known scope boundary: the ring-buffer state load() resets via
// start_new_file() (cur_lbuf/head_lbuf, the #include stack, etc. in
// lexer_utils.cc) is still file-scope static, not per-instance -- so two
// LexTokenStream instances alive at once would corrupt each other's
// buffers. Not a problem today: compile_file()'s reentrancy guard already
// prevents concurrent compiles, and only one LexTokenStream exists at a
// time in any code that constructs one.
class LexTokenStream {
 public:
   LexTokenStream();
   ~LexTokenStream();
   LexTokenStream(const LexTokenStream &) = delete;
   LexTokenStream &operator=(const LexTokenStream &) = delete;

   // (Re-)point this scanner at a new source, exactly like starting a fresh
   // top-level compile (ring buffer reset, BEGIN(INITIAL), fresh include
   // stack). `stream` is auto-preprocessed on demand using `session` if given,
   // or a fresh one-chunk-only session otherwise.
   void load(std::unique_ptr<LexStream> stream, std::shared_ptr<LexerSession> session = nullptr);

   // Pull the next token, filling in *yylval_param.
   int next(union YYSTYPE *yylval_param);

   void *scanner() const { return scanner_; }

 private:
   std::unique_ptr<compiler_context_t> ctx_;
   void *scanner_ = nullptr;
 };

#endif /* end of include guard: LEX_STREAM_H */
