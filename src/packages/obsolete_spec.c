#include "spec.h"

/* communications functions */
#ifndef NO_ENVIRONMENT
void say(string, void | object | object *);
void tell_room(object | string, string | object | int | float, void | object | object *);
#endif
void write(mixed);
void tell_object(object, string);
void shout(string);
void message(mixed, mixed, string | string * | object | object *, void | object | object *);
void printf(string, ...);
