#include "spec.h"

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
 * used efuns to the bottom of this file (and the most frequently used
 * ones to the top).  The opcprof() efun could help you find out which
 * efuns are most often and least often used.  The reason for ordering
 * the efuns is that only the first 255 efuns are represented using
 * a single byte.  Any additional efuns require two bytes.
 */

/* most frequently used functions */

object clone_object(string, ...);
unknown call_other(object | string | object *, string | mixed *,...);
mixed evaluate(mixed, ...);
function bind(function, object);
object present(object | string, void | object);
object this_object();
object this_player(int default: 0);
object this_interactive this_player( int default: 1);
object this_user this_player( int default: 0);
void move_object(object | string);
mixed previous_object(int default: 0);
object *all_previous_objects previous_object(int default: -1);
mixed *call_stack(int default: 0);
int sizeof(mixed);
int strlen sizeof(string);
void destruct(object);
string file_name(object default: F_THIS_OBJECT);
object environment(void | object);
string capitalize(string);
string *explode(string, string);
mixed implode(mixed *, string | function, void | mixed);
object *all_inventory(object default: F_THIS_OBJECT);
object first_inventory(object|string default: F_THIS_OBJECT);
object next_inventory(object default: F_THIS_OBJECT);
void call_out(string | function, int,...);
int member_array(mixed, string | mixed *, void | int);
int input_to(string | function,...);
int random(int);

#ifndef NO_ADD_ACTION
void add_action(string | function, string | string *, void | int);
string query_verb();
int command(string);
int remove_action(string, string);
int living(object);
mixed *commands();
void disable_commands();
void enable_commands();
void set_living_name(string);
object *livings();
object find_living(string);
object find_player(string);
void notify_fail(string | function);
#else
void set_this_player(object | int);
#endif

string lower_case(string);
string replace_string(string, string, string,...);
int restore_object(string, void | int);
int save_object(string, void | int);
string save_variable(mixed);
mixed restore_variable(string);
object *users();
mixed *get_dir(string, int default: 0);
int strsrch(string, string | int, int default: 0);

/* communication functions */

void write(mixed);
void tell_object(object, string);
void say(string, void | object | object *);
void shout(string);
void receive(string);
void tell_room(object | string, string | object | int | float, void | object *);
void message(mixed, string, string | string * | object | object *,
	          void | object | object *);

/* the find_* functions */

    object find_object(string, int default: 0);
    object load_object find_object(string, int default: 1);
    int find_call_out(string);

/* mapping functions */

    mapping allocate_mapping(int);
    void map_delete(mapping, mixed);
    mixed *values(mapping);
    mixed *keys(mapping);

#ifdef EACH
    mixed *each(mapping, int default: 0);
#endif
    mixed match_path(mapping, string);

/* all the *p() type functions */

    int clonep(mixed default: F_THIS_OBJECT);
    int intp(mixed);
    int undefinedp(mixed);
    int nullp(mixed);
    int floatp(mixed);
    int stringp(mixed);
    int virtualp(object);
    int functionp(mixed);
    int pointerp(mixed);
    int arrayp pointerp(mixed);
    int objectp(mixed);
    string typeof(mixed);

#ifndef DISALLOW_BUFFER_TYPE
    int bufferp(mixed);
#endif

    int inherits(string, object);
    void replace_program(string);

#ifndef DISALLOW_BUFFER_TYPE
    buffer allocate_buffer(int);
#endif
    mixed regexp(string | string *, string, void | int);
    mixed *reg_assoc(string, string *, mixed *, mixed | void);
    mixed *allocate(int);

/* do not remove to_int(), to_float() because they are also used by
   the compiler (compiler.pre)
*/
    int to_int(string | float | int | buffer);
    float to_float(string | float | int);

    mixed *call_out_info();

/* 32-bit cyclic redundancy code - see crc32.c and crctab.h */
    int crc32(string | buffer);

/* commands operating on files */

#ifndef DISALLOW_BUFFER_TYPE
    mixed read_buffer(string | buffer, void | int, void | int);
#endif
    int write_file(string, string, void | int);
    int rename(string, string);
    int write_bytes(string, int, string);

#ifndef DISALLOW_BUFFER_TYPE
    int write_buffer(string | buffer, int, string | buffer | int);
#endif
    int file_size(string);
    string read_bytes(string, void | int, void | int);
    string read_file(string, void | int, void | int);
    int cp(string, string);

#if !defined(LATTICE) && !defined(OS2)
    int link(string, string);
#endif
    int mkdir(string);
    int rm(string);
    int rmdir(string);

/* the bit string functions */

    string clear_bit(string, int);
    int test_bit(string, int);
    string set_bit(string, int);
    int next_bit(string, int);

    string crypt(string, string | int);	/* An int as second argument ? */
    string ctime(int);
    int exec(object, object);
    mixed *localtime(int);
    string function_exists(string, object default:F_THIS_OBJECT);

    object *objects(void | string | function, void | object);
    string process_string(string);
    mixed process_value(string);
    string break_string(int | string, int, void | int | string);
    string query_host_name();
    int query_idle(object);
    string query_ip_name(void | object);
    string query_ip_number(void | object);
    object query_snoop(object);
    object query_snooping(object);
    int remove_call_out(void | string);
    void set_heart_beat(int);
    int query_heart_beat(object default:F_THIS_OBJECT);
    void set_hide(int);

#ifdef LPC_TO_C
    int generate_source(string | string *, void | string);
    string lpc_info();
#endif

    void set_reset(object, void | int);

#ifndef NO_SHADOWS
    object shadow(object, int default: 1);
    object query_shadowing(object);
#endif
    object snoop(object, void | object);
    mixed *sort_array(mixed *, int | string | function, void | object | string);
    int tail(string);
    void throw(mixed);
    int time();
    mixed *unique_array(mixed *, string | function, void | mixed);
    mapping unique_mapping(mixed *, string | function, string | object | void, ...);
    string *deep_inherit_list(object default:F_THIS_OBJECT);
    string *inherit_list(object default:F_THIS_OBJECT);
    void printf(string,...);
    string sprintf(string,...);
    int mapp(mixed);
    mixed *stat(string, int default: 0);

/*
 * Object properties
 */
    int interactive(object default:F_THIS_OBJECT);
    string in_edit(object default:F_THIS_OBJECT);
    int in_input(object default:F_THIS_OBJECT);
    int userp(object);

#ifndef NO_WIZARDS
    void enable_wizard();
    void disable_wizard();
    int wizardp(object);
#endif

    object master();

/*
 * various mudlib statistics
 */
    int memory_info(object | void);
    mixed get_config(int);

#ifdef PRIVS
/* privledge functions */
    string query_privs(object default:F_THIS_OBJECT);
    void set_privs(object, int | string);
#endif				/* PRIVS */

    int get_char(string | function,...);
    object *children(string);

    void reload_object(object);

    void error(string);
    int uptime();
    int strcmp(string, string);

#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES)) || defined(LATTICE)
    mapping rusage();
#endif				/* RUSAGE */

#ifdef OLD_ED
    void ed(string | void, string | void, string | int | void, int | void);
#else
    string ed_start(string | void, int | void);
    string ed_cmd(string);
    int query_ed_mode();
#endif

#ifdef CACHE_STATS
    string cache_stats();
#endif

    object *deep_inventory(object);

    mixed filter(mixed * | mapping, string | function, mixed | void, ...);
    mixed filter_array filter(mixed *, string | function, mixed | void, ...);
    mapping filter_mapping filter(mapping, string | function, mixed | void, ...);

    mixed map(string | mapping | mixed *, string | function, mixed | void, ...);
    mapping map_mapping map(mapping, string | function, mixed | void, ...);
    mixed *map_array map(mixed *, string | function, mixed | void, ...);
/*
 * parser 'magic' functions, turned into efuns
 */
    string malloc_status();
    string mud_status(int default: 0);
    void dumpallobj(string | void);

    string dump_file_descriptors();
    string query_load_average();

#ifndef NO_LIGHT
/* set_light should die a dark death */
    int set_light(int);
#endif

    string origin();

/* the infrequently used functions */

    int reclaim_objects();

    int set_eval_limit(int);
    int reset_eval_cost set_eval_limit(int default: 0);
    int eval_cost set_eval_limit(int default: -1);
    int max_eval_cost set_eval_limit(int default: 1);

#ifdef DEBUG_MACRO
    void set_debug_level(int);
#endif

#if defined(OPCPROF) || defined(OPCPROF_2D)
    void opcprof(string | void);
#endif

#ifdef PROFILE_FUNCTIONS
    mapping *function_profile(object default:F_THIS_OBJECT);
#endif

#ifdef DEBUG
    void swap(object);		/* Only used for debugging */
#endif
    int resolve(string, string);

/* shutdown is at the end because it is only called once per boot cycle :) */
    void shutdown(void | int);
