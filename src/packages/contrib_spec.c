#include "spec.h"

#ifndef NO_SHADOWS
int remove_shadow(object);
#endif
#ifndef NO_ADD_ACTION
mixed query_notify_fail();
#endif
#if 0
void set_prompt(string, void|object);
#endif
mixed copy(mixed);
string *functions(object, int default: 0);
string *variables(object, int default: 0);
object *heart_beats();
string terminal_colour(string, mapping);
string pluralize(string);
int file_length(string);
string upper_case(string);
int replaceable(object);
string program_info();
void store_variable(string, mixed);
mixed fetch_variable(string);
int remove_interactive(object);
int query_ip_port(void | object);
