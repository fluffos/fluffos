/*
 * Some structure forward declarations are needed.
 */

#if defined(_SEQUENT_) || defined(linux) || defined(SVR4)
#include <malloc.h>
#endif

#include "mudlib_stats.h"

struct program;
struct buffer;
struct function;
#ifndef LATTICE
struct svalue;
struct sockaddr;
struct mapping;
struct node;
#endif

#ifdef BUFSIZ
#    define PROT_STDIO(x) PROT(x)
#else /* BUFSIZ */
#    define PROT_STDIO(x) ()
#endif /* BUFSIZ */

#ifdef __STDC__
#    define PROT(x) x
#    define VOLATILE volatile
#else /* __STDC__ */
#    define PROT(x) ()
#    define VOLATILE
#endif /* __STDC */

#ifdef USE_POSIX_SIGNALS
int port_sigblock();
int port_sigmask();
void (*port_signal())();
int port_sigsetmask();
#endif

#ifdef NeXT
int _setjmp();
void _longjmp();
#endif

/* INLINE is defined in config.h.  Be sure to include config.h before
   including lint.h
*/

#ifdef NeXT
#include <libc.h>
#else
#ifndef SunOS_5
#ifndef MSDOS
#if defined(sun) && defined(__STDC__)
#ifdef BUFSIZ
int fprintf(FILE *, char *, ...);
int fputs(char *, FILE *);
int fputc(char, FILE *);
int fwrite(char *, int, int, FILE *);
int fread(char *, int, int, FILE *);
#endif
int printf(char *, ...);
int sscanf(char *, char *, ...);
void perror(char *);
#endif

#if defined(SVR4) || defined(__386BSD__)
void exit PROT((int));
#endif

#if !defined(_SEQUENT_) && !defined(_AIX) && !defined(__386BSD__) && \
	!defined(linux) && !defined(cray) && !defined(sgi) && \
	!defined(__bsdi__) && !defined(LATTICE)
int read PROT((int, char *, int));
#endif /* !defined(_SEQUENT_) && !defined(_AIX) */
#if !defined(_AIX) && !defined(_SEQUENT_) && !defined(_YACC_) && \
	!defined(linux) && !defined(sun)
void *malloc PROT((unsigned));
void *realloc PROT((void *, unsigned));
void free PROT((void *));
#endif
#if !defined(sgi) && !defined(hpux) && !defined(_AIX) && \
	!defined(_SEQUENT_) && !defined(SVR4) && \
        !defined(__386BSD__) && !defined(linux) && !defined(hp68k) && \
	!defined(__bsdi__)
int mkdir PROT((char *, int));
#endif
int fclose PROT_STDIO((FILE *));
int pclose PROT_STDIO((FILE *));
#ifndef _AIX
#if defined(_SEQUENT_) || defined(sgi) || defined(SVR4) || defined(__386BSD__)
int atoi PROT((const char *));
#else
int atoi PROT((char *));
#endif
#endif
#if !defined(sgi) && !defined(hpux) && !defined(__386BSD__)
void srandom PROT((int));
#endif
#if !defined(_SEQUENT_) && !defined(__386BSD__) && !defined(linux) \
	&& !defined(sgi) && !defined(__bsdi__) && !defined(LATTICE)
int chdir PROT((char *));
#endif
#if !defined(linux)
int gethostname PROT((char *, int));
#endif
void abort PROT((void));
int fflush PROT_STDIO((FILE *));
#if !defined(_SEQUENT_) && !defined(__386BSD__) && !defined(linux) \
	&& !defined(sgi) && !defined(__bsdi__)
int rmdir PROT((char *));
#ifndef LATTICE
int unlink PROT((char *));
#endif
#endif
int fclose PROT_STDIO((FILE *));
#if !defined(sgi) && !defined(hpux) && !defined(_AIX) && !defined(M_UNIX) && !defined(_SEQUENT_) && !defined(SVR4) && !defined(OSF) && !defined(__386BSD__)
int system PROT((char *));
#endif
#if !defined(_AIX) && !defined(sgi)
#if defined(_SEQUENT_) || defined(SVR4) || defined(__386BSD__)
void qsort PROT((void *, size_t, size_t, int(*)(const void *, const void *)));
#else
void qsort PROT((char *, int, int, int (*)()));
#endif
#endif
#if !defined(hpux) && !defined(__386BSD__) && !defined(linux) && !defined(sgi) \
	&& !defined(__bsdi__)
int setsockopt PROT((int, int, int, char *, int));
#endif /* !defined(hpux) */
#if !defined(linux)
int fseek PROT_STDIO((FILE *, long, int));
#endif
int wait PROT((int *));
int pipe PROT((int *));
int dup2 PROT((int, int));
#if !defined(linux)
unsigned int alarm PROT((unsigned int));
#endif
#if !defined(hpux) && !defined(__386BSD__) && !defined(linux) \
	&& !defined(__bsdi__) && !defined(LATTICE)
int ioctl PROT((int, ...));
#endif /* !defined(hpux) */
int close PROT((int));
#if !defined(_SEQUENT_) && !defined(_AIX) && !defined(__386BSD__) && \
	!defined(linux) && !defined(cray) && !defined(sgi) && \
	!defined(__bsdi__) && !defined(LATTICE)
int write PROT((int, char *, int));
#endif /* !defined(_SEQUENT_) && !defined(_AIX) */
int _filbuf();
#if defined(__386BSD__) || defined(linux) || defined(__bsdi__)
char *crypt PROT((const char *, const char *));
#else
char *crypt PROT((char *, char *));
#endif
#ifdef sun
char *_crypt PROT((char *, char *));
#endif

#ifdef DRAND48
double drand48 PROT((void));
void srand48 PROT((long));
#endif
#ifdef RANDOM
long random PROT((void));
#endif

#ifndef _AIX
#if defined(_SEQUENT_) || defined(sgi) || defined(SVR4) || defined(__386BSD__)
long strtol PROT((const char *, char **, int));
#else
long strtol PROT((char *, char **, int));
#endif
#endif
#if !defined(_SEQUENT_) && !defined(__386BSD__) && !defined(linux) \
	&& !defined(sgi) && !defined(__bsdi__) && !defined(LATTICE)
int link PROT((char *, char *));
int unlink PROT((char *));
#endif
#endif /* MSDOS */
#endif /* SunOS_5 */

#endif /* NeXT */

struct object;
void init_rc PROT((int));
char *get_error_file PROT((char *));
void save_error PROT((char *, char *, int));
int write_file PROT((char *, char *));
int file_size PROT((char *));
char *check_file_name PROT((char *, int));
void remove_all_users PROT((void));
INLINE void remove_destructed_objects();
void load_dstats_file PROT((void));
struct mapping *get_domain_stats PROT((char *));
void backend();
char *xalloc PROT((int));
void init_string_space PROT((void));
void error();
void fatal();
void add_message();
void trace_log();
void debug_message();
void debug_message_value PROT((struct svalue *)),
	print_local_commands(),
	new_call_out PROT((struct object *, char *, int, int, struct svalue *)),
	add_action PROT((char *, char *, int)),
	list_files PROT((char *)),
	enable_commands PROT((int)),
	load_ob_from_swap PROT((struct object *));
int tail PROT((char *));
void enter_object_hash PROT((struct object *));
void remove_object_hash PROT((struct object *));
struct object *lookup_object_hash PROT((char *));
int show_otable_status PROT((int verbose));
void io_mode_stats();
void dumpstat PROT((char *));
void free_mapping PROT((struct mapping *));
INLINE struct vector *mapping_each PROT((struct mapping *));
struct svalue *find_in_mapping PROT((struct mapping *, struct svalue *));
struct svalue *find_for_insert PROT((struct mapping *, struct svalue *, int));
struct mapping *load_mapping_from_aggregate PROT((struct svalue *, int));
struct mapping *add_mapping PROT((struct mapping *, struct mapping *));
struct mapping *map_mapping 
   PROT((struct mapping *, char *, struct object *, struct svalue *));
struct mapping *compose_mapping PROT((struct mapping *, struct mapping *));
#ifndef LATTICE
struct vector;
#endif
struct vector *mapping_indices PROT((struct mapping *));
struct vector *mapping_values PROT((struct mapping *));
void free_vector PROT((struct vector *));
char *query_load_av PROT((void));
void update_compile_av PROT((int));
struct vector *map_array PROT((
			       struct vector *arr,
			       char *func,
			       struct object *ob,
			       struct svalue *extra
			       ));
struct vector *make_unique PROT((struct vector *arr,char *func,
    struct svalue *skipnum));

char *describe_items PROT((struct svalue *arr,char *func,int live));
struct vector *commands PROT((struct object *));
struct mapping *get_domain_info PROT((mudlib_stats_t *));
struct vector *domain_list();
struct vector *filter PROT((struct vector *arr,char *func,
			    struct object *ob, struct svalue *)); 
int match_string PROT((char *, char *));
int set_heart_beat PROT((struct object *, int));
int query_heart_beat PROT((struct object *));
struct object *get_empty_object PROT((int));
#ifndef LATTICE
struct svalue;
#endif
INLINE void assign_svalue PROT((struct svalue *, struct svalue *));
INLINE void assign_svalue_no_free PROT((struct svalue *to, struct svalue *from));
INLINE void free_svalue PROT((struct svalue *));
char *make_shared_string PROT((char *));
char *ref_string PROT((char *));
void free_string PROT((char *));
int add_string_status PROT((int verbose));
void notify_no_command PROT((void));
void clear_notify PROT((void));
void throw_error PROT((void));
void set_living_name PROT((struct object *,char *));
void remove_living_name PROT((struct object *));
struct object *find_living_object PROT((char *, int));
int lookup_predef PROT((char *));
void yyerror PROT((char *));
INLINE int hashstr PROT((char *, int, int));
INLINE int whashstr PROT((char *, int));
int lookup_predef PROT((char *));
char *dump_trace PROT((int));
int parse_command PROT((char *, struct object *));
struct svalue *safe_apply PROT((char *, struct object *, int));
struct svalue *apply PROT((char *, struct object *, int));
INLINE void push_string PROT((char *, int));
INLINE void push_number PROT((int));
INLINE void push_real PROT((double));
INLINE void push_object PROT((struct object *));
struct object *clone_object PROT((char *));
void init_num_args PROT((void));
int restore_object PROT((struct object *, char *, int));
struct vector *slice_array PROT((struct vector *,int,int));
int query_idle PROT((struct object *));
char *implode_string PROT((struct vector *, char *));
struct object *query_snoop PROT((struct object *));
struct object *query_snooping PROT((struct object *));
struct vector *all_inventory PROT((struct object *, int override));
struct vector *deep_inventory PROT((struct object *, int));
struct object *environment PROT((struct svalue *));
struct vector *add_array PROT((struct vector *, struct vector *));
char *get_f_name PROT((int));
#if SIGNAL_FUNC_TAKES_INT
void startshutdownMudOS PROT((int));
#else
void startshutdownMudOS PROT((void));
#endif
void shutdownMudOS PROT((int));
void set_notify_fail_message PROT((char *));
int swap PROT((struct object *));
int swap_line_numbers PROT((struct program *));
int transfer_object PROT((struct object *, struct object *));
struct vector *users PROT((void));
void do_write PROT((struct svalue *));
void log_file PROT((char *, char *));
int remove_call_out PROT((struct object *, char *));
char *create_wizard PROT((char *, char *));
void destruct_object PROT((struct svalue *));
void set_snoop PROT((struct object *, struct object *));
int new_set_snoop PROT((struct object *, struct object *));
void ed_start PROT((char *, char *, int, struct object *));
void say PROT((struct svalue *, struct vector *));
void tell_room PROT((struct object *, struct svalue *, struct vector *));
void shout_string PROT((char *));
int command_for_object PROT((char *, struct object *));
int remove_file PROT((char *));
int print_call_out_usage PROT((int verbose));
int input_to PROT((char *, int, int, struct svalue *));
int parse PROT((char *, struct svalue *, char *, struct svalue *, int));
struct object *object_present PROT((struct svalue *, struct object *));
void add_light PROT((struct object *, int));
int indent_program PROT((char *));
void call_function PROT((struct program *, struct function *));
void store_line_number_info PROT((void));
INLINE void push_constant_string PROT((char *));
void push_svalue PROT((struct svalue *));
struct variable *find_status PROT((char *, int));
void free_prog PROT((struct program *, int));
void stat_living_objects PROT((void));
int heart_beat_status PROT((int verbose));
void opcdump PROT((char *));
void slow_shut_down PROT((int));
struct vector *allocate_array PROT((int));
void yyerror PROT((char *));
void reset_machine PROT((int));
void clear_state PROT((void));
void preload_objects PROT((int));
int random_number PROT((int));
void reset_object PROT((struct object *, int));
int replace_interactive PROT((struct object *ob, struct object *obf));
char *get_log_file PROT((char *));
int get_current_time PROT((void));
char *time_string PROT((int));
char *process_string PROT((char *));
void update_ref_counts_for_users PROT((void));
void count_ref_from_call_outs PROT((void));
void check_a_lot_ref_counts PROT((struct program *));
int shadow_catch_message PROT((struct object *ob, char *str));
struct vector *get_all_call_outs PROT((void));
char *read_file PROT((char *file, int, int));
char *read_bytes PROT((char *file, int, int, int *));
char *read_buffer PROT((struct buffer *b, int, int, int *));
int write_bytes PROT((char *file, int, char *str, int theLength));
int write_buffer PROT((struct buffer *buf, int, char *str, int theLength));
char *check_valid_path PROT((char *, struct object *, char *, int));
int get_line_number_if_any PROT((void));
void logon PROT((struct object *ob));
struct svalue *apply_master_ob PROT((char *fun, int num_arg));
struct svalue *safe_apply_master_ob PROT((char *fun, int num_arg));
void assert_master_ob_loaded();
struct vector *explode_string PROT((char *str, char *del));
char *string_copy PROT((char *));
int find_call_out PROT((struct object *ob, char *fun));
void remove_object_from_stack PROT((struct object *ob));
#if !defined(sgi) && !defined(NeXT) && !defined(hpux) && !defined(sun) && \
	 !defined(_AIX) && !defined(LATTICE)
int getpeername PROT((int, struct sockaddr *, int *));
int shutdown PROT((int, int));
#endif
void compile_file PROT((void));
void unlink_swap_file();
char *function_exists PROT((char *, struct object *));
int is_static PROT((char *, struct object *));
void set_inc_list PROT((char *list));
int legal_path PROT((char *path));
struct vector *get_dir PROT((char *path, int));
#if defined(sun) && !defined(SunOS_5)
extern int rename PROT((char *, char *));
#endif
void set_simul_efun PROT((char *));
void set_global_include PROT((char *));
struct function *find_simul_efun PROT((char *));
char *query_simul_efun_file_name PROT((void));
struct vector *match_regexp PROT((struct vector *v, char *pattern));

int remove_action PROT((char *,char *));

void get_version PROT((char *));

INLINE int get_config_int PROT((int num));
INLINE char * get_config_str PROT((int num));
void set_defaults PROT((char * filename));
INLINE struct mapping *allocate_mapping PROT((int));

int check_in PROT((char *fn, char *msg, int maj, int min));
int check_out PROT((char *fn, int maj, int min));
char *rlog PROT((char *));

int copy_file PROT((char *from, char *to));
int get_char PROT((char *fun, int flag));

void dump_file_descriptors();
INLINE void copy_some_svalues PROT((struct svalue *, struct svalue *, int));
int svalue_save_size PROT((struct svalue *));
void save_svalue PROT((struct svalue *, char **buf));

char *string_print_formatted PROT((char *format_str, int argc, struct svalue *argv));
struct vector *children PROT((char *obj));
struct vector *livings PROT((void));
void do_message PROT((char *,char *, struct vector *, struct vector *, int));
char *add_slash PROT((char *));
struct object *load_extern_object PROT((char *name));
INLINE struct vector *prepend_vector PROT((struct vector *v, struct svalue *a));
INLINE struct vector *append_vector PROT((struct vector *v, struct svalue *a));
INLINE void push_control_stack PROT((struct function *funp));
INLINE struct function *setup_new_frame PROT((struct function *funp));
INLINE struct svalue *find_value PROT((int num));
INLINE void push_indexed_lvalue();
int inter_sscanf PROT((int num_arg));
INLINE struct vector *null_array();
INLINE struct buffer *null_buffer();
void free_vector PROT((struct vector *)), free_all_values();
void debug_fatal();

void bad_arg PROT((int, int));
INLINE void pop_n_elems PROT((int));
INLINE void pop_stack();
INLINE void push_vector PROT((struct vector *));
INLINE void push_mapping PROT((struct mapping *));
void break_point();
INLINE void push_undefined();
int strpref PROT((char *, char *));
void do_trace PROT((char *, char *, char *));
int apply_low PROT((char *, struct object *, int));
INLINE void push_malloced_string PROT((char *));
void check_for_destr PROT((struct vector *));
extern int do_rename PROT((char *, char *, int));
void mapping_delete PROT((struct mapping *, struct svalue *));
#ifndef NO_SHADOWS
int validate_shadowing PROT((struct object *ob));
#endif
void pop_control_stack();
void push_pop_error_context PROT((int));

void init_sockets();
int socket_bind PROT((int, int));
int socket_listen PROT((int, char *));
int socket_accept PROT((int, char *, char *));
int socket_connect PROT((int, char *, char *, char *));
int socket_write PROT((int, struct svalue *, char *));
void socket_read_select_handler PROT((int));
void socket_write_select_handler PROT((int));
int socket_close PROT((int));
int socket_release PROT((int, struct object *, char *));
int socket_acquire PROT((int, char *, char *, char *));
char *socket_error PROT((int));
int get_socket_address PROT((int, char *, int *));
struct object *get_socket_owner PROT((int));
void assign_socket_owner PROT((struct svalue *, struct object *));
void close_referencing_sockets PROT((struct object *));
void dump_socket_status PROT((void));

INLINE int set_socket_owner PROT((int, int));
INLINE int set_socket_async PROT((int, int));
INLINE int set_socket_nonblocking PROT((int, int));

void update_load_av();
void ed_cmd PROT((char *));
INLINE void free_some_svalues PROT((struct svalue *, int));
/* ack! */
INLINE struct mapping *mapTraverse
	PROT((struct mapping *,
	    int (*func) PROT((struct mapping *, struct node *, void *)),
		void *));
INLINE void process_queue();
void push_switches();
void pop_switches();
char *the_file_name PROT((char *));
char *query_instr_name PROT((int));
void init_comm();
char *findstring PROT((char *));
INLINE void process_io();
void new_user_handler();
int restore_svalue PROT((char *val, struct svalue *v));
INLINE void push_null();
#ifdef LAZY_RESETS
INLINE void try_reset PROT((struct object *));
#endif

INLINE void check_legal_string PROT((char *));

INLINE int object_visible PROT((struct object *));

#ifdef TRAP_CRASHES
void crash_MudOS();
#endif

#ifdef WRAPPEDMALLOC
INLINE void *wrappedmalloc PROT((int));
INLINE void *wrappedrealloc PROT((void *,int));
INLINE void *wrappedcalloc PROT((int,int));
INLINE void wrappedfree PROT((void *));
#endif
#ifdef DEBUGMALLOC
INLINE void *debugmalloc PROT((int,int,char *));
INLINE void *debugrealloc PROT((void *,int,int,char *));
INLINE void *debugcalloc PROT((int,int,int,char *));
INLINE void debugfree PROT((void *));
#ifdef DEBUGMALLOC_EXTENSIONS
void set_malloc_mask PROT((int));
void dump_debugmalloc PROT((char *, int));
#endif
#endif


/* mudlib_stats function prototypes */
void add_moves PROT((statgroup_t *, int));
INLINE void add_cost PROT((statgroup_t *, int));
INLINE void add_heart_beats PROT((statgroup_t *, int));
void add_array_size PROT((statgroup_t *, int));
void add_errors PROT((statgroup_t *, int));
void add_objects PROT((statgroup_t *, int));
void assign_stats PROT((statgroup_t *, struct object *));
void null_stats PROT((statgroup_t *));
mudlib_stats_t *set_backbone_domain PROT((char *));
mudlib_stats_t *set_root_author PROT((char *));
void init_stats_for_object PROT((struct object *));
void mudlib_stats_decay();
void add_mapping_pair PROT((struct mapping *m, char *key, int value));
mudlib_stats_t * add_stat_entry PROT((char *str, mudlib_stats_t **list));
char *domain_for_file PROT((char *));
char *author_for_file PROT((char *));
struct mapping *get_domain_stats PROT((char *str));
struct mapping *get_author_stats PROT((char *str));
void add_errors_for_file PROT((char *file, int errors));
void restore_stat_files();
void save_stat_files();
void set_author PROT((char *name));
void quickSort PROT((void *, int, int, int (*compar)()));
INLINE struct funp *make_funp PROT((struct svalue *, struct svalue *));
INLINE void free_funp PROT((struct funp *fp));
INLINE void push_funp PROT((struct funp *fp));
INLINE int valid_hide PROT((struct object *obj));
INLINE void free_string_svalue PROT((struct svalue *));
void absorb_mapping PROT((struct mapping *, struct mapping *));
void init_usec_clock();
void get_usec_clock PROT((long *sec, long *usec));
void bufcat PROT((char **buf, char *str));
int get_cpu_times PROT((unsigned long *secs, unsigned long *usecs));
char *_strstr PROT((char *, char *));
void print_swap_stats();
void load_line_numbers PROT((struct program *prog));
void remove_line_swap PROT((struct program *prog));
int locate_out PROT((struct program *prog));
int locate_in PROT((struct program *prog));
short store_prog_string PROT((char *str));
int load_binary PROT((char *name));
char *get_current_dir PROT((char *buf, int max));
void get_simul_efuns PROT((struct program *prog));
INLINE void free_buffer PROT((struct buffer *));
INLINE void push_buffer PROT((struct buffer *));
struct buffer *allocate_buffer PROT((int size));
UINT32 compute_crc32 PROT((unsigned char *buf, int len));
