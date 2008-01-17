#include "spec.h"

#ifndef NO_SHADOWS
int remove_shadow(object);
#endif
#ifndef NO_ADD_ACTION
mixed query_notify_fail();
object *named_livings();
#endif
#if 0
void set_prompt(string, void|object);
#endif
mixed copy(mixed);
mixed *functions(object, int default: 0);
mixed *variables(object, int default: 0);
object *heart_beats();
#ifdef COMPAT_32
object *heart_beat_info heart_beats();
#endif
string terminal_colour(string, mapping, void | int, void | int);
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
string query_replaced_program(void|object);
mapping network_stats();
int real_time();
#ifdef PACKAGE_COMPRESS
int compressedp(object);
#endif
void event(object|object *, string, ...);
string query_num(int, int default:0);
string base_name( string | object default: F__THIS_OBJECT);
object *get_garbage();
int num_classes( object );
mixed assemble_class( mixed * );
mixed *disassemble_class( mixed );
mixed fetch_class_member( mixed, int );
mixed store_class_member( mixed, int, mixed );
mixed *shuffle(mixed *);
mixed element_of(mixed *);
mixed max( mixed *, int | void );
mixed min( mixed *, int | void );
mixed abs( int | float );
