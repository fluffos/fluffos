#include "base/std.h"
#include "mainlib.h"

#include <unistd.h>

#include "vm/vm.h"  // for push_constant_string, etc
#include "comm.h"     // for init_user_conn
#include "backend.h" // for backend();

int main(int argc, char **argv) {
  auto base = init_main(argc, argv);

  // Make sure mudlib dir is correct.
  if (chdir(CONFIG_STR(__MUD_LIB_DIR__)) == -1) {
    fprintf(stderr, "Bad mudlib directory: %s\n", CONFIG_STR(__MUD_LIB_DIR__));
    exit(-1);
  }

  // Start running.
  vm_start();

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    } else {
      /*
       * Look at flags. ignore those already been tested.
       */
      switch (argv[i][1]) {
        case 'f':
          debug_message("Calling master::flag(\"%s\")...\n", argv[i] + 2);
          push_constant_string(argv[i] + 2);
          safe_apply_master_ob(APPLY_FLAG, 1);
          if (MudOS_is_being_shut_down) {
            debug_message("Shutdown by master object.\n");
            exit(0);
          }
          continue;
        case 'd':
          if (argv[i][2]) {
            debug_level_set(&argv[i][2]);
          } else {
            debug_level |= DBG_DEFAULT;
          }
          debug_message("Debug Level: %d\n", debug_level);
          continue;
        default:
          debug_message("Unknown flag: %s\n", argv[i]);
          exit(-1);
      }
    }
  }
  if (MudOS_is_being_shut_down) {
    exit(1);
  }

  // Initialize user connection socket
  if (!init_user_conn()) {
    exit(1);
  }

  debug_message("Initializations complete.\n\n");
  setup_signal_handlers();
  backend(base);

  return 0;
}
