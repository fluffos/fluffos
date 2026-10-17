/*
 * Hook efuns. See src/include/hooks.h for flag constants.
 */

int hook_attach(string, mixed, mapping, int);
void hook_detach(int);
mapping hook_query(int);
mapping *hook_list(object | void);
mixed hook_proceed(void | mixed *);
