#ifndef OBJECT_H
#define OBJECT_H

#include "mapping.h"
#include "interpret.h"
#include "program.h"

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

#include "uid.h"

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
#ifdef SOCKET_EFUNS
#define O_EFUN_SOCKET           0x800	/* efun socket references object     */
#endif
#define O_WILL_RESET            0x1000	/* reset will be called next time    */
#ifndef OLD_ED
#define O_IN_EDIT               0x2000  /* object has an ed buffer open      */
#endif

#if 0
/*
 * Note: use of more than 16 bits means extending flags to an unsigned long
 */
#define O_UNUSED                0x4000	/* used to be O_MASTER (obsolete)    */
#define O_UNUSED2               0x8000	/* reserved for future expansion     */
#endif

typedef struct object {
    unsigned int ref;		/* Reference count. */
#ifdef DEBUG
    unsigned int extra_ref;	/* Used to check ref count. */
#endif
    unsigned short flags;	/* Bits or'ed together from above */
    short heart_beat_ticks, time_to_heart_beat;
    int load_time;		/* time when this object was created */
    int next_reset;		/* Time of next reset of this object */
    int time_of_ref;		/* Time when last referenced. Used by swap */
    long swap_num;		/* Swap file offset. -1 is not swapped yet. */
    struct program *prog;
    char *name;
    struct object *next_all, *next_inv, *next_heart_beat, *next_hash;
    struct object *contains;
    struct object *super;	/* Which object surround us ? */
    struct interactive *interactive;	/* Data about an interactive user */
#ifndef NO_LIGHT
    short total_light;
#endif
#ifndef NO_SHADOWS
    struct object *shadowing;	/* Is this object shadowing ? */
    struct object *shadowed;	/* Is this object shadowed ? */
#endif				/* NO_SHADOWS */
#ifndef NO_ADD_ACTION
    struct sentence *sent;
    struct object *next_hashed_living;
    char *living_name;		/* Name of living object if in hash */
#endif
#ifndef NO_UIDS
    userid_t *uid;		/* the "owner" of this object */
    userid_t *euid;		/* the effective "owner" */
#endif
#ifdef PRIVS
    char *privs;		/* object's privledges */
#endif				/* PRIVS */
#ifndef NO_MUDLIB_STATS
    statgroup_t stats;		/* mudlib stats */
#endif
    svalue variables[1];	/* All variables to this program */
    /* The variables MUST come last in the struct */
}      object_t;

#ifdef DEBUG
#define add_ref(ob, str) do { ob->ref++; \
    if (d_flag > 1) \
     printf("Add_ref %s (%d) from %s\n", ob->name, ob->ref, str); } while (0)
#else
#define add_ref(ob, str) ob->ref++
#endif

#define ROB_STRING_ERROR 1
#define ROB_ARRAY_ERROR 2
#define ROB_MAPPING_ERROR 4
#define ROB_NUMERAL_ERROR 8
#define ROB_GENERAL_ERROR 16
#define ROB_ERROR 31

/*
 * object.c
 */
extern struct object *previous_ob;
extern int tot_alloc_object;
extern int tot_alloc_object_size;
extern int save_svalue_depth;

void bufcat PROT((char **, char *));
INLINE int svalue_save_size PROT((struct svalue *));
INLINE void save_svalue PROT((struct svalue *, char **));
INLINE int restore_svalue PROT((char *, struct svalue *));
int save_object PROT((struct object *, char *, int));
char *save_variable PROT((struct svalue *));
int restore_object PROT((struct object *, char *, int));
void restore_variable PROT((struct svalue *, char *));
struct object *get_empty_object PROT((int));
void reset_object PROT((struct object *));
void call_create PROT((struct object *, int));
void reload_object PROT((struct object *));
void free_object PROT((struct object *, char *));
struct object *find_living_object PROT((char *, int));
INLINE int valid_hide PROT((struct object *));
INLINE int object_visible PROT((struct object *));
void set_living_name PROT((struct object *, char *));
void remove_living_name PROT((struct object *));
void stat_living_objects PROT((void));
int shadow_catch_message PROT((struct object *, char *));
void tell_npc PROT((struct object *, char *));
void tell_object PROT((struct object *, char *));
/*
 * ed.c
 */
void ed_start PROT((char *, char *, char *, int, struct object *));
void ed_cmd PROT((char *));
void save_ed_buffer PROT((void));
void regerror PROT((char *));

/*
 * hash.c
 */
int hashstr PROT((char *, int, int));
int whashstr PROT((char *, int));

#endif
