#ifndef OBJECT_H
#define OBJECT_H

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
#define O_HIDDEN		0x400	/* We're hidden from nonprived objs  */
#ifdef PACKAGE_SOCKETS
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
#define O_UNUSED                0x8000

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
    int next_reset;		/* Time of next reset of this object */
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

#ifdef DEBUG
#define add_ref(ob, str) SAFE(\
			      ob->ref++; \
			      if (d_flag > 1) \
			      printf("Add_ref %s (%d) from %s\n", \
				     ob->name, ob->ref, str);\
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
object_t *find_living_object PROT((char *, int));
INLINE int valid_hide PROT((object_t *));
INLINE int object_visible PROT((object_t *));
void set_living_name PROT((object_t *, char *));
void remove_living_name PROT((object_t *));
void stat_living_objects PROT((outbuffer_t *));
int shadow_catch_message PROT((object_t *, char *));
void tell_npc PROT((object_t *, char *));
void tell_object PROT((object_t *, char *));
variable_t *find_status PROT((char *));
void dealloc_object PROT((object_t *, char *));

#endif
