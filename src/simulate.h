#ifndef SIMULATE_H
#define SIMULATE_H

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

#define V_SHORT         1
#define V_NOSPACE       2
#define V_FUNCTION      4

/* The end of a static buffer */
#define EndOf(x) (x + sizeof(x)/sizeof(x[0]))

/*
 * simulate.c
 */

extern object_t *obj_list;
extern object_t *obj_list_destruct;
extern object_t *current_object;
extern object_t *command_giver;
extern object_t *current_interactive;
extern char *inherit_file;
extern int tot_alloc_sentence;
extern int MudOS_is_being_shut_down;
#ifdef LPC_TO_C
extern int compile_to_c;
extern FILE *compilation_output_file;
extern char *compilation_ident;
#endif
#ifdef DEBUG
extern object_t *obj_list_dangling;
extern int tot_dangling_object;
#endif

char *strput PROT((char *, char *, char *));
char *strput_int PROT((char *, char *, int));
void CDECL debug_fatal PROT1V(char *);
void CDECL fatal PROT1V(char *);
void CDECL error PROT1V(char *);
INLINE void check_legal_string PROT((char *));
#ifndef NO_LIGHT
void add_light PROT((object_t *, int));
#endif
void free_sentence PROT((sentence_t *));

sentence_t *alloc_sentence PROT((void));
int input_to PROT((svalue_t *, int, int, svalue_t *));
int get_char PROT((svalue_t *, int, int, svalue_t *));

int strip_name PROT((char *, char *, int));
char *check_name PROT((char *));
#ifdef LPC_TO_C
#define load_object(x, y) int_load_object(x, y)
object_t *int_load_object PROT((char *, lpc_object_t *));
#else
#define load_object(x, y) int_load_object(x)
object_t *int_load_object PROT((char *));
#endif
object_t *clone_object PROT((char *, int));
object_t *environment PROT((svalue_t *));
object_t *first_inventory PROT((svalue_t *));
object_t *object_present PROT((svalue_t *, object_t *));
object_t *find_object PROT((char *));
object_t *find_object2 PROT((char *));
void move_object PROT((object_t *, object_t *));
void destruct_object PROT((object_t *));
void destruct2 PROT((object_t *));

void print_svalue PROT((svalue_t *));
void do_write PROT((svalue_t *));
void do_message PROT((svalue_t *, svalue_t *, array_t *, array_t *, int));
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
