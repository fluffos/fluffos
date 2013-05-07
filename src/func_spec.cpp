# 1 "func_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "func_spec.c"
# 1 "spec.h" 1




# 1 "std.h" 1







# 1 "arch.h" 1
# 9 "std.h" 2






# 1 "options_incl.h" 1
# 1 "local_options" 1
# 1 "options_incl.h" 2
# 16 "std.h" 2
# 1 "configure.h" 1
# 17 "std.h" 2







# 1 "portability.h" 1
# 25 "std.h" 2
# 1 "macros.h" 1
# 96 "macros.h"
# 1 "my_malloc.h" 1
# 97 "macros.h" 2
# 26 "std.h" 2







# 1 "debug.h" 1
# 34 "std.h" 2
# 5 "spec.h" 2
# 2 "func_spec.c" 2

# 1 "op_spec.c" 1





operator pop_value, push, efun0, efun1, efun2, efun3, efunv;

operator short_int, number, real, byte, nbyte, string, short_string, const0, const1;

operator aggregate, aggregate_assoc;







operator branch_ne, branch_ge, branch_le, branch_eq;
operator branch_when_zero, branch_when_non_zero, branch;
operator bbranch_when_zero, bbranch_when_non_zero, bbranch;

operator bbranch_lt;

operator foreach, next_foreach, exit_foreach;
operator loop_cond_local, loop_cond_number;
operator loop_incr;
operator while_dec;

operator lor, land;

operator catch, end_catch;
operator time_expression, end_time_expression;

operator switch ;

operator call_function_by_address, call_inherited, return, return_zero;


operator eq, ne, le, lt, ge, gt;

operator inc, dec, pre_inc, post_inc, pre_dec, post_dec;

operator transfer_local;

operator make_ref, kill_refs;


operator local, local_lvalue;
operator ref, ref_lvalue;
operator global, global_lvalue;
operator member, member_lvalue;
operator index, index_lvalue;
operator rindex, rindex_lvalue;
operator nn_range, nn_range_lvalue, rn_range, rn_range_lvalue;
operator rr_range, rr_range_lvalue, nr_range, nr_range_lvalue;
operator ne_range, re_range;


operator add_eq, sub_eq, and_eq, or_eq, xor_eq, lsh_eq, rsh_eq, mult_eq;
operator div_eq, mod_eq, assign;

operator void_add_eq, void_assign, void_assign_local;

operator add, subtract, multiply, divide, mod, and, or, xor, lsh, rsh;
operator not, negate, compl;

operator function_constructor;
operator simul_efun;

operator sscanf;
operator parse_command;

operator new_class, new_empty_class;
operator expand_varargs;
operator type_check;
# 4 "func_spec.c" 2
# 37 "func_spec.c"
unknown _call_other(object | string | object *, string | mixed *, ...);

mixed _evaluate(mixed, ...);

object _this_object();

int _to_int(string | float | int | buffer);
float _to_float(string | float | int);

object _new(string, ...);

unknown call_other _call_other(object | string | object *, string | mixed *, ...);
mixed evaluate _evaluate(mixed, ...);

mixed funcall _evaluate(mixed, ...);

object this_object _this_object();
int to_int _to_int(string | float | int | buffer);
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



int strwidth sizeof(string);

void destruct(object default: F__THIS_OBJECT);
string file_name(object default: F__THIS_OBJECT);
string capitalize(string);
string *explode(string, string);
mixed implode(mixed *, string | function, void | mixed);

int call_out(string | function, int, ...);



int member_array(mixed, string | mixed *, void | int, void | int);
int input_to(string | function, ...);
int random(int);
void defer(function);


object environment(void | object);
object *all_inventory(object default: F__THIS_OBJECT);
object *deep_inventory(object | object * | function, void | function default: F__THIS_OBJECT);
object first_inventory(object | string default: F__THIS_OBJECT);
object next_inventory(object default: F__THIS_OBJECT);
void say(string, void | object | object *);
void tell_room(object | string, string | object | int | float, void | object | object *);
object present(object | string, void | object);
void move_object(object | string);



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





string lower_case(string);
string replace_string(string, string, string, ...);
int restore_object(string, void | int);
mixed save_object(string | int | void, void | int);
string save_variable(mixed);
mixed restore_variable(string);
object *users();
mixed *get_dir(string, int default: 0);
int strsrch(string, string | int, int default: 0);

int strstr strsrch(string, string | int, int default: 0);




void write(mixed);
void tell_object(object, string);
void shout(string);
void receive(string | buffer);
void message(mixed, mixed, string | string * | object | object *,
             void | object | object *);



object find_object(string, int default: 0);
object load_object find_object(string, int default: 1);

int find_call_out(int | string);






mapping allocate_mapping(int | mixed *, void | mixed);
mixed *values(mapping);
mixed *keys(mapping);

mapping map_delete(mapping, mixed);
mapping m_delete map_delete(mapping, mixed);
mixed *m_values values(mapping);
mixed *m_indices keys(mapping);




mixed match_path(mapping, string);



int clonep(mixed default: F__THIS_OBJECT);
int intp(mixed);
int undefinedp(mixed);
int nullp undefinedp(mixed);
int floatp(mixed);
int stringp(mixed);
int virtualp(object default: F__THIS_OBJECT);
int functionp(mixed);

int closurep functionp(mixed);

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
mixed *call_out_info();


int crc32(string | buffer);




mixed read_buffer(string | buffer, void | int, void | int);
int write_buffer(string | buffer, int, string | buffer | int);

int write_file(string, string, int default:0);
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

object snoop(object, void | object);
object query_snoop(object);
object query_snooping(object);


int remove_call_out(int | void | string);



void set_heart_beat(int);
int query_heart_beat(object default:F__THIS_OBJECT);
void set_hide(int);


void set_reset(object, void | int);



object shadow(object, int default: 1);
object query_shadowing(object);

mixed *sort_array(mixed *, int | string | function, ...);
void throw(mixed);
int time();
mixed *unique_array(mixed *, string | function, void | mixed);
mapping unique_mapping(mixed *, string | function, ...);
string *deep_inherit_list(object default:F__THIS_OBJECT);
string *shallow_inherit_list(object default:F__THIS_OBJECT);

string *inherit_list deep_inherit_list(object default:F__THIS_OBJECT);



void printf(string, ...);
string sprintf(string, ...);
int mapp(mixed);
mixed *stat(string, int default: 0);




int interactive(object default:F__THIS_OBJECT);
int has_mxp(object default:F__THIS_OBJECT);
int has_zmp(object default:F__THIS_OBJECT);
void send_zmp(string, string *);
int has_gmcp(object default:F__THIS_OBJECT);
void send_gmcp(string);
string in_edit(object default:F__THIS_OBJECT);
int in_input(object default:F__THIS_OBJECT);
int userp(object);

int query_once_interactive userp(object);



void enable_wizard();
void disable_wizard();
int wizardp(object);


object master();




int memory_info(object | void);
mixed get_config(int);







int get_char(string | function, ...);
object *children(string);

void reload_object(object);

void error(string);

void raise_error error(string);

int uptime();
int strcmp(string, string);



mapping rusage();



void flush_messages(void | object);


void ed(string | void, string | int | void, string | int | void, int | void, int | void);







string cache_stats();


mixed filter(string | mixed * | mapping, string | function, ...);
mixed filter_array filter(mixed *, string | function, ...);
mapping filter_mapping filter(mapping, string | function, ...);

mixed map(string | mapping | mixed *, string | function, ...);
mapping map_mapping map(mapping, string | function, ...);
mixed *map_array map(mixed *, string | function, ...);



string malloc_status();
string mud_status(int default: 0);
void dumpallobj(string | void);

string dump_file_descriptors();
string query_load_average();






string origin();



int reclaim_objects();

int set_eval_limit(int);
int reset_eval_cost set_eval_limit(int default: 0);
int eval_cost set_eval_limit(int default: -1);
int max_eval_cost set_eval_limit(int default: 1);

int get_eval_cost set_eval_limit(int default: -1);
# 395 "func_spec.c"
mapping *function_profile(object default:F__THIS_OBJECT);


int resolve(string, string | function);







void act_mxp();
void websocket_handshake_done();
void request_term_type();
void start_request_term_type();
void request_term_size(void | int);

void shutdown(void | int);
# 1 "packages/trim_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/trim_spec.c"





# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 7 "packages/trim_spec.c" 2





string trim(mixed, string | void);
string ltrim(mixed, string | void);
string rtrim(mixed, string | void);
# 1 "packages/uids_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/uids_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/uids_spec.c" 2

int export_uid(object);
string geteuid(function | object default:F__THIS_OBJECT);
string getuid(object default:F__THIS_OBJECT);
int seteuid(string | int);
# 1 "packages/db_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/db_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/db_spec.c" 2

int db_close(int);
int db_commit(int);
int db_connect(string, string, string | void, int | void);
mixed db_exec(int, string);
mixed *db_fetch(int, int);
int db_rollback(int);
string db_status(void);
# 1 "packages/compress_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/compress_spec.c"





int compress_file(string, string | void);
int uncompress_file(string, string | void);

buffer compress(string | buffer);
buffer uncompress(string | buffer);
# 1 "packages/rl_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/rl_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/rl_spec.c" 2

int roll( int, int );
void dumpallhb(string | void);
object *objects_hb(void | string | function);
# 1 "packages/math_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/math_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/math_spec.c" 2

    float cos(float);
    float sin(float);
    float tan(float);
    float asin(float);
    float acos(float);
    float atan(float);
    float sqrt(float|int);
    float log(float);
    float log10(float|int);
    float log2(float|int);
    float pow(float|int, float|int);
    float exp(float);
    float floor(float);
    float ceil(float);
    float round(float);
    float norm(int *|float *);
    float dotprod(int *|float *, int *|float *);
    float distance(int *|float *, int *|float *);
    float angle(int *|float *, int *|float *);
# 1 "packages/develop_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/develop_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/develop_spec.c" 2

    mixed debug_info(int, object);
    int refs(mixed);







    void dump_prog(object,...);
# 1 "packages/external_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/external_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/external_spec.c" 2

int external_start(int, string | string *, string | function, string | function, string | function | void);
# 1 "packages/contrib_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/contrib_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/contrib_spec.c" 2


int remove_shadow(object);


mixed query_notify_fail();
object *named_livings();




mixed copy(mixed);
mixed *functions(object, int default: 0);
mixed *variables(object, int default: 0);
object *heart_beats();

object *heart_beat_info heart_beats();

string terminal_colour(string, mapping, int | void, int | void);
string pluralize(string);
int file_length(string);
string upper_case(string);
int replaceable(object, void | string *);
mapping program_info(void | object);
void store_variable(string, mixed);
mixed fetch_variable(string);
int remove_interactive(object);
int query_ip_port(void | object);
string zonetime(string, int);
int is_daylight_savings_time(string, int);
void debug_message(string);
object function_owner(function);
string repeat_string(string, int);
mapping memory_summary();
string query_replaced_program(void | object);
mapping network_stats();
int real_time();

int compressedp(object);

void event(object | object *, string, ...);
string query_num(int, int default:0);
string base_name(string | object default: F__THIS_OBJECT);
object *get_garbage();
int num_classes(object);
mixed assemble_class(mixed *);
mixed *disassemble_class(mixed);
mixed fetch_class_member(mixed, int);
mixed store_class_member(mixed, int, mixed);
mixed *shuffle(mixed *);
mixed element_of(mixed *);
mixed max(mixed *, int default:0);
mixed min(mixed *, int default:0);
mixed abs(int | float);
int string_difference(string, string);
int query_charmode(object);
int remove_charmode(object);
int remove_get_char(object);
int send_nullbyte(object);
void restore_from_string(string, int default:0);
mixed *classes(object, int default : 0);
int test_load(string);
# 1 "packages/sockets_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/sockets_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/sockets_spec.c" 2




    int socket_create(int, string | function, string | function | void);
    int socket_bind(int, int, string | void);
    int socket_listen(int, string | function);
    int socket_accept(int, string | function, string | function);
    int socket_connect(int, string, string | function, string | function);
    int socket_write(int, mixed, string | void);
    int socket_close(int);
    int socket_release(int, object, string | function);
    int socket_acquire(int, string | function, string | function, string | function);
    string socket_error(int);
    string socket_address(int | object, int default: 0);
# 26 "packages/sockets_spec.c"
    mixed *socket_status(void | int);
# 1 "packages/sha1_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/sha1_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/sha1_spec.c" 2

string sha1(string|buffer);
# 1 "packages/async_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/async_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/async_spec.c" 2

void async_read(string, function);
void async_write(string, string, int, function);

void async_getdir(string, function);


void async_db_exec(int, string, function);
# 1 "packages/mudlib_stats_spec.c"
# 1 "/home/zoilder/develmud/driver/fluffos/src//"
# 1 "<command-line>"
# 1 "packages/mudlib_stats_spec.c"
# 1 "./spec.h" 1




# 1 "./std.h" 1







# 1 "./arch.h" 1
# 9 "./std.h" 2






# 1 "./options_incl.h" 1
# 1 "./local_options" 1
# 1 "./options_incl.h" 2
# 16 "./std.h" 2
# 1 "./configure.h" 1
# 17 "./std.h" 2







# 1 "./portability.h" 1
# 25 "./std.h" 2
# 1 "./macros.h" 1
# 96 "./macros.h"
# 1 "./my_malloc.h" 1
# 97 "./macros.h" 2
# 26 "./std.h" 2







# 1 "./debug.h" 1
# 34 "./std.h" 2
# 5 "./spec.h" 2
# 2 "packages/mudlib_stats_spec.c" 2

    mapping domain_stats(void | string);
    void set_author(string);
    mapping author_stats(void | string);
