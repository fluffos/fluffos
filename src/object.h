#ifndef OBJECT_H
#define OBJECT_H

/* It is usually better to include "lpc_incl.h" instead of including this
   file directly */

/*
 * Definition of an object.
 * If the object is inherited, then it must not be destructed !
 *
 * The reset is used as follows:
 * 0: There is an error in the reset() in this object. Never call it again.
 * 1: Normal state.
 * 2 or higher: This is an interactive user, that has not given any commands
 *		for a number of reset periods.
 */

#include "packages/uids.h"
#include "packages/mudlib_stats.h"

#define MAX_OBJECT_NAME_SIZE 2048

#define O_HEART_BEAT		0x01	/* Does it have an heart beat ?      */
#ifndef NO_WIZARDS
#define O_IS_WIZARD		0x02	/* used to be O_IS_WIZARD            */
#endif

#define O_LISTENER              0x04    /* can hear say(), etc */
#ifndef NO_ADD_ACTION
#define O_ENABLE_COMMANDS	0x04	/* Can it execute commands ?         */
#else
#define O_CATCH_TELL            0x04
#endif

#define O_CLONE			0x08	/* Is it cloned from a master copy ? */
#define O_DESTRUCTED		0x10	/* Is it destructed ?                */
#define O_SWAPPED		0x20	/* Is it swapped to file             */
#define O_ONCE_INTERACTIVE	0x40	/* Has it ever been interactive ?    */
#define O_RESET_STATE		0x80	/* Object in a 'reset':ed state ?    */
#define O_WILL_CLEAN_UP		0x100	/* clean_up will be called next time */
#define O_VIRTUAL		0x200	/* We're a virtual object            */
#ifdef F_SET_HIDE
#define O_HIDDEN		0x400	/* We're hidden from nonprived objs  */
#endif
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
#define O_EFUN_SOCKET           0x800	/* efun socket references object     */
#endif
#define O_WILL_RESET            0x1000	/* reset will be called next time    */
#ifndef OLD_ED
#define O_IN_EDIT               0x2000  /* object has an ed buffer open      */
#endif
#ifdef LPC_TO_C
#define O_COMPILED_PROGRAM      0x4000  /* this is a marker for a compiled   */
                                        /* program                           */
#endif
#ifndef NO_SNOOP
#define O_SNOOP			0x8000
#endif

/*
 * Note: use of more than 16 bits means extending flags to an unsigned long
 */

typedef struct sentence_s {
#ifndef NO_ADD_ACTION
    char *verb;
#endif
    struct sentence_s *next;
    struct object_s *ob;
    union string_or_func function;
    int flags;
} sentence_t;

#ifdef LPC_TO_C
typedef struct { /* has to be the same as object_t below */
    unsigned short ref;
    unsigned short flags;
#ifdef DEBUG
    unsigned int extra_ref;
#endif
    char *name;
    struct object_s *next_hash;
    void (**jump_table)();
    struct string_switch_entry_s **string_switch_tables;
} lpc_object_t;
#endif

typedef struct object_s {
    unsigned short ref;		/* Reference count. */
    unsigned short flags;	/* Bits or'ed together from above */
#ifdef DEBUG
    unsigned int extra_ref;	/* Used to check ref count. */
#endif
    char *name;
    struct object_s *next_hash;
    /* the fields above must match lpc_object_t */
    int load_time;		/* time when this object was created */
#ifndef NO_RESET
    int next_reset;		/* Time of next reset of this object */
#endif
    int time_of_ref;		/* Time when last referenced. Used by swap */
    long swap_num;		/* Swap file offset. -1 is not swapped yet. */
    program_t *prog;
    struct object_s *next_all;
#ifndef NO_ENVIRONMENT
    struct object_s *next_inv;
    struct object_s *contains;
    struct object_s *super;	/* Which object surround us ? */
#endif
    struct interactive_s *interactive;	/* Data about an interactive user */
    char *replaced_program;	/* Program replaced with */
#ifndef NO_LIGHT
    short total_light;
#endif
#ifndef NO_SHADOWS
    struct object_s *shadowing;	/* Is this object shadowing ? */
    struct object_s *shadowed;	/* Is this object shadowed ? */
#endif				/* NO_SHADOWS */
#ifndef NO_ADD_ACTION
    sentence_t *sent;
    struct object_s *next_hashed_living;
    char *living_name;		/* Name of living object if in hash */
#endif
#ifdef PACKAGE_UIDS
    userid_t *uid;		/* the "owner" of this object */
    userid_t *euid;		/* the effective "owner" */
#endif
#ifdef PRIVS
    char *privs;		/* object's privledges */
#endif				/* PRIVS */
#ifdef PACKAGE_MUDLIB_STATS
    statgroup_t stats;		/* mudlib stats */
#endif
#ifdef PACKAGE_PARSER
    struct parse_info_s *pinfo;
#endif
    svalue_t variables[1];	/* All variables to this program */
    /* The variables MUST come last in the struct */
} object_t;

typedef int (* get_objectsfn_t) PROT((object_t *, void *));

#ifdef DEBUG
#define add_ref(ob, str) SAFE(\
			      ob->ref++; \
			      debug(d_flag, \
			      ("Add_ref %s (%d) from %s\n", \
				     ob->name, ob->ref, str));\
			      )
#else
#define add_ref(ob, str) ob->ref++
#endif

#define ROB_STRING_ERROR 1
#define ROB_ARRAY_ERROR 2
#define ROB_MAPPING_ERROR 4
#define ROB_NUMERAL_ERROR 8
#define ROB_GENERAL_ERROR 16
#define ROB_CLASS_ERROR 32
#define ROB_ERROR 63

extern object_t *previous_ob;
extern int tot_alloc_object;
extern int tot_alloc_object_size;
extern int save_svalue_depth;
extern object_t **cgsp;
#ifdef F_SET_HIDE
extern int num_hidden;
#endif

void bufcat PROT((char **, char *));
INLINE int svalue_save_size PROT((svalue_t *));
INLINE void save_svalue PROT((svalue_t *, char **));
INLINE int restore_svalue PROT((char *, svalue_t *));
int save_object PROT((object_t *, char *, int));
char *save_variable PROT((svalue_t *));
int restore_object PROT((object_t *, char *, int));
void restore_variable PROT((svalue_t *, char *));
object_t *get_empty_object PROT((int));
void reset_object PROT((object_t *));
void call_create PROT((object_t *, int));
void reload_object PROT((object_t *));
void free_object PROT((object_t *, char *));
#ifdef F_SET_HIDE
INLINE int valid_hide PROT((object_t *));
INLINE int object_visible PROT((object_t *));
#else
#define object_visible(x) 1
#endif
void tell_npc PROT((object_t *, char *));
void tell_object PROT((object_t *, char *, int));
int find_global_variable PROT((program_t *, char *, unsigned short *, int));
void dealloc_object PROT((object_t *, char *));
void get_objects PROT((object_t ***, int *, get_objectsfn_t, void *));
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_command_giver_stack PROT((void));
#endif
void save_command_giver PROT((object_t *));
void restore_command_giver PROT((void));
void set_command_giver PROT((object_t *));

#endif
