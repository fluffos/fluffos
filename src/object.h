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
 *              for a number of reset periods.
 */

#include "packages/uids.h"
#include "packages/mudlib_stats.h"

#define MAX_OBJECT_NAME_SIZE 2048

#define O_HEART_BEAT            0x01    /* Does it have an heart beat ?      */
#ifndef NO_WIZARDS
#define O_IS_WIZARD             0x02    /* used to be O_IS_WIZARD            */
#endif

#define O_LISTENER              0x04    /* can hear say(), etc */
#ifndef NO_ADD_ACTION
#define O_ENABLE_COMMANDS       0x04    /* Can it execute commands ?         */
#else
#define O_CATCH_TELL            0x04
#endif

#define O_CLONE                 0x08    /* Is it cloned from a master copy ? */
#define O_DESTRUCTED            0x10    /* Is it destructed ?                */
#define O_ONCE_INTERACTIVE      0x40    /* Has it ever been interactive ?    */
#define O_RESET_STATE           0x80    /* Object in a 'reset':ed state ?    */
#define O_WILL_CLEAN_UP         0x100   /* clean_up will be called next time */
#define O_VIRTUAL               0x200   /* We're a virtual object            */
#ifdef F_SET_HIDE
#define O_HIDDEN                0x400   /* We're hidden from nonprived objs  */
#endif
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
#define O_EFUN_SOCKET           0x800   /* efun socket references object     */
#endif
#define O_WILL_RESET            0x1000  /* reset will be called next time    */
#ifndef OLD_ED
#define O_IN_EDIT               0x2000  /* object has an ed buffer open      */
#endif
#define O_BEING_DESTRUCTED      0x4000
#ifndef NO_SNOOP
#define O_SNOOP                 0x8000
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

typedef struct object_s {
    unsigned short ref;         /* Reference count. */
    unsigned short flags;       /* Bits or'ed together from above */
#ifdef DEBUG
    unsigned int extra_ref;     /* Used to check ref count. */
#endif
    const char * const obname;
    struct object_s *next_hash;
    struct object_s *next_ch_hash;
    /* the fields above must match lpc_object_t */
    int load_time;              /* time when this object was created */
#ifndef NO_RESET
    int next_reset;             /* Time of next reset of this object */
#endif
    int time_of_ref;            /* Time when last referenced. Used by clean_uo */
    program_t *prog;
    struct object_s *next_all;
    struct object_s *prev_all;
#ifndef NO_ENVIRONMENT
    struct object_s *next_inv;
    struct object_s *contains;
    struct object_s *super;     /* Which object surround us ? */
#endif
    struct interactive_s *interactive;  /* Data about an interactive user */
    char *replaced_program;     /* Program replaced with */
#ifndef NO_LIGHT
    short total_light;
#endif
#ifndef NO_SHADOWS
    struct object_s *shadowing; /* Is this object shadowing ? */
    struct object_s *shadowed;  /* Is this object shadowed ? */
#endif                          /* NO_SHADOWS */
#ifndef NO_ADD_ACTION
    sentence_t *sent;
    struct object_s *next_hashed_living;
    char *living_name;          /* Name of living object if in hash */
#endif
#ifdef PACKAGE_UIDS
    userid_t *uid;              /* the "owner" of this object */
    userid_t *euid;             /* the effective "owner" */
#endif
#ifdef PRIVS
    char *privs;                /* object's privledges */
#endif                          /* PRIVS */
#ifdef PACKAGE_MUDLIB_STATS
    statgroup_t stats;          /* mudlib stats */
#endif
#ifdef PACKAGE_PARSER
    struct parse_info_s *pinfo;
#endif
    svalue_t variables[1];      /* All variables to this program */
    /* The variables MUST come last in the struct */
} object_t;

typedef int (* get_objectsfn_t) (object_t *, void *);

#ifdef DEBUG
#define add_ref(ob, str) SAFE(\
                              if(ob->ref++ > 32000){\
                            	  ob->flags |= O_BEING_DESTRUCTED;\
				destruct_object(ob);\
				error("ref count too high!\n");\
			      } \
                              debug(d_flag, \
                              ("Add_ref %s (%d) from %s\n", \
                                     ob->obname, ob->ref, str));\
                              )
#else
#define add_ref(ob, str) if(ob->ref++ > 32000){ob->flags|=O_BEING_DESTRUCTED;destruct_object(ob);error("ref count too high!\n");}
#endif

#define ROB_STRING_ERROR 1
#define ROB_ARRAY_ERROR 2
#define ROB_MAPPING_ERROR 4
#define ROB_NUMERAL_ERROR 8
#define ROB_GENERAL_ERROR 16
#define ROB_CLASS_ERROR 32
#define ROB_ERROR 63

#define SETOBNAME(ob,name) (*(const char **)&(ob->obname) = (char *) name)

extern object_t *previous_ob;
extern int tot_alloc_object;
extern int tot_alloc_object_size;
extern int save_svalue_depth;
extern object_t **cgsp;
#ifdef F_SET_HIDE
extern int num_hidden;
#endif

void bufcat (char **, char *);
INLINE int svalue_save_size (svalue_t *);
INLINE void save_svalue (svalue_t *, char **);
INLINE int restore_svalue (char *, svalue_t *);
int save_object (object_t *, const char *, int);
int save_object_str (object_t *, int, char *, int);
char *save_variable (svalue_t *);
int restore_object (object_t *, const char *, int);
void restore_variable (svalue_t *, char *);
object_t *get_empty_object (int);
void reset_object (object_t *);
void call_create (object_t *, int);
void reload_object (object_t *);
void free_object (object_t **, const char * const);
#ifdef F_SET_HIDE
INLINE int valid_hide (object_t *);
INLINE int object_visible (object_t *);
#else
#define object_visible(x) 1
#endif
void tell_npc (object_t *, const char *);
void tell_object (object_t *, const char *, int);
int find_global_variable (program_t *, const char * const, unsigned short *, int);
void dealloc_object (object_t *, const char *);
void get_objects (object_t ***, int *, get_objectsfn_t, void *);
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_command_giver_stack (void);
#endif
void save_command_giver (object_t *);
void restore_command_giver (void);
void set_command_giver (object_t *);
void clear_non_statics (object_t * ob);
void restore_object_from_buff (object_t * ob, char * theBuff, int noclear);
#endif
