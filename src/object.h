#ifndef _OBJECT_H_
#define _OBJECT_H_

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
#define O_IS_WIZARD		0x02	/* Is it a wizard user.c ?           */
#define O_ENABLE_COMMANDS	0x04	/* Can it execute commands ?         */
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

#if 0
/*
 * Note: use of more than 16 bits means extending flags to an unsigned long
 */
#define O_EXTERN_PROGRAM        0x2000	/* external program (not used)       */
#define O_UNUSED                0x4000	/* used to be O_MASTER (obsolete)    */
#define O_UNUSED2               0x8000	/* reserved for future expansion     */
#endif

typedef struct object {
    unsigned int ref;			/* Reference count. */
#ifdef DEBUG
    int extra_ref;		/* Used to check ref count. */
#endif
    unsigned short flags;	/* Bits or'ed together from above */
    short heart_beat_ticks, time_to_heart_beat;
    short total_light;
    int load_time;		/* time when this object was created */
    int next_reset;		/* Time of next reset of this object */
    int time_of_ref;		/* Time when last referenced. Used by swap */
    long swap_num;		/* Swap file offset. -1 is not swapped yet. */
    struct program *prog;
    char *name;
    struct object *next_all, *next_inv, *next_heart_beat, *next_hash;
    struct object *contains;
    struct object *super;	/* Which object surround us ? */
#ifndef NO_SHADOWS
    struct object *shadowing;	/* Is this object shadowing ? */
    struct object *shadowed;	/* Is this object shadowed ? */
#endif				/* NO_SHADOWS */
    struct interactive *interactive;	/* Data about an interactive user */
    struct sentence *sent;
    userid_t *uid;		/* the "owner" of this object */
    userid_t *euid;		/* the effective "owner" */
#ifdef PRIVS
    char *privs;		/* object's privledges */
#endif				/* PRIVS */
    statgroup_t stats;		/* mudlib stats */
    struct object *next_hashed_living;
    char *living_name;		/* Name of living object if in hash */
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

#endif
