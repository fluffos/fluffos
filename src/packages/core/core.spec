/*
 * This file specifies types and arguments for efuns.
 * An argument can have two different types with the syntax 'type1 | type2'.
 * An argument is marked as optional if it also takes the type 'void'.
 *
 */

#define OR_BUFFER | buffer

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
unknown _call_other(object | string | object *, string | mixed *, ...);
/* used by (*f)(...) */
mixed _evaluate(mixed, ...);
/* default argument for some efuns */
object _this_object();
/* used for implicit float/int conversions */
int _to_int(string | float | int OR_BUFFER);
float _to_float(string | float | int);
/* used by new() */
object _new(string, ...);

unknown call_other _call_other(object | string | object *, string | mixed *, ...);
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
object this_interactive this_player(int default: 1);
object this_user this_player(int default: 0);
mixed previous_object(int default: 0);
object *all_previous_objects previous_object(int default: -1);
mixed *call_stack(int default: 0);
int sizeof(mixed);
int strlen sizeof(string);
void destruct(object default: F__THIS_OBJECT);
string file_name(object default: F__THIS_OBJECT);
string capitalize(string);
string *explode(string, string);
mixed implode(mixed *, string | function, void | mixed);

int call_out(string | function, int|float, ...);
int call_out_walltime(string | function, int|float, ...);
mixed *call_out_info();
int find_call_out(int | string);
int remove_call_out(int | void | string);

int member_array(mixed, string | mixed *, void | int, void | int);
int input_to(string | function, ...);
int random(int);
int secure_random(int);
void defer(function);

#ifndef NO_ENVIRONMENT
object environment(void | object);
object *all_inventory(object default: F__THIS_OBJECT);
object *deep_inventory(object | object * | function, void | function default: F__THIS_OBJECT);
object first_inventory(object | string default: F__THIS_OBJECT);
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
void enable_commands(int default: 0);
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
string replace_string(string, string, string, ...);
int restore_object(string, void | int);
mixed save_object(string | int | void, void | int);
string save_variable(mixed);
mixed restore_variable(string);
object* users();
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

int bufferp(mixed);
buffer allocate_buffer(int);

int inherits(string, object default: F__THIS_OBJECT);
void replace_program(string);

mixed regexp(string | string *, string, void | int);
mixed *reg_assoc(string, string *, mixed *, mixed | void);
mixed *allocate(int, void | mixed);


/* 32-bit cyclic redundancy code - see crc32.c and crctab.h */
int crc32(string OR_BUFFER);

/* commands operating on files */

mixed read_buffer(string | buffer, void | int, void | int);
int write_buffer(string | buffer, int, string | buffer | int);
int write_file(string, string, int default:0);
int rename(string, string);
int write_bytes(string, int, string);

int file_size(string);
string read_bytes(string, void | int, void | int);
string read_file(string, void | int, void | int);
int cp(string, string);
#ifndef _WIN32
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

string crypt(string, string | int);
string oldcrypt(string, string | int);

string ctime(int | void);
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
void printf(string, ...);
string sprintf(string, ...);
int mapp(mixed);
mixed *stat(string, int default: 0);

/*
 * Object properties
 */
int interactive(object default:F__THIS_OBJECT);
int has_mxp(object default:F__THIS_OBJECT);
int has_zmp(object default:F__THIS_OBJECT);
void send_zmp(string, string *);
int has_gmcp(object default:F__THIS_OBJECT);
void send_gmcp(string);
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

/* Runtime configs */
mixed get_config(int);
void set_config(int, mixed);

#ifdef PRIVS
/* privledge functions */
string query_privs(object default:F__THIS_OBJECT);
void set_privs(object, int | string);
#endif                          /* PRIVS */

int get_char(string | function, ...);
object *children(string);

void reload_object(object);

void error(string);
#ifdef COMPAT_32
void raise_error error(string);
#endif
int uptime();
int strcmp(string, string);

mapping rusage();

void flush_messages(void | object);

#ifdef OLD_ED
void ed(string | void, string | int | void, string | int | void, int | void, int | void);
#else
string ed_start(string | void, int | void, int | void);
string ed_cmd(string);
int query_ed_mode();
#endif

string cache_stats();
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

void set_debug_level(int | string);
mapping debug_levels();
void clear_debug_level(string);

#ifdef PROFILE_FUNCTIONS
mapping *function_profile(object default:F__THIS_OBJECT);
#endif

int resolve(string, string | function);
string set_encoding(string | void);
string query_encoding();
string string_decode(buffer, string);
buffer string_encode(string, string);
buffer buffer_transcode(buffer, string, string);

void act_mxp();
void request_term_type();
void start_request_term_type();
void request_term_size(void | int);
void telnet_nop();

/* shutdown is at the end because it is only called once per boot cycle :) */
void shutdown(void | int);
// Get current LPC stacktrace
mixed* dump_trace();
// Get display width of given string
int strwidth(string);

// start to collect tracing data
void trace_start(string, int default: 10);
// stop to collect tracing data right away.
void trace_end();

// return highest resolution clock in nanoseconds
int perf_counter_ns();
