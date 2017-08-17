#include "base/std.h"

#include "main.h"

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include <iostream>  // for cout
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
#if USE_JEMALLOC == 1
#include "thirdparty/jemalloc/include/jemalloc/jemalloc.h"  // for mallctl
#endif
#include "vm/vm.h"  // for push_constant_string, etc

#include "packages/core/dns.h"  // for init_dns_event_base.

time_t boot_time;

static void sig_cld(int /*sig*/);

static void sig_usr1(int /*sig*/);
static void sig_usr2(int /*sig*/);

static void attempt_shutdown(int sig);
static void setup_signal_handlers();

inline void print_sep() { std::cout << std::string(72, '=') << std::endl; }

void print_version_and_time() {
  print_sep();

  /* Print current time */
  {
    time_t tm;
    time(&tm);
    std::cout << "Boot Time: " << ctime(&tm);
  }

  /* Print FluffOS version */
  std::cout << "FluffOS Version: " << PACKAGE_VERSION << "(" << SOURCE_REVISION << ")"
            << "@ (" << ARCH << ")" << std::endl;

#if USE_JEMALLOC == 1
  /* Print jemalloc version */
  {
    const char *ver;
    size_t resultlen = sizeof(ver);
    mallctl("version", &ver, &resultlen, NULL, 0);
    std::cout << "Jemalloc Version: " << ver << std::endl;
  }
#endif
}

void print_rlimit() {
  struct rlimit rlim;
  if (getrlimit(RLIMIT_CORE, &rlim)) {
    perror("Error reading RLIMIT_CORE: ");
    exit(1);
  } else {
    std::cout << "Core Dump: " << (rlim.rlim_cur == 0 ? "No" : "Yes") << ", ";
  }

  if (getrlimit(RLIMIT_NOFILE, &rlim)) {
    perror("Error reading RLIMIT_NOFILE: ");
    exit(1);
  } else {
    std::cout << "Max FD: " << rlim.rlim_cur << std::endl;
  }
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "C");
  tzset();
  boot_time = get_current_time();

  print_version_and_time();

  // try to bump FD limits.
  {
    struct rlimit rlim;
    rlim.rlim_cur = 65535;
    rlim.rlim_max = rlim.rlim_cur;
    if (setrlimit(RLIMIT_NOFILE, &rlim)) {
      // ignore this error.
    }
  }

  print_rlimit();

  std::cout << "Command: " << argv[0] << " ";
  for (int i = 1; i < argc; i++) {
    std::cout << argv[i] << " ";
  }
  std::cout << std::endl;

  print_sep();

#ifdef DEBUGMALLOC
  MDinit();
#endif

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
    fprintf(stderr, "Usage: %s config_file\n", argv[0]);
    exit(-1);
  }

  // TODO: dump configs

  // Make sure mudlib dir is correct.
  if (chdir(CONFIG_STR(__MUD_LIB_DIR__)) == -1) {
    fprintf(stderr, "Bad mudlib directory: %s\n", CONFIG_STR(__MUD_LIB_DIR__));
    exit(-1);
  }

  printf("Initializing internal stuff ....\n");

  // Initialize libevent, This should be done before executing LPC.
  auto base = init_backend();
  init_dns_event_base(base);

  // Initialize VM layer
  vm_init();

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
        case 'C':
          if (CONFIG_INT(__RC_HAS_CONSOLE__)) {
            has_console = 1;
          }
          continue;
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

  if (has_console == 1) {
    console_init(base);
  }

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
