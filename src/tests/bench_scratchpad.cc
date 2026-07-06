#include "base/std.h"

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "compiler/internal/scratchpad.h"

// ---------------------------------------------------------------------------
// bench_scratchpad -- proves the compile arena beats general-purpose heap
// allocation for the compiler's actual allocation patterns. Each benchmark
// runs the SAME workload twice: once on the scratchpad (ScratchString /
// ScratchVector / scratch_new_string, bulk-reset per "compile"), once on
// the heap (std::string / std::vector / new std::string, destructed
// per "compile"). Reports ns/op and the speedup ratio.
//
// Workloads mirror real usage:
//   tokens      -- materialize many small identifier-sized token strings
//                  (the value-stack pattern: write once, never freed
//                  individually, bulk release at compile end)
//   accum       -- string-literal/heredoc accumulation: += growth to ~4KB,
//                  then materialize a token from it
//   macro_args  -- function-like macro argument collection: a vector of
//                  short argument strings + their expanded copies
//   compile_mix -- a whole miniature "compile": tokens + accumulations +
//                  arg vectors, then the end-of-compile release
//
// Not part of ctest (timing-based); run manually, ideally RelWithDebInfo.
// ---------------------------------------------------------------------------

namespace {

// Prevent the optimizer from discarding a result.
template <class T>
inline void escape(T &&v) {
  asm volatile("" : : "g"(&v) : "memory");
}

using Clock = std::chrono::steady_clock;

double run_timed(int reps, void (*fn)(int)) {
  // Best of 3: reduces scheduler noise without averaging in cold caches.
  double best = 1e100;
  for (int attempt = 0; attempt < 3; attempt++) {
    auto t0 = Clock::now();
    fn(reps);
    auto t1 = Clock::now();
    double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
    if (ns < best) best = ns;
  }
  return best;
}

void report(const char *name, int ops, double scratch_ns, double heap_ns) {
  printf("%-12s %12.1f ns/op scratch %12.1f ns/op heap   speedup %5.2fx\n", name,
         scratch_ns / ops, heap_ns / ops, heap_ns / scratch_ns);
}

// --- tokens ----------------------------------------------------------------
constexpr int kTokensPerCompile = 2000;
const char *token_text(int i, char *buf) {
  snprintf(buf, 32, "ident_%d_%d", i, i * 7);
  return buf;
}

void tokens_scratch(int reps) {
  char buf[32];
  for (int r = 0; r < reps; r++) {
    for (int i = 0; i < kTokensPerCompile; i++) {
      ScratchString *s = scratch_new_string(token_text(i, buf));
      escape(s);
    }
    scratch_destroy();
  }
}

void tokens_heap(int reps) {
  char buf[32];
  std::vector<std::unique_ptr<std::string>> owned;
  owned.reserve(kTokensPerCompile);
  for (int r = 0; r < reps; r++) {
    for (int i = 0; i < kTokensPerCompile; i++) {
      owned.push_back(std::make_unique<std::string>(token_text(i, buf)));
      escape(owned.back().get());
    }
    owned.clear();  // the heap's "compile end": one delete per token
  }
}

// --- accum -----------------------------------------------------------------
constexpr int kAccumPerCompile = 50;
constexpr int kAccumAppends = 256;  // x16 bytes ~= 4KB final string

// Mirrors the real scanner: ONE persistent accumulator per compile
// (compiler_context_t::str_accum), .clear()ed per literal so capacity is
// reused; each finished literal is materialized as a token.
void accum_scratch(int reps) {
  for (int r = 0; r < reps; r++) {
    ScratchString accum;
    for (int a = 0; a < kAccumPerCompile; a++) {
      accum.clear();
      for (int i = 0; i < kAccumAppends; i++) {
        accum += "0123456789abcdef";
      }
      ScratchString *tok = scratch_new_string(accum);
      escape(tok);
    }
    accum = ScratchString();  // per-compile reset (lpc_lex_reset_context)
    scratch_destroy();
  }
}

void accum_heap(int reps) {
  std::vector<std::unique_ptr<std::string>> owned;
  for (int r = 0; r < reps; r++) {
    std::string accum;
    for (int a = 0; a < kAccumPerCompile; a++) {
      accum.clear();
      for (int i = 0; i < kAccumAppends; i++) {
        accum += "0123456789abcdef";
      }
      owned.push_back(std::make_unique<std::string>(accum));
      escape(owned.back().get());
    }
    owned.clear();  // the heap's compile-end release, one delete per token
  }
}

// --- macro_args ------------------------------------------------------------
constexpr int kMacroCallsPerCompile = 300;
constexpr int kArgsPerCall = 6;

void macro_args_scratch(int reps) {
  for (int r = 0; r < reps; r++) {
    for (int c = 0; c < kMacroCallsPerCompile; c++) {
      ScratchVector<ScratchString> args;
      args.reserve(kArgsPerCall);
      for (int i = 0; i < kArgsPerCall; i++) {
        args.emplace_back("argument_text_16");
      }
      ScratchVector<ScratchString> expanded;
      expanded.reserve(args.size());
      for (const auto &a : args) {
        expanded.emplace_back(a);
        expanded.back() += "_expanded";
      }
      escape(expanded);
    }
    scratch_destroy();
  }
}

void macro_args_heap(int reps) {
  for (int r = 0; r < reps; r++) {
    for (int c = 0; c < kMacroCallsPerCompile; c++) {
      std::vector<std::string> args;
      args.reserve(kArgsPerCall);
      for (int i = 0; i < kArgsPerCall; i++) {
        args.emplace_back("argument_text_16");
      }
      std::vector<std::string> expanded;
      expanded.reserve(args.size());
      for (const auto &a : args) {
        expanded.emplace_back(a);
        expanded.back() += "_expanded";
      }
      escape(expanded);
    }
  }
}

// --- compile_mix -------------------------------------------------------------
void mix_scratch(int reps) {
  char buf[32];
  for (int r = 0; r < reps; r++) {
    for (int i = 0; i < 400; i++) {
      escape(scratch_new_string(token_text(i, buf)));
    }
    ScratchString accum;
    for (int a = 0; a < 10; a++) {
      accum.clear();
      for (int i = 0; i < 64; i++) accum += "0123456789abcdef";
      escape(scratch_new_string(accum));
    }
    for (int c = 0; c < 30; c++) {
      ScratchVector<ScratchString> args;
      for (int i = 0; i < kArgsPerCall; i++) args.emplace_back("argument_text_16");
      escape(args);
    }
    scratch_destroy();
  }
}

void mix_heap(int reps) {
  char buf[32];
  std::vector<std::unique_ptr<std::string>> owned;
  for (int r = 0; r < reps; r++) {
    for (int i = 0; i < 400; i++) {
      owned.push_back(std::make_unique<std::string>(token_text(i, buf)));
    }
    std::string accum;
    for (int a = 0; a < 10; a++) {
      accum.clear();
      for (int i = 0; i < 64; i++) accum += "0123456789abcdef";
      owned.push_back(std::make_unique<std::string>(accum));
    }
    for (int c = 0; c < 30; c++) {
      std::vector<std::string> args;
      for (int i = 0; i < kArgsPerCall; i++) args.emplace_back("argument_text_16");
      escape(args);
    }
    owned.clear();
  }
}

}  // namespace

int main(int argc, char **argv) {
  int reps = (argc > 1) ? atoi(argv[1]) : 200;
  printf("scratchpad vs heap, %d simulated compiles per workload (best of 3)\n\n", reps);

  double s, h;
  s = run_timed(reps, tokens_scratch);
  h = run_timed(reps, tokens_heap);
  report("tokens", reps * kTokensPerCompile, s, h);

  s = run_timed(reps, accum_scratch);
  h = run_timed(reps, accum_heap);
  report("accum", reps * kAccumPerCompile, s, h);

  s = run_timed(reps, macro_args_scratch);
  h = run_timed(reps, macro_args_heap);
  report("macro_args", reps * kMacroCallsPerCompile, s, h);

  s = run_timed(reps, mix_scratch);
  h = run_timed(reps, mix_heap);
  report("compile_mix", reps, s, h);

  return 0;
}
