/*
 * lint.h
 *
 * INLINE is defined in config.h.  In the event it's used here in the
 * function prototypes, be sure to include config.h before this file.
 */

#ifndef _LINT_H_
#define _LINT_H_

#if defined(_SEQUENT_) || defined(linux) || defined(SVR4)
#include <malloc.h>
#endif

/*
 * Stuff needed to make this file compile stand alone; useful for
 * producing precompiled header files (aka global symbol table files),
 * if your compiler supports it.  =)
 */
#include <stdio.h>
#include <setjmp.h>
#include <sys/types.h>
#ifdef _AIX
#include <sys/select.h>
#endif

/*
 * Some useful macros...
 */
#ifdef BUFSIZ
#define PROT_STDIO(x) PROT(x)
#else				/* BUFSIZ */
#define PROT_STDIO(x) ()
#endif				/* BUFSIZ */

/* ANSI/K&R compatibility stuff;
 *
 * The correct way to prototype a function now is:
 *
 * foobar PROT((int, char *));
 *
 * foobar P2(int, x, char *, y) { ... }
 */
/* xlc can't handle an ANSI protoype followed by a K&R def, and varargs
 * functions must be done K&R (b/c va_dcl is K&R style) so don't prototype
 * vararg function arguments under AIX
 */
#ifdef __STDC__
#define PROT(x) x
#define P1(t1, v1) (t1 v1)
#define P2(t1, v1, t2, v2) (t1 v1, t2 v2)
#define P3(t1, v1, t2, v2, t3, v3) (t1 v1, t2 v2, t3 v3)
#define P4(t1, v1, t2, v2, t3, v3, t4, v4) (t1 v1, t2 v2, t3 v3, t4 v4)
#define P5(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5)
#define P6(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6) (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6)
#define P7(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7) (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6, t7 v7)
#define VOLATILE volatile
#define SIGNED signed
#else				/* __STDC__ */
#define PROT(x) ()
#define P1(t1, v1) (v1) t1 v1;
#define P2(t1, v1, t2, v2) (v1, v2) t1 v1; t2 v2;
#define P3(t1, v1, t2, v2, t3, v3) (v1, v2, v3) t1 v1; t2 v2; t3 v3;
#define P4(t1, v1, t2, v2, t3, v3, t4, v4) (v1, v2, v3, v4) t1 v1; t2 v2; t3 v3; t4 v4;
#define P5(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) (v1, v2, v3, v4, v5) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5;
#define P6(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6) (v1, v2, v3, v4, v5, v6) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6;
#define P7(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7) (v1, v2, v3, v4, v5, v6, v7) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6; t7 v7;
#define VOLATILE
#define SIGNED
#endif				/* __STDC__ */

/*
 * varargs can't be prototyped portably ... *sigh*
 */
#if defined(LATTICE)
#define PROTVARGS(x) (char *, ...)
#define PVARGS(v) (char *v, ...)
#else
#define PROTVARGS(x) ()
#define PVARGS(v) (v) va_dcl
#endif


/******************************************************************
 *  Structure forward declarations are needed by some compilers.  *
 ******************************************************************/

#ifndef LATTICE
struct program;
struct object;
struct buffer;
struct function;
struct svalue;
struct sockaddr;
struct mapping;
struct node;
struct vector;
struct sentence;
struct interactive;
struct variable;
struct control_stack;
struct lpc_predef_s;
struct instr;
struct mem_block;
struct lpc_socket;
struct funp;
#endif

/*
 * Perhaps prototypes & declarations for each module should be
 * in a separate header file, like these:
 */
#include "config.h"
#include "exec.h"
#include "avltree.h"
#include "mudlib_stats.h"
#include "regexp.h"
#include "replace_program.h"
#include "uid.h"
#include "interpret.h"
#include "buffer.h"
#include "incralloc.h"
#include "instrs.h"
#include "sent.h"
#include "object.h"
#include "mapping.h"
#include "lex.h"
#ifdef SOCKET_EFUNS
#include "socket_efuns.h"
#endif

/*******************************************************
 *  Declarations and/or prototypes for lib functions.  *
 *******************************************************/

#ifdef USE_POSIX_SIGNALS
int port_sigblock();
int port_sigmask();

#ifndef _M_UNIX
void (*port_signal()) ();
#endif

int port_sigsetmask();
#endif

#ifdef NeXT
int _setjmp();
void _longjmp();

#endif

#ifdef NeXT
#include <libc.h>
#else
#ifndef SunOS_5
#ifndef MSDOS
#if defined(sun) && defined(__STDC__)
#ifdef BUFSIZ
int fprintf(FILE *, char *,...);
int fputs(char *, FILE *);
int fputc(char, FILE *);
int fwrite(char *, int, int, FILE *);
int fread(char *, int, int, FILE *);
#endif
int printf(char *,...);
int sscanf(char *, char *,...);
void perror(char *);
#endif

#if (defined(SVR4) && !defined(sgi)) || defined(__386BSD__)
void exit PROT((int));
#endif

#if !defined(_AIX) && !defined(_SEQUENT_) && !defined(_YACC_) && \
	!defined(linux) && !defined(sun) && !defined(sgi) && !defined(LATTICE)
void *malloc PROT((unsigned));
void *realloc PROT((void *, unsigned));
void free PROT((void *));

#endif
#if !defined(sgi) && !defined(hpux) && !defined(_AIX) && \
	!defined(_SEQUENT_) && !defined(SVR4) && !defined(sun) && \
        !defined(__386BSD__) && !defined(linux) && !defined(hp68k) && \
	!defined(__bsdi__) && !defined(LATTICE) && !defined(__SASC) \
	&& !defined(ultrix)
int mkdir PROT((const char *, unsigned short));
#endif
#ifndef LATTICE
int fclose PROT_STDIO((FILE *));

#endif
int pclose PROT_STDIO((FILE *));

#if !defined(_AIX) && !defined(LATTICE) && !defined(__SASC)
#if defined(_SEQUENT_) || defined(sgi) || defined(SVR4) || defined(__386BSD__)
int atoi PROT((const char *));
#else
int atoi PROT((char *));
#endif
#endif
#if !defined(sgi) && !defined(hpux) && !defined(__386BSD__) && !defined(OSF)
void srandom PROT((int));
#endif
#if !defined(linux)
int gethostname PROT((char *, int));
#endif
#ifndef LATTICE
void abort PROT(());
int fflush PROT_STDIO((FILE *));
int fclose PROT_STDIO((FILE *));
#endif
#if !defined(sgi) && !defined(hpux) && !defined(_AIX) && !defined(M_UNIX) && \
    !defined(_SEQUENT_) && !defined(SVR4) && !defined(OSF) && \
    !defined(__386BSD__) && !defined(LATTICE) && !defined(__SASC)
int system PROT((char *));
#endif
#if !defined(_AIX) && !defined(sgi) && !defined(LATTICE) \
    && !defined(SunOS_5) && !defined(__SASC)
#if defined(_SEQUENT_) || defined(SVR4) || \
    (defined(__386BSD__) && defined(__FreeBSD__))
void qsort PROT((void *, size_t, size_t, int (*) (const void *, const void *)));
#else
void qsort PROT((char *, int, int, int (*) ()));
#endif
#endif
#if !defined(hpux) && !defined(__386BSD__) && !defined(linux) && !defined(sgi) \
	&& !defined(__bsdi__) && !defined(LATTICE)
int setsockopt PROT((int, int, int, char *, int));

#endif				/* !defined(hpux) */
#if !defined(linux) && !defined(LATTICE)
int fseek PROT_STDIO((FILE *, long, int));

#endif
#if !defined(LATTICE) && !defined(__SASC)
int wait PROT((int *));

#endif
int pipe PROT((int *));
int dup2 PROT((int, int));

#if !defined(linux)
unsigned int alarm PROT((unsigned int));

#endif
#if !defined(hpux) && !defined(__386BSD__) && !defined(linux) \
	&& !defined(__bsdi__) && !defined(LATTICE) && !defined(sgi)
int ioctl PROT((int,...));

#endif				/* !defined(hpux) */
#ifndef LATTICE
int close PROT((int));

#endif
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

#if !defined(_AIX) && !defined(LATTICE) && !defined(__SASC)
#if defined(_SEQUENT_) || defined(sgi) || defined(SVR4) || defined(__386BSD__)
long strtol PROT((const char *, char **, int));

#else
long strtol PROT((char *, char **, int));

#endif
#endif
#endif				/* MSDOS */
#endif				/* SunOS_5 */
#endif				/* NeXT */

#if defined(sun) && !defined(SunOS_5)
extern int rename PROT((char *, char *));

#endif

#if !defined(sgi) && !defined(NeXT) && !defined(hpux) && !defined(sun) && \
	 !defined(_AIX) && !defined(LATTICE)
int getpeername PROT((int, struct sockaddr *, int *));
int shutdown PROT((int, int));

#endif

/********************************************************
 *  Declarations & prototypes for globals & functions.  *
 ********************************************************/

void debug_message PROTVARGS(());
void add_message PROTVARGS(());
void debug_fatal PROTVARGS(());
void fatal PROTVARGS(());
void error PROTVARGS(());

/*
 * main.c
 */
extern int d_flag;
extern int t_flag;
extern int comp_flag;
extern int time_to_swap;
extern int time_to_clean_up;
extern char *default_fail_message;
extern int port_number;
extern int boot_time;
extern int max_cost;
extern int max_array_size;
extern int max_buffer_size;
extern int max_string_length;
extern char *master_file_name;
extern char *reserved_area;
extern struct svalue const0;
extern struct svalue const1;
extern struct svalue const0u;
extern struct svalue const0n;
extern double consts[];
extern int slow_shut_down_to_do;
extern struct object *master_ob;

char *xalloc PROT((int));
char *string_copy PROT((char *));

#ifdef TRAP_CRASHES
void crash_MudOS PROT((char *));
#endif

/*
 * rc.c
 */
void set_defaults PROT((char *));
int get_config_int PROT((int));
char *get_config_str PROT((int));
int get_config_item PROT((struct svalue *, struct svalue *));

/*
 * comm.c
 */
extern int total_users;
extern fd_set readmask;
extern fd_set writemask;
extern int inet_packets;
extern int inet_volume;
extern int num_user;
extern int num_hidden;
extern struct interactive *all_users[];
extern int add_message_calls;

#ifdef SIGNAL_FUNC_TAKES_INT
void sigalrm_handler PROT((int));
#else
void sigalrm_handler PROT((void));
#endif
void update_ref_counts_for_users PROT((void));
void make_selectmasks PROT((void));
void init_user_conn PROT((void));
void init_addr_server PROT((char *, int));
void ipc_remove PROT((void));
void set_prompt PROT((char *));
void notify_no_command PROT((void));
void set_notify_fail_message PROT((char *));
void process_io PROT((void));
int process_user_command PROT((void));
int set_call PROT((struct object *, struct sentence *, int));
void remove_interactive PROT((struct object *));
int replace_interactive PROT((struct object *, struct object *));
int query_addr_number PROT((char *, char *));
char *query_ip_name PROT((struct object *));
char *query_ip_number PROT((struct object *));
char *query_host_name PROT((void));
int query_idle PROT((struct object *));
int new_set_snoop PROT((struct object *, struct object *));
struct object *query_snoop PROT((struct object *));
struct object *query_snooping PROT((struct object *));
void set_notify_fail_function PROT((struct funp *));

/*
 * backend.c
 */
extern jmp_buf error_recovery_context;
extern int error_recovery_context_exists;
extern int current_time;
extern int heart_beat_flag;
extern struct object *current_heart_beat;
extern int eval_cost;

void backend PROT((void));
void clear_state PROT((void));
void logon PROT((struct object *));
int parse_command PROT((char *, struct object *));
int set_heart_beat PROT((struct object *, int));
int query_heart_beat PROT((struct object *));
int heart_beat_status PROT((int));
void preload_objects PROT((int));
void remove_destructed_objects PROT((void));
void update_load_av PROT((void));
void update_compile_av PROT((int));
char *query_load_av PROT((void));

/*
 * simulate.c
 */
extern struct object *obj_list;
extern struct object *obj_list_destruct;
extern struct object *current_object;
extern struct object *command_giver;
extern struct object *current_interactive;
extern char *inherit_file;
extern char *last_verb;
extern int num_parse_error;
extern int num_error;
extern int tot_alloc_sentence;
extern int MudOS_is_being_shut_down;
#ifdef LPC_TO_C
extern int compile_to_c;
extern FILE *compilation_output_file;
extern char *compilation_ident;
#endif

void check_legal_string PROT((char *));
int command_for_object PROT((char *, struct object *));
void add_light PROT((struct object *, int));
void free_sentence PROT((struct sentence *));
int user_parser PROT((char *));

void enable_commands PROT((int));
int input_to PROT((char *, int, int, struct svalue *));
int get_char PROT((char *, int, int, struct svalue *));
void add_action PROT((char *, char *, int));
int remove_action PROT((char *, char *));

struct object *load_object PROT((char *, int));
struct object *clone_object PROT((char *));
struct object *environment PROT((struct svalue *));
struct object *first_inventory PROT((struct svalue *));
struct object *object_present PROT((struct svalue *, struct object *));
struct object *find_object PROT((char *));
struct object *find_object2 PROT((char *));
void move_object PROT((struct object *, struct object *));
void destruct_object PROT((struct svalue *));
void destruct2 PROT((struct object *));

void print_svalue PROT((struct svalue *));
void do_write PROT((struct svalue *));
void do_message PROT((struct svalue *, char *, struct vector *, struct vector *, int));
void say PROT((struct svalue *, struct vector *));
void tell_room PROT((struct object *, struct svalue *, struct vector *));
void shout_string PROT((char *));

void error_needs_free PROT((char *));
void throw_error PROT((void));

#ifdef SIGNAL_FUNC_TAKES_INT
void startshutdownMudOS PROT((int));
#else
void startshutdownMudOS PROT((void));

#endif
void shutdownMudOS PROT((int));
void slow_shut_down PROT((int));

/*
 * ed.c
 */
void ed_start PROT((char *, char *, char *, int, struct object *));
void ed_cmd PROT((char *));
void save_ed_buffer PROT((void));
void regerror PROT((char *));

/*
 * hash.c
 */
int hashstr PROT((char *, int, int));
int whashstr PROT((char *, int));

/*
 * call_out.c
 */
void call_out PROT((void));
void new_call_out PROT((struct object *, char *, int, int, struct svalue *));
int remove_call_out PROT((struct object *, char *));
void remove_all_call_out PROT((struct object *));
int find_call_out PROT((struct object *, char *));
struct vector *get_all_call_outs PROT((void));
int print_call_out_usage PROT((int));
void count_ref_from_call_outs PROT((void));

/*
 * object.c
 */
extern struct object *previous_ob;
extern int tot_alloc_object;
extern int tot_alloc_object_size;
extern int save_svalue_depth;

void bufcat PROT((char **, char *));
void reference_prog PROT((struct program *, char *));
void free_prog PROT((struct program *, int));
int svalue_save_size PROT((struct svalue *));
void save_svalue PROT((struct svalue *, char **));
int restore_svalue PROT((char *, struct svalue *));
int save_object PROT((struct object *, char *, int));
char *save_variable PROT((struct svalue *));
int restore_object PROT((struct object *, char *, int));
void restore_variable PROT((struct svalue *, char *));
struct object *get_empty_object PROT((int));
void reset_object PROT((struct object *, int));
void reload_object PROT((struct object *));
void free_object PROT((struct object *, char *));
struct object *find_living_object PROT((char *, int));
int valid_hide PROT((struct object *));
int object_visible PROT((struct object *));
void set_living_name PROT((struct object *, char *));
void remove_living_name PROT((struct object *));
void stat_living_objects PROT((void));
int shadow_catch_message PROT((struct object *, char *));
void tell_npc PROT((struct object *, char *));
void tell_object PROT((struct object *, char *));

/*
 * otable.c
 */
void init_otable PROT((void));
void enter_object_hash PROT((struct object *));
void remove_object_hash PROT((struct object *));
struct object *lookup_object_hash PROT((char *));
int show_otable_status PROT((int));

/*
 * compiler.pre
 */
void yyerror PROT((char *));
void yywarn PROT((char *));
void compile_file PROT((FILE *));
void store_line_number_info PROT((void));
int get_id_number PROT((void));
void save_include PROT((char *name));
char *the_file_name PROT((char *));
int add_local_name PROT((char *, int));
void pop_include PROT((void));

/*
 * compiler_shared.c
 */
extern struct mem_block mem_block[];
extern int exact_types;
extern int approved_object;
extern int current_type;
extern int current_block;
extern struct program NULL_program;
extern struct program *prog;
extern unsigned char string_tags[0x20];
extern short freed_string;
extern struct ident_hash_elem *locals[];
extern unsigned short type_of_locals[];
extern int current_number_of_locals;
extern int current_break_stack_need;
extern int max_break_stack_need;
extern unsigned short a_functions_root;
extern struct mem_block type_of_arguments;

char *get_two_types PROT((int, int));
char *get_type_name PROT((int));

void free_all_local_names PROT((void));
void copy_variables PROT((struct program *, int));
int copy_functions PROT((struct program *, int));
void type_error PROT((char *, int));
int compatible_types PROT((int, int));
void add_arg_type PROT((unsigned short));
int find_in_table PROT((struct function *, int));
void find_inherited PROT((struct function *));
int define_new_function PROT((char *, int, int, int, int, int));
int define_variable PROT((char *, int, int));
short store_prog_string PROT((char *));
void free_prog_string PROT((short));
#ifdef DEBUG
int dump_function_table PROT((void));
#endif

/*
 * this should be removed when switch support moves into icode.c
 * 
 * icode.c
 */
short read_short PROT((int));
void upd_short PROT((int, short));
INLINE void ins_f_byte PROT((unsigned int));
void ins_short PROT((short));
void ins_long PROT((int));
int pop_address PROT((void));

/*
 * interpret.c
 */
extern struct program *current_prog;
extern short caller_type;
extern char *pc;
extern struct svalue *sp;
extern struct svalue *fp;
extern short *break_sp;
extern struct svalue catch_value;
extern struct control_stack control_stack[30];
extern struct control_stack *csp;
extern int too_deep_error;
extern int max_eval_error;
extern unsigned int apply_low_call_others;
extern unsigned int apply_low_cache_hits;
extern unsigned int apply_low_slots_used;
extern unsigned int apply_low_collisions;
extern int function_index_offset;
extern int master_ob_is_loading;
extern struct function fake_func;
extern struct program fake_prog;

/* with LPC_TO_C off, these are defines using eval_instruction */
#ifdef LPC_TO_C
void call_program PROT((struct program *, int));
void call_absolute PROT((char *));
#endif
void eval_instruction PROT((char *p));
void assign_svalue PROT((struct svalue *, struct svalue *));
void assign_svalue_no_free PROT((struct svalue *, struct svalue *));
void copy_some_svalues PROT((struct svalue *, struct svalue *, int));
void transfer_push_some_svalues PROT((struct svalue *, int));
void push_some_svalues PROT((struct svalue *, int));
#ifdef DEBUG
void int_free_svalue PROT((struct svalue *, char *));
#else
void int_free_svalue PROT((struct svalue *));
#endif
void free_string_svalue PROT((struct svalue *));
void free_some_svalues PROT((struct svalue *, int));
void push_object PROT((struct object *));
void push_number PROT((int));
void push_real PROT((double));
void push_undefined PROT((void));
void push_null PROT((void));
void push_string PROT((char *, int));
void push_svalue PROT((struct svalue *));
void push_vector PROT((struct vector *));
void push_refed_vector PROT((struct vector *));
void push_buffer PROT((struct buffer *));
void push_refed_buffer PROT((struct buffer *));
void push_mapping PROT((struct mapping *));
void push_refed_mapping PROT((struct mapping *));
void push_malloced_string PROT((char *));
void push_constant_string PROT((char *));
void pop_stack PROT((void));
void pop_n_elems PROT((int));
void pop_2_elems PROT((void));
void pop_3_elems PROT((void));
void remove_object_from_stack PROT((struct object *));

void setup_fake_frame PROT((struct funp *));
void remove_fake_frame PROT((void));

char *type_name PROT((int c));
void bad_arg PROT((int, int));
void bad_argument PROT((struct svalue *, int, int, int));
void check_for_destr PROT((struct vector *));
int is_static PROT((char *, struct object *));
int apply_low PROT((char *, struct object *, int));
struct svalue *apply PROT((char *, struct object *, int, int));
struct svalue *safe_apply PROT((char *, struct object *, int, int));
struct vector *call_all_other PROT((struct vector *, char *, int));
char *function_exists PROT((char *, struct object *));
void call_function PROT((struct program *, struct function *));
struct svalue *apply_master_ob PROT((char *, int));
struct svalue *safe_apply_master_ob PROT((char *, int));
int assert_master_ob_loaded PROT((char *, char *));

char *add_slash PROT((char *));
int strpref PROT((char *, char *));
struct vector *get_svalue_trace PROT((void));
void do_trace PROT((char *, char *, char *));
char *dump_trace PROT((int));
void opcdump PROT((char *));
int inter_sscanf PROT((struct svalue *, struct svalue *, struct svalue *, int));
char * get_line_number_if_any PROT((void));
void get_version PROT((char *));
int reset_machine PROT((int));

#ifndef NO_SHADOWS
int validate_shadowing PROT((struct object *));
#endif

#ifdef LAZY_RESETS
void try_reset PROT((struct object *));
#endif

void push_pop_error_context PROT((int));
void pop_control_stack PROT((void));
struct function *setup_new_frame PROT((struct function *));
void push_control_stack PROT((struct function *));

#ifdef DEBUG
void check_a_lot_ref_counts PROT((struct program *));
#endif

/*
 * lex.c
 */
extern int current_line;
extern int total_lines;
extern char *current_file;
extern int pragmas;
extern int optimization;
extern struct lpc_predef_s *lpc_predefs;
extern int efun_arg_types[];
extern char yytext[1024];
extern struct instr instrs[];

int yylex PROT((void));
void init_num_args PROT((void));
char *query_instr_name PROT((int));
char *get_f_name PROT((int));
void set_inc_list PROT((char *));
void start_new_file PROT((FILE *));
void end_new_file PROT((void));
int lookup_predef PROT((char *));
void add_predefines PROT((void));
char *main_file_name PROT((void));
char *get_defined_name PROT((defined_name *));
struct ident_hash_elem *find_or_add_ident PROT((char *, int));
struct ident_hash_elem *find_or_add_perm_ident PROT((char *));
struct ident_hash_elem *lookup_ident PROT((char *));
void free_unused_identifiers PROT((void));
void init_identifiers PROT((void));

/*
 * binaries.c
 */
extern char driver_name[];

FILE *crdir_fopen PROT((char *));
void init_binaries PROT((int, char **));
int load_binary PROT((char *));
void save_binary PROT((struct program *, struct mem_block *, struct mem_block *));

/*
 * swap.c
 */
extern int total_num_prog_blocks;
extern int total_prog_block_size;

int swap PROT((struct object *));
int swap_line_numbers PROT((struct program *));
void load_ob_from_swap PROT((struct object *));
void load_line_numbers PROT((struct program *));
void remove_swap_file PROT((struct object *));
void unlink_swap_file PROT((void));
void remove_line_swap PROT((struct program *));
int locate_in PROT((struct program *));
int locate_out PROT((struct program *));
void print_swap_stats PROT((void));

/*
 * socket_ctrl.c
 */
int set_socket_nonblocking PROT((int, int));
int set_socket_owner PROT((int, int));
int set_socket_async PROT((int, int));

/*
 * socket_err.h
 */
extern char *error_strings[];

/*
 * efuns_main.c
 */
extern int using_bsd_malloc;
extern int using_smalloc;
extern int call_origin;

void print_cache_stats PROT((void));
int inherits PROT((struct program *, struct program *));
void add_mapping_pair PROT((struct mapping *, char *, int));
void add_mapping_string PROT((struct mapping *, char *, char *));
void add_mapping_object PROT((struct mapping *, char *, struct object *));
void add_mapping_array PROT((struct mapping *, char *, struct vector *));
void add_mapping_shared_string PROT((struct mapping *, char *, char *));

/*
 * eoperators.c
 */
struct funp *make_funp PROT((struct svalue *, struct svalue *));
void push_funp PROT((struct funp *));
void free_funp PROT((struct funp *));
struct svalue *call_function_pointer PROT((struct funp *, int));
int merge_arg_lists PROT((int, struct vector *, int));

/*
 * file.c
 */
int legal_path PROT((char *));
char *check_valid_path PROT((char *, struct object *, char *, int));
void smart_log PROT((char *, int, char *, int));
void dump_file_descriptors PROT((void));

char *read_file PROT((char *, int, int));
char *read_bytes PROT((char *, int, int, int *));
int write_file PROT((char *, char *, int));
int write_bytes PROT((char *, int, char *, int));
struct vector *get_dir PROT((char *, int));
int tail PROT((char *));
int file_size PROT((char *));
int copy_file PROT((char *, char *));
int do_rename PROT((char *, char *, int));
int remove_file PROT((char *));

/*
 * simul_efun.c
 */
extern struct object *simul_efun_ob;
extern char *simul_efun_file_name;
extern struct function **simuls;

void set_simul_efun PROT((char *));
void get_simul_efuns PROT((struct program *));
int find_simul_efun PROT((char *));

/*
 * array.c
 */
extern struct vector null_vector;
extern int num_arrays;
extern int total_array_size;

int sameval PROT((struct svalue *, struct svalue *));
struct vector *null_array PROT((void));
struct vector *allocate_array PROT((int));
void free_vector PROT((struct vector *));
struct vector *add_array PROT((struct vector *, struct vector *));
struct vector *subtract_array PROT((struct vector *, struct vector *));
struct vector *slice_array PROT((struct vector *, int, int));
struct vector *explode_string PROT((char *, char *));
char *implode_string PROT((struct vector *, char *));
struct vector *users PROT((void));
struct vector *commands PROT((struct object *));
struct vector *deep_inherit_list PROT((struct object *));
struct vector *inherit_list PROT((struct object *));
struct vector *children PROT((char *));
struct vector *livings PROT((void));
struct vector *objects PROT((struct funp *));
struct vector *all_inventory PROT((struct object *, int));
struct vector *deep_inventory PROT((struct object *, int));
struct vector *filter PROT((struct vector *, struct funp *, struct svalue *));
struct vector *builtin_sort_array PROT((struct vector *, int));
struct vector *fp_sort_array PROT((struct vector *, struct funp *));
struct vector *sort_array PROT((struct vector *, char *, struct object *));
struct vector *make_unique PROT((struct vector *, char *, struct funp *, struct svalue *));
void map_array PROT((struct svalue *arg, int num_arg));
struct vector *intersect_array PROT((struct vector *, struct vector *));
struct vector *match_regexp PROT((struct vector *, char *));

/*
 * mapping.c
 */
extern int num_mappings;
extern int total_mapping_size;
extern int total_mapping_nodes;

int mapping_save_size PROT((struct mapping *));
struct mapping *mapTraverse PROT((struct mapping *, int (*) (struct mapping *, struct node *, void *), void *));
struct mapping *load_mapping_from_aggregate PROT((struct svalue *, int));
struct mapping *allocate_mapping PROT((int));
void free_mapping PROT((struct mapping *));
struct svalue *find_in_mapping PROT((struct mapping *, struct svalue *));
struct svalue *find_for_insert PROT((struct mapping *, struct svalue *, int));
void absorb_mapping PROT((struct mapping *, struct mapping *));
void mapping_delete PROT((struct mapping *, struct svalue *));
struct mapping *add_mapping PROT((struct mapping *, struct mapping *));
void map_mapping PROT((struct svalue *, int));
struct mapping *compose_mapping PROT((struct mapping *, struct mapping *, unsigned short));
struct vector *mapping_indices PROT((struct mapping *));
struct vector *mapping_values PROT((struct mapping *));
struct vector *mapping_each PROT((struct mapping *));
char *save_mapping PROT((struct mapping *));

void add_mapping_pair PROT((struct mapping *, char *, int));
void add_mapping_string PROT((struct mapping *, char *, char *));
void add_mapping_malloced_string PROT((struct mapping *, char *, char *));
void add_mapping_object PROT((struct mapping *, char *, struct object *));
void add_mapping_array PROT((struct mapping *, char *, struct vector *));
void add_mapping_shared_string PROT((struct mapping *, char *, char *));

/*
 * buffer.c
 */
extern struct buffer null_buf;

struct buffer *null_buffer PROT((void));
void free_buffer PROT((struct buffer *));
struct buffer *allocate_buffer PROT((int));
int write_buffer PROT((struct buffer *, int, char *, int));
char *read_buffer PROT((struct buffer *, int, int, int *));

/*
 * stralloc.c
 */
void init_strings PROT((void));
char *findstring PROT((char *));
char *make_shared_string PROT((char *));
char *ref_string PROT((char *));
void free_string PROT((char *));
int add_string_status PROT((int));

/*
 * wrappedmalloc.c
 */
#ifdef WRAPPEDMALLOC
void *wrappedmalloc PROT((int));
void *wrappedrealloc PROT((void *, int));
void *wrappedcalloc PROT((int, int));
void wrappedfree PROT((void *));

void wrappedmalloc_init PROT((void));
void dump_malloc_data PROT((void));

#endif

/*
 * debugmalloc.c
 */
#ifdef DEBUGMALLOC
void *debugmalloc PROT((int, int, char *));
void *debugrealloc PROT((void *, int, int, char *));
void *debugcalloc PROT((int, int, int, char *));
void debugfree PROT((void *));

void debugmalloc_init PROT((void));
void dump_malloc_data PROT((void));

#ifdef DEBUGMALLOC_EXTENSIONS
void set_malloc_mask PROT((int));
void dump_debugmalloc PROT((char *, int));

#endif
#endif

/*
 * smalloc.c
 */
#ifdef DO_MSTATS
void show_mstats PROT((char *));

#endif

/*
 * sprintf.c
 */
void svalue_to_string PROT((struct svalue *, char **, int, int, int, int));
char *string_print_formatted PROT((char *, int, struct svalue *));

/*
 * parse.c
 */
int parse PROT((char *, struct svalue *, char *, struct svalue *, int));
char *process_string PROT((char *));
struct svalue *process_value PROT((char *));
char *break_string PROT((char *, int, struct svalue *));

/*
 * reclaim.c
 */
int reclaim_objects PROT((void));

/*
 * dumpstat.c
 */
int data_size PROT((struct object *));
void dumpstat PROT((char *));

/*
 * qsort.c
 */
void quickSort PROT((void *, int, int, int (*) ()));

/*
 * crc32.c
 */
UINT32 compute_crc32 PROT((unsigned char *, int));

/*
 * functab_tree.c
 */
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
int lookup_function PROT((struct function *, int, char *));
void add_function PROT((struct function *, unsigned short *, int));

#endif

/*
 * strstr.c
 */
char *_strstr PROT((char *, char *));

/*
 *  scratchpad.c
 */
void scratch_destroy PROT((void));
char *scratch_copy PROT((char *));
char *scratch_alloc PROT((int));
void scratch_free PROT((char *));
char *scratch_join PROT((char *, char *));
char *scratch_join2 PROT((char *, char *));
char *scratch_realloc PROT((char *, int));
char *scratch_copy_string PROT((char *));
char *scratch_large_alloc PROT((int));

/*
 * port.c
 */
int random_number PROT((int));
int get_current_time PROT((void));
char *time_string PROT((int));
void init_usec_clock PROT((void));
void get_usec_clock PROT((long *, long *));
int get_cpu_times PROT((unsigned long *, unsigned long *));
char *get_current_dir PROT((char *, int));

#endif
