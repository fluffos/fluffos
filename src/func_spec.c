#define _FUNC_SPEC_
#include "config.h"
#include "op_spec.c"
/*
 * This file specifies types and arguments for efuns.
 * An argument can have two different types with the syntax 'type1 | type2'.
 * An argument is marked as optional if it also takes the type 'void'.
 *
 * Comment out the efuns that you do not want.  Be careful not to comment
 * out something that you need.
 *
 * The order in which the efuns are listed here is the order in which opcode
 * #'s will be assigned.  It is in your interest to move the least frequently
 * used efuns to the bottomm of this file (and the most frequently used
 * ones to the top).  The opcprof() efun could help you find out which
 * efuns are most often and least often used.  The reason for ordering
 * the efuns is that only the first 255 efuns are represented using
 * a single byte.  Any additional efuns require two bytes.
 */

/* most frequently used functions */

unknown call_other(object|string|object *, string|mixed *, ...);
object present(object|string, void|object);
object this_object();
object this_player(void|int);
object new(string);
object clone_object new(string);
void move_object(object, void|object|string);
object previous_object();
int sizeof(mixed);
int destruct(object);
string file_name(object default: F_THIS_OBJECT);
object environment(void|object);
string capitalize(string);
string *explode(string, string);
string implode(string *, string);
object *all_inventory(object default: F_THIS_OBJECT);
void call_out(string, int, ...);
int member_array(mixed, mixed *, void|int);
int notify_fail(string);
void input_to(string, ...);
int random(int);
int save_object(string, void|int);
void add_action(string, string, void|int);
string query_verb();
string lower_case(string);
object first_inventory(object|string default: F_THIS_OBJECT);
object next_inventory(object default: F_THIS_OBJECT);
int command(string, void|object);
string replace_string(string, string, string, ...);
int restore_object(string, void|int);
object *users();
string *get_dir(string, int default: F_CONST0);
int strsrch(string, string|int, int default: F_CONST0);

/* communication functions */

void write(mixed);
void tell_object(object, string);
void say(string, void|object|object *);
void shout(string);
int receive(string);
void tell_room(object|string, string|object|int|float, void|object *);
void message(string, string, string|string *|object|object *,
             void|object|object *);

/* the find_* functions */

object find_object(string);
object find_living(string);
object find_player(string);
int find_call_out(string);

/* mapping functions */

mapping allocate_mapping(int);
void map_delete(mapping,mixed);
mixed *values(mapping);
mixed *keys(mapping);
#ifdef EACH
mixed *each(mapping, int default: F_CONST0);
#endif
mixed match_path(mapping, string);

/* all the *p() type functions */

int clonep(mixed);
int intp(mixed);
int undefinedp(mixed);
int nullp(mixed);
int floatp(mixed);
int stringp(mixed);
int virtualp(object);
int functionp(mixed);
int pointerp(mixed);
int objectp(mixed);
#ifndef DISALLOW_BUFFER_TYPE
int bufferp(mixed);
#endif

int inherits(string, object);
void replace_program(string);

#ifndef DISALLOW_BUFFER_TYPE
buffer allocate_buffer(int);
#endif
string *regexp(string *, string);
mixed *allocate(int);

/* do not remove to_int() and to_float() because they are also used by
   the compiler (compiler.y)
*/
int to_int(string|float|int|buffer);
float to_float(string|float|int);

mixed *call_out_info();

/* 32-bit cyclic redundancy code - see crc32.c and crctab.h */
int crc32(string|buffer);

/* commands operating on files */

#ifndef DISALLOW_BUFFER_TYPE
mixed read_buffer(string|buffer, void|int, void|int);
#endif
int write_file(string, string);
int rename(string, string);
int write_bytes(string, int, string);
#ifndef DISALLOW_BUFFER_TYPE
int write_buffer(string|buffer, int, string|buffer|int);
#endif
int file_size(string);
string read_bytes(string, void|int, void|int);
string read_file(string, void|int, void|int);
int cp(string, string);
#ifndef LATTICE
int link(string, string);
#endif
int mkdir(string);
int rm(string);
void rmdir(string);

/* the bit string functions */

string clear_bit(string, int);
int test_bit(string, int);
string set_bit(string, int);

string crypt(string, string|int);	/* An int as second argument ? */
string ctime(int);
mixed debug_info(int, object);
void disable_commands();
void enable_commands();
int exec(object, object);
mixed *localtime(int);
string function_exists(string, object default: F_THIS_OBJECT);

object *livings();
object *objects(void|string, void|object);
string process_string(string);
mixed process_value(string);
string break_string(int|string, int, void|int|string);
string query_host_name();
int query_idle(object);
string query_ip_name(void|object);
string query_ip_number(void|object);
object query_snoop(object);
object query_snooping(object);
int remove_call_out(string);
int set_heart_beat(int);
int query_heart_beat(object default: F_THIS_OBJECT);
void set_hide(int);

void set_living_name(string);
void set_reset(object, void|int);
#ifndef NO_SHADOWS 
object shadow(object, int);
object query_shadowing(object);
#endif
object snoop(object, void|object);
mixed *sort_array(mixed *,string,object|string default: F_THIS_OBJECT);
void tail(string);
void throw(mixed);
int time();
mixed *unique_array(mixed *, string, void|mixed);
string *deep_inherit_list(object default: F_THIS_OBJECT);
string *inherit_list(object default: F_THIS_OBJECT);
int strlen(string);
void printf(string, ...);
string sprintf(string, ...);
int mapp(mixed);
mixed *stat(string, int default: F_CONST0);
int remove_action(string, string);

/*
 * Object properties
 */
int living(object);
int interactive(object default: F_THIS_OBJECT);
int in_edit(object default : F_THIS_OBJECT);
int in_input(object default : F_THIS_OBJECT);
mixed *commands();
void enable_wizard();
void disable_wizard();
int userp(object);
int wizardp(object);

object master();

/*
 * mudlib statistics
 */
mapping domain_stats(void|string);
void set_author(string);
mapping author_stats(void|string);
int memory_info(object|void);

/* uid functions */

int export_uid(object);
string geteuid(function|object default: F_THIS_OBJECT);
string getuid(object default: F_THIS_OBJECT);
int seteuid(string|int);

#ifdef PRIVS
/* privledge functions */
string query_privs(object default: F_THIS_OBJECT);
void set_privs(object, int|string);
#endif /* PRIVS */

void get_char(string, void|int);
object *children(string);

void reload_object(object);

#ifdef SOCKET_EFUNS
/*
 * socket efuns
 */
int socket_create(int, string, string|void);
int socket_bind(int, int);
int socket_listen(int, string);
int socket_accept(int, string, string);
int socket_connect(int, string, string, string);
int socket_write(int, mixed, string|void);
int socket_close(int);
int socket_release(int, object, string);
int socket_acquire(int, string, string, string);
string socket_error(int);
string socket_address(int|object);
void dump_socket_status();
#endif /* SOCKET_EFUNS */

void error(string);
int errorp(mixed);
int uptime();
int strcmp(string, string);

#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES))
mapping rusage();
#endif /* RUSAGE */

void ed(string|void, string|void, int|void);

#ifdef MATH
float cos(float);
float sin(float);
float tan(float);
float asin(float);
float acos(float);
float atan(float);
float sqrt(float);
float log(float);
float pow(float, float);
float exp(float);
float floor(float);
float ceil(float);
#endif

/*
 * Matrix efuns for Jacques' 3d mud etc.
 */
#ifdef MATRIX
float *id_matrix();
float *translate(float *, float, float, float);
float *scale(float *, float, float, float);
float *rotate_x(float *, float);
float *rotate_y(float *, float);
float *rotate_z(float *, float);
float *lookat_rotate(float *, float, float, float);
float *lookat_rotate2(float *, float, float, float, float, float, float);
#endif /* MATRIX */

#ifdef CACHE_STATS
void cache_stats();
#endif

object *deep_inventory(object);

/*
 * MIRE efuns for the MIRE project at MIT
 */

#ifdef MIRE
int *editor_list (string, int);
int remove_editor_list (string);
string *fetch_article (int);
void doppel_mod (string, string, int);
mixed *find_keywords (string);
void init_mire();
#endif

mixed filter_array(mixed *, string, object|string, void|mixed);
mixed *map_array(mixed *, string, object|string, void|mixed);

/*
 * parser 'magic' functions, turned into efuns
 */
void malloc_status();
void mud_status(int default : F_CONST0);
void dumpallobj(string|void);

void dump_file_descriptors();
string query_load_average();

/* set_light should die a dark death */
int set_light(int);

int origin();

/* the infrequently used functions */

int reclaim_objects();
int refs(mixed);

#if defined(PROFILING) && defined(HAS_MONCONTROL)
void moncontrol(int);
#endif

/* dump_prog: disassembler... comment out this line if you don't want the
   disassembler compiled in.
*/
void dump_prog(object, ...);

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
void debugmalloc(string,int);
void set_malloc_mask(int);
#endif

void set_eval_limit(int);

#ifdef DEBUG_MACRO
void set_debug_level(int);
#endif

#ifdef OPCPROF
void opcprof(string|void);
#endif

#ifdef PROFILE_FUNCTIONS
mapping *function_profile(object default: F_THIS_OBJECT);
#endif

#ifdef TRACE
string traceprefix(string|int);
int trace(int);
#endif

void swap(object);		/* Only used for debugging */

/* shutdown is at the end because it is only called once per boot cycle :) */
void shutdown(void|int);

int resolve(string, string);
#if defined(NeXT) && defined(NEXT_MALLOC_DEBUG)
int malloc_check();
int malloc_debug(int);
#endif
