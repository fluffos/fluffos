#include "config.h"
/*
 * This file specifies types and arguments for efuns.
 * An argument can have two different types with the syntax 'type1 | type2'.
 * An argument is marked as optional if it also takes the type 'void'.
 *
 * Look at the end for the list of functions that are optionally available.
 * If you don't want them, simply comment out them. All other functions must
 * remain defined.
 */

string *regexp(string *, string);
void add_action(string, void|string, void|int);
void add_verb(string);
void add_worth(int, void|object);
void add_xverb(string);
object *all_inventory(object default: F_THIS_OBJECT);
mixed *allocate(int);
void break_point();
unknown call_other(object|string|object *, string, ...);
void call_out(string, int, ...);
mixed *call_out_info();
string capitalize(string);
int cat(string, void|int, void|int);
string clear_bit(string, int);
object clone_object(string);
int clonep(mixed);
int command(string);
string crypt(string, string|int);	/* An int as second argument ? */
string ctime(int);
mixed debug_info(int, object);
object *deep_inventory(object);
int destruct(object);
void disable_commands();
void enable_commands();
object environment(void|object);
int exec(object, object);
string *explode(string, string);
string extract(string, void|int, void|int);
string file_name(object default: F_THIS_OBJECT);
int file_size(string);
mixed filter_array(mixed *, string, object|string, void|mixed);
int find_call_out(string);
object find_living(string);
mixed *localtime(int);
object find_object(string);
object find_player(string);
string function_exists(string, object default: F_THIS_OBJECT);
string implode(string *, string);
mixed *keys(mapping);
mixed *values(mapping);
#ifdef EACH
mixed *each(mapping);
#endif
void input_to(string, ...);
int interactive(object default: F_THIS_OBJECT);
int intp(mixed);
int functionp(mixed);
int living(object);
object *livings();
void log_file(string, string);
string lower_case(string);
string *get_dir(string, int default: F_CONST0);
mixed *map_array(mixed *, string, object|string, void|mixed);
int member_array(mixed, mixed *);
int mkdir(string);
void move_object(object|string, void|object|string);
void notify_fail(string);
int objectp(mixed);
int pointerp(mixed);
object present(object|string, void|object);
object previous_object();
string process_string(string);
string query_host_name();
int query_idle(object);
string query_ip_name(void|object);
string query_ip_number(void|object);
string query_load_average();
object query_snoop(object);
string query_verb();
int random(int);
string read_bytes(string, void|int, void|int);
string read_file(string, void|int, void|int);
int remove_call_out(string);
string replace_string(string, string, string);
int restore_object(string, void|int);
int rm(string);
void rmdir(string);
void save_object(string, void|int);
void say(string, void|object|object *);
string set_bit(string, int);
int set_heart_beat(int);
void set_hide(int);
int set_light(int);
void set_living_name(string);
#ifndef NO_SHADOWS 
object shadow(object, int);
#endif
void shout(string);
void shutdown(void|int);
int sizeof(mixed);
object snoop(object, void|object);
mixed *sort_array(mixed *,string,object|string default: F_THIS_OBJECT);
int stringp(mixed);
int strlen(string);
void swap(object);		/* Only used for debugging */
void tail(string);
void tell_object(object, string);
void tell_room(object|string, string|object|int, void|object *);
int test_bit(string, int);
object this_object();
object this_player(void|int);
void throw(mixed);
int time();
int trace(int);
string traceprefix(string|int);
mixed *unique_array(mixed *, string, void|mixed);
object *users();
string version();
void write(mixed);
int write_bytes(string, int, string);
int write_file(string, string);
void dump_file_descriptors();
string *deep_inherit_list(object default: F_THIS_OBJECT);
string *inherit_list(object default: F_THIS_OBJECT);

#if defined(PRINTF)
void printf(string, ...);
string sprintf(string, ...);
#endif

void enable_wizard();
void disable_wizard();
object next_living(object);
int mapp(mixed);
mixed *stat(string, int default: F_CONST0);
object new(string);
int receive(string);
int remove_action(string, string);
mapping allocate_mapping(int);
void map_delete(mapping,mixed);

/*
 * Object properties
 */
int userp(object);
int wizardp(object);
int in_edit(object default : F_THIS_OBJECT);
int in_input(object default : F_THIS_OBJECT);
mixed *commands();

/*
 * Globally scoped functions
 */
object master();


/*
 * mudlib statistics
 */
mapping domain_stats(void|string);
void set_author(string);
mapping author_stats(void|string);

int rename(string, string);
int export_uid(object);
string geteuid(object default: F_THIS_OBJECT);
string getuid(object default: F_THIS_OBJECT);
int seteuid(string|int);

object first_inventory(object|string default: F_THIS_OBJECT);
object next_inventory(object default: F_THIS_OBJECT);

/*
 * New functions for MudOS
 */
int cp(string, string);
int link(string, string);
void get_char(string, void|int);
string mud_name();
object *children(string);
void message(string, string, string|string *|object|object *,
             void|object|object *);
int undefinedp(mixed);
int nullp(mixed);

int refs(mixed);

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
string socket_address(int);
void dump_socket_status();

/*
 * parser 'magic' functions, turned into efuns
 */

void malloc_status();
void dumpallobj(string|void);
void mud_status(int default : F_CONST0);

#ifdef PROFILING
void moncontrol(int);
#endif

string arch();

void error(string);

int uptime();

int strcmp(string, string);

#if (defined(RUSAGE) || defined(TIMES))
mapping rusage();
#endif /* RUSAGE */

#ifdef ED
void ed(string|void, string|void, int|void);
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
void debugmalloc(string,int);
void set_malloc_mask(int);
#endif

#ifdef DEBUG_MACRO
void set_debug_level(int);
#endif

#ifdef OPCPROF
void opcprof(string|void);
#endif

#ifdef CACHE_STATS
void cache_stats();
#endif

#ifdef SET_EVAL_LIMIT
void set_eval_limit(int);
#endif

/*
 * MIRE efuns
 */

#ifdef MIRE
int *editor_list (string, int);
int remove_editor_list (string);
string *fetch_article (int);
void doppel_mod (string, string, int);
mixed *find_keywords (string);
void init_mire();
#endif
