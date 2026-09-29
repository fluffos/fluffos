// AFL++ deferred-fork-server harness for restore_variable()/save_variable().
//
// Boots the VM once (master + simul_efun, same as lpcc/lpcshell), then
// calls __AFL_INIT() so AFL forks a fresh child per test case AFTER that
// expensive one-time setup. Each child splits the input on a "\n===\n"
// delimiter into a SEQUENCE of save-strings and runs restore_variable() on
// each in turn, in the SAME process -- not just one call per exec. This
// matters: object.cc's restore_svalue()/restore_array()/restore_mapping()/
// restore_class() share file-scope scratch state (save_svalue_depth,
// sizes[]) across calls, and the known bug class here (see AGENTS.md
// section 13 point 4, and restore_variable_class.lpc) is exactly "one
// restore's error() path leaves that scratch state dirty, and the NEXT
// restore in the same process reads the stale value." A harness that forks
// fresh per input and calls restore_variable() only once could never find
// that -- it needs a first call to dirty the state and a second to observe
// it, both inside one exec. Every successfully-restored value is also
// round-tripped through save_variable()/restore_variable() once more, the
// same sequence used while chasing the reported printf("%O") corruption.
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
#include "vm/internal/base/object.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/base/interpret.h"

#ifdef __AFL_HAVE_MANUAL_CONTROL
#include <unistd.h>
#endif

char* save_variable(svalue_t* var);

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

constexpr std::string_view kDelim = "\n===\n";
constexpr size_t kMaxChunks = 64;  // bound worst-case work per exec

// One restore_variable() call, in its own caught error context -- exactly
// how f_restore_variable() runs it -- followed by a save_variable()/
// restore_variable() round-trip on success.
void run_chunk(std::string chunk) {
  error_context_t econ{};
  save_context(&econ);
  try {
    svalue_t v;
    v.type = T_NUMBER;
    restore_variable(&v, chunk.data());

    char* s2 = save_variable(&v);
    svalue_t v2;
    v2.type = T_NUMBER;
    restore_variable(&v2, s2);
    FREE_MSTR(s2);
    free_svalue(&v2, "fuzz_restore: v2");
    free_svalue(&v, "fuzz_restore: v");
  } catch (const char*) {
    restore_context(&econ);
  } catch (...) {
    restore_context(&econ);
  }
  pop_context(&econ);
}

// Split on the delimiter and feed each piece to restore_variable() in
// sequence, in this same process -- see the file header for why a sequence
// (not a single call) is what this harness needs to find.
void run_sequence(const std::vector<char>& raw) {
  std::string_view all(raw.data(), raw.size());
  size_t pos = 0, chunks = 0;
  while (chunks < kMaxChunks) {
    size_t next = all.find(kDelim, pos);
    std::string_view piece =
        (next == std::string_view::npos) ? all.substr(pos) : all.substr(pos, next - pos);
    // restore_variable() takes a plain NUL-terminated char*; embedded NULs
    // truncate the save string exactly like a real one read from a file
    // would if it somehow contained one (LPC strings can't, a fuzzed byte
    // can).
    run_chunk(std::string(piece));
    chunks++;
    if (next == std::string_view::npos) break;
    pos = next + kDelim.size();
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: fuzz_restore <config> <input_file>\n");
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
