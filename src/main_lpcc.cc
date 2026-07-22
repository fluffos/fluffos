#include "base/std.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#include "mainlib.h"

#include <nlohmann/json.hpp>

#include "thirdparty/scope_guard/scope_guard.hpp"
#include "compiler/internal/disassembler.h"
#include "compiler/internal/stage_output.h"
#include "compiler/internal/compiler.h"
#include <fcntl.h>
#include "base/internal/rc.h"
#include "base/internal/tracing.h"
#include "vm/internal/eval_limit.h"
#include "vm/vm.h"

static int lpcc_main(int argc, char** argv) {
  Tracer::start("trace_lpcc.json");

  Tracer::setThreadName("lpcc main");

  ScopedTracer const trace(__PRETTY_FUNCTION__);

  // Staged-output flags (see compiler/internal/stage_output.h):
  //   -E        preprocessed source (lex+pp, no parse)
  //   --tokens  one token per line (line:col kind spelling)
  //   --ast     dump parse trees before codegen (then compile as usual)
  //   -O0       compile with the tree optimizer off -> the dump_prog
  //             output is PRE-optimization bytecode
  //   --json    machine-readable variant of a stage output (--tokens, --ast,
  //             and the default/-O0 bytecode dump). The payload is ONE line
  //             starting with {"fluffos_lpcc":1,...} so consumers can find
  //             it in mixed stdout. The bytecode model comes from the same
  //             disassembler switch as the text dump (DisSink json backend).
  //   --batch   compile MANY files against ONE VM boot instead of one file
  //             per process invocation -- see the batch block below.
  bool flag_pp = false, flag_tokens = false, flag_ast = false, flag_noopt = false;
  bool flag_json = false, flag_batch = false;
  const char* pos[3] = {argv[0], nullptr, nullptr};
  int npos = 1;
  std::vector<const char*> batch_files;
  for (int i = 1; i < argc; i++) {
    std::string_view a = argv[i];
    if (a == "-E")
      flag_pp = true;
    else if (a == "--tokens")
      flag_tokens = true;
    else if (a == "--ast")
      flag_ast = true;
    else if (a == "-O0")
      flag_noopt = true;
    else if (a == "--json")
      flag_json = true;
    else if (a == "--batch")
      flag_batch = true;
    else if (flag_batch) {
      // First non-flag arg after --batch is still the config file (pos[1]);
      // everything after that is a file to compile.
      if (npos < 2)
        pos[npos++] = argv[i];
      else
        batch_files.push_back(argv[i]);
    } else if (npos < 3)
      pos[npos++] = argv[i];
  }
  if (flag_batch) {
    if (flag_pp || flag_tokens || flag_ast || flag_json || flag_noopt) {
      std::cerr << "lpcc: --batch cannot be combined with -E/--tokens/--ast/-O0/--json"
                << std::endl;
      return 1;
    }
    if (npos != 2) {
      std::cerr << "Usage: lpcc --batch config_file [lpc_file ...]  (reads newline-separated "
                   "paths from stdin if none given)"
                << std::endl;
      return 1;
    }
  } else {
    if (npos != 3) {
      std::cerr << "Usage: lpcc [-E|--tokens|--ast|-O0] [--json] config_file lpc_file"
                << std::endl;
      return 1;
    }
    if (flag_json && flag_pp) {
      std::cerr << "lpcc: --json is not available for -E" << std::endl;
      return 1;
    }
  }
  argv = const_cast<char**>(pos);
  argc = flag_batch ? 2 : 3;

  Tracer::begin("init_main", EventCategory::DEFAULT);

  // Initialize libevent, This should be done before executing LPC.
  auto config = get_argument(0, argc, argv);
  auto* base = init_main(config);

  Tracer::end("init_main", EventCategory::DEFAULT);

  // Start running.
  {
    ScopedTracer const tracer("vm_start");

    vm_start();
  }

  if (flag_batch) {
    // Compile MANY files against this ONE VM boot (master/simul_efun loaded
    // once above) instead of paying a fresh boot per file -- a full boot is
    // the dominant cost of single-file `lpcc config file` for a sweep over
    // hundreds/thousands of mudlib files. This mirrors how a real driver
    // boot compiles many objects in the same process with no state reset
    // between them, so it's not just faster, it's a more realistic test.
    std::vector<std::string> files(batch_files.begin(), batch_files.end());
    if (files.empty()) {
      std::string line;
      while (std::getline(std::cin, line)) {
        if (!line.empty()) files.push_back(line);
      }
    }
    int failed = 0;
    for (const auto& f : files) {
      printf("===== %s =====\n", f.c_str());
      fflush(stdout);

      // set_eval() arms a real OS timer (see eval_limit.cc) that isn't reset
      // just by looping here -- without this, elapsed wall-clock time keeps
      // accumulating across every file's compile+create() in this one
      // process, and every file after the budget is used up spuriously
      // fails with "Too long evaluation. Execution aborted." regardless of
      // its own cost. The real driver rearms this before every top-level
      // command/heartbeat (comm.cc, vm.cc); do the same per batch file.
      set_eval(max_eval_cost);

      current_object = master_ob;
      struct object_t* obj = nullptr;
      error_context_t econ{};
      save_context(&econ);
      try {
        obj = find_object(f.c_str());
      } catch (...) {
        restore_context(&econ);
      }
      pop_context(&econ);
      current_object = master_ob;

      bool ok = obj != nullptr && obj->prog != nullptr;
      if (!ok) {
        fprintf(stderr, "Fail to load object %s.\n", f.c_str());
        failed++;
      }
      printf("%s %s\n", ok ? "PASS" : "FAIL", f.c_str());
      fflush(stdout);
    }
    Tracer::collect();
    clear_state();
    return failed > 0 ? 1 : 0;
  }

  current_object = master_ob;
  const char* file = argv[2];
  struct object_t* obj = nullptr;

  if (flag_pp || flag_tokens) {
    // Pre-parse stages: drive the lexer+preprocessor directly, no object.
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
      fprintf(stderr, "lpcc: cannot open %s\n", file);
      return 1;
    }
    bool ok = flag_json ? lpc_dump_stage_tokens_json(fd, file, stdout)
                        : lpc_dump_stage_tokens(fd, file, flag_pp, stdout);
    close(fd);
    return ok ? 0 : 1;
  }
  g_compile.opt_dump_ast = flag_ast && !flag_json;
  g_compile.opt_dump_ast_json = flag_ast && flag_json;
  g_compile.opt_no_optimize = flag_noopt;

  {
    ScopedTracer const tracer("find_object");

    error_context_t econ{};
    save_context(&econ);
    try {
      if (flag_ast) {
        // The AST only exists during an actual compile. An object that was
        // already loaded during boot (the master, the simul_efun object, and
        // everything they inherit) would produce no --ast output at all, so
        // force a fresh compile of those through the hot-reload path.
        obj = find_object2(file);
        if (obj != nullptr && obj->prog != nullptr) {
          // recompile_object() refuses while current_object executes the old
          // program, which would reject the master itself; nothing is
          // executing here, so drop the context for the duration.
          current_object = nullptr;
          recompile_object(obj);
        }
      }
      if (obj == nullptr || obj->prog == nullptr) {
        current_object = master_ob;
        obj = find_object(file);
      }
    } catch (...) {
      restore_context(&econ);
    }
    pop_context(&econ);
    current_object = master_ob;
  }

  if (obj == nullptr || obj->prog == nullptr) {
    fprintf(stderr, "Fail to load object %s. \n", file);
    return 1;
  }

  if (flag_json && flag_ast) {
    // Companion envelope for --ast --json: the AST's "l" values are
    // ABSOLUTE compilation-unit lines (includes inlined) and its
    // (string N) atoms are string-table indices; this supplies both
    // mappings. Consumers accumulate `segments` in order to translate an
    // absolute line to (file, line-within-file).
    nlohmann::json seg = nlohmann::json::array();
    unsigned short* fi = obj->prog->file_info;
    if (obj->prog->line_info != nullptr && fi != nullptr) {
      auto* li_start = reinterpret_cast<unsigned char*>(fi + fi[1]);
      for (unsigned short* p = fi + 2; p < reinterpret_cast<unsigned short*>(li_start); p += 2) {
        seg.push_back({{"lines", p[0]}, {"file", obj->prog->strings[p[1] - 1]}});
      }
    }
    nlohmann::json strs = nlohmann::json::array();
    for (int i = 0; i < static_cast<int>(obj->prog->num_strings); i++) {
      strs.push_back(obj->prog->strings[i]);
    }
    nlohmann::json envelope = {
        {"fluffos_lpcc", 1}, {"stage", "files"}, {"segments", seg}, {"strings", strs}};
    // LPC strings are arbitrary bytes; replace invalid UTF-8 instead of throwing.
    printf("%s\n",
           envelope.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace).c_str());
  }

  if (!flag_ast) {
    ScopedTracer const tracer("dump_prog");

    if (flag_json) {
      nlohmann::json envelope = {{"fluffos_lpcc", 1},
                                 {"stage", "bytecode"},
                                 {"file", file},
                                 {"optimized", !flag_noopt},
                                 {"program", dump_prog_json(obj->prog, 1 | 2)}};
      printf("%s\n",
             envelope.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace).c_str());
    } else {
      dump_prog(obj->prog, stdout, 1 | 2);
    }
  }

  Tracer::collect();

  clear_state();

  return 0;
}

int main(int argc, char** argv) {
  // Nothing may throw past main() (json serialization errors, an LPC
  // error() unwind escaping the guarded compile) -- that would be
  // std::terminate/abort instead of a clean CLI failure.
  try {
    return lpcc_main(argc, argv);
  } catch (const std::exception& e) {
    fprintf(stderr, "lpcc: fatal: %s\n", e.what());
    return 1;
  } catch (...) {
    fprintf(stderr, "lpcc: fatal: unhandled exception\n");
    return 1;
  }
}
