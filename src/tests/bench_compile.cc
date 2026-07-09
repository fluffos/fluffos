#include "base/std.h"

#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

#include "mainlib.h"
#include "vm/vm.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/scratchpad.h"

// ---------------------------------------------------------------------------
// bench_compile -- WHOLE-compile throughput across many rounds, i.e. the
// number that matters for a long-lived driver: compile the same
// representative LPC program N times through the real compile_file()
// pipeline (lex + preprocess + parse + codegen), deallocating the program
// each round, and report the per-round trend.
//
// What "good" looks like:
//   - steady-state ns/compile flat across rounds (no degradation over the
//     driver's lifetime),
//   - arena chunk mallocs STOP after warmup (the warm-chunk cache absorbs
//     every later compile -- printed from scratch_stats()).
//
// Boots the driver against the LPC testsuite config, like the unit tests.
// Not part of ctest (timing-based); run manually, ideally RelWithDebInfo:
//   ./src/tests/bench_compile [rounds]
// ---------------------------------------------------------------------------

namespace {

// Representative source: macros (object- and function-like), string
// literals and adjacent-literal folding, a template literal, classes,
// locals, loops, switches, calls -- the allocation-relevant constructs.
const char* kSource = R"(
#define SIZE 16
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define GREETING "hello " "world"

class point { int x; int y; }

private int counter;
private string label;

string describe(int n) {
    string out = GREETING;
    out += ` n=${n} max=${MAX(n, SIZE)}`;
    return out;
}

int sum_to(int n) {
    int total = 0;
    for (int i = 0; i < n; i++) total += i;
    return total;
}

string classify(int v) {
    switch (v) {
        case 0: return "zero";
        case 1..9: return "small";
        default: return "big";
    }
}

class point make_point(int x, int y) {
    class point p = new(class point, x: x, y: y);
    return p;
}

void create() {
    counter = sum_to(SIZE);
    label = describe(counter) + classify(counter);
    class point p = make_point(1, 2);
    counter = MAX(p.x, p.y);
}
)";

// The whole-compile rounds must OVERFLOW the arena's first chunk, or the
// retained-chunk machinery is never observed (a small program lives its
// whole life in the 64K persistent base and "0 mallocs" is trivially
// true). Append generated functions carrying ~2KB string literals until
// arena traffic per compile is several chunks deep.
std::string build_big_source() {
  std::string src = kSource;
  for (int i = 0; i < 96; i++) {
    std::string blob(2048, 'a' + (i % 26));
    src += "string blob_" + std::to_string(i) + "() {\n";
    src += "    string s = \"" + blob + "\" \" tail_" + std::to_string(i) + "\";\n";
    src += "    return s + ` v=${" + std::to_string(i) + " + MAX(" + std::to_string(i) +
           ", SIZE)}`;\n}\n";
  }
  return src;
}

using Clock = std::chrono::steady_clock;

double compile_once(const std::string& src) {
  auto t0 = Clock::now();
  program_t* prog = compile_file(src, "bench_compile_unit");
  auto t1 = Clock::now();
  if (prog == nullptr) {
    fprintf(stderr, "FATAL: benchmark source failed to compile\n");
    exit(1);
  }
  deallocate_program(prog);
  return std::chrono::duration<double, std::nano>(t1 - t0).count();
}

double avg(const std::vector<double>& v, size_t from, size_t to) {
  double s = 0;
  for (size_t i = from; i < to && i < v.size(); i++) s += v[i];
  return s / static_cast<double>(std::min(to, v.size()) - from);
}

}  // namespace

int main(int argc, char** argv) {
  int rounds = (argc > 1) ? atoi(argv[1]) : 2000;
  long chunk = (argc > 2) ? atol(argv[2]) : 0;  // e.g. 400: worst-case geometry

  chdir(TESTSUITE_DIR);
  init_main("etc/config.test");
  vm_start();
  current_object = master_ob;
  if (chunk > 0) {
    scratch_set_chunk_size_for_testing(static_cast<size_t>(chunk));
    printf("chunk payload FORCED to %ld bytes (worst-case mode)\n", chunk);
  }

  const std::string src = build_big_source();

  std::vector<double> ns;
  ns.reserve(rounds);
  ScratchStats warm{};
  for (int r = 0; r < rounds; r++) {
    ns.push_back(compile_once(src));
    if (r == 9) warm = scratch_stats();  // state after warmup window
  }
  ScratchStats end = scratch_stats();

  std::vector<double> sorted = ns;
  std::sort(sorted.begin(), sorted.end());
  size_t n = ns.size();
  printf("\nwhole-compile, %d rounds of the same %zu-byte program\n", rounds, src.size());
  printf("  first round      %10.1f us   (cold: includes first chunk mallocs)\n", ns[0] / 1e3);
  printf("  rounds 2-10      %10.1f us\n", avg(ns, 1, 10) / 1e3);
  printf("  first 10%%        %10.1f us\n", avg(ns, 0, n / 10) / 1e3);
  printf("  last 10%%         %10.1f us   (steady state; must not degrade)\n",
         avg(ns, n - n / 10, n) / 1e3);
  printf("  median           %10.1f us\n", sorted[n / 2] / 1e3);
  printf("  p99              %10.1f us\n", sorted[n * 99 / 100] / 1e3);
  printf("  throughput       %10.1f compiles/sec (steady)\n", 1e9 / avg(ns, n - n / 10, n));
  printf(
      "\narena: %zu chunk mallocs after warmup, %zu at end "
      "(delta %zu -- MUST be 0 for a leak-free steady state), "
      "%d retained chunks, %zu resets\n",
      warm.chunk_mallocs, end.chunk_mallocs, end.chunk_mallocs - warm.chunk_mallocs,
      end.retained_chunks, end.resets);

  double degradation = avg(ns, n - n / 10, n) / avg(ns, 0, n / 10);
  printf("last-10%% / first-10%% ratio: %.3f (>1.10 would indicate lifetime degradation)\n",
         degradation);
  return degradation > 1.10 ? 1 : 0;
}
