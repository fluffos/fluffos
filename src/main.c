#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "file_incl.h"
#include "lpc_incl.h"
#include "backend.h"
#include "simul_efun.h"
#include "binaries.h"
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

port_def_t external_port[5];

static int e_flag = 0;    /* Load empty, without preloads. */
int t_flag = 0;     /* Disable heart beat and reset */
int comp_flag = 0;    /* Trace compilations */
int max_cost;
int time_to_clean_up;
const char *default_fail_message;
int boot_time;
int max_array_size;
int max_buffer_size;
int max_string_length;
static int reserved_size;
char *reserved_area;    /* reserved for MALLOC() */
static char *mud_lib;

double consts[NUM_CONSTS];

#ifndef NO_IP_DEMON
int no_ip_demon = 0;
void init_addr_server();
#endif        /* NO_IP_DEMON */

#ifdef SIGNAL_FUNC_TAKES_INT
#define SIGPROT (int)
#define PSIG(z) z (int sig)
#else
#define SIGPROT (void)
#define PSIG(z) z()
#endif

static void CDECL sig_fpe SIGPROT;
static void CDECL sig_cld SIGPROT;

#ifdef TRAP_CRASHES
static void CDECL sig_usr1 SIGPROT;
static void CDECL sig_usr2 SIGPROT;
static void CDECL sig_term SIGPROT;
static void CDECL sig_int SIGPROT;

#ifndef DEBUG
static void CDECL sig_hup SIGPROT,
    CDECL sig_abrt SIGPROT,
    CDECL sig_segv SIGPROT,
    CDECL sig_ill SIGPROT,
    CDECL sig_bus SIGPROT,
    CDECL sig_iot SIGPROT;
#endif
#endif

#ifdef DEBUG_MACRO
/* used by debug.h: please leave this in here -- Tru (you can change its
   value if you like).
*/
int debug_level = 0;
#endif

int main (int argc, char ** argv)
{
    time_t tm;
    int i, new_mudlib = 0, got_defaults = 0;
    char *p;
    char version_buf[80];
#if 0
    int dtablesize;
#endif
    error_context_t econ;

#ifdef PROTO_TZSET
    void tzset();
#endif

#ifdef INCL_LOCALE_H
    setlocale(LC_ALL, "");
#endif

#if !defined(__SASC) && (defined(AMITCP) || defined(AS225))
    amiga_sockinit();
    atexit(amiga_sockexit);
#endif
#ifdef WRAPPEDMALLOC
    wrappedmalloc_init();
#endif        /* WRAPPEDMALLOC */
#ifdef DEBUGMALLOC
    MDinit();
#endif

#if (defined(PROFILING) && !defined(PROFILE_ON) && defined(HAS_MONCONTROL))
    moncontrol(0);
#endif
#ifdef USE_TZSET
    tzset();
#endif
    boot_time = get_current_time();

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

#ifdef RAND
    srand(get_current_time());
#else
#  ifdef DRAND48
    srand48(get_current_time());
#  else
#    ifdef RANDOM
    srandom(get_current_time());
#    else
    fprintf(stderr, "Warning: no random number generator specified!\n");
#    endif
#  endif
#endif
    current_time = get_current_time();
    /*
     * Initialize the microsecond clock.
     */
    init_usec_clock();

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
    if(max_array_size > 65535){
      fprintf(stderr, "Maximum array size can not exceed 65535");
      max_array_size = 65535;
    }
    max_buffer_size = MAX_BUFFER_SIZE;
    max_string_length = MAX_STRING_LENGTH;
    mud_lib = (char *) MUD_LIB;
    set_inc_list(INCLUDE_DIRS);
    if (reserved_size > 0)
  reserved_area = (char *) DMALLOC(reserved_size, TAG_RESERVED, "main.c: reserved_area");
    for (i = 0; i < sizeof consts / sizeof consts[0]; i++)
  consts[i] = exp(-i / 900.0);
    reset_machine(1);
    /*
     * The flags are parsed twice ! The first time, we only search for the -m
     * flag, which specifies another mudlib, and the D-flags, so that they
     * will be available when compiling master.c.
     */
    for (i = 1; i < argc; i++) {
  if (argv[i][0] != '-')
      continue;
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
  case 'N':
      no_ip_demon++;
      continue;
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

#ifdef BINARIES
    init_binaries(argc, argv);
#endif
    add_predefines();
#ifdef WIN32
    _tzset();
#endif

#ifndef NO_IP_DEMON
    if (!no_ip_demon && ADDR_SERVER_IP)
  init_addr_server(ADDR_SERVER_IP, ADDR_SERVER_PORT);
#endif        /* NO_IP_DEMON */

    set_eval(max_cost);

    save_context(&econ);
    if (SETJMP(econ.context)) {
  debug_message("The simul_efun (%s) and master (%s) objects must be loadable.\n", 
          SIMUL_EFUN, MASTER_FILE);
  exit(-1);
    } else {
  init_simul_efun(SIMUL_EFUN);
  init_master();
    }
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
    continue;
      case 'f':
    save_context(&econ);
    if (SETJMP(econ.context)) {
        debug_message("Error while calling master::flag(\"%s\"), aborting ...\n", argv[i] + 2);
        exit(-1);
    }
    push_constant_string(argv[i] + 2);
    apply_master_ob(APPLY_FLAG, 1);
    if (MudOS_is_being_shut_down) {
        debug_message("Shutdown by master object.\n");
        exit(0);
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
    if (argv[i][2])
        debug_level_set(&argv[i][2]);
    else
        debug_level |= DBG_d_flag;
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
    if (MudOS_is_being_shut_down)
  exit(1);
    if (*(DEFAULT_FAIL_MESSAGE)) {
  char buf[8192];

  strcpy(buf, DEFAULT_FAIL_MESSAGE);
  strcat(buf, "\n");
  default_fail_message = make_shared_string(buf);
    } else
  default_fail_message = "What?\n";
#ifdef PACKAGE_MUDLIB_STATS
    restore_stat_files();
#endif
    preload_objects(e_flag);
#ifdef SIGFPE
    signal(SIGFPE, sig_fpe);
#endif
#ifdef TRAP_CRASHES
#ifdef SIGUSR1
    signal(SIGUSR1, sig_usr1);
#endif
#ifdef SIGUSR2
    signal(SIGUSR2, sig_usr2);
#endif
    signal(SIGTERM, sig_term);
    signal(SIGINT, sig_int);
#ifndef DEBUG
#if defined(SIGABRT) 
    signal(SIGABRT, sig_abrt);
#endif
#ifdef SIGIOT
    signal(SIGIOT, sig_iot);
#endif
#ifdef SIGHUP
    signal(SIGHUP, sig_hup);
#endif
#ifdef SIGBUS
    signal(SIGBUS, sig_bus);
#endif
    signal(SIGSEGV, sig_segv);
    signal(SIGILL, sig_ill);
#endif        /* DEBUG */
#endif
#ifndef WIN32
#ifdef USE_BSD_SIGNALS
    signal(SIGCHLD, sig_cld);
#else
    signal(SIGCLD, sig_cld);
#endif
#endif
    backend();
    return 0;
}

#ifdef DEBUGMALLOC
char *int_string_copy (const char * const str, char * desc)
#else
char *int_string_copy (const char * const str)
#endif
{
    char *p;
    int len;

    DEBUG_CHECK(!str, "Null string passed to string_copy.\n");
    len = strlen(str);
    if (len > max_string_length) {
  len = max_string_length;
  p = new_string(len, desc);
  (void) strncpy(p, str, len);
  p[len] = '\0';
    } else {
  p = new_string(len, desc);
  (void) strncpy(p, str, len + 1);
    }
    return p;
}

#ifdef DEBUGMALLOC
char *int_string_unlink (const char * str, char * desc)
#else
char *int_string_unlink (const char * str)
#endif
{
    malloc_block_t *mbt, *newmbt;

    mbt = ((malloc_block_t *)str) - 1;
    mbt->ref--;
    
    if (mbt->size == USHRT_MAX) {
  int l = strlen(str + USHRT_MAX) + USHRT_MAX; /* ouch */

  newmbt = (malloc_block_t *)DXALLOC(l + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, desc);
  memcpy((char *)(newmbt + 1), (char *)(mbt + 1), l+1);
  newmbt->size = USHRT_MAX;
  ADD_NEW_STRING(USHRT_MAX, sizeof(malloc_block_t));
    } else {
  newmbt = (malloc_block_t *)DXALLOC(mbt->size + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, desc);
  memcpy((char *)(newmbt + 1), (char *)(mbt + 1), mbt->size+1);
  newmbt->size = mbt->size;
  ADD_NEW_STRING(mbt->size, sizeof(malloc_block_t));
    }
    newmbt->ref = 1;
    CHECK_STRING_STATS;
    
    return (char *)(newmbt + 1);
}

static FILE *debug_message_fp = 0;

void debug_message (const char *fmt, ...)
{
    static char deb_buf[1024];
    static char *deb = deb_buf;
    va_list args;

    if (!debug_message_fp) {
  /*
   * check whether config file specified this option
   */
  if (strlen(DEBUG_LOG_FILE))
      sprintf(deb, "%s/%s", LOG_DIR, DEBUG_LOG_FILE);
  else
      sprintf(deb, "%s/debug.log", LOG_DIR);
  while (*deb == '/')
      deb++;
  debug_message_fp = fopen(deb, "w");
  if (!debug_message_fp) {
      /* darn.  We're in trouble */
      perror(deb);
      abort();
  }
    }

    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vfprintf(debug_message_fp, fmt, args);
    fflush(debug_message_fp);
    va_end(args);
    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vfprintf(stderr, fmt, args);
    fflush(stderr);
    va_end(args);
}

int slow_shut_down_to_do = 0;

char *xalloc (int size)
{
    char *p;
    const char *t;
    static int going_to_exit;

    if (going_to_exit)
  exit(3);
#ifdef DEBUG
    if (size == 0)
  fatal("Tried to allocate 0 bytes.\n");
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

static void CDECL PSIG(sig_cld) 
{
#ifndef WIN32
    int status;
#ifdef USE_BSD_SIGNALS
    while (wait3(&status, WNOHANG, NULL) > 0)
  ;
#else
    wait(&status);
    signal(SIGCLD, sig_cld);
#endif
#endif
}


static void CDECL PSIG(sig_fpe)
{
    signal(SIGFPE, sig_fpe);
}

#ifdef TRAP_CRASHES

/* send this signal when the machine is about to reboot.  The script
   which restarts the MUD should take an exit code of 1 to mean don't
   restart
 */

static void CDECL PSIG(sig_usr1)
{
    push_constant_string("Host machine shutting down");
    push_undefined();
    push_undefined();
    apply_master_ob(APPLY_CRASH, 3);
    debug_message("Received SIGUSR1, calling exit(-1)\n");
    exit(-1);
}

/* Abort evaluation */
static void CDECL PSIG(sig_usr2)
{
    outoftime = 1;
}

/*
 * Actually, doing all this stuff from a signal is probably illegal
 * -Beek
 */
static void CDECL PSIG(sig_term)
{
    fatal("Process terminated");
}

static void CDECL PSIG(sig_int)
{
    fatal("Process interrupted");
}

#ifndef DEBUG
static void CDECL PSIG(sig_segv)
{
    fatal("Segmentation fault");
}

static void CDECL PSIG(sig_bus)
{
    fatal("Bus error");
}

static void CDECL PSIG(sig_ill)
{
    fatal("Illegal instruction");
}

static void CDECL PSIG(sig_hup)
{
    fatal("Hangup!");
}

static void CDECL PSIG(sig_abrt)
{
    fatal("Aborted");
}

static void CDECL PSIG(sig_iot)
{
    fatal("Aborted(IOT)");
}
#endif        /* !DEBUG */

#endif        /* TRAP_CRASHES */
