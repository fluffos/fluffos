#ifndef SIMULATE_H
#define SIMULATE_H

#include "sent.h"
#include "object.h"
#include "interpret.h"
#include "array.h"

/*
 * simulate.c
 */
extern struct object *obj_list;
extern struct object *obj_list_destruct;
extern struct object *current_object;
extern struct object *command_giver;
extern struct object *current_interactive;
extern char *inherit_file;
extern char *last_verb;
extern int num_error;
extern int tot_alloc_sentence;
extern int MudOS_is_being_shut_down;
#ifdef LPC_TO_C
extern int compile_to_c;
extern FILE *compilation_output_file;
extern char *compilation_ident;
#endif

void debug_fatal PROTVARGS(());
void fatal PROTVARGS(());
void error PROTVARGS(());
INLINE void check_legal_string PROT((char *));
int command_for_object PROT((char *, struct object *));
void add_light PROT((struct object *, int));
void free_sentence PROT((struct sentence *));
int user_parser PROT((char *));

void enable_commands PROT((int));
int input_to PROT((char *, int, int, struct svalue *));
int get_char PROT((char *, int, int, struct svalue *));
void add_action PROT((char *, char *, int));
int remove_action PROT((char *, char *));

struct object *load_object PROT((char *, int));
struct object *clone_object PROT((char *));
struct object *environment PROT((struct svalue *));
struct object *first_inventory PROT((struct svalue *));
struct object *object_present PROT((struct svalue *, struct object *));
struct object *find_object PROT((char *));
struct object *find_object2 PROT((char *));
void move_object PROT((struct object *, struct object *));
void destruct_object PROT((struct svalue *));
void destruct2 PROT((struct object *));

void print_svalue PROT((struct svalue *));
void do_write PROT((struct svalue *));
void do_message PROT((struct svalue *, char *, struct vector *, struct vector *, int));
void say PROT((struct svalue *, struct vector *));
void tell_room PROT((struct object *, struct svalue *, struct vector *));
void shout_string PROT((char *));

void error_needs_free PROT((char *));
void throw_error PROT((void));

#ifdef SIGNAL_FUNC_TAKES_INT
void startshutdownMudOS PROT((int));
#else
void startshutdownMudOS PROT((void));

#endif
void shutdownMudOS PROT((int));
void slow_shut_down PROT((int));

#endif
