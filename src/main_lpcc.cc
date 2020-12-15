#include "base/std.h"

#include <cstdlib>
#include <cstdio>
#include <event2/event.h>
#include <iostream>
#include <unistd.h>

#include "mainlib.h"

#include "thirdparty/scope_guard/scope_guard.hpp"
#include "compiler/internal/disassembler.h"
#include "base/internal/rc.h"
#include "base/internal/tracing.h"
#include "vm/vm.h"

int main(int argc, char** argv) {
  Tracer::start("trace_lpcc.json");

  Tracer::setThreadName("lpcc main");

  ScopedTracer _trace(__PRETTY_FUNCTION__);

  if (argc != 3) {
    std::cerr << "Usage: lpcc <config> lpc_file" << std::endl;
    return 1;
  }

  Tracer::begin("init_main", EventCategory::DEFAULT);

  // Initialize libevent, This should be done before executing LPC.
  auto base = init_main(argc, argv);

  Tracer::end("init_main", EventCategory::DEFAULT);

  // Start running.
  {
    ScopedTracer _tracer("vm_start");

    vm_start();
  }

  current_object = master_ob;
  const char* file = argv[2];
  struct object_t* obj = nullptr;

  {
    ScopedTracer _tracer("find_object");

    error_context_t econ{};
    save_context(&econ);
    try {
      obj = find_object(file);
    } catch (...) {
      restore_context(&econ);
      pop_context(&econ);
    }
  }

  if (obj == nullptr || obj->prog == nullptr) {
    fprintf(stderr, "Fail to load object %s. \n", file);
    return 1;
  }

  {
    ScopedTracer _tracer("dump_prog");

    dump_prog(obj->prog, stdout, 1 | 2);
  }

  clear_state();

  Tracer::collect();

  return 0;
}
