// AFL++ deferred-fork-server harness for the LPC compiler front-end (lexer
// + preprocessor + parser + codegen).
//
// Boots the VM once (master + simul_efun, same as lpcc/lpcshell), then
// calls __AFL_INIT() so AFL forks a fresh child per test case AFTER that
// expensive one-time setup. Each child splits the input on a "\n#==#\n"
// delimiter into a SEQUENCE of LPC source texts and compiles each in turn
// via load_object_from_source() -- the same in-memory "restart pattern"
// lpcshell uses -- in the SAME process, not just one compile per exec.
//
// A sequence (not a single compile) matters here for the same reason it
// mattered for the restore_variable() harness: AGENTS.md section 11 is
// explicit that the compiler keeps per-compile scratch state at module
// scope -- the scratchpad arena, the macro/predefine tables, expansion_
// frames/live_expansion_stack/live_guard_counts, the diagnostics stream --
// and every one of it "must be re-initialized per compile" (see
// lpc_lex_reset_context). A single-compile-per-exec harness can only find
// a bug that crashes DURING one compile; it can never find a bug where one
// compile's error path leaves that module-global state dirty for the NEXT
// compile in the same process -- exactly the object.cc save_svalue_depth/
// sizes[] bug class this file's sibling (main_fuzz_restore.cc) found, one
// level up the stack.
//
// deliberately does NOT run create()/__INIT (load_object_from_source's
// callcreate=0): the goal is the compiler surface specifically (matching
// lpcc, which also only compiles and dumps, never executes), not the VM/
// interpreter surface main_fuzz_restore.cc and the ordinary LPC testsuite
// already cover.
//
// Not wired into the default build (see BUILD_FUZZERS in CMakeLists.txt);
// throwaway investigation tooling, not shipped functionality.
#include "base/std.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "mainlib.h"
#include "vm/vm.h"
#include "vm/internal/simulate.h"
#include "vm/internal/base/interpret.h"
#include "vm/internal/base/object.h"

#ifdef __AFL_HAVE_MANUAL_CONTROL
#include <unistd.h>
#endif

namespace {

std::vector<char> read_file(const char* path) {
  std::vector<char> data;
  FILE* f = fopen(path, "rb");
  if (!f) return data;
  char buf[65536];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
    data.insert(data.end(), buf, buf + n);
  }
  fclose(f);
  return data;
}

constexpr std::string_view kDelim = "\n#==#\n";
constexpr size_t kMaxChunks = 8;  // compiles are heavier than restores

// One compile, in its own caught error context -- exactly how lpcshell's
// RunAttempt() drives load_object_from_source(). A successfully-compiled
// object is destructed immediately: this is a deferred-fork-server harness
// (the process exits right after run_sequence() regardless), but leaving
// compiled objects registered in the global object table until then would
// make LeakSanitizer flag ordinary "still reachable, by design" state as a
// leak, drowning out real findings.
void compile_one(int index, const std::string& src) {
  std::string name = "/fuzz_compile#" + std::to_string(index);

  error_context_t econ{};
  save_context(&econ);
  try {
    object_t* ob = load_object_from_source(src, name.c_str(), /*callcreate=*/0);
    if (ob && !(ob->flags & O_DESTRUCTED)) {
      destruct_object(ob);
    }
  } catch (const char*) {
    restore_context(&econ);
  } catch (...) {
    restore_context(&econ);
  }
  pop_context(&econ);
}

// Split on the delimiter and compile each piece in sequence, in this same
// process -- see the file header for why a sequence (not a single compile)
// is what this harness needs to find.
void run_sequence(const std::vector<char>& raw) {
  std::string_view all(raw.data(), raw.size());
  size_t pos = 0, chunks = 0;
  while (chunks < kMaxChunks) {
    size_t next = all.find(kDelim, pos);
    std::string_view piece =
        (next == std::string_view::npos) ? all.substr(pos) : all.substr(pos, next - pos);
    compile_one(static_cast<int>(chunks), std::string(piece));
    chunks++;
    if (next == std::string_view::npos) break;
    pos = next + kDelim.size();
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: fuzz_compile <config> <input_file>\n");
    return 1;
  }

  auto config = get_argument(0, argc, argv);
  init_main(config);
  vm_start();
  current_object = master_ob;

#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif

  std::vector<char> input = read_file(argv[2]);
  run_sequence(input);

  return 0;
}
