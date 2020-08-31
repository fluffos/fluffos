#ifndef OBJECT_H
#define OBJECT_H

#include <cstdint>  // for uint32_t

#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif

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

#define MAX_OBJECT_NAME_SIZE 2048

#define O_HEART_BEAT 0x01u /* Does it have an heart beat ?      */
#ifndef NO_WIZARDS
#define O_IS_WIZARD 0x02u /* used to be O_IS_WIZARD            */
#endif

#define O_LISTENER 0x04u /* can hear say(), etc */
#ifndef NO_ADD_ACTION
#define O_ENABLE_COMMANDS 0x04u /* Can it execute commands ?         */
#else
#define O_CATCH_TELL 0x04u
#endif

#define O_CLONE 0x08u            /* Is it cloned from a master copy ? */
#define O_DESTRUCTED 0x10u       /* Is it destructed ?                */
#define O_ONCE_INTERACTIVE 0x40u /* Has it ever been interactive ?    */
#define O_RESET_STATE 0x80u      /* Object in a 'reset':ed state ?    */
#define O_WILL_CLEAN_UP 0x100u   /* clean_up will be called next time */
#define O_VIRTUAL 0x200u         /* We're a virtual object            */
#ifdef F_SET_HIDE
#define O_HIDDEN 0x400u /* We're hidden from nonprived objs  */
#endif
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
#define O_EFUN_SOCKET 0x800u /* efun socket references object     */
#endif
#define O_WILL_RESET 0x100u /* reset will be called next time    */
#ifndef OLD_ED
#define O_IN_EDIT 0x2000u /* object has an ed buffer open      */
#endif
#define O_BEING_DESTRUCTED 0x4000u
#ifndef NO_SNOOP
#define O_SNOOP 0x8000u
#endif

/*
 * Note: use of more than 16 bits means extending flags to an unsigned long
 */

struct sentence_t {
#ifndef NO_ADD_ACTION
  char *verb;
#endif
  struct sentence_t *next;
  struct object_t *ob;
  union string_or_func function;
  int flags;
};

struct object_t {
  uint32_t ref;         /* Reference count. */
  unsigned short flags; /* Bits or'ed together from above */
#ifdef DEBUGMALLOC_EXTENSIONS
  unsigned int extra_ref; /* Used to check ref count. */
#endif
  const char *obname;
  struct object_t *next_hash;
  struct object_t *next_ch_hash;
  /* the fields above must match lpc_object_t */
  int load_time; /* time when this object was created */
#ifndef NO_RESET
  int next_reset; /* Time of next reset of this object */
#endif
  uint64_t time_of_ref; /* Time when last referenced. Used by clean_uo */
  program_t *prog;
  struct object_t *next_all;
  struct object_t *prev_all;
#ifndef NO_ENVIRONMENT
  struct object_t *next_inv;
  struct object_t *contains;
  struct object_t *super; /* Which object surround us ? */
#endif
  struct interactive_t *interactive; /* Data about an interactive user */
  char *replaced_program;            /* Program replaced with */
#ifndef NO_LIGHT
  short total_light;
#endif
#ifndef NO_SHADOWS
  struct object_t *shadowing; /* Is this object shadowing ? */
  struct object_t *shadowed;  /* Is this object shadowed ? */
#endif                        /* NO_SHADOWS */
#ifndef NO_ADD_ACTION
  sentence_t *sent;
  struct object_t *next_hashed_living;
  char *living_name; /* Name of living object if in hash */
#endif
#ifdef PACKAGE_UIDS
  struct userid_t *uid;  /* the "owner" of this object */
  struct userid_t *euid; /* the effective "owner" */
#endif
#ifdef PRIVS
  char *privs; /* object's privledges */
#endif         /* PRIVS */
#ifdef PACKAGE_MUDLIB_STATS
  struct statgroup_t stats; /* mudlib stats */
#endif
#ifdef PACKAGE_PARSER
  struct parse_info_s *pinfo;
#endif
  svalue_t variables[1]; /* All variables to this program */
                         /* The variables MUST come last in the struct */
};

typedef int (*get_objectsfn_t)(object_t *, void *);

#define add_ref(ob, str)                                                                        \
  SAFE(if (ob->ref++ > 0xfffffff0) {                                                            \
    debug_message("Ref count dangerously high: %s (%d) calling from %s\n", ob->obname, ob->ref, \
                  str);                                                                         \
  })

#define ROB_STRING_ERROR 1
#define ROB_ARRAY_ERROR 2
#define ROB_MAPPING_ERROR 4
#define ROB_NUMERAL_ERROR 8
#define ROB_GENERAL_ERROR 16
#define ROB_CLASS_ERROR 32
#define ROB_STRING_UTF8_ERROR 64
#define ROB_ERROR 127

#define SETOBNAME(ob, name) (*(const char **)&(ob->obname) = (char *)name)

extern object_t *previous_ob;
extern int save_svalue_depth;
extern object_t **cgsp;
#ifdef F_SET_HIDE
extern int num_hidden;
#endif

void bufcat(char **, char *);
int svalue_save_size(svalue_t *);
void save_svalue(svalue_t *, char **);
int restore_svalue(char *, svalue_t *);
int save_object(object_t *, const char *, int);
int save_object_str(object_t *, int, char *, int);
int restore_object(object_t *, const char *, int);
void restore_variable(svalue_t *, char *);
object_t *get_empty_object(int);
void reset_object(object_t *);
void call_create(object_t *, int);
void reload_object(object_t *);
void free_object(object_t **, const char *const);
#ifdef F_SET_HIDE
int valid_hide(object_t *);
int object_visible(object_t *);
#else
#define object_visible(x) 1
#endif
int find_global_variable(program_t *, const char *const, unsigned short *, int);
void dealloc_object(object_t *, const char *);
void get_objects(object_t ***, int *, get_objectsfn_t, void *);
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_command_giver_stack(void);
#endif
void save_command_giver(object_t *);
void restore_command_giver(void);
void set_command_giver(object_t *);
void clear_non_statics(object_t *ob);
void restore_object_from_buff(object_t *, const char *, int);
#endif
