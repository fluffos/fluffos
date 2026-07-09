#include "base/std.h"

#include <cstdlib>
#include <cstdio>
#include <event2/event.h>
#include <iostream>
#include <unistd.h>

#include "mainlib.h"

#include "thirdparty/scope_guard/scope_guard.hpp"
#include "compiler/internal/disassembler.h"
#include "compiler/internal/stage_output.h"
#include "compiler/internal/compiler.h"
#include <fcntl.h>
#include "base/internal/rc.h"
#include "base/internal/tracing.h"
#include "vm/vm.h"

int main(int argc, char** argv) {
  Tracer::start("trace_lpcc.json");

  Tracer::setThreadName("lpcc main");

  ScopedTracer const trace(__PRETTY_FUNCTION__);

  // Staged-output flags (see compiler/internal/stage_output.h):
  //   -E        preprocessed source (lex+pp, no parse)
  //   --tokens  one token per line (line:col kind spelling)
  //   --ast     dump parse trees before codegen (then compile as usual)
  //   -O0       compile with the tree optimizer off -> the dump_prog
  //             output is PRE-optimization bytecode
  bool flag_pp = false, flag_tokens = false, flag_ast = false, flag_noopt = false;
  const char* pos[3] = {argv[0], nullptr, nullptr};
  int npos = 1;
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
    else if (npos < 3)
      pos[npos++] = argv[i];
  }
  if (npos != 3) {
    std::cerr << "Usage: lpcc [-E|--tokens|--ast|-O0] config_file lpc_file" << std::endl;
    return 1;
  }
  argv = const_cast<char**>(pos);
  argc = 3;

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
    bool ok = lpc_dump_stage_tokens(fd, file, flag_pp, stdout);
    close(fd);
    return ok ? 0 : 1;
  }
  g_compile.opt_dump_ast = flag_ast;
  g_compile.opt_no_optimize = flag_noopt;

  {
    ScopedTracer const tracer("find_object");

    error_context_t econ{};
    save_context(&econ);
    try {
      obj = find_object(file);
    } catch (...) {
      restore_context(&econ);
    }
    pop_context(&econ);
  }

  if (obj == nullptr || obj->prog == nullptr) {
    fprintf(stderr, "Fail to load object %s. \n", file);
    return 1;
  }

  if (!flag_ast) {
    ScopedTracer const tracer("dump_prog");

    dump_prog(obj->prog, stdout, 1 | 2);
  }

  Tracer::collect();

  clear_state();

  return 0;
}
