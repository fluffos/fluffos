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
void add_worth(int, void|object|string);
void add_xverb(string);
object *all_inventory(object default: F_THIS_OBJECT);
mixed *allocate(int);
void break_point();
unknown call_other(object|string|object *, string, ...);
void call_out(string, int, void|mixed);
mixed *call_out_info();
string capitalize(string);
int cat(string, void|int, void|int);
int cindent(string);
string clear_bit(string, int);
object clone_object(string);
int command(string);
string crypt(string, string|int);	/* An int as second argument ? */
string ctime(int);
mixed debug_info(int, mixed|void, ...);
object *deep_inventory(object);
void destruct(object);
void disable_commands();
void ed(void|string, void|string);
void enable_commands();
object environment(void|object);
int exec(object, object);
string *explode(string, string);
string extract(string, void|int, void|int);
string file_name(object default: F_THIS_OBJECT);
int file_size(string);
mixed filter_array(mapping|mixed *, string, object|string, void|mixed);
int find_call_out(string);
object find_living(string);
object find_object(string);
object find_player(string);
string function_exists(string, object default: F_THIS_OBJECT);
string implode(string *, string);
mixed *indices(mapping);
mixed *keys(mapping); /* synonym for indices -- eventually move to this one */
void input_to(string, int default: F_CONST0);
int interactive(object default: F_THIS_OBJECT);
int intp(mixed);
int living(object);
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
int restore_object(string);
int rm(string);
void rmdir(string);

#if defined(RUSAGE)
string rusage();
#endif

void save_object(string);
void say(string|mixed *, void|object|object *);
string set_bit(string, int);
int set_heart_beat(int);
void set_hide(int);
int set_light(int);
void set_living_name(string);
#ifndef NO_SHADOWS /* LPCA */
object shadow(object, int);
#endif
void shout(string);
void shutdown();
int sizeof(int|mapping|mixed *);
object snoop(void|object, void|object);
mixed *sort_array(mixed *,string,object|string default: F_THIS_OBJECT);
int stringp(mixed);
int strlen(string);
void swap(object);		/* Only used for debugging */
void tail(string);
void tell_object(object, string);
void tell_room(object|string, string, void|object *);
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
void wizlist(void|string);
void write(mixed);
int write_bytes(string, int, string);
int write_file(string, string);
string *deep_inherit_list(object default: F_THIS_OBJECT);
string *inherit_list(object default: F_THIS_OBJECT);

#if defined(PRINTF)
void printf(string, ...);
string sprintf(string, ...);
#endif

void enable_privileges(object);
void enable_wizard();
object next_living(object);
mixed *find_wizard(string);
int mapp(mixed);
mixed *stat(string, int default: F_CONST0);
object new(string);
int receive(string);
int remove_action(string, string);
void set_prompt(string);
mapping allocate_mapping(int);
void map_delete(mapping,mixed);

/*
 * Object properties
 */
int userp(object);
int privp(object);
int wizardp(object);
int in_edit(object default : F_THIS_OBJECT);
int in_input(object default : F_THIS_OBJECT);
mixed *commands();

/*
 * Globally scoped functions
 */
object master();
string *wizards();

int rename(string, string);
int export_uid(object);
string geteuid(object default: F_THIS_OBJECT);
string getuid(object default: F_THIS_OBJECT);
int seteuid(string|int);

/*
 *
 * The following functions are optional. Comment out the ones not wanted.
 * Beware that some may be used in mudlib 2.4.5.
 *
 */
object first_inventory(object|string default: F_THIS_OBJECT);
object next_inventory(object default: F_THIS_OBJECT);

/*
 * New functions for MudOS
 */
int type(mixed);
int cp(string, string);
int link(string, string);
void get_char(string, void|int);
string mud_name();
object *children(string);
void message(string, string, string|string *|object|object *,
             void|object|object *);
int undefinedp(mixed);


/*
 * parser 'magic' functions, turned into efuns
 */

#if 0
string *mud_status (string default : F_CONST0);
void dump_all_obj (string default : F_CONST0);
string *malloc_status();
int debugmalloc();
#endif

