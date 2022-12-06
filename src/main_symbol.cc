#include "base/std.h"

#include <cstdlib>
#include <cstdio>
#include <event2/event.h>
#include <iostream>
#include <unistd.h>

#include "mainlib.h"

#include "vm/vm.h"
#include "symbol.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: symbol <config> lpc_file" << std::endl;
    return 1;
  }

  symbol_enable(1);

  // Initialize libevent, This should be done before executing LPC.
  auto* base = init_main(argc, argv);

  vm_start();

  current_object = master_ob;
  const char* file = argv[2];
  struct object_t* obj = nullptr;

  error_context_t econ{};
  save_context(&econ);
  try {
    obj = find_object(file);
  } catch (...) {
    restore_context(&econ);
  }
  pop_context(&econ);

  if (obj == nullptr || obj->prog == nullptr) {
    fprintf(stderr, "Fail to load object %s. \n", file);
    return 1;
  }

  clear_state();

  return 0;
}
