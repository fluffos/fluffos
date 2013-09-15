#include "std.h"

#include <cstdlib>  // for std::srand

#include "file_incl.h"
#include "lpc_incl.h"
#include "backend.h"
#include "simul_efun.h"
#include "main.h"
#include "otable.h"
#include "comm.h"
#include "compiler.h"
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

static int e_flag = 0;    /* Load empty, without preloads. */
int t_flag = 0;     /* Disable heart beat and reset */
int comp_flag = 0;    /* Trace compilations */
int time_to_clean_up;
const char *default_fail_message;
time_t boot_time;
int max_array_size;
int max_buffer_size;
int max_string_length;
static int reserved_size;
char *reserved_area;    /* reserved for MALLOC() */
static char *mud_lib;

double consts[NUM_CONSTS];

static void CDECL sig_fpe(int);
static void CDECL sig_cld(int);

#ifdef HAS_CONSOLE
static void CDECL sig_ttin(int);
#endif

static void CDECL sig_usr1(int);
static void CDECL sig_usr2(int);
static void CDECL sig_term(int);
static void CDECL sig_int(int);
static void CDECL sig_abrt(int);
static void CDECL sig_segv(int);
static void CDECL sig_ill(int);
static void CDECL sig_bus(int);
static void CDECL sig_iot(int);

#ifdef DEBUG_MACRO
/* used by debug.h: please leave this in here -- Tru (you can change its
   value if you like).
 */
int debug_level = 0;
#endif

static void setup_signal_handlers();

int main(int argc, char **argv)
{
  time_t tm;
  int i, new_mudlib = 0, got_defaults = 0;
  char *p;
  char version_buf[80];
  struct rlimit rlim;
#if 0
  int dtablesize;
#endif
  error_context_t econ;

  setup_signal_handlers();

  /* Warn on core dump limit. */
  if (getrlimit(RLIMIT_CORE, &rlim)) {
    fprintf(stderr, "Error getting RLIMIT_CORE.");
  } else {
    if (rlim.rlim_cur == 0) {
      fprintf(stderr, "WARNING: rlimit for core dump is 0, you will "
              "not get core on crash.\n");
    }
  }

#ifdef INCL_LOCALE_H
  setlocale(LC_ALL, "C");
#endif

#ifdef WRAPPEDMALLOC
  wrappedmalloc_init();
#endif        /* WRAPPEDMALLOC */
#ifdef DEBUGMALLOC
  MDinit();
#endif

#ifdef HAVE_TZSET
  tzset();
#endif
  boot_time = get_current_time();

  // Seed random number
  std::srand(boot_time);

  const0.type = T_NUMBER;
  const0.u.number = 0;
  const1.type = T_NUMBER;
  const1.u.number = 1;

  /* const0u used by undefinedp() */
  const0u.type = T_NUMBER;
  const0u.subtype = T_UNDEFINED;
  const0u.u.number = 0;

  //fake_prog.program_size = 0; //0 anyway

  /*
   * Check that the definition of EXTRACT_UCHAR() is correct.
   */
  p = (char *) &i;
  *p = -10;
  if (EXTRACT_UCHAR(p) != 0x100 - 10) {
    fprintf(stderr, "Bad definition of EXTRACT_UCHAR() in interpret.h.\n");
    exit(-1);
  }

  /*
   * An added test: can we do EXTRACT_UCHAR(x++)?
   * (read_number, etc uses it)
   */
  p = (char *) &i;
  (void) EXTRACT_UCHAR(p++);
  if ((p - (char *) &i) != 1) {
    fprintf(stderr, "EXTRACT_UCHAR() in interpret.h evaluates its argument more than once.\n");
    exit(-1);
  }

  /*
   * Check the living hash table size
   */
  if (CFG_LIVING_HASH_SIZE != 4 && CFG_LIVING_HASH_SIZE != 16 &&
      CFG_LIVING_HASH_SIZE != 64 && CFG_LIVING_HASH_SIZE != 256 &&
      CFG_LIVING_HASH_SIZE != 1024 && CFG_LIVING_HASH_SIZE != 4096) {
    fprintf(stderr, "CFG_LIVING_HASH_SIZE in options.h must be one of 4, 16, 64, 256, 1024, 4096, ...\n");
    exit(-1);
  }

  current_virtual_time = get_current_time();
#ifdef POSIX_TIMERS
  /*
   * Initialize the POSIX timers.
   */
  init_posix_timers();
#endif

  /* read in the configuration file */

  got_defaults = 0;
  for (i = 1; (i < argc) && !got_defaults; i++) {
    if (argv[i][0] != '-') {
      set_defaults(argv[i]);
      got_defaults = 1;
    }
  }
  get_version(version_buf);
  if (!got_defaults) {
    fprintf(stderr, "%s for %s.\n", version_buf, ARCH);
    fprintf(stderr, "You must specify the configuration filename as an argument.\n");
    exit(-1);
  }

  printf("Initializing internal tables....\n");
  init_strings();   /* in stralloc.c */
  init_otable();    /* in otable.c */
  init_identifiers();   /* in lex.c */
  init_locals();              /* in compiler.c */

  /*
   * If our estimate is larger than FD_SETSIZE, then we need more file
   * descriptors than the operating system can handle.  This is a problem
   * that can be resolved by decreasing MAX_USERS, MAX_EFUN_SOCKS, or both.
   *
   * Unfortunately, since neither MAX_USERS or MAX_EFUN_SOCKS exist any more,
   * we have no clue how many we will need.  This code really should be
   * moved to places where ENFILE/EMFILE is returned.
   */
#if 0
  if (dtablesize > FD_SETSIZE) {
    fprintf(stderr, "Warning: File descriptor requirements exceed system capacity!\n");
    fprintf(stderr, "         Configuration exceeds system capacity by %d descriptor(s).\n",
            dtablesize - FD_SETSIZE);
  }
#ifdef HAS_SETDTABLESIZE
  /*
   * If the operating system supports setdtablesize() then we can request
   * the number of file descriptors we really need.  First check to see if
   * wee already have enough.  If so dont bother the OS. If not, attempt to
   * allocate the number we estimated above.  There are system imposed
   * limits on file descriptors, so we may not get as many as we asked for.
   * Check to make sure we get enough.
   */
  if (getdtablesize() < dtablesize)
    if (setdtablesize(dtablesize) < dtablesize) {
      fprintf(stderr, "Warning: Could not allocate enough file descriptors!\n");
      fprintf(stderr, "         setdtablesize() could not allocate %d descriptor(s).\n",
              getdtablesize() - dtablesize);
    }
  /*
   * Just be polite and tell the administrator how many he has.
   */
  fprintf(stderr, "%d file descriptors were allocated, (%d were requested).\n",
          getdtablesize(), dtablesize);
#endif
#endif
  time_to_clean_up = TIME_TO_CLEAN_UP;
  max_cost = MAX_COST;
  reserved_size = RESERVED_SIZE;
  max_array_size = MAX_ARRAY_SIZE;
  if (max_array_size > INT_MAX) {
    fprintf(stderr, "Maximum array size can not exceed %i", INT_MAX);
    max_array_size = INT_MAX;
  }
  max_buffer_size = MAX_BUFFER_SIZE;
  max_string_length = MAX_STRING_LENGTH;
  mud_lib = (char *) MUD_LIB;
  set_inc_list(INCLUDE_DIRS);
  if (reserved_size > 0) {
    reserved_area = (char *) DMALLOC(reserved_size, TAG_RESERVED, "main.c: reserved_area");
  }
  for (i = 0; i < sizeof consts / sizeof consts[0]; i++) {
    consts[i] = exp(-i / 900.0);
  }
  reset_machine(1);
  /*
   * The flags are parsed twice ! The first time, we only search for the -m
   * flag, which specifies another mudlib, and the D-flags, so that they
   * will be available when compiling master.c.
   */
  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    }
    switch (argv[i][1]) {
      case 'D':
        if (argv[i][2]) {
          lpc_predef_t *tmp = ALLOCATE(lpc_predef_t, TAG_PREDEFINES,
                                       "predef");
          tmp->flag = argv[i] + 2;
          tmp->next = lpc_predefs;
          lpc_predefs = tmp;
          continue;
        }
        fprintf(stderr, "Illegal flag syntax: %s\n", argv[i]);
        exit(-1);
#ifdef HAS_CONSOLE
      case 'C':
        has_console = 1;
        continue;
#endif
#ifdef YYDEBUG
      case 'y':
        yydebug = 1;
        continue;
#endif        /* YYDEBUG */
      case 'm':
        mud_lib = alloc_cstring(argv[i] + 2, "mudlib dir");
        if (chdir(mud_lib) == -1) {
          fprintf(stderr, "Bad mudlib directory: %s\n", mud_lib);
          exit(-1);
        }
        new_mudlib = 1;
        break;
    }
  }
  if (!new_mudlib && chdir(mud_lib) == -1) {
    fprintf(stderr, "Bad mudlib directory: %s\n", mud_lib);
    exit(-1);
  }
  time(&tm);
  debug_message("----------------------------------------------------------------------------\n%s (%s) starting up on %s - %s\n\n", MUD_NAME, version_buf, ARCH, ctime(&tm));

  add_predefines();
#ifdef WIN32
  _tzset();
#endif

  auto base = init_event_base();
  init_dns_event_base(base);

  save_context(&econ);

  try {
    init_simul_efun(SIMUL_EFUN);
    init_master();
  } catch (const char *) {
    debug_message("The simul_efun (%s) and master (%s) objects must be loadable.\n",
                  SIMUL_EFUN, MASTER_FILE);
    exit(-1);
  }
  set_eval(max_cost);
  pop_context(&econ);

  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      continue;
    } else {
      /*
       * Look at flags. -m and -o has already been tested.
       */
      switch (argv[i][1]) {
        case 'D':
        case 'N':
        case 'm':
        case 'y':
        case 'C':
          continue;
        case 'f':
          debug_message("Calling master::flag(\"%s\")...\n", argv[i] + 2);
          save_context(&econ);
          try {
            push_constant_string(argv[i] + 2);
            apply_master_ob(APPLY_FLAG, 1);
            if (MudOS_is_being_shut_down) {
              debug_message("Shutdown by master object.\n");
              exit(0);
            }
          } catch (const char *) {
            debug_message("Error while calling master::flag(\"%s\"), aborting ...\n", argv[i] + 2);
            exit(-1);
          }
          pop_context(&econ);
          continue;
        case 'e':
          e_flag++;
          continue;
        case 'p':
          external_port[0].port = atoi(argv[i] + 2);
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
          break;
        case 'c':
          comp_flag++;
          continue;
        case 't':
          t_flag++;
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
  if (*(DEFAULT_FAIL_MESSAGE)) {
    char buf[8192];

    strcpy(buf, DEFAULT_FAIL_MESSAGE);
    strcat(buf, "\n");
    default_fail_message = make_shared_string(buf);
  } else {
    default_fail_message = "What?\n";
  }
#ifdef PACKAGE_MUDLIB_STATS
  restore_stat_files();
#endif
  preload_objects(e_flag);

  // initialize user connection socket
  init_user_conn();

#ifdef HAS_CONSOLE
  init_console(base);
#endif

  debug_message("Initializations complete.\n\n");

  backend(base);

  return 0;
}

static FILE *debug_message_fp = 0;

void debug_message(const char *fmt, ...)
{
  static char deb_buf[1024];
  static char *deb = deb_buf;
  va_list args;

  if (!debug_message_fp) {
    /*
     * check whether config file specified this option
     */
    if (strlen(DEBUG_LOG_FILE)) {
      snprintf(deb, 1023, "%s/%s", LOG_DIR, DEBUG_LOG_FILE);
    } else {
      snprintf(deb, 1023, "%s/debug.log", LOG_DIR);
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
  V_START(args, fmt);
  V_VAR(char *, fmt, args);
  vsnprintf(message, 1024, fmt, args);
  va_end(args);

  if (debug_message_fp != stderr) {
    fprintf(debug_message_fp, "%s", message);
    fflush(debug_message_fp);
  }
  fprintf(stderr, "%s", message);
  fflush(stderr);
}

int slow_shut_down_to_do = 0;

char *xalloc(int size)
{
  char *p;
  const char *t;
  static int going_to_exit;

  if (going_to_exit) {
    exit(3);
  }
#ifdef DEBUG
  if (size == 0) {
    fatal("Tried to allocate 0 bytes.\n");
  }
#endif
  p = (char *) DMALLOC(size, TAG_MISC, "main.c: xalloc");
  if (p == 0) {
    if (reserved_area) {
      FREE(reserved_area);
      t = "Temporarily out of MEMORY. Freeing reserve.\n";
      write(1, t, strlen(t));
      reserved_area = 0;
      slow_shut_down_to_do = 6;
      return xalloc(size);/* Try again */
    }
    going_to_exit = 1;
    fatal("Totally out of MEMORY.\n");
  }
  return p;
}

static void setup_signal_handlers()
{
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

static void try_dump_stacktrace()
{
#if !defined(__CYGWIN__) && __GNUC__ > 2
  void *bt[100];
  size_t bt_size;
  bt_size = backtrace(bt, 100);
  backtrace_symbols_fd(bt, bt_size, STDERR_FILENO);
#else
  debug_message("Not able to generate backtrace, please use core.\n");
#endif
}

static void CDECL sig_cld(int sig)
{
#ifndef WIN32
  int status;
  while (wait3(&status, WNOHANG, NULL) > 0) {
    ;
  }
#endif
}

static void CDECL sig_fpe(int sig)
{
  signal(SIGFPE, sig_fpe);
}

#ifdef HAS_CONSOLE
void restore_sigttin(void)
{
  if (has_console >= 0) {
    signal(SIGTTIN, sig_ttin);
  }
}

/* The console goes to sleep when backgrounded and can
 * be woken back up with kill -SIGTTIN <pid>
 */
static void CDECL sig_ttin(int sig)
{
  char junk[1024];
  int fl;

  has_console = !has_console;

  signal(SIGTTIN, SIG_IGN);

  if (has_console) {
    /* now eat all the gibberish they typed in the console when it was dead */
    fl = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK);

    while (read(STDIN_FILENO, junk, 1023) > 0) { ; } /* ; */

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
static void CDECL sig_usr1(int sig)
{
  push_constant_string("Host machine shutting down");
  push_undefined();
  push_undefined();
  apply_master_ob(APPLY_CRASH, 3);
  debug_message("Received SIGUSR1, calling exit(-1)\n");
  exit(-1);
}

/* Abort evaluation */
static void CDECL sig_usr2(int sig)
{
  debug_message("Received SIGUSR2, current eval aborted.\n");
  outoftime = 1;
}

/*
 * Actually, doing all this stuff from a signal is probably illegal
 * -Beek
 */
static void CDECL sig_term(int sig)
{
  fatal("SIGTERM: Process terminated");
}

static void CDECL sig_int(int sig)
{
  fatal("SIGINT: Process interrupted");
}

static void CDECL sig_segv(int sig)
{
  /* attempt to dump backtrace using gdb. */
  try_dump_stacktrace();
  fatal("SIGSEGV: Segmentation fault");
}

static void CDECL sig_bus(int sig)
{
  try_dump_stacktrace();
  fatal("SIGBUS: Bus error");
}

static void CDECL sig_ill(int sig)
{
  try_dump_stacktrace();
  fatal("SIGILL: Illegal instruction");
}

static void CDECL sig_abrt(int sig)
{
  try_dump_stacktrace();
  fatal("SIGABRT: Aborted");
}

static void CDECL sig_iot(int sig)
{
  fatal("Aborted(IOT)");
}

