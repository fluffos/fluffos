#include "base/std.h"

#include <cstdlib>
#include <cstdio>
#include <event2/event.h>
#include <iostream>
#include <unistd.h>

#include "mainlib.h"
#include "compiler/internal/disassembler.h"
#include "base/internal/rc.h"
#include "vm/vm.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: lpcc <config> lpc_file";
    return 1;
  }
  // Initialize libevent, This should be done before executing LPC.
  auto base = init_main(argc, argv);

  // Make sure mudlib dir is correct.
  if (chdir(CONFIG_STR(__MUD_LIB_DIR__)) == -1) {
    fprintf(stderr, "Bad mudlib directory: %s\n", CONFIG_STR(__MUD_LIB_DIR__));
    exit(-1);
  }

  // Start running.
  vm_start();

  current_object = master_ob;
  const char* file = argv[2];

  error_context_t econ;
  save_context(&econ);
  struct object_t* obj = nullptr;
  try {
    obj = find_object(file);
  } catch (...) {
    restore_context(&econ);
    pop_context(&econ);
  }
  if (obj == nullptr || obj->prog == nullptr) {
    fprintf(stderr, "Fail to load object %s", file);
    return 1;
  }

  dump_prog(obj->prog, stdout, 1 | 2);

  clear_state();

  return 0;
}
