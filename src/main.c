#include "std.h"
#include "file_incl.h"
#include "lpc_incl.h"
#include "lex.h"
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
#include "compile_file.h"
#include "socket_efuns.h"

port_def_t external_port[5];

static int e_flag = 0;		/* Load empty, without preloads. */
#ifdef DEBUG
int d_flag = 0;			/* Run with debug */
#endif
int t_flag = 0;			/* Disable heart beat and reset */
int comp_flag = 0;		/* Trace compilations */
int max_cost;
int time_to_swap;
int time_to_clean_up;
char *default_fail_message;
int boot_time;
int max_array_size;
int max_buffer_size;
int max_string_length;
char *master_file_name;
static int reserved_size;
char *reserved_area;		/* reserved for MALLOC() */
static char *mud_lib;

svalue_t const0, const1, const0u, const0n;

double consts[NUM_CONSTS];

/* -1 indicates that we have never had a master object.  This is so the
 * simul_efun object can load before the master. */
object_t *master_ob = (object_t *) -1;

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
	argv[0] = alloc_cstring(bing, "main");
    }
    startup_windows(argc, argv);
}				/* main() */


int start_mudos()
{
    int argc = old_argc;
    char **argv = old_argv;

#else
int main P2(int, argc, char **, argv)
{
#endif
    time_t tm;
    int i, new_mudlib = 0, got_defaults = 0;
    int no_ip_demon = 0;
    char *p;
    char version_buf[80];
#if 0
    int dtablesize;
#endif
    error_context_t econ;

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

    fake_prog.program_size = 0;

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

#ifdef RAND
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
	fprintf(stderr, "You must specify the configuration filename as an argument.\n");
	exit(-1);
    }

    printf("Initializing internal tables....\n");
    init_strings();		/* in stralloc.c */
    init_otable();		/* in otable.c */
    init_identifiers();		/* in lex.c */
    init_locals();              /* in compiler.c */

/* disable this for now */
#if 0
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
    get_version(version_buf);
    time(&tm);
    debug_message("----------------------------------------------------------------------------\n%s (%s) starting up on %s - %s\n\n", MUD_NAME, version_buf, ARCH, ctime(&tm));

#ifdef BINARIES
    init_binaries(argc, argv);
#endif
#ifdef LPC_TO_C
    init_lpc_to_c();
#endif
    add_predefines();

#ifndef NO_IP_DEMON
    if (!no_ip_demon && ADDR_SERVER_IP)
	init_addr_server(ADDR_SERVER_IP, ADDR_SERVER_PORT);
#endif				/* NO_IP_DEMON */

    eval_cost = max_cost;	/* needed for create() functions */

    save_context(&econ);
    if (SETJMP(econ.context)) {
	debug_message("The simul_efun (/%s) and master (/%s) objects must be loadable.\n", 
		simul_efun_file_name, master_file_name);
	exit(-1);
    } else {
	set_simul_efun(SIMUL_EFUN);
	if (!load_object(master_file_name, 0))
	    error("Master object doesn't exist!\n");
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
		    debug_message("Error while calling master::flag(\"%s\"), aborting ...", argv[i] + 2);
		    exit(-1);
		}
		push_constant_string(argv[i] + 2);
		(void) apply_master_ob(APPLY_FLAG, 1);
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
#ifdef DEBUG
                d_flag++;
#else
                debug_message("Driver must be compiled with DEBUG on to use -d.\n");
#endif
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
    if (strlen(DEFAULT_FAIL_MESSAGE))
	default_fail_message = DEFAULT_FAIL_MESSAGE;
    else
	default_fail_message = "What?";
#ifdef PACKAGE_MUDLIB_STATS
    restore_stat_files();
#endif
#ifdef PACKAGE_SOCKETS
    init_sockets();		/* initialize efun sockets           */
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
char *int_string_unlink P2(char *, str, char *, desc)
#else
char *int_string_unlink P1(char *, str)
#endif
{
    malloc_block_t *mbt, *newmbt;

    mbt = ((malloc_block_t *)str) - 1;
    mbt->ref--;
    
    if (mbt->size == MAXSHORT) {
	int l = strlen(str + MAXSHORT) + MAXSHORT; /* ouch */

	newmbt = (malloc_block_t *)DXALLOC(l + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, desc);
	memcpy((char *)(newmbt + 1), (char *)(mbt + 1), l+1);
	newmbt->size = MAXSHORT;
	ADD_NEW_STRING(l, sizeof(malloc_block_t));
    } else {
	newmbt = (malloc_block_t *)DXALLOC(mbt->size + sizeof(malloc_block_t) + 1, TAG_MALLOC_STRING, desc);
	memcpy((char *)(newmbt + 1), (char *)(mbt + 1), mbt->size+1);
	newmbt->size = mbt->size;
	ADD_NEW_STRING(mbt->size, sizeof(malloc_block_t));
    }
    newmbt->ref = 1;

    return (char *)(newmbt + 1);
}

void debug_message P1V(char *, fmt)
{
    static int append = 0;
    static char deb_buf[100];
    static char *deb = deb_buf;
    va_list args;
    FILE *fp = NULL;
    V_DCL(char *fmt);

    if (!append) {
	/*
	 * check whether config file specified this option
	 */
	if (strlen(DEBUG_LOG_FILE))
	    sprintf(deb, "%s/%s", LOG_DIR, DEBUG_LOG_FILE);
	else
	    sprintf(deb, "%s/debug.log", LOG_DIR);
	while (*deb == '/')
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
	/* darn.  We're in trouble */
	perror(deb);
	abort();
    }
    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vfprintf(fp, fmt, args);
    fflush(fp);
    vfprintf(stderr, fmt, args);
    fflush(stderr);
    va_end(args);

    /*
     * don't close stdout
     */
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
	fatal("Totally out of MEMORY.\n");
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
    debug_message("Received SIGUSR1, calling exit(-1)\n");
#if defined(OS2) && !defined(COMMAND_LINE)
    message_box_string("Host machine is shutting down.\n");
    FileExit();
    return;
#else
    exit(-1);
#endif
}

/*
 * Actually, doing all this stuff from a signal is probably illegal
 * -Beek
 */
#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_term P1(int, sig)
#else
static void sig_term()
#endif
{
    fatal("Process terminated");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_int P1(int, sig)
#else
static void sig_int()
#endif
{
    fatal("Process interrupted");
}

#ifndef DEBUG

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_segv P1(int, sig)

#else
static void sig_segv()
#endif
{
    fatal("Segmentation fault");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_bus P1(int, sig)
#else
static void sig_bus()
#endif
{
    fatal("Bus error");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_ill P1(int, sig)
#else
static void sig_ill()
#endif
{
    fatal("Illegal instruction");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_hup P1(int, sig)
#else
static void sig_hup()
#endif
{
    fatal("Hangup!");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_abrt P1(int, sig)
#else
static void sig_abrt()
#endif
{
    fatal("Aborted");
}

#ifdef SIGNAL_FUNC_TAKES_INT
static void sig_iot P1(int, sig)
#else
static void sig_iot()
#endif
{
    fatal("Aborted(IOT)");
}

#endif				/* !DEBUG */

#endif				/* TRAP_CRASHES */
