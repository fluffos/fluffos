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

#ifdef NO_BUFFER_TYPE
#define OR_BUFFER
#else
#define OR_BUFFER | buffer
#endif

/* These next few efuns are used internally; do not remove them.
 * The leading _ is used to keep track of which efuns should exist,
 * but not be added to the identifier lookup table.
 * These names MUST exist, and may either be real efuns, or aliases
 * for another efun.  For example, one could remove clone_object
 * above, and change the internal one to:
 *
 * object _new(string, ...);
 */
/* used by X->f() */
unknown _call_other(object | string | object *, string | mixed *,...);
/* used by (*f)(...) */
mixed _evaluate(mixed, ...);
/* default argument for some efuns */
object _this_object();
/* used for implicit float/int conversions */
int _to_int(string | float | int OR_BUFFER);
float _to_float(string | float | int);
/* used by new() */
object _new(string, ...);

unknown call_other _call_other(object | string | object *, string | mixed *,...);
mixed evaluate _evaluate(mixed, ...);
#ifdef COMPAT_32
mixed funcall _evaluate(mixed, ...);
#endif
object this_object _this_object();
int to_int _to_int(string | float | int OR_BUFFER);
float to_float _to_float(string | float | int);
object clone_object _new(string, ...);

function bind(function, object);
object this_player(int default: 0);
object this_interactive this_player( int default: 1);
object this_user this_player( int default: 0);
mixed previous_object(int default: 0);
object *all_previous_objects previous_object(int default: -1);
mixed *call_stack(int default: 0);
int sizeof(mixed);
int strlen sizeof(string);
#ifdef USE_ICONV
int strwidth(string);
#else
int strwidth sizeof(string);
#endif
void destruct(object default: F__THIS_OBJECT);
string file_name(object default: F__THIS_OBJECT);
string capitalize(string);
string *explode(string, string);
mixed implode(mixed *, string | function, void | mixed);
#ifdef CALLOUT_HANDLES
int call_out(string | function, int,...);
#else
void call_out(string | function, int,...);
#endif
int member_array(mixed, string | mixed *, void | int, void | int);
int input_to(string | function,...);
int random(int);

#ifndef NO_ENVIRONMENT
object environment(void | object);
object *all_inventory(object default: F__THIS_OBJECT);
object *deep_inventory(object default: F__THIS_OBJECT);
object first_inventory(object|string default: F__THIS_OBJECT);
object next_inventory(object default: F__THIS_OBJECT);
void say(string, void | object | object *);
void tell_room(object | string, string | object | int | float, void | object | object *);
object present(object | string, void | object);
void move_object(object | string);
#endif

#ifndef NO_ADD_ACTION
void add_action(string | function, string | string *, void | int);
string query_verb();
int command(string);
int remove_action(string, string);
int living(object default: F__THIS_OBJECT);
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
void set_this_user set_this_player(object | int);
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
#ifdef COMPAT_32
int strstr strsrch(string, string | int, int default: 0);
#endif

/* communication functions */

void write(mixed);
void tell_object(object, string);
void shout(string);
void receive(string OR_BUFFER);
void message(mixed, mixed, string | string * | object | object *,
                  void | object | object *);

/* the find_* functions */

    object find_object(string, int default: 0);
    object load_object find_object(string, int default: 1);
#ifdef CALLOUT_HANDLES
    int find_call_out(int|string);
#else
    int find_call_out(string);
#endif

/* mapping functions */

    mapping allocate_mapping(int | mixed *, void | mixed);
    mixed *values(mapping);
    mixed *keys(mapping);
#ifdef COMPAT_32
    mapping map_delete(mapping, mixed);
    mapping m_delete map_delete(mapping, mixed);
    mixed *m_values values(mapping);
    mixed *m_indices keys(mapping);
#else
    void map_delete(mapping, mixed);
#endif

    mixed match_path(mapping, string);

/* all the *p() type functions */

    int clonep(mixed default: F__THIS_OBJECT);
    int intp(mixed);
    int undefinedp(mixed);
    int nullp undefinedp(mixed);
    int floatp(mixed);
    int stringp(mixed);
    int virtualp(object default: F__THIS_OBJECT);
    int functionp(mixed);
#ifdef COMPAT_32
    int closurep functionp(mixed);
#endif
    int pointerp(mixed);
    int arrayp pointerp(mixed);
    int objectp(mixed);
    int classp(mixed);
    string typeof(mixed);

#ifndef NO_BUFFER_TYPE
    int bufferp(mixed);
    buffer allocate_buffer(int);
#endif

    int inherits(string, object default: F__THIS_OBJECT);
    void replace_program(string);

    mixed regexp(string | string *, string, void | int);
    mixed *reg_assoc(string, string *, mixed *, mixed | void);
    mixed *allocate(int, void | mixed);
    mixed *call_out_info();

/* 32-bit cyclic redundancy code - see crc32.c and crctab.h */
    int crc32(string OR_BUFFER);

/* commands operating on files */

#ifndef NO_BUFFER_TYPE
    mixed read_buffer(string | buffer, void | int, void | int);
    int write_buffer(string | buffer, int, string | buffer | int);
#endif
    int write_file(string, string, void | int);
    int rename(string, string);
    int write_bytes(string, int, string);

    int file_size(string);
    string read_bytes(string, void | int, void | int);
    string read_file(string, void | int, void | int);
    int cp(string, string);

    int link(string, string);
    int mkdir(string);
    int rm(string);
    int rmdir(string);

/* the bit string functions */

    string clear_bit(string, int);
    int test_bit(string, int);
    string set_bit(string, int);
    int next_bit(string, int);

    string crypt(string, string | int);
    string oldcrypt(string, string | int);
   
    string ctime(int|void);
    int exec(object, object);
    mixed *localtime(int);
    string function_exists(string, void | object, void | int);

    object *objects(void | string | function);
    string query_host_name();
    int query_idle(object);
    string query_ip_name(void | object);
    string query_ip_number(void | object);
#ifndef NO_SNOOP
    object snoop(object, void | object);
    object query_snoop(object);
    object query_snooping(object);
#endif
#ifdef CALLOUT_HANDLES
    int remove_call_out(int | void | string);
#else
    int remove_call_out(void | string);
#endif
    void set_heart_beat(int);
    int query_heart_beat(object default:F__THIS_OBJECT);
    void set_hide(int);

#ifndef NO_RESETS
    void set_reset(object, void | int);
#endif

#ifndef NO_SHADOWS
    object shadow(object, int default: 1);
    object query_shadowing(object);
#endif
    mixed *sort_array(mixed *, int | string | function, ...);
    void throw(mixed);
    int time();
    mixed *unique_array(mixed *, string | function, void | mixed);
    mapping unique_mapping(mixed *, string | function, ...);
    string *deep_inherit_list(object default:F__THIS_OBJECT);
    string *shallow_inherit_list(object default:F__THIS_OBJECT);
#ifdef COMPAT_32
    string *inherit_list deep_inherit_list(object default:F__THIS_OBJECT);
#else
    string *inherit_list shallow_inherit_list(object default:F__THIS_OBJECT);
#endif
    void printf(string,...);
    string sprintf(string,...);
    int mapp(mixed);
    mixed *stat(string, int default: 0);

/*
 * Object properties
 */
    int interactive(object default:F__THIS_OBJECT);
    int has_mxp(object default:F__THIS_OBJECT);
    string in_edit(object default:F__THIS_OBJECT);
    int in_input(object default:F__THIS_OBJECT);
    int userp(object);
#ifdef COMPAT_32
    int query_once_interactive userp(object);
#endif

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
    string query_privs(object default:F__THIS_OBJECT);
    void set_privs(object, int | string);
#endif                          /* PRIVS */

    int get_char(string | function,...);
    object *children(string);

    void reload_object(object);

    void error(string);
#ifdef COMPAT_32
    void raise_error error(string);
#endif
    int uptime();
    int strcmp(string, string);

#ifndef WIN32
#if (defined(RUSAGE) || defined(GET_PROCESS_STATS) || defined(TIMES))
    mapping rusage();
#endif                          /* RUSAGE */
#endif

    void flush_messages(void | object);

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

    mixed filter(string | mixed * | mapping, string | function, ...);
    mixed filter_array filter(mixed *, string | function, ...);
    mapping filter_mapping filter(mapping, string | function, ...);

    mixed map(string | mapping | mixed *, string | function, ...);
    mapping map_mapping map(mapping, string | function, ...);
    mixed *map_array map(mixed *, string | function, ...);
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
#ifdef COMPAT_32
    int get_eval_cost set_eval_limit(int default: -1);
#endif

#ifdef DEBUG_MACRO
    void set_debug_level(int|string);
    mapping debug_levels();
    void clear_debug_level(string);
#endif

#if defined(OPCPROF) || defined(OPCPROF_2D)
    void opcprof(string | void);
#endif

#ifdef PROFILE_FUNCTIONS
    mapping *function_profile(object default:F__THIS_OBJECT);
#endif

    int resolve(string, string|function);
#ifdef USE_ICONV
    int set_encoding(string);
    string to_utf8(string, string);
    string utf8_to(string, string);
    int *str_to_arr(string);
    string arr_to_str(int *);
#endif
    void act_mxp();
    void request_term_type();
    void start_request_term_type();
    void request_term_size();
/* shutdown is at the end because it is only called once per boot cycle :) */
    void shutdown(void | int);
