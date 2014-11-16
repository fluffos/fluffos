#include "std.h"

#include <cstdlib>  // for std::srand

#include "file_incl.h"
#include "lpc_incl.h"
#include "backend.h"
#include "simul_efun.h"
#include "main.h"
#include "otable.h"
#include "comm.h"
#include "lpc/compiler/compiler.h"
#include "port.h"
#include "md.h"
#include "main.h"
#include "socket_efuns.h"
#include "master.h"
#include "eval.h"
#include "posix_timers.h"
#include "console.h"
#include "event.h"
#include "dns.h"

port_def_t external_port[5];

int time_to_clean_up;
time_t boot_time;

static void sig_fpe(int);
static void sig_cld(int);

#ifdef HAS_CONSOLE
static void sig_ttin(int);
#endif
static void sig_usr1(int);
static void sig_usr2(int);
static void sig_term(int);
static void sig_int(int);
static void sig_abrt(int);
static void sig_segv(int);
static void sig_ill(int);
static void sig_bus(int);
static void sig_iot(int);

#ifdef DEBUG_MACRO
/* used by debug.h: please leave this in here -- Tru (you can change its
   value if you like).
 */
int debug_level = 0;
#endif

static void setup_signal_handlers();

int main(int argc, char **argv) {
  setlocale(LC_ALL, "C");
  tzset();

  setup_signal_handlers();

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

  {
    /*
     * Check that the definition of EXTRACT_UCHAR() is correct.
     */
    int i;
    char *p = (char *)&i;
    *p = -10;
    if (EXTRACT_UCHAR(p) != 0x100 - 10) {
      fprintf(stderr, "Bad definition of EXTRACT_UCHAR() in interpret.h.\n");
      exit(-1);
    }
  }

  {
    /*
     * An added test: can we do EXTRACT_UCHAR(x++)?
     * (read_number, etc uses it)
     */
    int i;
    char *p = (char *)&i;
    (void)EXTRACT_UCHAR(p++);
    if ((p - (char *)&i) != 1) {
      fprintf(stderr,
              "EXTRACT_UCHAR() in interpret.h evaluates its argument more than "
              "once.\n");
      exit(-1);
    }
  }

  /*
   * Check the living hash table size
   */
  if (CFG_LIVING_HASH_SIZE != 4 && CFG_LIVING_HASH_SIZE != 16 && CFG_LIVING_HASH_SIZE != 64 &&
      CFG_LIVING_HASH_SIZE != 256 && CFG_LIVING_HASH_SIZE != 1024 && CFG_LIVING_HASH_SIZE != 4096) {
    fprintf(stderr,
            "CFG_LIVING_HASH_SIZE in options.h must be one of 4, 16, 64, 256, "
            "1024, 4096, ...\n");
    exit(-1);
  }

  /*
   * Initialize the POSIX timers.
   */
  init_posix_timers();

  /* read in the configuration file */

  int got_defaults = 0;
  for (int i = 1; (i < argc) && !got_defaults; i++) {
    if (argv[i][0] != '-') {
      set_defaults(argv[i]);
      got_defaults = 1;
    }
  }
  if (!got_defaults) {
    fprintf(stderr, "You must specify the configuration filename as an argument.\n");
    exit(-1);
  }
  printf("Initializing internal tables....\n");
  init_strings();     /* in stralloc.c */
  init_otable();      /* in otable.c */
  init_identifiers(); /* in lex.c */
  init_locals();      /* in compiler.c */

  max_cost = CONFIG_INT(__MAX_EVAL_COST__);
  set_inc_list(CONFIG_STR(__INCLUDE_DIRS__));

  /*
   * The flags are parsed twice ! The first time, we only search for the -m
   * flag, which specifies another mudlib, and the D-flags, so that they
   * will be available when compiling master.c.
   */
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    }
    switch (argv[i][1]) {
      case 'D':
        if (argv[i][2]) {
          auto *def = (lpc_predef_t *)DMALLOC(sizeof(lpc_predef_t), TAG_PREDEFINES, "predef");
          def->flag = argv[i] + 2;
          def->next = lpc_predefs;
          lpc_predefs = def;
          continue;
        }
        fprintf(stderr, "Illegal flag syntax: %s\n", argv[i]);
        exit(-1);
#ifdef YYDEBUG
      case 'y':
        yydebug = 1;
        continue;
#endif /* YYDEBUG */
    }
  }

  if (chdir(CONFIG_STR(__MUD_LIB_DIR__)) == -1) {
    fprintf(stderr, "Bad mudlib directory: %s\n", CONFIG_STR(__MUD_LIB_DIR__));
    exit(-1);
  }

  {
    time_t tm;
    time(&tm);

    char version_buf[80];
    get_version(version_buf);

    debug_message(
        "------------------------------------------------------------------------"
        "----\n%s (%s) starting up on %s - %s\n\n",
        CONFIG_STR(__MUD_NAME__), version_buf, ARCH, ctime(&tm));
  }

  add_predefines();

  // Initialize libevent, This should be done before executing LPC.
  auto base = init_event_base();
  init_dns_event_base(base);

  reset_machine(1);

  try {
    init_simul_efun(CONFIG_STR(__SIMUL_EFUN_FILE__));
    init_master(CONFIG_STR(__MASTER_FILE__));
  } catch (const char *) {
    debug_message("The simul_efun (%s) and master (%s) objects must be loadable.\n",
                  CONFIG_STR(__SIMUL_EFUN_FILE__), CONFIG_STR(__MASTER_FILE__));
    exit(-1);
  }

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    } else {
      /*
       * Look at flags. ignore those already been tested.
       */
      switch (argv[i][1]) {
        case 'D':
        case 'y':
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
#ifdef DEBUG_MACRO
          if (argv[i][2]) {
            debug_level_set(&argv[i][2]);
          } else {
            debug_level |= DBG_DEFAULT;
          }
          debug_message("Debug Level: %d\n", debug_level);
#else
          debug_message("Driver must be compiled with DEBUG_MACRO on to use -d.\n");
#endif
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
#ifdef PACKAGE_MUDLIB_STATS
  restore_stat_files();
#endif

  preload_objects();

  // Initialize user connection socket
  init_user_conn();

#ifdef HAS_CONSOLE
  init_console(base);
#endif

  debug_message("Initializations complete.\n\n");

  backend(base);

  return 0;
}

void debug_message(const char *fmt, ...) {
  static FILE *debug_message_fp = nullptr;
  static char deb_buf[1024];
  static char *deb = deb_buf;
  va_list args;

  if (!debug_message_fp) {
    /*
     * check whether config file specified this option
     */
    auto dlf = CONFIG_STR(__DEBUG_LOG_FILE__);
    if (strlen(dlf)) {
      snprintf(deb, 1023, "%s/%s", CONFIG_STR(__LOG_DIR__), dlf);
    } else {
      snprintf(deb, 1023, "%s/debug.log", CONFIG_STR(__LOG_DIR__));
    }
    deb[1023] = 0;
    while (*deb == '/') {
      deb++;
    }
    debug_message_fp = fopen(deb, "w");
    if (!debug_message_fp) {
      perror(deb);
      /* use stderr by default. */
      debug_message_fp = stderr;
    }
  }

  char message[1024];

  va_start(args, fmt);
  vsnprintf(message, 1024, fmt, args);
  va_end(args);

  if (debug_message_fp != stderr) {
    fprintf(debug_message_fp, "%s", message);
    fflush(debug_message_fp);
  }
  fprintf(stderr, "%s", message);
  fflush(stderr);
}

static void setup_signal_handlers() {
  signal(SIGFPE, sig_fpe);
  signal(SIGUSR1, sig_usr1);
  signal(SIGUSR2, sig_usr2);
  signal(SIGTERM, sig_term);
  signal(SIGINT, sig_int);
  signal(SIGABRT, sig_abrt);
#ifdef SIGIOT
  signal(SIGIOT, sig_iot);
#endif
  signal(SIGHUP, startshutdownMudOS);
  signal(SIGBUS, sig_bus);
  signal(SIGSEGV, sig_segv);
  signal(SIGILL, sig_ill);
  signal(SIGCHLD, sig_cld);
  /*
   * register signal handler for SIGPIPE.
   */
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    debug_perror("can't ignore signal SIGPIPE", 0);
    exit(5);
  }
#ifdef HAS_CONSOLE
  if (has_console >= 0) {
    signal(SIGTTIN, sig_ttin);
  }
  signal(SIGTTOU, SIG_IGN);
#endif
}

static void try_dump_stacktrace() {
#if !defined(__CYGWIN__) && __GNUC__ > 2
  void *bt[100];
  size_t bt_size;
  bt_size = backtrace(bt, 100);
  backtrace_symbols_fd(bt, bt_size, STDERR_FILENO);
#else
  debug_message("Not able to generate backtrace, please use core.\n");
#endif
}

static void sig_cld(int sig) {
  int status;
  while (wait3(&status, WNOHANG, NULL) > 0) {
    ;
  }
}

static void sig_fpe(int sig) { signal(SIGFPE, sig_fpe); }

#ifdef HAS_CONSOLE
void restore_sigttin(void) {
  if (has_console >= 0) {
    signal(SIGTTIN, sig_ttin);
  }
}

/* The console goes to sleep when backgrounded and can
 * be woken back up with kill -SIGTTIN <pid>
 */
static void sig_ttin(int sig) {
  char junk[1024];
  int fl;

  has_console = !has_console;

  signal(SIGTTIN, SIG_IGN);

  if (has_console) {
    /* now eat all the gibberish they typed in the console when it was dead */
    fl = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK);

    while (read(STDIN_FILENO, junk, 1023) > 0) {
      ;
    } /* ; */

    /* leaving the output nonblocking is a bad idea.  large outputs tend
         to get truncated.
     */
    fcntl(STDIN_FILENO, F_SETFL, fl);
  }
}
#endif

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
static void sig_term(int sig) { fatal("SIGTERM: Process terminated"); }

static void sig_int(int sig) { fatal("SIGINT: Process interrupted"); }

static void sig_segv(int sig) {
  /* attempt to dump backtrace using gdb. */
  try_dump_stacktrace();
  fatal("SIGSEGV: Segmentation fault");
}

static void sig_bus(int sig) {
  try_dump_stacktrace();
  fatal("SIGBUS: Bus error");
}

static void sig_ill(int sig) {
  try_dump_stacktrace();
  fatal("SIGILL: Illegal instruction");
}

static void sig_abrt(int sig) {
  try_dump_stacktrace();
  fatal("SIGABRT: Aborted");
}

static void sig_iot(int sig) { fatal("Aborted(IOT)"); }
