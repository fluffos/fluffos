#include "config.h"
#include <sys/types.h>
#ifndef LATTICE
#include <sys/socket.h>
#endif
#include <stdio.h>
#if defined(__386BSD__) || defined(SunOS_5)
#include <stdlib.h>
#include <unistd.h>
#endif
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#if defined(sun)
#include <alloca.h>
#endif
#ifndef LATTICE
#include <varargs.h>
#else
#include "amiga.h"
#endif
#include "lint.h"
#include "interpret.h"
#include "object.h"
#include "lex.h"
#include "sent.h"
#include "md.h"
#include "arch.h" /* after config.h */

extern char *prog;

extern int current_line;

int d_flag = 0;	/* Run with debug */
int t_flag = 0;	/* Disable heart beat and reset */
int e_flag = 0;	/* Load empty, without castles. */
int comp_flag = 0; /* Trace compilations */
int max_cost;
int time_to_swap;
int time_to_clean_up;
char *default_fail_message;

#ifdef YYDEBUG
extern int yydebug;
#endif

int port_number;
int boot_time;
int max_array_size;
int max_buffer_size;
int max_string_length;
char *master_file_name;
int reserved_size;
char *reserved_area;	/* reserved for MALLOC() */
char *mud_lib;

struct svalue const0, const1, const0u, const0n;

double consts[NUM_CONSTS];

extern jmp_buf error_recovery_context;
extern int error_recovery_context_exists;

extern struct object *master_ob;

#ifndef NO_IP_DEMON
void init_addr_server();
#endif /* NO_IP_DEMON */

#ifdef TRAP_CRASHES
static void sig_usr1();
static void sig_term();
static void sig_int();
#ifndef DEBUG
static void sig_int(), sig_hup(),
            sig_abrt(), sig_segv(), sig_ill(), sig_bus(), sig_iot();
#endif /* DEBUG */
#endif

#ifdef DEBUG_MACRO
/* used by debug.h: please leave this in here -- Tru (you can change its
   value if you like).
*/
int debug_level = 32768;
#endif /* DEBUG_MACRO */

int main(argc, argv)
     int argc;
     char **argv;
{
  extern int MudOS_is_being_shut_down;
  extern int current_time;
  int i, new_mudlib = 0, got_defaults = 0;
  int no_ip_demon = 0;
  char *p;
  char version_buf[80];
  struct svalue *ret;
  void init_strings(), init_otable();
  int dtablesize;
#ifdef SAVE_BINARIES
  void init_binaries();
#endif
#if !defined(LATTICE) && !defined(OLD_ULTRIX) && !defined(sequent)
  void tzset();
#endif
  struct lpc_predef_s predefs;
#ifdef WRAPPEDMALLOC
  wrappedmalloc_init();
#endif /* WRAPPEDMALLOC */
#ifdef DEBUGMALLOC
  MDinit();
#endif

#if (defined(PROFILING) && !defined(PROFILE_ON) && defined(HAS_MONCONTROL))
  moncontrol(0);
#endif
#if !defined(OLD_ULTRIX) && !defined(LATTICE) && !defined(sequent)
  tzset();
#endif
  boot_time = get_current_time();
  get_version(version_buf);
  printf("%s (%s)\n",version_buf, ARCH);

  const0.type = T_NUMBER; const0.u.number = 0;
  const1.type = T_NUMBER; const1.u.number = 1;
  /* const0u used by undefinedp() */
  const0u.type = T_NUMBER; const0u.subtype = T_UNDEFINED;
  const0u.u.number = 0;
  /* const0n used by nullp() */
  const0n.type = T_NUMBER; const0n.subtype = T_NULLVALUE;
  const0n.u.number = 0;
  /*
   * Check that the definition of EXTRACT_UCHAR() is correct.
   */
  p = (char *)&i;
  *p = -10;
  if(EXTRACT_UCHAR(p) != 0x100 - 10){
    fprintf(stderr, "Bad definition of EXTRACT_UCHAR() in config.h.\n");
    exit(-1);
  }
#ifdef DRAND48
  srand48(get_current_time());
#else
#ifdef RANDOM
  srandom(get_current_time());
#else
  fprintf(stderr,"Warning: no random number generator specified!\n");
#endif
#endif /* DRAND48 */
  current_time = get_current_time();;
  /*
   * Initialize the microsecond clock.
   */
  init_usec_clock();

  /* read in the configuration file */

  got_defaults = 0;
  for(i=1; (i < argc) && !got_defaults; i++){
    if(argv[i][0] != '-'){
      set_defaults(argv[i]);
      got_defaults = 1;
    }
  }
  if(!got_defaults){
    fprintf(stderr,
	    "You must specify the configuration filename as an argument.\n");
    exit(-1);
  }
  init_strings(); /* in stralloc.c */
  init_otable();  /* in otable.c */

  /*
   * We estimate that we will need MAX_USERS + MAX_EFUN_SOCKS + 10 file
   * descriptors if the maximum number of users were to log in and all LPC
   * sockets were in use.  This is a pretty close estimate.
   */
  dtablesize = MAX_USERS + MAX_EFUN_SOCKS + 10;

  /*
   * If our estimate is larger than FD_SETSIZE, then we need more file
   * descriptors than the operating system can handle.  This is a problem
   * that can be resolved by decreasing MAX_USERS, MAX_EFUN_SOCKS, or both.
   */
  if (dtablesize > FD_SETSIZE) {
    fprintf(stderr,
	    "Warning: File descriptor requirements exceed system capacity!\n");
    fprintf(stderr,
	    "         Configuration exceeds system capacity by %d descriptor(s).\n",
	    FD_SETSIZE - dtablesize);
  }
#ifdef HAS_SETDTABLESIZE
  /*
    * If the operating system supports setdtablesize() then we can request
    * the number of file descriptors we really need.  First check to see if
    * wee already have enough.  If so dont bother the OS. If not, attempt to
    * allocate the number we estimated above.  There are system imposed limits
    * on file descriptors, so we may not get as many as we asked for.  Check to
    * make sure we get enough.
    */
  if (getdtablesize() < dtablesize)
    if (setdtablesize(dtablesize) < dtablesize) {
      fprintf(stderr,
        "Warning: Could not allocate enough file descriptors!\n");
      fprintf(stderr,
        "         setdtablesize() could not allocate %d descriptor(s).\n",
        getdtablesize() - dtablesize);
    }

  /*
   * Just be polite and tell the administrator how many he has.
   */
  fprintf(stderr, "%d file descriptors were allocated, (%d were requested).\n",
    getdtablesize(), dtablesize);
#endif

  time_to_clean_up = TIME_TO_CLEAN_UP;
  port_number = PORTNO;
  time_to_swap = TIME_TO_SWAP;
  max_cost = MAX_COST;
  reserved_size = RESERVED_SIZE;
  max_array_size = MAX_ARRAY_SIZE;
  max_buffer_size = MAX_BUFFER_SIZE;
  max_string_length = MAX_STRING_LENGTH;
  master_file_name = (char *)MASTER_FILE;
  mud_lib = (char *)MUD_LIB;
  set_inc_list(INCLUDE_DIRS);

  if(reserved_size > 0)
    reserved_area = (char *)DMALLOC(reserved_size,69,"main.c: reserved_area");
  for(i=0; i < sizeof consts / sizeof consts[0]; i++)
    consts[i] = exp(- i / 900.0);
  init_num_args();
  reset_machine(1);

  /*
   * The flags are parsed twice !
   * The first time, we only search for the -m flag, which specifies
   * another mudlib, and the D-flags, so that they will be available
   * when compiling master.c.
   */
  for(i=1; i < argc; i++){
    if(argv[i][0] != '-')
      continue;
    switch(argv[i][1]){
    case 'D':
      if (argv[i][2]) { /* Amylaar : allow flags to be passed down to
			   the LPC preprocessor */
	struct lpc_predef_s *tmp;
		
	tmp = &predefs;
	tmp->flag = argv[i]+2;
	tmp->next = lpc_predefs;
	lpc_predefs = tmp;
	continue;
      }
      fprintf(stderr, "Illegal flag syntax: %s\n", argv[i]);
      exit(-1);
    case 'N':
      no_ip_demon++; continue;
    case 'm':
      mud_lib = string_copy(argv[i]+2);
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

#ifdef SAVE_BINARIES
  init_binaries(argc, argv);
#endif

#ifndef NO_IP_DEMON
  if(!no_ip_demon)
    init_addr_server(ADDR_SERVER_IP,ADDR_SERVER_PORT);
#endif /* NO_IP_DEMON */

  set_simul_efun(SIMUL_EFUN);

  if(SETJMP(error_recovery_context)){
    clear_state();
    add_message("Anomaly in the fabric of world space.\n");
  } else {
    error_recovery_context_exists = 1;
    master_ob = load_object(master_file_name,0);
  }
  error_recovery_context_exists = 0;
  if (master_ob == 0) {
    fprintf(stderr, "The file master file must be loadable.\n");
    exit(-1);
  }
      /*
	* Make sure master_ob is never made a dangling pointer.
	* Look at apply_master_ob() for more details.
	*/
  add_ref(master_ob, "main");
  ret = apply_master_ob("get_root_uid", 0);
  if (ret == 0 || ret->type != T_STRING) {
    fprintf(stderr, "get_root_uid() in the master file does not work\n");
    exit(-1);
  }
  master_ob->uid = set_root_uid (ret->u.string);
  master_ob->euid = master_ob->uid;
  set_root_author (ret->u.string);
  master_ob->flags |= O_MASTER;
  ret = apply_master_ob("get_bb_uid", 0);
  if (ret == 0 || ret->type != T_STRING) {
    fprintf(stderr, "get_bb_uid() in the master file does not work\n");
    exit(-1);
  }
  set_backbone_uid(ret->u.string);
  set_backbone_domain(ret->u.string);
  for (i=1; i < argc; i++) {
    if (argv[i][0] != '-') {
	  continue;
	} else {
	  /*
	   * Look at flags. -m and -o has already been tested.
	   */
	  switch(argv[i][1]) {
	  case 'f':
	    push_constant_string(argv[i]+2);
	    (void)apply_master_ob("flag", 1);
	    if (MudOS_is_being_shut_down) {
	      fprintf(stderr, "Shutdown by master object.\n");
	      exit(0);
	    }
	    continue;
	  case 'e':
	    e_flag++; continue;
	  case 'D':
	    continue;
	  case 'N':
	    continue;
	  case 'p':
	    port_number = atoi(argv[i]+2);
	    continue;
	  case 'm':
	    continue;
	  case 'd':
	    d_flag++; continue;
	  case 'c':
	    comp_flag++; continue;
	  case 't':
	    t_flag++; continue;
#ifdef YYDEBUG
	  case 'y':
	    yydebug = 1; continue;
#endif /* YYDEBUG */
	  default:
	    fprintf(stderr, "Unknown flag: %s\n", argv[i]);
	    exit(-1);
	  }
	}
      }
    if (MudOS_is_being_shut_down)
	exit(1);
  if (strlen(DEFAULT_FAIL_MESSAGE))
     default_fail_message = DEFAULT_FAIL_MESSAGE;
  else
     default_fail_message = "What?";
  restore_stat_files();
  preload_objects(e_flag);
#ifdef TRAP_CRASHES
    signal(SIGUSR1, sig_usr1);
    signal(SIGTERM, sig_term);
    signal(SIGINT, sig_int);
#ifndef DEBUG
#if defined(SIGABRT) && !defined(LATTICE)
    signal(SIGABRT, sig_abrt);
#endif
#ifdef SIGIOT
    signal(SIGIOT, sig_iot);
#endif
    signal(SIGHUP, sig_hup);
#ifdef SIGBUS
    signal(SIGBUS, sig_bus);
#endif
#ifndef LATTICE
    signal(SIGSEGV, sig_segv);
    signal(SIGILL, sig_ill);
#endif
#endif /* DEBUG */
#endif
    backend();
    return 0;
}

char *string_copy(str)
     char *str;
{
  char *p;
  int len;

  len = strlen(str);
  if (len > max_string_length) {
     len = max_string_length;
  }
  p = DXALLOC(len + 1, 70, "string_copy");
  (void)strncpy(p, str, len);
  p[len] = '\0'; /* strncpy doesn't put on \0 if 'from' too long */
  return p;
}

void debug_message(va_alist)
  va_dcl
{
    static FILE *fp = NULL;
    char deb_buf[100], *deb = deb_buf;
  va_list args;
  char *fmt;

    if (fp == NULL) 
      {
	sprintf(deb,"%s/debug.log",LOG_DIR);
	if (deb[0] == '/') deb++;
	fp = fopen(deb, "w");
	if (fp == NULL) 
	  {
	    perror(deb);
	    abort();
	  }
      }

  va_start(args);
  fmt = va_arg(args, char *);
  vfprintf(fp, fmt, args);
  va_end(args);
    (void)fflush(fp);
}

void debug_message_svalue(v)
    struct svalue *v;
{
    if (v == 0) {
	debug_message("<NULL>");
	return;
    }
    switch(v->type) {
    case T_NUMBER:
	debug_message("%d", v->u.number);
	return;
    case T_STRING:
	debug_message("\"%s\"", v->u.string);
	return;
    case T_OBJECT:
	debug_message("OBJ(%s)", v->u.ob->name);
	return;
    case T_LVALUE:
	debug_message("Pointer to ");
	debug_message_svalue(v->u.lvalue);
	return;
    default:
	debug_message("<INVALID>\n");
	return;
    }
}

int slow_shut_down_to_do = 0;

char *xalloc(size)
    int size;
{
    char *p;
    static int going_to_exit;

    if (going_to_exit)
	exit(3);
    if (size == 0)
	fatal("Tried to allocate 0 bytes.\n");
    p = (char *)DMALLOC(size, 71, "main.c: xalloc");
    if (p == 0) {
	if (reserved_area) {
	    FREE(reserved_area);
	    p = "Temporarily out of MEMORY. Freeing reserve.\n";
	    write(1, p, strlen(p));
	    reserved_area = 0;
	    slow_shut_down_to_do = 6;
	    return xalloc(size);	/* Try again */
	}
	going_to_exit = 1;
	p = "Totally out of MEMORY.\n";
	write(1, p, strlen(p));
	(void)dump_trace(0);
	exit(2);
    }
    return p;
}

#ifdef TRAP_CRASHES

/* send this signal when the machine is about to reboot.  The script
   which restarts the MUD should take an exit code of 1 to mean don't
   restart
 */

static void
sig_usr1()
{
	push_string("Host machine shutting down", STRING_CONSTANT);
	push_undefined();
	push_undefined();
	current_object = master_ob;
	apply_master_ob("crash", 3);
	fprintf(stderr,"Received SIGUSR1, calling exit(-1)\n");
	exit(-1);
}

static void sig_term() {
    crash_MudOS("Process terminated");
}

static void sig_int() {
    crash_MudOS("Process interrupted");
}

#ifndef DEBUG

static void sig_segv() {
    crash_MudOS("Segmentation fault");
}

static void sig_bus() {
    crash_MudOS("Bus error");
}

static void sig_ill() {
    crash_MudOS("Illegal instruction");
}

static void sig_hup() {
    crash_MudOS("Hangup!");
}

static void sig_abrt() {
    crash_MudOS("Aborted");
}

static void sig_iot() {
    crash_MudOS("Aborted(IOT)");
}

#endif /* !DEBUG */

#endif /* TRAP_CRASHES */

static int crash_condition = 0; 

void crash_MudOS(str)
     char *str;
{
#ifdef DROP_CORE
   char buf[SMALL_STRING_SIZE];
#endif

  /* Something really, really bad just happened.  Nothing we can do about it,
     so tell the master object to clean up, and exit. */
  if(crash_condition){
    fprintf(stderr,"Too many simultaneous fatal errors!\n");
    fprintf(stderr,"Exiting before crash could be called successfully.\n");
    fprintf(stderr,"Dying: %s\n", str);
    exit(-3);
  }
  else {
	/* restore default action for SIGILL/SIGABRT so we don't loop when
	   crash_MudOS calls abort() at the end of this function.
	*/
#ifdef SIGABRT
	signal(SIGABRT, SIG_DFL);
#endif
#ifdef SIGILL
	signal(SIGILL, SIG_DFL);
#endif
#ifdef SIGIOT
	signal(SIGIOT, SIG_DFL);
#endif

    fprintf(stderr, "Shutting down: %s\n", str);
    crash_condition++;
#if 0   /* what is this?  -bobf 9/21/93 */
{
	char tmp[10];
	gets(tmp);
}
#endif /* 0 */
    save_stat_files();
    push_string(str, STRING_CONSTANT);
	if (command_giver) {
		push_object(command_giver);
	} else {
		push_undefined();
	}
	if (current_object) {
		push_object(current_object);
	} else {
		push_undefined();
	}
    /* 
     * set the current object to be the master object.
     * since we're crashing, we don't need to save the actual current
     * object. This is so crash() can be static.
     */
    current_object = master_ob;
    apply_master_ob("crash", 3);
  }
#ifdef DROP_CORE
  strncpy(buf,mud_lib,SMALL_STRING_SIZE);
  strcat(buf,"/cores");
  if (chdir(buf) == -1) {
     chdir(mud_lib);
  }
     abort();
#else
     exit(-2);
#endif
}
