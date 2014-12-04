#include "base/std.h"

#include "main.h"

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include <locale.h>  // for setlocale, LC_ALL
#include <signal.h>  // for signal, SIG_DFL, SIGABRT, etc
#include <stddef.h>  // for size_t
#include <stdio.h>   // for fprintf, stderr, printf, etc
#include <stdlib.h>  // for exit
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>  // for getrlimit
#endif
#include <time.h>
#include <unistd.h>

#include "backend.h"  // for backend, init_backend
#include "cc.h"       // for SOURCE_REVISION
#include "comm.h"     // for init_user_conn
#include "console.h"  // for console_init, has_console
#include "vm/vm.h"    // for push_constant_string, etc

#include "packages/core/dns.h"  // for init_dns_event_base.

time_t boot_time;

static void sig_cld(int);

static void sig_usr1(int);
static void sig_usr2(int);

static void attempt_shutdown(int sig);
static void setup_signal_handlers();

int main(int argc, char **argv) {
  setlocale(LC_ALL, "C");
  tzset();
  boot_time = get_current_time();

  /* Warn on core dump limit. */
  {
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim)) {
      fprintf(stderr, "Error getting RLIMIT_CORE.");
    } else {
      if (rlim.rlim_cur == 0) {
        fprintf(stderr,
                "WARNING: rlimit for core dump is 0, you will "
                "not get core on crash.\n");
      }
    }
  }

#ifdef DEBUGMALLOC
  MDinit();
#endif

  /*
   * Check the living hash table size
   */
  /*
  if (CFG_LIVING_HASH_SIZE != 4 && CFG_LIVING_HASH_SIZE != 16 && CFG_LIVING_HASH_SIZE != 64 &&
      CFG_LIVING_HASH_SIZE != 256 && CFG_LIVING_HASH_SIZE != 1024 && CFG_LIVING_HASH_SIZE != 4096) {
    fprintf(stderr,
            "CFG_LIVING_HASH_SIZE in options.h must be one of 4, 16, 64, 256, "
            "1024, 4096, ...\n");
    exit(-1);
  }*/

  /* read in the configuration file */
  bool got_config = false;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      read_config(argv[i]);
      got_config = true;
      break;
    }
  }
  if (!got_config) {
    fprintf(stderr, "You must specify the configuration filename as an argument.\n");
    exit(-1);
  }

  if (chdir(CONFIG_STR(__MUD_LIB_DIR__)) == -1) {
    fprintf(stderr, "Bad mudlib directory: %s\n", CONFIG_STR(__MUD_LIB_DIR__));
    exit(-1);
  }

  printf("Initializing internal stuff ....\n");

  // Init base layer.
  {
    time_t tm;
    time(&tm);
    char version_buf[80];
    sprintf(version_buf, "%s", SOURCE_REVISION);

    debug_message(
        "------------------------------------------------------------------------"
        "----\n%s (%s) starting up on %s - %s\n\n",
        CONFIG_STR(__MUD_NAME__), version_buf, ARCH, ctime(&tm));
  }

  // Initialize libevent, This should be done before executing LPC.
  auto base = init_backend();
  init_dns_event_base(base);

  // Initiliaze VM layer
  vm_init();

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    } else {
      /*
       * Look at flags. ignore those already been tested.
       */
      switch (argv[i][1]) {
#ifdef HAS_CONSOLE
        case 'C':
          has_console = 1;
          continue;
#endif
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

#ifdef HAS_CONSOLE
  console_init(base);
#endif

  debug_message("Initializations complete.\n\n");

  setup_signal_handlers();

  backend(base);

  return 0;
}

static void setup_signal_handlers() {
  signal(SIGFPE, attempt_shutdown);
  signal(SIGTERM, attempt_shutdown);
  signal(SIGINT, attempt_shutdown);
  signal(SIGABRT, attempt_shutdown);
  signal(SIGBUS, attempt_shutdown);
  signal(SIGSEGV, attempt_shutdown);
  signal(SIGILL, attempt_shutdown);

  // User signal
  signal(SIGUSR1, sig_usr1);
  signal(SIGUSR2, sig_usr2);

  // shutdown
  signal(SIGHUP, startshutdownMudOS);

  // for external events?
  signal(SIGCHLD, sig_cld);

  /*
   * we use nonblocking socket, must ignore SIGPIPE.
   */
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    debug_perror("can't ignore signal SIGPIPE", 0);
    exit(5);
  }
}

static void try_dump_stacktrace() {
#if !defined(__CYGWIN__) && __GNUC__ > 2
  static void *bt[100];
  size_t bt_size;
  bt_size = backtrace(bt, 100);
  backtrace_symbols_fd(bt, bt_size, STDERR_FILENO);
#else
  debug_message("Not able to generate backtrace, please use core.\n");
#endif
}

static void sig_cld(int sig) {
  /*FIXME: restore this
  int status;
  while (wait3(&status, WNOHANG, NULL) > 0) {
    ;
  }*/
}

/* send this signal when the machine is about to reboot.  The script
   which restarts the MUD should take an exit code of 1 to mean don't
   restart
 */
static void sig_usr1(int sig) {
  push_constant_string("Host machine shutting down");
  push_undefined();
  push_undefined();
  apply_master_ob(APPLY_CRASH, 3);
  debug_message("Received SIGUSR1, calling exit(-1)\n");
  exit(-1);
}

/* Abort evaluation */
static void sig_usr2(int sig) {
  debug_message("Received SIGUSR2, current eval aborted.\n");
  outoftime = 1;
}

/*
 * Actually, doing all this stuff from a signal is probably illegal
 * -Beek
 */
static void attempt_shutdown(int sig) {
  const char *msg = "Unkonwn signal!";
  switch (sig) {
    case SIGABRT:
      msg = "SIGABRT: Aborted";
      break;
    case SIGTERM:
      msg = "SIGTERM: Process terminated";
      break;
    case SIGINT:
      msg = "SIGINT: Process interrupted";
      break;
    case SIGSEGV:
      msg = "SIGSEGV: Segmentation fault";
      break;
    case SIGFPE:
      msg = "SIGFPE: Floating point exception";
      break;
    case SIGBUS:
      msg = "SIGBUS: Bus error";
      break;
    case SIGILL:
      msg = "SIGILL: Illegal instruction";
      break;
  }

  // Reverse all traps
  signal(SIGFPE, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  signal(SIGSEGV, SIG_DFL);
  signal(SIGILL, SIG_DFL);

  try_dump_stacktrace();
  fatal(msg);
}
