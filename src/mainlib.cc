#include "base/std.h"

#include "mainlib.h"

#include <locale.h>  // for setlocale, LC_ALL
#ifdef HAVE_SIGNAL_H
#include <signal.h>  //  for signal, SIG_DFL, SIGABRT, etc
#endif
#include <stddef.h>  // for size_t
#include <stdio.h>   // for fprintf, stderr, printf, etc
#include <stdlib.h>  // for exit
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>  // for getrlimit
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#include <unistd.h>
#ifdef HAVE_JEMALLOC
#define JEMALLOC_MANGLE
#include <jemalloc/jemalloc.h>  // for mallctl
#endif
#include <unicode/uversion.h>

#include "base/internal/tracing.h"
#include "thirdparty/scope_guard/scope_guard.hpp"
#include "packages/core/dns.h"                   // for init_dns_event_base.
#include "vm/vm.h"                               // for push_constant_string, etc
#include "comm.h"                                // for init_user_conn
#include "backend.h"                             // for backend();
#include "thirdparty/backward-cpp/backward.hpp"  // for backtracing

// from lex.cc
extern void print_all_predefines();

namespace {
inline void print_sep() { debug_message("%s\n", std::string(72, '=').c_str()); }

void incrase_fd_rlimit() {
#ifndef _WIN32
  // try to bump FD limits.
  struct rlimit rlim;
  rlim.rlim_cur = 65535;
  rlim.rlim_max = rlim.rlim_cur;
  if (setrlimit(RLIMIT_NOFILE, &rlim)) {
    // ignore this error.
  }
#endif
}

void print_rlimit() {
#ifndef _WIN32
  // try to bump FD limits.
  {
    struct rlimit rlim;
    rlim.rlim_cur = 65535;
    rlim.rlim_max = rlim.rlim_cur;
    if (setrlimit(RLIMIT_NOFILE, &rlim)) {
      // ignore this error.
    }
  }

  struct rlimit rlim;
  if (getrlimit(RLIMIT_CORE, &rlim)) {
    perror("Error reading RLIMIT_CORE: ");
    exit(1);
  } else {
    debug_message("Core Dump: %s, ", (rlim.rlim_cur == 0 ? "No" : "Yes"));
  }

  if (getrlimit(RLIMIT_NOFILE, &rlim)) {
    perror("Error reading RLIMIT_NOFILE: ");
    exit(1);
  } else {
    debug_message("Max FD: %lu.\n", rlim.rlim_cur);
  }
#endif
}

void print_commandline(int argc, char **argv) {
  debug_message("Full Command Line: ");
  for (int i = 0; i < argc; i++) {
    debug_message("%s ", argv[i]);
  }
  debug_message("\n");
}

void print_version_and_time() {
  /* Print current time */
  {
    time_t tm = get_current_time();
    char buf[256] = {};
    debug_message("Boot Time: %s", ctime_r(&tm, buf));
  }

  /* Print FluffOS version */
  debug_message("Version: %s (%s)\n", PROJECT_VERSION, ARCH);

#ifdef HAVE_JEMALLOC
  /* Print jemalloc version */
  {
    const char *ver;
    size_t resultlen = sizeof(ver);
    mallctl("version", &ver, &resultlen, NULL, 0);
    debug_message("jemalloc Version: %s\n", ver);
  }
#else
  debug_message("Jemalloc is disabled, this is not suitable for production.\n");
#endif
  debug_message("ICU Version: %s\n", U_ICU_VERSION);

#ifndef _WIN32
#if BACKWARD_HAS_DW == 1
  debug_message("Backtrace support: libdw.\n");
#elif BACKWARD_HAS_BFD == 1
  debug_message("Backtrace support: libbfd.\n");
#else
  debug_message("libdw or libbfd is not found, you will only get very limited crash stacktrace.\n");
#endif
#endif /* _WIN32 */
}

void sig_cld(int sig) {
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
void sig_usr1(int sig) {
  push_constant_string("Host machine shutting down");
  push_undefined();
  push_undefined();
  apply_master_ob(APPLY_CRASH, 3);
  debug_message("Received SIGUSR1, calling exit(-1)\n");
  exit(-1);
}

/* Abort evaluation */
void sig_usr2(int sig) {
  debug_message("Received SIGUSR2, current eval aborted.\n");
  outoftime = 1;
}

/*
 * Actually, doing all this stuff from a signal is probably illegal
 * -Beek
 */
void attempt_shutdown(int sig) {
  const char *msg = "Unkonwn signal!";
  switch (sig) {
    case SIGTERM:
      msg = "SIGTERM: Process terminated";
      break;
    case SIGINT:
      msg = "SIGINT: Process interrupted";
      break;
  }

  // Reverse all traps
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // Print backtrace
  {
    using namespace backward;
    StackTrace st;
    st.load_here(64);
    Printer p;
    p.object = true;
    p.color_mode = ColorMode::automatic;
    p.address = true;
    p.print(st, stderr);
  }

  // Attempt to call crash()
  fatal(msg);
}

void init_locale() {
  setlocale(LC_ALL, "");
  // Verify locale is UTF8, complain otherwise
  std::string current_locale = setlocale(LC_ALL, nullptr);
  std::transform(current_locale.begin(), current_locale.end(), current_locale.begin(),
                 [](unsigned char c) { return std::tolower(c); });
#ifndef _WIN32
  if (current_locale.find(".utf-8") == std::string::npos) {
    debug_message("Your locale '%s' is not UTF8 compliant, you will likely run into issues.\n",
                  current_locale.c_str());
  }
#endif
}

void init_tz() {
#ifndef _WIN32
  tzset();
#else
  _tzset();
#endif
}
}  // namespace

struct event_base *init_main(int argc, char **argv) {
  /* read in the configuration file */
  bool got_config = false;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      // skip --flag val .
      if (argv[i][1] == '-') {
        i++;
      }
      continue;
    }
    read_config(argv[i]);
    got_config = true;
    break;
  }
  if (!got_config) {
    debug_message("Usage: %s config_file\n", argv[0]);
    exit(-1);
  }

  reset_debug_message_fp();

  // Make sure mudlib dir is correct.
  if (chdir(CONFIG_STR(__MUD_LIB_DIR__)) == -1) {
    debug_message("Bad mudlib directory: '%s'.\n", CONFIG_STR(__MUD_LIB_DIR__));
    exit(-1);
  }

  debug_message("Initializing internal stuff ....\n");

  // Initialize libevent, This should be done before executing LPC.
  auto base = init_backend();
  init_dns_event_base(base);

  // Initialize VM layer
  vm_init();

  return base;
}

void setup_signal_handlers() {
  signal(SIGTERM, attempt_shutdown);
  signal(SIGINT, attempt_shutdown);

#ifndef _WIN32
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
    debug_perror("can't ignore signal SIGPIPE", nullptr);
    exit(5);
  }
#endif
}

extern "C" {
int driver_main(int argc, char **argv);
}

void init_win32() {
#ifdef _WIN32
  WSADATA wsa_data;
  int err = WSAStartup(0x0202, &wsa_data);
  if (err != 0) {
    /* Tell the user that we could not find a usable */
    /* Winsock DLL.                                  */
    printf("WSAStartup failed with error: %d\n", err);
    exit(-1);
  }

  // try to get UTF-8 output
  SetConsoleOutputCP(65001);
#endif
}

int driver_main(int argc, char **argv) {
  init_locale();
  init_tz();
  incrase_fd_rlimit();
#ifdef _WIN32
  init_win32();
#endif

  print_sep();
  print_commandline(argc, argv);
  print_version_and_time();
  print_rlimit();
  print_sep();

  // backward-cpp doesn't yet work on win32

  // register crash handlers
  backward::SignalHandling sh;
  if (!sh.loaded()) {
    debug_message("Warning: Signal handler installation failed, not backtrace on crash!\n");
  }

  // First look for '--tracing' to decide if enable tracing from driver start.
  std::string trace_log;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    }

    if (strcmp(argv[i], "--tracing") == 0) {
      if (i + 1 >= argc) {
        debug_message("--tracing require an argument");
        exit(-1);
      }
      trace_log = argv[i + 1];
      break;
    }
  }

  DEFER { Tracer::collect(); };

  if (!trace_log.empty()) {
    debug_message("Saving tracing log to: %s\n", trace_log.c_str());
    Tracer::start(trace_log.c_str());
  }

  Tracer::setThreadName("FluffOS Main");
  ScopedTracer _main_tracer(__PRETTY_FUNCTION__);

  // Set debug log level first.
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    }
    switch (argv[i][1]) {
      case 'd':
        if (argv[i][2]) {
          debug_level_set(&argv[i][2]);
          debug_message("Debug log enabled: %s\n", &argv[i][2]);
        } else {
          debug_level |= DBG_DEFAULT;
          debug_message("Debug log enabled: %s\n", "default");
        }
        continue;
    }
  }
  debug_message("Final Debug Level: %d\n", debug_level);

  auto base = init_main(argc, argv);

  debug_message("==== Runtime Config Table ====\n");
  print_rc_table();
  debug_message("==============================\n");

  // from lex.cc
  debug_message("==== LPC Predefines ====\n");
  print_all_predefines();
  debug_message("========================\n");

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
        case 'f': {
          ScopedTracer _tracer("Driver Flag: calling master::flag", EventCategory::DEFAULT,
                               json{std::string(argv[i] + 2)});

          debug_message("Calling master::flag(\"%s\")...\n", argv[i] + 2);

          push_constant_string(argv[i] + 2);
          auto ret = safe_apply_master_ob(APPLY_FLAG, 1);
          if (ret == (svalue_t *)-1 || ret == nullptr || MudOS_is_being_shut_down) {
            debug_message("Shutdown by master object.\n");
            return -1;
          }
        }
          continue;
        case 'd':
          continue;
        case '-':
          if (strcmp(argv[i], "--tracing") == 0) {
            i++;
            continue;
          }
          // fall-through
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
