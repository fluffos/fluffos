#ifndef SIMULATE_H
#define SIMULATE_H

#define V_SHORT         1
#define V_NOSPACE       2
#define V_FUNCTION      4

/*
 * simulate.c
 */

extern int illegal_sentence_action;
extern object_t *obj_list;
extern object_t *obj_list_destruct;
extern object_t *current_object;
extern object_t *command_giver;
extern object_t *current_interactive;
extern char *inherit_file;
#ifndef NO_ADD_ACTION
extern char *last_verb;
#endif
extern int num_error;
extern int tot_alloc_sentence;
extern int MudOS_is_being_shut_down;
#ifdef LPC_TO_C
extern int compile_to_c;
extern FILE *compilation_output_file;
extern char *compilation_ident;
#endif

void debug_fatal PROT1V(char *);
void fatal PROT1V(char *);
void error PROT1V(char *);
INLINE void check_legal_string PROT((char *));
#ifndef NO_ADD_ACTION
int user_parser PROT((char *));
int command_for_object PROT((char *));
void enable_commands PROT((int));
void add_action PROT((svalue_t *, char *, int));
int remove_action PROT((char *, char *));
#endif
#ifndef NO_LIGHT
void add_light PROT((object_t *, int));
#endif
void free_sentence PROT((sentence_t *));

int input_to PROT((svalue_t *, int, int, svalue_t *));
int get_char PROT((svalue_t *, int, int, svalue_t *));

int strip_name PROT((char *, char *, int));
char *check_name PROT((char *));
object_t *load_object PROT((char *, lpc_object_t *));
object_t *clone_object PROT((char *, int));
object_t *environment PROT((svalue_t *));
object_t *first_inventory PROT((svalue_t *));
object_t *object_present PROT((svalue_t *, object_t *));
object_t *find_object PROT((char *));
object_t *find_object2 PROT((char *));
void move_object PROT((object_t *, object_t *));
void destruct_object PROT((svalue_t *));
void destruct2 PROT((object_t *));

void print_svalue PROT((svalue_t *));
void do_write PROT((svalue_t *));
void do_message PROT((svalue_t *, char *, array_t *, array_t *, int));
void say PROT((svalue_t *, array_t *));
void tell_room PROT((object_t *, svalue_t *, array_t *));
void shout_string PROT((char *));

void error_needs_free PROT((char *));
void throw_error PROT((void));
void error_handler PROT((char *));

#ifdef SIGNAL_FUNC_TAKES_INT
void startshutdownMudOS PROT((int));
#else
void startshutdownMudOS PROT((void));
#endif
void shutdownMudOS PROT((int));
void slow_shut_down PROT((int));

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_free_sentences PROT((void));
#endif

#endif
