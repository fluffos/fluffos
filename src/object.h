/*
 * Definition of an object.
 * If the object is inherited, then it must not be destructed !
 *
 * The reset is used as follows:
 * 0: There is an error in the reset() in this object. Never call it again.
 * 1: Normal state.
 * 2 or higher: This is an interactive player, that has not given any commands
 *		for a number of reset periods.
 */

#define O_HEART_BEAT		0x01  /* Does it have an heart beat ? */
#define O_IS_WIZARD		0x02  /* Is it a wizard player.c ? */
#define O_ENABLE_COMMANDS	0x04  /* Can it execute commands ? */
#define O_CLONE			0x08  /* Is it cloned from a master copy ? */
#define O_DESTRUCTED		0x10  /* Is it destructed ? */
#define O_SWAPPED		0x20  /* Is it swapped to file */
#define O_ONCE_INTERACTIVE	0x40  /* Has it ever been interactive ? */
#define O_PRIVILEGED		0x80  /* Privileged? */
#define O_RESET_STATE		0x100 /* Object in a 'reset':ed state ? */
#define O_WILL_CLEAN_UP		0x200 /* clean_up will be called next time */
#define O_VIRTUAL		0x400 /* We're a virtual object */
#define O_HIDDEN		0x800 /* We're hidden from nonprived objs */
#define O_MASTER       0x1000

struct object {
    unsigned short flags;	/* Bits or'ed together from above */
    short total_light;
	int load_time;    /* time when this object was created */
    int next_reset;		/* Time of next reset of this object */
    int time_of_ref;		/* Time when last referenced. Used by swap */
    int ref;			/* Reference count. */
#ifdef DEBUG
    int extra_ref;		/* Used to check ref count. */
#endif
    long swap_num;		/* Swap file offset. -1 is not swapped yet. */
    struct program *prog;
    char *name;
    struct object *next_all, *next_inv, *next_heart_beat, *next_hash;
    struct object *contains;
    struct object *super;		/* Which object surround us ? */
#ifndef NO_SHADOWS /* LPCA */
    struct object *shadowing;		/* Is this object shadowing ? */
    struct object *shadowed;		/* Is this object shadowed ? */
#endif NO_SHADOWS
    struct interactive *interactive;	/* Data about an interactive player */
    struct sentence *sent;
    struct wiz_list *user;		/* What wizard defined this object */
    struct wiz_list *eff_user;		/* Used for permissions */
    struct object *next_hashed_living;
    char *living_name;			/* Name of living object if in hash */
    struct svalue variables[1];		/* All variables to this program */
    /* The variables MUST come last in the struct */
};

extern struct object *load_object PROT((char *, int)),
        *find_object PROT((char *));
extern struct object *get_empty_object(), *find_object PROT((char *)),
	*find_object2 PROT((char *));
extern struct object *current_object, *command_giver;

extern struct object *obj_list;
extern struct object *obj_list_destruct;

struct value;
void remove_destructed_objects(), save_object PROT((struct object *, char *)),
    move_object PROT((struct object *, struct object *)),
    tell_npc PROT((struct object *, char *)),
    free_object PROT((struct object *, char *)),
    reference_prog PROT((struct program *, char *));

void tell_object PROT((struct object *, char *));

int restore_object PROT((struct object *, char *));

#ifdef DEBUG
#define add_ref(ob, str) ob->ref++; \
    if (d_flag > 1) printf("Add_ref %s (%d) from %s\n", ob->name, ob->ref, str)
#else
#define add_ref(ob, str) ob->ref++
#endif
