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
#ifdef DEBUG
extern object_t *obj_list_dangling;
extern int tot_dangling_object;
#endif

char *strput (char *, char *, const char *);
char *strput_int (char *, char *, int);
void CDECL debug_fatal (char *, ...);
void CDECL fatal (const char *, ...);
void CDECL error (const char * const, ...);
INLINE void check_legal_string (const char *);
#ifndef NO_LIGHT
void add_light (object_t *, int);
#endif
void free_sentence (sentence_t *);

sentence_t *alloc_sentence (void);
int input_to (svalue_t *, int, int, svalue_t *);
int get_char (svalue_t *, int, int, svalue_t *);

int strip_name (const char *, char *, int);
char *check_name (char *);
#define load_object(x, y) int_load_object(x)
object_t *int_load_object (const char *);
object_t *clone_object (const char *, int);
object_t *environment (svalue_t *);
object_t *first_inventory (svalue_t *);
object_t *object_present (svalue_t *, object_t *);
object_t *find_object (const char *);
object_t *find_object2 (const char *);
void move_object (object_t *, object_t *);
void destruct_object (object_t *);
void destruct2 (object_t *);

void print_svalue (svalue_t *);
void do_write (svalue_t *);
void do_message (svalue_t *, svalue_t *, array_t *, array_t *, int);
void say (svalue_t *, array_t *);
void tell_room (object_t *, svalue_t *, array_t *);
void shout_string (const char *);

void error_needs_free (char *);
void throw_error (void);
void error_handler (char *);

#ifdef SIGNAL_FUNC_TAKES_INT
void startshutdownMudOS (int);
#else
void startshutdownMudOS (void);
#endif
void shutdownMudOS (int);
void slow_shut_down (int);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_free_sentences (void);
#endif

#endif
