/*
 * rc.c
 * description: runtime configuration for lpmud
 * author: erikkay@mit.edu
 * last modified: July 4, 1994 [robo]
 * Mar 26, 1995: edited heavily by Beek
 */

#include "std.h"
#include "include/runtime_config.h"
#include "lpc_incl.h"
#include "main.h"

#define MAX_LINE_LENGTH 120

char *config_str[NUM_CONFIG_STRS];
int config_int[NUM_CONFIG_INTS];

static char *buff;
static int buff_size;

static void read_config_file PROT((FILE *));
static int scan_config_line PROT((char *, void *, int));
static void config_init PROT((void)); /* don't ask */

static void config_init() {
    int i;

    for (i = 0; i < NUM_CONFIG_INTS; i++) {
	config_int[i] = 0;
    }
    for (i = 0; i < NUM_CONFIG_STRS; i++) {
	config_str[i] = "";
    }
    
}

static void read_config_file P1(FILE *, file)
{
    char str[MAX_LINE_LENGTH * 4];
    int size = 2, len, tmp;
    char *p;

    buff_size = MAX_LINE_LENGTH * (NUM_CONFIG_INTS + 1) * (NUM_CONFIG_STRS + 1);
    p = buff = CALLOCATE(buff_size, char, TAG_CONFIG, "read_config_file: 1");
    *p++ = '\n';

    while (1) {
	if (fgets(str, MAX_LINE_LENGTH * 4, file) == NULL)
	    break;
	if (!str)
	    break;
	len = strlen(str);
	if (len > MAX_LINE_LENGTH) {
	    fprintf(stderr, "*Error in config file: line too long:\n%s...\n", str);
	    exit(-1);
	}
	if (str[0] != '#' && str[0] != '\n') {
	    size += len + 1;
	    if (size > buff_size) {
		tmp = p - buff;
		buff = RESIZE(buff, buff_size *= 2, char, 
			      TAG_CONFIG, "read_config_file: 2");
		p = buff + tmp;
	    }
	    strncpy(p, str, len);
	    p += len;
	    *p++ = '\n';
	}
    }
    *p = 0;
}


/*
 * If the required flag is 0, it will only give a warning if the line is
 * missing from the config file.  Otherwise, it will give an error and exit
 * if the line isn't there.
 */
/* required:
      1  : Must have
      0  : optional
      -1 : warn if missing */
static int scan_config_line P3(char *, fmt, void *, dest, int, required)
{
    char *tmp, *end;
    char missing_line[MAX_LINE_LENGTH];

    /* zero the destination.  It is either a pointer to an int or a char
       buffer, so this will work */
    *((int *)dest) = 0;
    tmp = buff;
    while (tmp) {
	while ((tmp = (char *) strchr(tmp, '\n'))) {
	    if (*(++tmp) == fmt[0]) break;
	}
	/* don't allow sscanf() to scan to next line for blank entries */
	end = (tmp ? strchr(tmp, '\n') : 0);
	if (end) *end = '\0';
	if (tmp && sscanf(tmp, fmt, dest) == 1) {
	    if (end) *end = '\n';
	    break;
	}
	if (end) *end = '\n';
    }
    if (!tmp) {
	strcpy(missing_line, fmt);
	tmp = (char *) strchr(missing_line, ':');
	*tmp = '\0';
	if (required == -1) {
	    fprintf(stderr, "*Warning: Missing line in config file:\n\t%s\n",
			  missing_line);
	    return 0;
	}
	if (!required) return 0;
	fprintf(stderr, "*Error in config file.  Missing line:\n\t%s\n",
		missing_line);
	exit(-1);
    }
    return 1;
}

void set_defaults P1(char *, filename)
{
    FILE *def;
    char defaults[SMALL_STRING_SIZE];
    char *p;
    char tmp[MAX_LINE_LENGTH];
    char kind[MAX_LINE_LENGTH];
    int i, port, port_start = 0;

    max_string_length = 120; /* needed for string_copy() below */
    config_init();
    def = fopen(filename, "r");
    if (def) {
	fprintf(stderr, "using config file: %s\n", filename);
    } else {
#ifdef OS2
	sprintf(defaults, "%s\\%s", CONFIG_FILE_DIR, filename);
#else
#ifdef LATTICE
	if (strchr(CONFIG_FILE_DIR, ':'))
	    sprintf(defaults, "%s%s", CONFIG_FILE_DIR, filename);
	else
#endif
	    sprintf(defaults, "%s/%s", CONFIG_FILE_DIR, filename);
#endif

	def = fopen(defaults, "r");
	if (def) {
	    fprintf(stderr, "using config file: %s\n", defaults);
	}
    }
    if (!def) {
	fprintf(stderr, "*Error: couldn't find or open config file: '%s'\n", filename);
	exit(-1);
    }
    read_config_file(def);

    scan_config_line("global include file : %[^\n]", tmp, 0);
    p = CONFIG_STR(__GLOBAL_INCLUDE_FILE__) = alloc_cstring(tmp, "config file: gif");

    /* check if the global include file is quoted */
    if (*p && *p != '\"' && *p != '<') {
	char *ptr;

	fprintf(stderr, "Missing '\"' or '<' around global include file name; adding quotes.\n");
	for (ptr = p; *ptr; ptr++)
	    ;
	ptr[2] = 0;
	ptr[1] = '\"';
	while (ptr > p) {
	    *ptr = ptr[-1];
	    ptr--;
	}
	*p = '\"';
    }

    scan_config_line("name : %[^\n]", tmp, 1);
    CONFIG_STR(__MUD_NAME__) = alloc_cstring(tmp, "config file: mn");
    scan_config_line("address server ip : %[^\n]", tmp, 0);
    CONFIG_STR(__ADDR_SERVER_IP__) = alloc_cstring(tmp, "config file: asi");

    scan_config_line("mudlib directory : %[^\n]", tmp, 1);
    CONFIG_STR(__MUD_LIB_DIR__) = alloc_cstring(tmp, "config file: mld");
    scan_config_line("binary directory : %[^\n]", tmp, 1);
    CONFIG_STR(__BIN_DIR__) = alloc_cstring(tmp, "config file: bd");

    scan_config_line("log directory : %[^\n]", tmp, 1);
    CONFIG_STR(__LOG_DIR__) = alloc_cstring(tmp, "config file: ld");
    scan_config_line("include directories : %[^\n]", tmp, 1);
    CONFIG_STR(__INCLUDE_DIRS__) = alloc_cstring(tmp, "config file: id");
#ifdef BINARIES
    scan_config_line("save binaries directory : %[^\n]", tmp, 1);
    CONFIG_STR(__SAVE_BINARIES_DIR__) = alloc_cstring(tmp, "config file: sbd");
#endif

    scan_config_line("master file : %[^\n]", tmp, 1);
    CONFIG_STR(__MASTER_FILE__) = alloc_cstring(tmp, "config file: mf");
    scan_config_line("simulated efun file : %[^\n]", tmp, 0);
    CONFIG_STR(__SIMUL_EFUN_FILE__) = alloc_cstring(tmp, "config file: sef");
    scan_config_line("swap file : %[^\n]", tmp, 1);
    CONFIG_STR(__SWAP_FILE__) = alloc_cstring(tmp, "config file: sf");
    scan_config_line("debug log file : %[^\n]", tmp, -1);
    CONFIG_STR(__DEBUG_LOG_FILE__) = alloc_cstring(tmp, "config file: dlf");

    scan_config_line("default error message : %[^\n]", tmp, 0);
    CONFIG_STR(__DEFAULT_ERROR_MESSAGE__) = alloc_cstring(tmp, "config file: dem");
    scan_config_line("default fail message : %[^\n]", tmp, 0);
    CONFIG_STR(__DEFAULT_FAIL_MESSAGE__) = alloc_cstring(tmp, "config file: dfm");

    if (scan_config_line("port number : %d\n", &CONFIG_INT(__MUD_PORT__), 0)) {
	external_port[0].port = PORTNO;
	external_port[0].kind = PORT_TELNET;
	port_start = 1;
    }
    
    scan_config_line("address server port : %d\n",
		     &CONFIG_INT(__ADDR_SERVER_PORT__), 0);

    scan_config_line("time to clean up : %d\n",
		     &CONFIG_INT(__TIME_TO_CLEAN_UP__), 1);
    scan_config_line("time to reset : %d\n", 
		     &CONFIG_INT(__TIME_TO_RESET__), 1);
    scan_config_line("time to swap : %d\n",
		     &CONFIG_INT(__TIME_TO_SWAP__), 1);

#if 0
    /*
     * not currently used...see options.h
     */
    scan_config_line("maximum efun sockets : %d\n",
		     &CONFIG_INT(__MAX_EFUN_SOCKS__), 0);
    scan_config_line("compiler stack size : %d\n",
		     &CONFIG_INT(__COMPILER_STACK_SIZE__), 0);
    scan_config_line("evaluator stack size : %d\n", 
		     &CONFIG_INT(__EVALUATOR_STACK_SIZE__), 0);
    scan_config_line("maximum local variables : %d\n",
		     &CONFIG_INT(__MAX_LOCAL_VARIABLES__), 0);
    scan_config_line("maximum call depth : %d\n",
		     &CONFIG_INT(__MAX_CALL_DEPTH__), 0);
    scan_config_line("living hash table size : %d\n",
		     &CONFIG_INT(__LIVING_HASH_TABLE_SIZE__), 0);
#endif

    scan_config_line("inherit chain size : %d\n",
		     &CONFIG_INT(__INHERIT_CHAIN_SIZE__), 1);
    scan_config_line("maximum evaluation cost : %d\n", 
		     &CONFIG_INT(__MAX_EVAL_COST__), 1);

    scan_config_line("maximum array size : %d\n",
		     &CONFIG_INT(__MAX_ARRAY_SIZE__), 1);
#ifndef DISALLOW_BUFFER_TYPE
    scan_config_line("maximum buffer size : %d\n", 
		     &CONFIG_INT(__MAX_BUFFER_SIZE__), 1);
#endif
    scan_config_line("maximum mapping size : %d\n", 
		     &CONFIG_INT(__MAX_MAPPING_SIZE__), 1);
    scan_config_line("maximum string length : %d\n",
		     &CONFIG_INT(__MAX_STRING_LENGTH__), 1);
    scan_config_line("maximum bits in a bitfield : %d\n",
		     &CONFIG_INT(__MAX_BITFIELD_BITS__), 1);

    scan_config_line("maximum byte transfer : %d\n", 
		     &CONFIG_INT(__MAX_BYTE_TRANSFER__), 1);
    scan_config_line("maximum read file size : %d\n",
		     &CONFIG_INT(__MAX_READ_FILE_SIZE__), 1);

    scan_config_line("reserved size : %d\n",
		     &CONFIG_INT(__RESERVED_MEM_SIZE__), 0);

    scan_config_line("hash table size : %d\n",
		     &CONFIG_INT(__SHARED_STRING_HASH_TABLE_SIZE__), 1);
    scan_config_line("object table size : %d\n",
		     &CONFIG_INT(__OBJECT_HASH_TABLE_SIZE__), 1);

    for (i = port_start; i < 5; i++) {
	external_port[i].kind = 0;
	external_port[i].fd = -1;
	sprintf(kind, "external_port_%i : %%[^\n]", i + 1);
	if (scan_config_line(kind, tmp, 0)) {
	    if (sscanf(tmp, "%s %d", kind, &port) == 2) {
		external_port[i].port = port;
		if (!strcmp(kind, "telnet")) 
		    external_port[i].kind = PORT_TELNET;
		else
		if (!strcmp(kind, "binary"))
		    external_port[i].kind = PORT_BINARY;
		else
		if (!strcmp(kind, "ascii"))
		    external_port[i].kind = PORT_ASCII;
		else {
		    fprintf(stderr, "Unknown kind of external port: %s\n",
				  kind);
		    exit(-1);
		}
	    } else {
		fprintf(stderr, "Syntax error in port specification\n");
		exit(-1);
	    }
	}
    }		    

    FREE(buff);
    fclose(def);

    /*
     * from options.h
     */
    config_int[__MAX_EFUN_SOCKS__ - BASE_CONFIG_INT] = MAX_EFUN_SOCKS;
    config_int[__COMPILER_STACK_SIZE__ - BASE_CONFIG_INT] = COMPILER_STACK_SIZE;
    config_int[__EVALUATOR_STACK_SIZE__ - BASE_CONFIG_INT] = EVALUATOR_STACK_SIZE;
    config_int[__MAX_LOCAL_VARIABLES__ - BASE_CONFIG_INT] = MAX_LOCAL;
    config_int[__MAX_CALL_DEPTH__ - BASE_CONFIG_INT] = MAX_TRACE;
    config_int[__LIVING_HASH_TABLE_SIZE__ - BASE_CONFIG_INT] = LIVING_HASH_SIZE;
}

int get_config_item P2(svalue_t *, res, svalue_t *, arg)
{
    int num;

    num = arg->u.number;

    if (num < 0 || num >= RUNTIME_CONFIG_NEXT) {
	return 0;
    }
    if (num >= BASE_CONFIG_INT) {
	res->type = T_NUMBER;
	res->u.number = config_int[num - BASE_CONFIG_INT];
    } else {
	res->type = T_STRING;
	res->subtype = STRING_CONSTANT;
	res->u.string = config_str[num];
    }

    return 1;
}
