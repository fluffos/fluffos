#include "std.h"
#include "file_incl.h"
#include "config.h"
#include "rc.h"
#include "applies.h"
#include "lex.h"
#include "program.h"
#include "backend.h"
#include "simul_efun.h"
#include "simulate.h"
#include "binaries.h"
#include "main.h"
#include "interpret.h"
#include "stralloc.h"
#include "otable.h"
#include "comm.h"
#include "compiler.h"
#include "port.h"
#include "md.h"

static int e_flag = 0;		/* Load empty, without castles. */
#ifdef DEBUG
int d_flag = 0;			/* Run with debug */
#endif
int t_flag = 0;			/* Disable heart beat and reset */
int comp_flag = 0;		/* Trace compilations */
int max_cost;
int time_to_swap;
int time_to_clean_up;
char *default_fail_message;
int port_number;
int boot_time;
int max_array_size;
int max_buffer_size;
int max_string_length;
char *master_file_name;
static int reserved_size;
char *reserved_area;		/* reserved for MALLOC() */
static char *mud_lib;

struct svalue const0, const1, const0u, const0n;

double consts[NUM_CONSTS];

/* -1 indicates that we have never had a master object.  This is so the
 * simul_efun object can load before the master. */
struct object *master_ob = (struct object *) -1;

#ifndef NO_IP_DEMON
void init_addr_server();
#endif				/* NO_IP_DEMON */

#ifdef TRAP_CRASHES
#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_usr1 PROT((int));
static void sig_term PROT((int));
static void sig_int PROT((int));

#ifndef DEBUG
static void sig_hup PROT((int)),
        sig_abrt PROT((int)),
        sig_segv PROT((int)),
        sig_ill PROT((int)),
        sig_bus PROT((int)),
        sig_iot PROT((int));

#endif				/* DEBUG */
#else
static void sig_usr1 PROT((void));
static void sig_term PROT((void));
static void sig_int PROT((void));

#ifndef DEBUG
static void sig_hup PROT((void)),
        sig_abrt PROT((void)),
        sig_segv PROT((void)),
        sig_ill PROT((void)),
        sig_bus PROT((void)),
        sig_iot PROT((void));

#endif				/* DEBUG */
#endif				/* SIGNAL_FUNC_TAKES_INT */
#endif				/* TRAP_CRASHES */

#ifdef DEBUG_MACRO
/* used by debug.h: please leave this in here -- Tru (you can change its
   value if you like).
*/
int debug_level = 32768;
#endif				/* DEBUG_MACRO */

#ifdef OS2
int old_argc;
char **old_argv;

int main(argc, argv)
    int argc;
    char **argv;
{
    old_argc = argc;
    old_argv = argv;
    if (argv[0][1] != ':' && argv[0][0] != '\\') {
/* Relative thingy... */
	char bing[80];

	getcwd(bing, 80);
	strcat(bing, "\\");
	strcat(bing, argv[0]);
	argv[0] = string_copy(bing, "main");
    }
    startup_windows(argc, argv);
}				/* main() */


int start_mudos()
{
    int argc = old_argc;
    char **argv = old_argv;

#else
int main(argc, argv)
    int argc;
    char **argv;
{
#endif
    int i, new_mudlib = 0, got_defaults = 0;
    int no_ip_demon = 0;
    char *p;
    char version_buf[80];
    int dtablesize;

#if !defined(LATTICE) && !defined(OLD_ULTRIX) && !defined(sequent) && \
    !defined(sgi)
    void tzset();
#endif
    struct lpc_predef_s predefs;

#if !defined(__SASC) && (defined(AMITCP) || defined(AS225))
    amiga_sockinit();
    atexit(amiga_sockexit);
#endif
#ifdef WRAPPEDMALLOC
    wrappedmalloc_init();
#endif				/* WRAPPEDMALLOC */
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
    printf("%s (%s)\n", version_buf, ARCH);

    const0.type = T_NUMBER;
    const0.u.number = 0;
    const1.type = T_NUMBER;
    const1.u.number = 1;

    /* const0u used by undefinedp() */
    const0u.type = T_NUMBER;
    const0u.subtype = T_UNDEFINED;
    const0u.u.number = 0;

    /* const0n used by nullp() */
    const0n.type = T_NUMBER;
    const0n.subtype = T_NULLVALUE;
    const0n.u.number = 0;

#ifdef NEW_FUNCTIONS
    fake_prog.p.i.program_size = 0;
#endif

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
    if (LIVING_HASH_SIZE != 4 && LIVING_HASH_SIZE != 16 &&
	LIVING_HASH_SIZE != 64 && LIVING_HASH_SIZE != 256 &&
	LIVING_HASH_SIZE != 1024 && LIVING_HASH_SIZE != 4096) {
	fprintf(stderr, "LIVING_HASH_SIZE in options.h must be one of 4, 16, 64, 256, 1024, 4096, ...\n");
	exit(-1);
    }

    /*
     * Check that memmove() works
     */
    sprintf(version_buf, "0123456789ABCDEF");
    memmove(&version_buf[1], &version_buf[4], 13);
    if (strcmp(version_buf, "0456789ABCDEF")) {
	fprintf(stderr, "Bad definition of memmove() for your system.\n");
	exit(-1);
    }
    memmove(&version_buf[8], &version_buf[6], 9);
    if (strcmp(version_buf, "0456789A9ABCDEF")) {
	fprintf(stderr, "Bad definition of memmove() for your system.\n");
	exit(-1);
    }
#ifdef OS2
    srand(get_current_time());
#else
#ifdef DRAND48
    srand48(get_current_time());
#else
#ifdef RANDOM
    srandom(get_current_time());
#else
    fprintf(stderr, "Warning: no random number generator specified!\n");
#endif
#endif				/* DRAND48 */
#endif				/* OS2 */
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
    if (!got_defaults) {
	fprintf(stderr,
	   "You must specify the configuration filename as an argument.\n");
	exit(-1);
    }
    init_strings();		/* in stralloc.c */
    init_otable();		/* in otable.c */
    init_identifiers();		/* in lex.c */
    init_locals();              /* in compiler.c */
    /*
     * We estimate that we will need MAX_USERS + MAX_EFUN_SOCKS + 10 file
     * descriptors if the maximum number of users were to log in and all LPC
     * sockets were in use.  This is a pretty close estimate.
     */
#ifndef LATTICE
    dtablesize = MAX_USERS + MAX_EFUN_SOCKS + 10;
#else
    /*
     * Amiga sockets separate from file descriptors
     */
    dtablesize = MAX_USERS + MAX_EFUN_SOCKS;
#endif

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
     * allocate the number we estimated above.  There are system imposed
     * limits on file descriptors, so we may not get as many as we asked for.
     * Check to make sure we get enough.
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
    master_file_name = (char *) MASTER_FILE;
    /* fix the filename */
    while (*master_file_name == '/') master_file_name++;
    p = master_file_name;
    while (*p++);
    if (p[-2]=='c' && p[-3]=='.')
	p[-3]=0;
    mud_lib = (char *) MUD_LIB;
    set_inc_list(INCLUDE_DIRS);
    if (reserved_size > 0)
	reserved_area = (char *) DMALLOC(reserved_size, TAG_RESERVED, "main.c: reserved_area");
    for (i = 0; i < sizeof consts / sizeof consts[0]; i++)
	consts[i] = exp(-i / 900.0);
    init_num_args();
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
	    if (argv[i][2]) {	/* Amylaar : allow flags to be passed down to
				 * the LPC preprocessor */
		struct lpc_predef_s *tmp;

		tmp = &predefs;
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
#endif				/* YYDEBUG */
	case 'm':
	    mud_lib = string_copy(argv[i] + 2, "mudlib dir");
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
#ifdef BINARIES
    init_binaries(argc, argv);
#endif
#ifdef LPC_TO_C
    init_lpc_to_c();
#endif
    add_predefines();

#ifndef NO_IP_DEMON
    if (!no_ip_demon)
	init_addr_server(ADDR_SERVER_IP, ADDR_SERVER_PORT);
#endif				/* NO_IP_DEMON */

    eval_cost = max_cost;	/* needed for create() functions */

    if (SETJMP(error_recovery_context)) {
	fprintf(stderr, "The simul_efun (/%s) and master (/%s) objects must be loadable.\n", 
		simul_efun_file_name, master_file_name);
	exit(-1);
    } else {
	error_recovery_context_exists = NORMAL_ERROR_CONTEXT;
	set_simul_efun(SIMUL_EFUN);
	(void) load_object(master_file_name, 0);
    }
    error_recovery_context_exists = NULL_ERROR_CONTEXT;

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
		push_constant_string(argv[i] + 2);
		(void) apply_master_ob(APPLY_FLAG, 1);
		if (MudOS_is_being_shut_down) {
		    fprintf(stderr, "Shutdown by master object.\n");
		    exit(0);
		}
		continue;
	    case 'e':
		e_flag++;
		continue;
	    case 'p':
		port_number = atoi(argv[i] + 2);
		continue;
            case 'd':
#ifdef DEBUG
                d_flag++;
#else
                fprintf(stderr, "Driver must be compiled with DEBUG on to use -d.\n");
#endif
	    case 'c':
		comp_flag++;
		continue;
	    case 't':
		t_flag++;
		continue;
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
#ifndef NO_MUDLIB_STATS
    restore_stat_files();
#endif
    preload_objects(e_flag);
#ifdef TRAP_CRASHES
#ifdef SIGUSR1
    signal(SIGUSR1, sig_usr1);
#endif
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
#endif				/* DEBUG */
#endif
    backend();
    return 0;
}

#ifdef DEBUGMALLOC
char *int_string_copy P2(char *, str, char *, desc)
#else
char *int_string_copy P1(char *, str)
#endif
{
    char *p;
    int len;

    DEBUG_CHECK(!str, "Null string passed to string_copy.\n");
    len = strlen(str);
    if (len > max_string_length) {
	len = max_string_length;
	p = DXALLOC(len + 1, TAG_STRING, desc);
	(void) strncpy(p, str, len);
	p[len] = '\0';
    } else {
	p = DXALLOC(len + 1, TAG_STRING, desc);
	(void) strncpy(p, str, len + 1);
    }
    return p;
}

void debug_message PVARGS(va_alist)
{
    static int append = 0;
    static char deb_buf[100];
    static char *deb = deb_buf;
    va_list args;
    FILE *fp = NULL;
    char *fmt;

    if (!append) {
	/*
	 * check whether config file specified this option
	 */
	if (strlen(DEBUG_LOG_FILE))
	    sprintf(deb, "%s/%s", LOG_DIR, DEBUG_LOG_FILE);
	else
	    sprintf(deb, "%s/debug.log", LOG_DIR);
	if (*deb == '/')
	    deb++;
    }
    fp = fopen(deb, append ? "a" : "w");

    /*
     * re-use stdout's file descriptor if system or process runs out
     * 
     * OS/2 doesn't have ENFILE.
     */
    if (!fp && (errno == EMFILE 
#ifdef ENFILE
		|| errno == ENFILE
#endif
		)) {
	fp = freopen(deb, append ? "a" : "w", stdout);
	append = 2;
    }
    if (!fp) {
	perror(deb);
	abort();
    }
#ifdef HAS_STDARG_H
    va_start(args, va_alist);
    fmt = va_alist;
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif
    vfprintf(fp, fmt, args);
    va_end(args);

    /*
     * don't close stdout
     */
    fflush(fp);
    if (append != 2)
	(void) fclose(fp);

    /*
     * append to debug.log next time thru
     */
    if (!append)
	append = 1;
}

int slow_shut_down_to_do = 0;

char *xalloc P1(int, size)
{
    char *p;
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
	    p = "Temporarily out of MEMORY. Freeing reserve.\n";
	    write(1, p, strlen(p));
	    reserved_area = 0;
	    slow_shut_down_to_do = 6;
	    return xalloc(size);/* Try again */
	}
	going_to_exit = 1;
	p = "Totally out of MEMORY.\n";
	write(1, p, strlen(p));
	(void) dump_trace(0);
	exit(2);
    }
    return p;
}

#ifdef TRAP_CRASHES

/* send this signal when the machine is about to reboot.  The script
   which restarts the MUD should take an exit code of 1 to mean don't
   restart
 */

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_usr1 P1(int, sig)
#else
static void sig_usr1()
#endif
{
    push_string("Host machine shutting down", STRING_CONSTANT);
    push_undefined();
    push_undefined();
    apply_master_ob(APPLY_CRASH, 3);
    fprintf(stderr, "Received SIGUSR1, calling exit(-1)\n");
#if defined(OS2) && !defined(COMMAND_LINE)
    message_box_string("Host machine is shutting down.\n");
    FileExit();
    return;
#else
    exit(-1);
#endif
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_term P1(int, sig)
#else
static void sig_term()
#endif
{
    crash_MudOS("Process terminated");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_int P1(int, sig)
#else
static void sig_int()
#endif
{
    crash_MudOS("Process interrupted");
}

#ifndef DEBUG

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_segv P1(int, sig)

#else
static void sig_segv()
#endif
{
    crash_MudOS("Segmentation fault");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_bus P1(int, sig)
#else
static void sig_bus()
#endif
{
    crash_MudOS("Bus error");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_ill P1(int, sig)
#else
static void sig_ill()
#endif
{
    crash_MudOS("Illegal instruction");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_hup P1(int, sig)
#else
static void sig_hup()
#endif
{
    crash_MudOS("Hangup!");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_abrt P1(int, sig)
#else
static void sig_abrt()
#endif
{
    crash_MudOS("Aborted");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_iot P1(int, sig)
#else
static void sig_iot()
#endif
{
    crash_MudOS("Aborted(IOT)");
}

#endif				/* !DEBUG */

#endif				/* TRAP_CRASHES */

static int crash_condition = 0;

void crash_MudOS P1(char *, str)
{
#ifdef DROP_CORE
    char buf[SMALL_STRING_SIZE];

#endif

    /*
     * Something really, really bad just happened.  Nothing we can do about
     * it, so tell the master object to clean up, and exit.
     */
    if (crash_condition) {
	fprintf(stderr, "Too many simultaneous fatal errors!\n");
	fprintf(stderr, "Exiting before crash could be called successfully.\n");
	fprintf(stderr, "Dying: %s\n", str);
#if defined(OS2) && !defined(COMMAND_LINE)
	message_box_string("Too many simultaneous errors.\n");
	FileExit();
	return;
#else
	exit(-3);
#endif
    } else {
	/*
	 * restore default action for SIGILL/SIGABRT so we don't loop when
	 * crash_MudOS calls abort() at the end of this function.
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

#if defined(OS2) && !defined(COMMAND_LINE)
	message_box_string(str);
#endif

	fprintf(stderr, "Shutting down: %s\n", str);
	crash_condition++;
#ifndef NO_MUDLIB_STATS
	save_stat_files();
#endif
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
	apply_master_ob(APPLY_CRASH, 3);
    }
#if defined(OS2) && !defined(COMMAND_LINE)
    FileExit();
    return;
#else
#ifdef DROP_CORE
    strncpy(buf, mud_lib, SMALL_STRING_SIZE - 6);
    buf[SMALL_STRING_SIZE - 6] = '\0';
    strcat(buf, "/cores");
    if (chdir(buf) == -1) {
	chdir(mud_lib);
    }
    abort();
#else
    exit(-2);
#endif
#endif
}
