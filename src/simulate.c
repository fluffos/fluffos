#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "backend.h"
#include "simul_efun.h"
#include "compiler.h"
#include "otable.h"
#include "comm.h"
#include "lex.h"
#include "binaries.h"
#include "swap.h"
#include "socket_efuns.h"
#include "interpret.h"
#include "md.h"
#include "eoperators.h"

/*
 * 'inherit_file' is used as a flag. If it is set to a string
 * after yyparse(), this string should be loaded as an object,
 * and the original object must be loaded again.
 */
char *inherit_file;
#ifdef LPC_TO_C
int compile_to_c;
FILE *compilation_output_file;
char *compilation_ident;
#endif

/* prevents infinite inherit loops.
   No, mark-and-sweep solution won't work.  Exercise for reader.  */
static int num_objects_this_thread = 0;

static struct object *restrict_destruct;

char *last_verb = 0;

struct object *obj_list, *obj_list_destruct;
struct object *current_object;	/* The object interpreting a function. */
struct object *command_giver;	/* Where the current command came from. */
struct object *current_interactive;	/* The user who caused this execution */

#ifdef PRIVS
static void init_privs_for_object PROT((struct object *));
#endif
#ifndef NO_UIDS
static int give_uid_to_object PROT((struct object *));
#endif
static int init_object PROT((struct object *));
static struct svalue *load_virtual_object PROT((char *));
static char *make_new_name PROT((char *));
static void destruct_object_two PROT((struct object *));
static void send_say PROT((struct object *, char *, struct vector *));
static struct sentence *alloc_sentence PROT((void));
#ifndef NO_ADD_ACTION
static void remove_sent PROT((struct object *, struct object *));
#endif
static void error_handler PROT((void));

struct variable *find_status P2(char *, str, int, must_find)
{
    int i;

    for (i = 0; i < (int) current_object->prog->p.i.num_variables; i++) {
	if (strcmp(current_object->prog->p.i.variable_names[i].name, str) == 0)
	    return &current_object->prog->p.i.variable_names[i];
    }
    if (!must_find)
	return 0;
    error("--Status %s not found in prog for %s\n", str,
	  current_object->name);
    return 0;
}

INLINE void check_legal_string P1(char *, s)
{
    if (strlen(s) >= LARGEST_PRINTABLE_STRING) {
	error("Printable strings limited to length of %d.\n",
	      LARGEST_PRINTABLE_STRING);
    }
}

#ifdef PRIVS
static char *privs_file_fname = (char *) 0;

static void
init_privs_for_object P1(struct object *, ob)
{
    struct object *tmp_ob;
    struct svalue *value;
    int err;

    err = assert_master_ob_loaded("[internal] init_privs_for_object", "");
    if (err == -1) {
	tmp_ob = ob;
    } else {
	tmp_ob = master_ob;
    }
    if (!current_object
#ifndef NO_UIDS
	|| !current_object->uid
#endif
    ) {
	ob->privs = NULL;
	return;
    }
    push_string(ob->name, STRING_CONSTANT);
    if (!privs_file_fname)
	privs_file_fname = make_shared_string(APPLY_PRIVS_FILE);
    value = apply(privs_file_fname, tmp_ob, 1, ORIGIN_DRIVER);
    if (value == NULL || value->type != T_STRING)
	ob->privs = NULL;
    else
	ob->privs = make_shared_string(value->u.string);
}
#endif				/* PRIVS */

/*
 * Give the correct uid and euid to a created object.
 */
#ifndef NO_UIDS
static char *creator_file_fname = (char *) 0;

static int give_uid_to_object P1(struct object *, ob)
{
    struct svalue *ret;
    char *creator_name;
    int err;

    err = assert_master_ob_loaded("[internal] give_uid_to_object", "");
    if (err == -1) {
	/*
	 * Only for the master and void object. Note that back_bone_uid is
	 * not defined when master.c is being loaded.
	 */
	ob->uid = add_uid("NONAME");
#ifdef AUTO_SETEUID
	ob->euid = ob->uid;
#else	
	ob->euid = NULL;
#endif
	return 1;
    }
    /*
     * Ask master object who the creator of this object is.
     */
    push_string(ob->name, STRING_CONSTANT);
    if (!creator_file_fname)
	creator_file_fname = make_shared_string(APPLY_CREATOR_FILE);
    ret = apply_master_ob(creator_file_fname, 1);
    if (!ret)
	error("master object: No function " APPLY_CREATOR_FILE "() defined!\n");
    if (!ret || ret == (struct svalue *)-1 || ret->type != T_STRING) {
	struct svalue arg;

	arg.type = T_OBJECT;
	arg.u.ob = ob;
	destruct_object(&arg);
	if (!ret) error("Master object has no function " APPLY_CREATOR_FILE "().\n");
	if (ret == (struct svalue *)-1) error("Can't load objects without a master object.");
	error("Illegal object to load: return value of master::" APPLY_CREATOR_FILE "() was not a string.\n");
    }
    creator_name = ret->u.string;
    /*
     * Now we are sure that we have a creator name. Do not call apply()
     * again, because creator_name will be lost !
     */
    if (strcmp(current_object->uid->name, creator_name) == 0) {
	/*
	 * The loaded object has the same uid as the loader.
	 */
	ob->uid = current_object->uid;
	ob->euid = current_object->euid;	/* FIXME - is this right? */
	return 1;
    }
#ifdef AUTO_TRUST_BACKBONE
    if (strcmp(backbone_uid->name, creator_name) == 0) {
	/*
	 * The object is loaded from backbone. This is trusted, so we let it
	 * inherit the value of eff_user.
	 */
	ob->uid = current_object->euid;
	ob->euid = current_object->euid;
	return 1;
    }
#endif

    /*
     * The object is not loaded from backbone, nor from from the loading
     * objects path. That should be an object defined by another wizard. It
     * can't be trusted, so we give it the same uid as the creator. Also give
     * it eff_user 0, which means that user 'a' can't use objects from user
     * 'b' to load new objects nor modify files owned by user 'b'.
     * 
     * If this effect is wanted, user 'b' must let his object do 'seteuid()' to
     * himself. That is the case for most rooms.
     */
    ob->uid = add_uid(creator_name);
#ifdef AUTO_SETEUID
    ob->euid = ob->uid;
#else
    ob->euid = NULL;
#endif
    return 1;
}
#endif

static int init_object P1(struct object *, ob)
{
#ifndef NO_MUDLIB_STATS
    init_stats_for_object(ob);
#endif
#ifdef PRIVS
    init_privs_for_object(ob);
#endif				/* PRIVS */
#ifndef NO_MUDLIB_STATS
    add_objects(&ob->stats, 1);
#endif
#ifdef NO_ADD_ACTION
    if (function_exists("catch_tell", ob))
	ob->flags |= O_LISTENER;
#endif
#ifndef NO_UIDS
    return give_uid_to_object(ob);
#else
    return 1;
#endif
}


static struct svalue *
       load_virtual_object P1(char *, name)
{
    struct svalue *v;

    if (master_ob == (struct object *)-1) return 0;
    push_string(name, STRING_MALLOC);
    v = apply_master_ob(APPLY_COMPILE_OBJECT, 1);
    if (!v || (v->type != T_OBJECT)) {
	fprintf(stderr, "Could not load file: %s\n", name);
	error("Failed to load file: %s\n", name);
	return 0;
    }
    return v;
}

void set_master P1(struct object *, ob) {
#if !defined(NO_UIDS) || !defined(NO_MUDLIB_STATS)
    int first_load = (master_ob == (struct object *)-1);
#endif
#ifndef NO_UIDS
    struct svalue *ret;
#endif

    master_ob = ob;
    /* Make sure master_ob is never made a dangling pointer. */
    add_ref(master_ob, "set_master");
#ifdef NO_UIDS
#  ifndef NO_MUDLIB_STATS
    if (first_load) {
      set_backbone_domain("BACKBONE");
      set_master_author("NONAME");
    }
#  endif
#else
    ret = apply_master_ob(APPLY_GET_ROOT_UID, 0);
    /* can't be -1 or we wouldn't be here */
    if (ret == 0 || ret->type != T_STRING) {
        debug_fatal("%s() in master object does not work\n",
		    APPLY_GET_ROOT_UID);
    }
    if (first_load) {
      master_ob->uid = set_root_uid(ret->u.string);
      master_ob->euid = master_ob->uid;
#  ifndef NO_MUDLIB_STATS
      set_master_author(ret->u.string);
#  endif
      ret = apply_master_ob(APPLY_GET_BACKBONE_UID, 0);
      if (ret == 0 || ret->type != T_STRING) {
        fatal("%s() in the master file does not work\n", APPLY_GET_BACKBONE_UID);
      }
      set_backbone_uid(ret->u.string);
#  ifndef NO_MUDLIB_STATS
      set_backbone_domain(ret->u.string);
#  endif
    } else {
      master_ob->uid = add_uid(ret->u.string);
      master_ob->euid = master_ob->uid;
    }
#endif
 }

/*
 * Load an object definition from file. If the object wants to inherit
 * from an object that is not loaded, discard all, load the inherited object,
 * and reload again.
 *
 * In mudlib3.0 when loading inherited objects, their reset() is not called.
 * - why is this??  it makes no sense and causes a problem when a developer
 * inherits code from a real used item.  Say a room for example.  In this case
 * the room is loaded but is never set up properly, so when someone enters it
 * it's all messed up.  Realistically, I know that it's pretty bad style to
 * inherit from an object that's actually being used and isn't just a building
 * block, but I see no reason for this limitation.  It happens, and when it
 * does occur, produces mysterious results than can be hard to track down.
 * for now, I've reenabled resetting.  We'll see if anything breaks. -WF
 *
 * Save the command_giver, because reset() in the new object might change
 * it.
 *
 */
/* Beek - changed dont_reset to flags.  It's backwards compatible since
 *        LO_DONT_RESET = 1.  The new flag (LO_SAVE_OUTPUT indicates that
 *        the intermediate C file should not be deleted, allowing it to
 *        be optimized/customized
 */
struct object *load_object P2(char *, lname, int, flags)
{
    int f;

    struct object *ob, *save_command_giver = command_giver;
    struct svalue *mret;
    struct stat c_st;
    int name_length;
    char real_name[200], name[200];
    char *p;
    int is_master_ob=0;
#ifdef LPC_TO_C
    char out_name[200];
    char *out_ptr = out_name;
#ifdef RUNTIME_LOADING
    char *code;
    void (**jump_table) (struct svalue *);
    int err;
#endif
    int save_compile_to_c = compile_to_c;
#endif

    if (++num_objects_this_thread > INHERIT_CHAIN_SIZE)
	error("Inherit chain too deep: > %d\n", INHERIT_CHAIN_SIZE);
#ifndef NO_UIDS
    if (current_object && current_object->euid == NULL)
	error("Can't load objects when no effective user.\n");
#endif
    /* don't allow consecutive "/"'s - Wayfarer */
    p = lname;
    while (*p) {
	if (*p == '/' && *(p + 1) == '/') {
	    error("Filenames with consecutive /'s in them aren't allowed.\n");
	    return 0;
	}
	p++;
    }
    /* Truncate possible .c in the object name. */
    /* Remove leading '/' if any. */
    while (lname[0] == '/')
	lname++;
    strncpy(name, lname, sizeof(name) - 1);
    name[sizeof name - 1] = '\0';
    name_length = strlen(name);
    if (name_length > sizeof name - 4)
	name_length = sizeof name - 4;
    name[name_length] = '\0';
    if (name[name_length - 2] == '.' &&
#ifndef LPC_TO_C
	name[name_length - 1] == 'c') {
#else
	(name[name_length - 1] == 'c' || name[name_length - 1] == 'C')) {
#endif
	name[name_length - 2] = '\0';
	name_length -= 2;
    }
    if (!master_ob_is_loading && !strcmp(name, master_file_name)) {
	master_ob_is_loading = 1;
	is_master_ob = 1;
	/* free the old copy */
	if (master_ob && master_ob != (struct object *)-1) {
	    free_object(master_ob, "apply_master_ob");
	    master_ob = 0;
	}
    }
    /*
     * First check that the c-file exists.
     */
    (void) strcpy(real_name, name);
    (void) strcat(real_name, ".c");
    if (!simul_efun_is_loading && !strcmp(real_name, simul_efun_file_name)){
        simul_efun_is_loading = 1;
    }
    if (stat(real_name, &c_st) == -1) {
#ifdef LPC_TO_C
	real_name[strlen(real_name) - 1] = 'C';

	if (stat(real_name, &c_st) != -1) {
	    struct svalue *mret;

	    /* will get copied */
	    push_string(real_name, STRING_MALLOC);
	    mret = apply_master_ob(APPLY_VALID_COMPILE_TO_C, 1);
	    if (MASTER_APPROVED(mret)) {
		compile_to_c = 1;
		strcpy(out_ptr, SAVE_BINARIES);
		if (*out_ptr == '/')
		    out_ptr++;
		strcat(out_ptr, "/");
		strcat(out_ptr, name);
		strcat(out_ptr, ".c");
	    }
	} else {
#endif
	    struct svalue *v;

	    if (is_master_ob) {
		master_ob_is_loading = 0;
		return 0;
	    }
	    if (simul_efun_is_loading){
	        simul_efun_is_loading = 0;
                return 0;
	    }
	    if (!(v = load_virtual_object(name))) {
		return 0;
	    }
	    /* Now set the file name of the specified object correctly... */
	    ob = v->u.ob;
	    remove_object_hash(ob);
	    if (ob->name)
		FREE(ob->name);
	    ob->name = string_copy(name, "load_object");
	    SET_TAG(ob->name, TAG_OBJ_NAME);
	    enter_object_hash(ob);
	    ob->flags |= O_VIRTUAL;
	    ob->load_time = current_time;
#ifndef LPC_TO_C
	    return ob;
#else
	    compile_to_c = save_compile_to_c;
	    return ob;
	}
#endif
    }
    /*
     * Check if it's a legal name.
     */
    if (!legal_path(real_name)) {
	if (is_master_ob) master_ob_is_loading = 0;
	if (simul_efun_is_loading) simul_efun_is_loading = 0;
	fprintf(stderr, "Illegal pathname: %s\n", real_name);
	error("Illegal path name '%s'.\n", real_name);
	return 0;
    }
#ifdef BINARIES
    if (!load_binary(real_name)) {
#endif
	/* maybe move this section into compile_file? */
	if (comp_flag) {
	    fprintf(stderr, " compiling %s ...", real_name);
#ifdef LATTICE
	    fflush(stderr);
#endif
	}
	f = open(real_name, O_RDONLY);
	if (f == -1) {
	    if (is_master_ob) master_ob_is_loading = 0;
	    if (simul_efun_is_loading) simul_efun_is_loading = 0;
	    perror(real_name);
	    error("Could not read the file '%s'.\n", real_name);
	}
#ifdef LPC_TO_C
	if (compile_to_c) {
	    compilation_output_file = crdir_fopen(out_ptr);
	    if (compilation_output_file == 0) {
		if (is_master_ob) master_ob_is_loading = 0;
		if (simul_efun_is_loading) simul_efun_is_loading = 0;
		perror(out_ptr);
		error("Could not open output file '%s'.\n", out_ptr);
	    }
	    compilation_ident = 0;
	    compile_file(f, real_name);
	    fclose(compilation_output_file);
	    if (prog) {
#ifdef RUNTIME_LOADING
		err = compile_and_link(out_ptr, &jump_table, &code, "errors", "OUT");
		if (err) {
		    compile_file_error(err, "compile_and_link");
		    if (prog)
			free_prog(prog, 1);
		    prog = 0;
		} else
		    link_jump_table(prog, jump_table, code);
#else
		if (prog)
		    free_prog(prog, 1);
		prog = 0;
#endif
	    }
	    if (!(flags & LO_SAVE_OUTPUT))
		unlink(out_ptr);
	} else {
#endif
	    compile_file(f, real_name);
#ifdef LPC_TO_C
	}
#endif
	if (comp_flag)
	    fprintf(stderr, " done\n");
	update_compile_av(total_lines);
	total_lines = 0;
	close(f);
#ifdef BINARIES
    }
#endif

    /* Sorry, can't handle objects without programs yet. */
    if (inherit_file == 0 && (num_parse_error > 0 || prog == 0)) {
	if (is_master_ob) master_ob_is_loading = 0;
	if (simul_efun_is_loading) simul_efun_is_loading = 0;
	if (prog)
	    free_prog(prog, 1);
	if (num_parse_error == 0 && prog == 0)
	    error("No program in object '%s'!\n", lname);
	error("Error in loading object '%s'\n", lname);
    }
    /*
     * This is an iterative process. If this object wants to inherit an
     * unloaded object, then discard current object, load the object to be
     * inherited and reload the current object again. The global variable
     * "inherit_file" will be set by lang.y to point to a file name.
     */
    if (inherit_file) {
	char *tmp = inherit_file;

	if (prog) {
	    free_prog(prog, 1);
	    prog = 0;
	}
	if (strcmp(inherit_file, name) == 0) {
	    if (is_master_ob) master_ob_is_loading = 0;
	    if (simul_efun_is_loading) simul_efun_is_loading = 0;
	    FREE(inherit_file);
	    inherit_file = 0;
	    error("Illegal to inherit self.\n");
	}
	inherit_file = 0;
#if 0				/* MUDLIB3_NEED, It's very awkard to have to
				 * have a debug3 /JnA */
	load_object(tmp, LO_DONT_RESET);
#else
	/*
	 * Beek - all the flags refer to this file and shouldn't be passed
	 * down
	 */
	load_object(tmp, 0);	/* Remove this feature for now */
#endif
	FREE(tmp);

	/*
	 * Yes, the following is necessary.  It is possible that when we
	 * loaded the inherited object, it loaded this object from it's
	 * create function. Without this check, that would crash the driver.
	 * -Beek
	 */
	if (!(ob = lookup_object_hash(name))) {
	    ob = load_object(name, flags);
	    ob->load_time = current_time;
	}
	num_objects_this_thread--;
	if (is_master_ob) {
	  master_ob_is_loading = 0;
	  set_master(ob);
	}
#ifdef LPC_TO_C
	compile_to_c = save_compile_to_c;
#endif
	return ob;
    }
    ob = get_empty_object(prog->p.i.num_variables);
    /* Shared string is no good here */
    ob->name = string_copy(name, "load_object");
    SET_TAG(ob->name, TAG_OBJ_NAME);
    ob->prog = prog;
    ob->flags |= O_WILL_RESET;	/* must be before reset is first called */
    ob->next_all = obj_list;
    obj_list = ob;
    enter_object_hash(ob);	/* add name to fast object lookup table */
    push_object(ob);
    mret = apply_master_ob(APPLY_VALID_OBJECT, 1);
    if (mret && !MASTER_APPROVED(mret)) {
        if (simul_efun_is_loading) simul_efun_is_loading = 0;
	destruct_object_two(ob);
	error("master object: " APPLY_VALID_OBJECT "() denied permission to load %s.\n", name);
    }
    /* allow updating of simul_efun and adding of new functions -bobf */
    /* moved to here by Beek so we can set the simul_efun_ob */
    if (simul_efun_is_loading && prog && simul_efun_file_name &&
	(strcmp(prog->name, simul_efun_file_name) == 0)) {
        simul_efun_is_loading = 0;
	get_simul_efuns(ob->prog);
	if (simul_efun_ob)
	    free_object(simul_efun_ob, "load_object");
	simul_efun_ob = ob;
	add_ref(simul_efun_ob, "load_object");
    }

    if (init_object(ob) && !(flags & LO_DONT_RESET))
	call_create(ob, 0);
    if (!(ob->flags & O_DESTRUCTED) &&
	function_exists(APPLY_CLEAN_UP, ob)) {
	ob->flags |= O_WILL_CLEAN_UP;
    }
    command_giver = save_command_giver;
#ifdef DEBUG
    if (d_flag > 1 && ob)
	debug_message("--%s loaded\n", ob->name);
#endif
    ob->load_time = current_time;
    num_objects_this_thread--;
    if (is_master_ob) {
      master_ob_is_loading = 0;
      set_master(ob);
    }
#ifdef LPC_TO_C
    compile_to_c = save_compile_to_c;
#endif
    return ob;
}

static char *make_new_name P1(char *, str)
{
    static int i;
    char *p = DXALLOC(strlen(str) + 10, TAG_OBJ_NAME, "make_new_name");

    (void) sprintf(p, "%s#%d", str, i);
    i++;
    return p;
}


/*
 * Save the command_giver, because reset() in the new object might change
 * it.
 */
struct object *clone_object P2(char *, str1, int, num_arg)
{
    struct object *ob, *new_ob;
    struct object *save_command_giver = command_giver;

#ifndef NO_UIDS
    if (current_object && current_object->euid == 0) {
	error("Object must call seteuid() prior to calling clone_object().\n");
    }
#endif
    num_objects_this_thread = 0;
    ob = find_object(str1);
    if (ob && !object_visible(ob))
	ob = 0;
    /*
     * If the object self-destructed...
     */
    if (ob == 0) {		/* fix from 3.1.1 */
	pop_n_elems(num_arg);
	return (0);
    }
    if (ob->super)
	error("Cannot clone a object which has an environment.\n");
    if (ob->flags & O_CLONE)
	if (!(ob->flags & O_VIRTUAL) || strrchr(str1, '#'))
	    error("Cannot clone from a clone\n");
	else {
	    /*
	     * well... it's a virtual object.  So now we're going to "clone"
	     * it.
	     */
	    struct svalue *v;
	    char *p;

	    pop_n_elems(num_arg); /* possibly this should be smarter */
	                          /* but then, this whole section is a
				     kludge and should be looked at.

				     Note that create() never gets called
				     in clones of virtual objects.
				     -Beek */
	    
	    /* Remove leading '/' if any. */
	    while (str1[0] == '/')
		str1++;
	    p = str1;
	    while (*p) {
		if (*p == '/' && *(p + 1) == '/') {
		    error("Filenames with consecutive /'s in them aren't allowed.\n");
		    return (0);
		}
		p++;
	    }
	    if (ob->ref == 1 && !ob->super && !ob->contains) {
		/*
		 * ob unused so reuse it instead to save space. (possibly
		 * loaded just for cloning)
		 */
		new_ob = ob;
	    } else {
		/* can't reuse, so load another */
		if (!(v = load_virtual_object(str1))) {
		    return 0;
		}
		new_ob = v->u.ob;
	    }
	    remove_object_hash(new_ob);
	    if (new_ob->name)
		FREE(new_ob->name);
	    /* Now set the file name of the specified object correctly... */
	    new_ob->name = make_new_name(str1);
	    enter_object_hash(new_ob);
	    new_ob->flags |= O_VIRTUAL;
	    new_ob->load_time = current_time;
	    command_giver = save_command_giver;
	    return (new_ob);
	    /*
	     * we can skip all of the stuff below since we were already
	     * cloned once to have gotten to this stage.
	     */
	}
    /* We do not want the heart beat to be running for unused copied objects */
    if (ob->flags & O_HEART_BEAT)
	(void) set_heart_beat(ob, 0);
    new_ob = get_empty_object(ob->prog->p.i.num_variables);
    new_ob->name = make_new_name(ob->name);
    new_ob->flags |= (O_CLONE | (ob->flags & (O_WILL_CLEAN_UP | O_WILL_RESET)));
    new_ob->load_time = ob->load_time;
    new_ob->prog = ob->prog;
    reference_prog(ob->prog, "clone_object");
    DEBUG_CHECK(!current_object, "clone_object() from no current_object !\n");

    init_object(new_ob);

    new_ob->next_all = obj_list;
    obj_list = new_ob;
    enter_object_hash(new_ob);	/* Add name to fast object lookup table */
    call_create(new_ob, num_arg);
    command_giver = save_command_giver;
    /* Never know what can happen ! :-( */
    if (new_ob->flags & O_DESTRUCTED)
	return (0);
    return (new_ob);
}

struct object *environment P1(struct svalue *, arg)
{
    struct object *ob = current_object;

    if (arg && arg->type == T_OBJECT)
	ob = arg->u.ob;
    if (ob == 0 || ob->super == 0 || (ob->flags & O_DESTRUCTED))
	return 0;
    if (ob->flags & O_DESTRUCTED)
	error("environment() of destructed object.\n");
    return ob->super;
}

/*
 * Execute a command for an object. Copy the command into a
 * new buffer, because 'parse_command()' can modify the command.
 * If the object is not current object, static functions will not
 * be executed. This will prevent forcing users to do illegal things.
 *
 * Return cost of the command executed if success (> 0).
 * When failure, return 0.
 */
#ifndef NO_ADD_ACTION
int command_for_object P2(char *, str, struct object *, ob)
{
    char buff[1000];
    int save_eval_cost = eval_cost;

    if (strlen(str) > sizeof(buff) - 1)
	error("Too long command.\n");
    if (ob == 0)
	ob = current_object;
    else if (ob->flags & O_DESTRUCTED)
	return 0;
    strncpy(buff, str, sizeof buff);
    buff[sizeof buff - 1] = '\0';
    if (parse_command(buff, ob))
	return save_eval_cost - eval_cost;
    else
	return 0;
}
#endif

/*
 * With no argument, present() looks in the inventory of the current_object,
 * the inventory of our super, and our super.
 * If the second argument is nonzero, only the inventory of that object
 * is searched.
 */


static struct object *object_present2 PROT((char *, struct object *));


struct object *object_present P2(struct svalue *, v, struct object *, ob)
{
    struct svalue *ret;
    struct object *ret_ob;
    int specific = 0;

    if (ob == 0)
	ob = current_object;
    else
	specific = 1;
    if (ob->flags & O_DESTRUCTED)
	return 0;
    if (v->type == T_OBJECT) {
	if (specific) {
	    if (v->u.ob->super == ob)
		return v->u.ob;
	    else
		return 0;
	}
	if (v->u.ob->super == ob ||
	    (v->u.ob->super == ob->super && ob->super != 0))
	    return v->u.ob->super;
	return 0;
    }
    ret_ob = object_present2(v->u.string, ob->contains);
    if (ret_ob)
	return ret_ob;
    if (specific)
	return 0;
    if (ob->super) {
	push_string(v->u.string, STRING_CONSTANT);
	ret = apply(APPLY_ID, ob->super, 1, ORIGIN_DRIVER);
	if (ob->super->flags & O_DESTRUCTED)
	    return 0;
	if (!IS_ZERO(ret)) {
#if 0
	    /*
	     * if id() returns a value of type object then query that object.
	     * this will allow container objects to allow objects inside them
	     * to be referred to (for attack or whatever).
	     */
	    if (ret->type == T_OBJECT)
		return ret->u.ob;
	    else
#endif				/* 0 */
		return ob->super;
	}
	return object_present2(v->u.string, ob->super->contains);
    }
    return 0;
}

static struct object *object_present2 P2(char *, str, struct object *, ob)
{
    struct svalue *ret;
    char *p;
    int count = 0, length;
    char *item;

    item = string_copy(str, "object_present2");
    length = strlen(item);
    p = item + length - 1;
    if (*p >= '0' && *p <= '9') {
	while (p > item && *p >= '0' && *p <= '9')
	    p--;
	if (p > item && *p == ' ') {
	    count = atoi(p + 1) - 1;
	    *p = '\0';
	    length = p - item;	/* This is never used again ! */
	}
    }
    for (; ob; ob = ob->next_inv) {
	push_string(item, STRING_CONSTANT);
	ret = apply(APPLY_ID, ob, 1, ORIGIN_DRIVER);
	if (ob->flags & O_DESTRUCTED) {
	    FREE(item);
	    return 0;
	}
	if (IS_ZERO(ret))
	    continue;
	if (count-- > 0)
	    continue;
	FREE(item);
#if 0
	if (ret->type == T_OBJECT)
	    return ret->u.ob;
	else
#endif
	    return ob;
    }
    FREE(item);
    return 0;
}

/*
 * Remove an object. It is first moved into the destruct list, and
 * not really destructed until later. (see destruct2()).
 */
static void destruct_object_two P1(struct object *, ob)
{
    struct object *super;
    struct object **pp;
    int removed;
    struct object *save_restrict_destruct = restrict_destruct;

    if (restrict_destruct && restrict_destruct != ob)
	error("Only this_object() can be destructed from move_or_destruct.\n");
#ifdef SOCKET_EFUNS
    /*
     * check if object has an efun socket referencing it for a callback. if
     * so, close the efun socket.
     */
    if (ob->flags & O_EFUN_SOCKET) {
	close_referencing_sockets(ob);
    }
#endif

    if (ob->flags & O_DESTRUCTED) {
	return;
    }
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    remove_object_from_stack(ob);
    /*
     * If this is the first object being shadowed by another object, then
     * destruct the whole list of shadows.
     */
#ifndef NO_SHADOWS
    if (ob->shadowed && !ob->shadowing) {
	struct svalue svp;
	struct object *ob2;

	svp.type = T_OBJECT;
	/*
	 * move from bottom to top of shadow chain
	 */
	for (ob2 = ob->shadowed; ob2->shadowed; ob2 = ob2->shadowed);
	/*
	 * remove shadows top...down being careful to unlink shadow being
	 * destructed from chain
	 */
	for (; ob2;) {
	    svp.u.ob = ob2;
	    ob2 = ob2->shadowing;
	    if (ob2)
		ob2->shadowed = 0;
	    svp.u.ob->shadowing = 0;
	    destruct_object(&svp);
	}
	return;
    }
    /*
     * The chain of shadows is a double linked list. Take care to update it
     * correctly.
     */
    if (ob->shadowing)
	ob->shadowing->shadowed = ob->shadowed;
    if (ob->shadowed)
	ob->shadowed->shadowing = ob->shadowing;
    ob->shadowing = 0;
    ob->shadowed = 0;
#endif

#ifdef DEBUG
    if (d_flag > 1)
	debug_message("Destruct object %s (ref %d)\n", ob->name, ob->ref);
#endif

    /* try to move our contents somewhere */
    super = ob->super;

    while (ob->contains) {
	struct svalue svp;
	svp.type = T_OBJECT;
	svp.u.ob = ob->contains;
	/*
	 * An error here will not leave destruct() in an inconsistent
	 * stage.
	 */
	if (super && !(super->flags & O_DESTRUCTED))
	    push_object(super);
	else
	    push_number(0);
	    
	restrict_destruct = ob->contains;
	(void)apply(APPLY_MOVE, ob->contains, 1, ORIGIN_DRIVER);
	restrict_destruct = save_restrict_destruct;
	/* OUCH! we could be dested by this. -Beek */
	if (ob->flags & O_DESTRUCTED) return;
	if (svp.u.ob == ob->contains)
	    destruct_object(&svp);
    }
    
#ifndef NO_MUDLIB_STATS
    add_objects(&ob->stats, -1);
#endif
    if (ob->interactive) {
	struct object *save = command_giver;

	command_giver = ob;
#ifdef F_ED
	if (ob->interactive->ed_buffer) {
	    save_ed_buffer();
	}
#endif
#ifndef OLD_ED
	if (ob->flags & O_IN_EDIT) {
	    object_save_ed_buffer(ob);
	    ob->flags &= ~O_IN_EDIT;
	}
#endif
	command_giver = save;
    }
    /*
     * Remove us out of this current room (if any). Remove all sentences
     * defined by this object from all objects here.
     */
    if (ob->super) {
#ifndef NO_LIGHT
	add_light(ob->super, -ob->total_light);
#endif
#ifndef NO_ADD_ACTION
	if (ob->super->flags & O_ENABLE_COMMANDS)
	    remove_sent(ob, ob->super);
#endif
	for (pp = &ob->super->contains; *pp;) {
#ifndef NO_ADD_ACTION
	    if ((*pp)->flags & O_ENABLE_COMMANDS)
		remove_sent(ob, *pp);
#endif
	    if (*pp != ob)
		pp = &(*pp)->next_inv;
	    else
		*pp = (*pp)->next_inv;
	}
    }
    /*
     * Now remove us out of the list of all objects. This must be done last,
     * because an error in the above code would halt execution.
     */
    removed = 0;
    for (pp = &obj_list; *pp; pp = &(*pp)->next_all) {
	if (*pp != ob)
	    continue;
	*pp = (*pp)->next_all;
	removed = 1;
	remove_object_hash(ob);
	break;
    }
    DEBUG_CHECK(!removed, "Failed to delete object.\n");

#ifndef NO_ADD_ACTION
    if (ob->living_name)
	remove_living_name(ob);
    ob->flags &= ~O_ENABLE_COMMANDS;
#endif
    ob->super = 0;
    ob->next_inv = 0;
    ob->contains = 0;
    ob->next_all = obj_list_destruct;
    obj_list_destruct = ob;
    set_heart_beat(ob, 0);
    ob->flags |= O_DESTRUCTED;
    /* moved this here from destruct2() -- see comments in destruct2() */
    if (ob->interactive) {
	remove_interactive(ob);
    }
}

void destruct_object P1(struct svalue *, v)
{
    struct object *ob = (struct object *) NULL;

    if (v->type == T_OBJECT) {
	ob = v->u.ob;
	destruct_object_two(ob);
    } else {
	error("destruct_object: called without an object argument\n");
    }
}

/*
 * This one is called when no program is executing from the main loop.
 */
void destruct2 P1(struct object *, ob)
{
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("Destruct-2 object %s (ref %d)\n", ob->name, ob->ref);
    }
#endif
#if 0
    /*
     * moved this into destruct_object() to deal with the 0 in users() efun
     * output problem
     */
    if (ob->interactive)
	remove_interactive(ob);
#endif
    /*
     * We must deallocate variables here, not in 'free_object()'. That is
     * because one of the local variables may point to this object, and
     * deallocation of this pointer will also decrease the reference count of
     * this object. Otherwise, an object with a variable pointing to itself,
     * would never be freed. Just in case the program in this object would
     * continue to execute, change string and object variables into the
     * number 0.
     */
    if (ob->prog->p.i.num_variables > 0) {
	/*
	 * Deallocate variables in this object. The space of the variables
	 * are not deallocated until the object structure is freed in
	 * free_object().
	 */
	int i;

	for (i = 0; i < (int) ob->prog->p.i.num_variables; i++) {
	    free_svalue(&ob->variables[i], "destruct2");
	    ob->variables[i] = const0n;
	}
    }
    free_object(ob, "destruct_object");
}

/*
 * say() efun - send a message to:
 *  all objects in the inventory of the source,
 *  all objects in the same environment as the source,
 *  and the object surrounding the source.
 *
 * when there is no command_giver, current_object is used as the source,
 *  otherwise, command_giver is used.
 *
 * message never goes to objects in the avoid vector, or the source itself.
 *
 * rewritten, bobf@metronet.com (Blackthorn) 9/6/93
 */

static void send_say P3(struct object *, ob, char *, text, struct vector *, avoid)
{
    int valid, j;

    for (valid = 1, j = 0; j < avoid->size; j++) {
	if (avoid->item[j].type != T_OBJECT)
	    continue;
	if (avoid->item[j].u.ob == ob) {
	    valid = 0;
	    break;
	}
    }

    if (!valid)
	return;

    tell_object(ob, text);
}

void say P2(struct svalue *, v, struct vector *, avoid)
{
    struct object *ob, *origin, *save_command_giver = command_giver;
    char *buff;

    check_legal_string(v->u.string);
    buff = v->u.string;

    if (current_object->flags & O_LISTENER || current_object->interactive)
	command_giver = current_object;
    if (command_giver)
	origin = command_giver;
    else
	origin = current_object;

    /* To our surrounding object... */
    if ((ob = origin->super)) {
	if (ob->flags & O_LISTENER || ob->interactive)
	    send_say(ob, buff, avoid);

	/* And its inventory... */
	for (ob = origin->super->contains; ob; ob = ob->next_inv) {
	    if (ob != origin && (ob->flags & O_LISTENER || ob->interactive)) {
		send_say(ob, buff, avoid);
		if (ob->flags & O_DESTRUCTED)
		    break;
	    }
	}
    }
    /* Our inventory... */
    for (ob = origin->contains; ob; ob = ob->next_inv) {
	if (ob->flags & O_LISTENER || ob->interactive) {
	    send_say(ob, buff, avoid);
	    if (ob->flags & O_DESTRUCTED)
		break;
	}
    }

    command_giver = save_command_giver;
}

/*
 * Sends a string to all objects inside of a specific object.
 * Revised, bobf@metronet.com 9/6/93
 */

void tell_room P3(struct object *, room, struct svalue *, v, struct vector *, avoid)
{
    struct object *ob;
    char *buff;
    int valid, j;
    static char txt_buf[LARGEST_PRINTABLE_STRING];

    switch (v->type) {
    case T_STRING:
	check_legal_string(v->u.string);
	buff = v->u.string;
	break;
    case T_OBJECT:
	buff = v->u.ob->name;
	break;
    case T_NUMBER:
	buff = txt_buf;
	sprintf(buff, "%d", v->u.number);
	break;
    case T_REAL:
	buff = txt_buf;
	sprintf(buff, "%g", v->u.real);
	break;
    default:
	bad_argument(v, T_OBJECT | T_NUMBER | T_REAL | T_STRING,
		     2, F_TELL_ROOM);
    }

    for (ob = room->contains; ob; ob = ob->next_inv) {
	if (!ob->interactive && !(ob->flags & O_LISTENER))
	    continue;

	for (valid = 1, j = 0; j < avoid->size; j++) {
	    if (avoid->item[j].type != T_OBJECT)
		continue;
	    if (avoid->item[j].u.ob == ob) {
		valid = 0;
		break;
	    }
	}

	if (!valid)
	    continue;

	if (!ob->interactive) {
	    tell_npc(ob, buff);
	    if (ob->flags & O_DESTRUCTED)
		break;
	} else {
	    tell_object(ob, buff);
	    if (ob->flags & O_DESTRUCTED)
		break;
	}
    }
}

void shout_string P1(char *, str)
{
    struct object *ob;

    check_legal_string(str);

    for (ob = obj_list; ob; ob = ob->next_all) {
	if (!(ob->flags & O_LISTENER) || (ob == command_giver)
	    || !ob->super)
	    continue;
	tell_object(ob, str);
    }
}

/*
 * This will enable an object to use commands normally only
 * accessible by interactive users.
 * Also check if the user is a wizard. Wizards must not affect the
 * value of the wizlist ranking.
 */

#ifndef NO_ADD_ACTION
void enable_commands P1(int, num)
{
    struct object *pp;

    if (current_object->flags & O_DESTRUCTED)
	return;
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("Enable commands %s (ref %d)\n",
		      current_object->name, current_object->ref);
    }
#endif
    if (num) {
	current_object->flags |= O_ENABLE_COMMANDS;
	command_giver = current_object;
    } else {
	/*
	 * Remove all sentences defined by this object from all objects here.
	 */
	if (current_object->flags & O_ENABLE_COMMANDS) {
	    if (current_object->super) {
		if (current_object->flags & O_ENABLE_COMMANDS)
		    remove_sent(current_object, current_object->super);
		for (pp = current_object->super->contains; pp; pp = pp->next_inv) {
		    if (pp->flags & O_ENABLE_COMMANDS)
			remove_sent(current_object, pp);
		}
	    }
	}
	current_object->flags &= ~O_ENABLE_COMMANDS;
	command_giver = 0;
    }
}
#endif

/*
 * Set up a function in this object to be called with the next
 * user input string.
 */
int input_to P4(struct svalue *, fun, int, flag, int, num_arg, struct svalue *, args)
{
    struct sentence *s;
    struct svalue *x;
    int i;

    if (!command_giver || command_giver->flags & O_DESTRUCTED)
	return 0;
    s = alloc_sentence();
    if (set_call(command_giver, s, flag & ~I_SINGLE_CHAR)) {
	/*
	 * If we have args, we copy them, and adjust the stack automatically
	 * (elsewhere) to avoid double free_svalue()'s
	 */
	if (num_arg) {
	    i = num_arg * sizeof(struct svalue);
	    if ((x = (struct svalue *)
		 DMALLOC(i, TAG_TEMPORARY, "input_to: 1")) == NULL)
		fatal("Out of memory!\n");
	    memcpy(x, args, i);
	} else
	    x = NULL;

	command_giver->interactive->carryover = x;
	command_giver->interactive->num_carry = num_arg;
	if (fun->type == T_STRING) {
	    s->function.s = make_shared_string(fun->u.string);
	    s->flags = 0;
	} else {
	    s->function.f = fun->u.fp;
	    fun->u.fp->ref++;
	    s->flags = V_FUNCTION;
	}
	s->ob = current_object;
	add_ref(current_object, "input_to");
	return 1;
    }
    free_sentence(s);
    return 0;
}

/*
 * Set up a function in this object to be called with the next
 * user input character.
 */
int get_char P4(struct svalue *, fun, int, flag, int, num_arg, struct svalue *, args)
{
    struct sentence *s;
    struct svalue *x;
    int i;

    if (!command_giver || command_giver->flags & O_DESTRUCTED)
	return 0;
    s = alloc_sentence();
    if (set_call(command_giver, s, flag | I_SINGLE_CHAR)) {
	/*
	 * If we have args, we copy them, and adjust the stack automatically
	 * (elsewhere) to avoid double free_svalue()'s
	 */
	if (num_arg) {
	    i = num_arg * sizeof(struct svalue);
	    if ((x = (struct svalue *)
		 DMALLOC(i, TAG_TEMPORARY, "get_char: 1")) == NULL)
		fatal("Out of memory!\n");
	    memcpy(x, args, i);
	} else
	    x = NULL;

	command_giver->interactive->carryover = x;
	command_giver->interactive->num_carry = num_arg;
	if (fun->type == T_STRING) {
           s->function.s = make_shared_string(fun->u.string);
           s->flags = 0;
	} else {
	    s->function.f = fun->u.fp;
	    fun->u.fp->ref++;
	    s->flags = V_FUNCTION;
	}
	s->ob = current_object;
	add_ref(current_object, "get_char");
	return 1;
    }
    free_sentence(s);
    return 0;
}

void print_svalue P1(struct svalue *, arg)
{
    static char tbuf[2048];

    if (arg == 0) {
	tell_object(command_giver, "<NULL>");
    } else
	switch (arg->type) {
	case T_STRING:
	    check_legal_string(arg->u.string);
	    tell_object(command_giver, arg->u.string);
	    break;
	case T_OBJECT:
	    sprintf(tbuf, "OBJ(%s)", arg->u.ob->name);
	    tell_object(command_giver, tbuf);
	    break;
	case T_NUMBER:
	    sprintf(tbuf, "%d", arg->u.number);
	    tell_object(command_giver, tbuf);
	    break;
	case T_REAL:
	    sprintf(tbuf, "%g", arg->u.real);
	    tell_object(command_giver, tbuf);
	    break;
	case T_POINTER:
	    tell_object(command_giver, "<ARRAY>");
	    break;
	case T_MAPPING:
	    tell_object(command_giver, "<MAPPING>");
	    break;
	case T_FUNCTION:
	    tell_object(command_giver, "<FUNCTION>");
	    break;
	case T_BUFFER:
	    tell_object(command_giver, "<BUFFER>");
	    break;
	default:
	    tell_object(command_giver, "<UNKNOWN>");
	    break;
	}
    return;
}

void do_write P1(struct svalue *, arg)
{
    struct object *save_command_giver = command_giver;

#ifndef NO_SHADOWS
    if (command_giver == 0 && current_object->shadowing)
	command_giver = current_object;
    if (command_giver) {
	/* Send the message to the first object in the shadow list */
	while (command_giver->shadowing)
	    command_giver = command_giver->shadowing;
    }
#else
    if (!command_giver)
	command_giver = current_object;
#endif				/* NO_SHADOWS */
    print_svalue(arg);
    command_giver = save_command_giver;
}

/* Find an object. If not loaded, load it !
 * The object may selfdestruct, which is the only case when 0 will be
 * returned.
 */

struct object *find_object P1(char *, str)
{
    struct object *ob;
    char *p;

    /* don't allow consecutive "/"'s - Wayfarer */
    p = str;
    while (*p) {
	if (*p == '/' && *(p + 1) == '/')
	    return 0;
	p++;
    }

    /* Remove leading '/' if any. */
    while (str[0] == '/')
	str++;
    ob = find_object2(str);
    if (ob)
	return ob;
    ob = load_object(str, 0);
    if (!ob || (ob->flags & O_DESTRUCTED))	/* *sigh* */
	return 0;
    if (ob && ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    return ob;
}

#define MAX_OBJECT_NAME_SIZE 2048

/* Look for a loaded object. Return 0 if non found. */
struct object *find_object2 P1(char *, str)
{
    register struct object *ob;
    register int length;
    char p[MAX_OBJECT_NAME_SIZE];

    /* Remove leading '/' if any. */
    while (str[0] == '/')
	str++;
    /* Truncate possible .c in the object name. */
    length = strlen(str);
    if (str[length - 2] == '.' &&
#ifndef LPC_TO_C
	str[length - 1] == 'c') {
#else
	(str[length - 1] == 'c' || str[length - 1] == 'C')) {
#endif
	/* A new writeable copy of the name is needed. */
	strncpy(p, str, MAX_OBJECT_NAME_SIZE);
	str = p;
	str[length - 2] = '\0';
    }
    if ((ob = lookup_object_hash(str))) {
	if (ob->flags & O_SWAPPED)
	    load_ob_from_swap(ob);
	return ob;
    }
    return 0;
}

/*
 * Transfer an object.
 * The object has to be taken from one inventory list and added to another.
 * The main work is to update all command definitions, depending on what is
 * living or not. Note that all objects in the same inventory are affected.
 */
void move_object P2(struct object *, item, struct object *, dest)
{
    struct object **pp, *ob;
#ifndef NO_ADD_ACTION
    struct object *next_ob;
    struct object *save_cmd = command_giver;
#endif

    if (item != current_object)
	error("move_object(): can't move anything other than this_object()\n");
    /* Recursive moves are not allowed. */
    for (ob = dest; ob; ob = ob->super)
	if (ob == item)
	    error("Can't move object inside itself.\n");
#ifndef NO_SHADOWS
    if (item->shadowing)
	error("Can't move an object that is shadowing.\n");
#endif

#ifdef LAZY_RESETS
    try_reset(dest);
#endif
#ifndef NO_LIGHT
    add_light(dest, item->total_light);
#endif
    if (item->super) {
	int okey = 0;

#ifndef NO_ADD_ACTION
	if (item->flags & O_ENABLE_COMMANDS) {
	    remove_sent(item->super, item);
	}
	if (item->super->flags & O_ENABLE_COMMANDS)
	    remove_sent(item, item->super);
#endif
#ifndef NO_LIGHT
	add_light(item->super, -item->total_light);
#endif
	for (pp = &item->super->contains; *pp;) {
	    if (*pp != item) {
#ifndef NO_ADD_ACTION
		if ((*pp)->flags & O_ENABLE_COMMANDS)
		    remove_sent(item, *pp);
		if (item->flags & O_ENABLE_COMMANDS)
		    remove_sent(*pp, item);
#endif
		pp = &(*pp)->next_inv;
		continue;
	    }
	    /*
	     * unlink object from original inventory list
	     */
	    *pp = item->next_inv;
	    okey = 1;
	}
	if (!okey)
	    fatal("Failed to find object %s in super list of %s.\n",
		  item->name, item->super->name);
    }
    /*
     * link object into target's inventory list
     */
    item->next_inv = dest->contains;
    dest->contains = item;
    item->super = dest;

#ifndef NO_ADD_ACTION
    /*
     * Setup the new commands. The order is very important, as commands in
     * the room should override commands defined by the room. Beware that
     * init() in the room may have moved 'item' !
     * 
     * The call of init() should really be done by the object itself (except in
     * the -o mode). It might be too slow, though :-(
     */
    if (item->flags & O_ENABLE_COMMANDS) {
	command_giver = item;
	(void) apply(APPLY_INIT, dest, 0, ORIGIN_DRIVER);
	if ((dest->flags & O_DESTRUCTED) || item->super != dest) {
	    command_giver = save_cmd;	/* marion */
	    return;
	}
    }
    /*
     * Run init of the item once for every present user, and for the
     * environment (which can be a user).
     */
    for (ob = dest->contains; ob; ob = next_ob) {
	next_ob = ob->next_inv;
	if (ob == item)
	    continue;
	if (ob->flags & O_DESTRUCTED)
	    error("An object was destructed at call of " APPLY_INIT "()\n");
	if (ob->flags & O_ENABLE_COMMANDS) {
	    command_giver = ob;
	    (void) apply(APPLY_INIT, item, 0, ORIGIN_DRIVER);
	    if (dest != item->super) {
		command_giver = save_cmd;	/* marion */
		return;
	    }
	}
	if (item->flags & O_DESTRUCTED)	/* marion */
	    error("The object to be moved was destructed at call of " APPLY_INIT "()\n");
	if (item->flags & O_ENABLE_COMMANDS) {
	    command_giver = item;
	    (void) apply(APPLY_INIT, ob, 0, ORIGIN_DRIVER);
	    if (dest != item->super) {
		command_giver = save_cmd;	/* marion */
		return;
	    }
	}
    }
    if (dest->flags & O_DESTRUCTED)	/* marion */
	error("The destination to move to was destructed at call of " APPLY_INIT "()\n");
    if (dest->flags & O_ENABLE_COMMANDS) {
	command_giver = dest;
	(void) apply(APPLY_INIT, item, 0, ORIGIN_DRIVER);
    }
    command_giver = save_cmd;
#endif
}

#ifndef NO_LIGHT
/*
 * Every object has a count of the number of light sources it contains.
 * Update this.
 */

void add_light P2(struct object *, p, int, n)
{
    if (n == 0)
	return;
    p->total_light += n;
    while (p = p->super)
	p->total_light += n;
}
#endif

static struct sentence *sent_free = 0;
int tot_alloc_sentence;

static struct sentence *alloc_sentence()
{
    struct sentence *p;

    if (sent_free == 0) {
	p = ALLOCATE(struct sentence, TAG_SENTENCE, "alloc_sentence");
	tot_alloc_sentence++;
    } else {
	p = sent_free;
	sent_free = sent_free->next;
    }
#ifndef NO_ADD_ACTION
    p->verb = 0;
#endif
    p->function.s = 0;
    p->next = 0;
    return p;
}

#ifdef free
void free_all_sent()
{
    struct sentence *p;

    for (; sent_free; sent_free = p) {
	p = sent_free->next;
	FREE(sent_free);
    }
}
#endif

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_free_sentences() {
    struct sentence *sent = sent_free;

#ifndef NO_UIDS
    if (creator_file_fname)
	EXTRA_REF(BLOCK(creator_file_fname))++;
#endif
#ifdef PRIVS
    if (privs_file_fname)
	EXTRA_REF(BLOCK(privs_file_fname))++;
#endif

    while (sent) {
	DO_MARK(sent, TAG_SENTENCE);
/* Freed sentences should have been freed.  right?
	if (sent->function)
	    EXTRA_REF(BLOCK(sent->function))++;
	if (sent->verb)
	    EXTRA_REF(BLOCK(sent->verb))++;
*/
	sent = sent->next;
    }
}
#endif

void free_sentence P1(struct sentence *, p)
{
    if (p->flags & V_FUNCTION) {
      if (p->function.f)
          free_funp(p->function.f);
      else p->function.f = 0;
    } else {
      if (p->function.s)
          free_string(p->function.s);
      else p->function.s = 0;
    }
#ifndef NO_ADD_ACTION
    if (p->verb)
	free_string(p->verb);
    p->verb = 0;
#endif
    p->next = sent_free;
    sent_free = p;
}

/*
 * Find the sentence for a command from the user.
 * Return success status.
 */

#define MAX_VERB_BUFF 100

#ifndef NO_ADD_ACTION
int user_parser P1(char *, buff)
{
    static char verb_buff[MAX_VERB_BUFF];
    struct object *super;
    struct sentence *s;
    char *p;
    int length;
    struct object *save_current_object = current_object, *save_command_giver = command_giver;
    char *user_verb = 0;
    int where;

#ifdef DEBUG
    if (d_flag > 1)
	debug_message("cmd [%s]: %s\n", command_giver->name, buff);
#endif
    /* strip trailing spaces. */
    for (p = buff + strlen(buff) - 1; p >= buff; p--) {
	if (*p != ' ')
	    break;
	*p = '\0';
    }
    if (buff[0] == '\0')
	return 0;
    length = p - buff + 1;
    p = strchr(buff, ' ');
    if (p == 0) {
	user_verb = findstring(buff);
    } else {
	*p = '\0';
	user_verb = findstring(buff);
	*p = ' ';
	length = p - buff;
    }
    if (!user_verb) {
	/* either an xverb or a verb without a specific add_action */
	user_verb = buff;
    }
    /*
     * copy user_verb into a static character buffer to be pointed to by
     * last_verb.
     */
    strncpy(verb_buff, user_verb, MAX_VERB_BUFF - 1);
    if (p) {
	int pos;

	pos = p - buff;
	if (pos < MAX_VERB_BUFF) {
	    verb_buff[pos] = '\0';
	}
    }
    /* clear_notify(); *//* moved to process_user_command() */
    s = save_command_giver->sent;
    for (; s; s = s->next) {
	struct svalue *ret;
	struct object *command_object;

	if (s->verb == 0)
	    error("No action linked to verb.\n");
	if (s->flags & (V_NOSPACE | V_SHORT)) {
	    if (strncmp(buff, s->verb, strlen(s->verb)) != 0)
		continue;
	} else {
	    /* note: if was add_action(blah, "") then accept it */
	    if (s->verb[0] && (user_verb != s->verb))
		continue;
	}
	/*
	 * Now we have found a special sentence !
	 */
#ifdef DEBUG
	if (d_flag > 1)
	    debug_message("Local command %s on %s\n",
			  s->function, s->ob->name);
#endif
	last_verb = verb_buff;
	if (s->verb && s->verb[0]) {
	    strcpy(verb_buff, s->verb);
	}
	/*
	 * If the function is static and not defined by current object, then
	 * it will fail. If this is called directly from user input, then we
	 * set current_object so that static functions are allowed.
	 * current_object is reset just after the call to apply().
	 */
	where = (current_object ? ORIGIN_EFUN : ORIGIN_DRIVER);
	/*
	 * This is really a kludge for stupid libs (TMI-2) that try to
	 * use previous_object() to figure out who is doing certain things,
	 * and can't really deal with it being zero.
	 */
	if (!current_object)
	    current_object = s->ob;

	/*
	 * Remember the object, to update moves.
	 */
	command_object = s->ob;
	super = command_object->super;
        if (s->flags & V_NOSPACE) {
            push_constant_string(&buff[strlen(s->verb)]);
        } else if (buff[length] == ' ') {
            push_constant_string(&buff[length + 1]);
        } else {
	    push_null();
	}
        if (s->flags & V_FUNCTION) {
            ret = call_function_pointer(s->function.f, 1);
        } else {
            ret = apply(s->function.s, s->ob, 1, where);
        }
 
	/*
	 * prevent an action from moving its associated object into another
	 * another object prior to returning 0.  closes a security hole which
	 * was making the static keyword of no use on actions.
	 * 
	 * BE CAREFUL HERE!.  if the command giver was moved as a result
	 * of the command, or several other likely possibilities, then
	 * s has been freed and possibly reallocated.  command_object
	 * is saved for that reason but whoever added this fix blew it.
	 * -Beek
	 */
	if (IS_ZERO(ret) && (super != command_object->super)) {
	    fprintf(stderr,
	    "** Check '%s' as a possible attempted breach of security **\n",
		    s->ob->name);
	    current_object = save_current_object;
	    break;
	}
	if (current_object->flags & O_DESTRUCTED) {
	    /*
	     * If disable_commands() were called, then there is no
	     * command_giver any longer.
	     */
	    if (command_giver == 0) {
		return 1;
	    }
	    s = command_giver->sent;	/* Restart :-( */
	}
	current_object = save_current_object;
	last_verb = 0;
	/* If we get fail from the call, it was wrong second argument. */
	if (ret && ret->type == T_NUMBER && ret->u.number == 0)
	    continue;
	if (command_giver) {
#ifndef NO_MUDLIB_STATS
	    if (s && command_giver->interactive
#ifndef NO_WIZARDS
		&& !(command_giver->flags & O_IS_WIZARD)
#endif
            )
		add_moves(&command_object->stats, 1);
#endif
	    if (ret == 0)
		add_vmessage("Error: action %s not found.\n", s->function);
	}
	return 1;
    }
    notify_no_command();
    return 0;
}

/*
 * Associate a command with function in this object.
 *
 * The optinal third argument is a flag that will state that the verb should
 * only match against leading characters.
 *
 * The object must be near the command giver, so that we ensure that the
 * sentence is removed when the command giver leaves.
 *
 * If the call is from a shadow, make it look like it is really from
 * the shadowed object.
 */
void add_action P3(struct svalue *, str, char *, cmd, int, flag)
{
    struct sentence *p;
    struct object *ob;

    if (current_object->flags & O_DESTRUCTED)
	return;
    ob = current_object;
#ifndef NO_SHADOWS
    while (ob->shadowing) {
	ob = ob->shadowing;
    }
    /* don't allow add_actions of a static function from a shadowing object */
    if ((ob != current_object) && str->type == T_STRING && is_static(str->u.string, ob)) {
	return;
    }
#endif
    if (command_giver == 0 || (command_giver->flags & O_DESTRUCTED))
	return;
    if (ob != command_giver && ob->super != command_giver &&
	ob->super != command_giver->super && ob != command_giver->super)
	return;			/* No need for an error, they know what they
				 * did wrong. */
#ifdef DEBUG
    if (d_flag > 1)
	debug_message("--Add action %s\n", str);
#endif
    p = alloc_sentence();
    if (str->type == T_STRING) {
      p->function.s = make_shared_string(str->u.string);
      p->flags = flag;
    } else {
      p->function.f = str->u.fp;
      str->u.fp->ref++;
      p->flags = flag | V_FUNCTION;
    }
    p->ob = ob;
    p->next = command_giver->sent;
    if (cmd)
	p->verb = make_shared_string(cmd);
    else
	p->verb = 0;
    command_giver->sent = p;
}

/*
 * Remove sentence with specified verb and action.  Return 1
 * if success.  If command_giver, remove his action, otherwise
 * remove current_object's action.
 */
int remove_action P2(char *, act, char *, verb)
{
    struct object *ob;
    struct sentence **s;

    if (command_giver)
	ob = command_giver;
    else
	ob = current_object;

    if (ob) {
	for (s = &ob->sent; *s; s = &((*s)->next)) {
	    struct sentence *tmp;

	    if (((*s)->ob == current_object) && (!((*s)->flags & V_FUNCTION))
		&& !strcmp((*s)->function.s, act) &&
		!strcmp((*s)->verb, verb)) {
		tmp = *s;
		*s = tmp->next;
		free_sentence(tmp);
		return 1;
	    }
	}
    }
    return 0;
}

/*
 * Remove all commands (sentences) defined by object 'ob' in object
 * 'user'
 */
static void remove_sent P2(struct object *, ob, struct object *, user)
{
    struct sentence **s;

    for (s = &user->sent; *s;) {
	struct sentence *tmp;

	if ((*s)->ob == ob) {
#ifdef DEBUG
	    if (d_flag > 1)
		debug_message("--Unlinking sentence %s\n", (*s)->function);
#endif
	    tmp = *s;
	    *s = tmp->next;
	    free_sentence(tmp);
	} else
	    s = &((*s)->next);
    }
}
#endif /* NO_ADD_ACTION */

void debug_fatal PVARGS(va_alist)
{
    va_list args;
    static char msg_buf[2049];
    char *fmt;
    static int in_fatal = 0;

    /* Prevent double fatal. */
    if (in_fatal)
	abort();
    in_fatal = 1;
#ifdef HAS_STDARG_H
    va_start(args, va_alist);
    fmt = va_alist;
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif
    vsprintf(msg_buf, fmt, args);
    va_end(args);
    fprintf(stderr, "%s", msg_buf);
    fflush(stderr);
    if (current_file)
	(void) fprintf(stderr, "Compilation in progress: %s:%d\n",
		       current_file, current_line);
    if (current_object)
	(void) fprintf(stderr, "Current object was %s\n",
		       current_object->name);
    debug_message("%s", msg_buf);
    if (current_object)
	debug_message("Current object was %s\n", current_object->name);
    debug_message("Dump of variables:\n");
    (void) dump_trace(1);
}

void fatal PVARGS(va_alist)
{
    va_list args;
    char *fmt;
    static char msg_buf[2049];

#ifdef HAS_STDARG_H
    va_start(args, va_alist);
    fmt = va_alist;
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif
    vsprintf(msg_buf, fmt, args);
    debug_fatal("%s", msg_buf);
    va_end(args);
#if !defined(DEBUG_NON_FATAL) || !defined(DEBUG)
    abort();
#endif
}

int num_error = 0;

#ifdef MUDLIB_ERROR_HANDLER
static int num_mudlib_error = 0;
#endif

/*
 * Error() has been "fixed" so that users can catch and throw them.
 * To catch them nicely, we really have to provide decent error information.
 * Hence, all errors that are to be caught
 * (error_recovery_context_exists == 2) construct a string containing
 * the error message, which is returned as the
 * thrown value.  Users can throw their own error values however they choose.
 */

/*
 * This is here because throw constructs its own return value; we dont
 * want to replace it with the system's error string.
 */

void throw_error()
{
    if (error_recovery_context_exists > 1) {
	LONGJMP(error_recovery_context, 1);
	fatal("Throw_error failed!");
    }
    error("Throw with no catch.\n");
}

static char emsg_buf[2000];

static void error_handler()
{				/* message is in emsg_buf */
#ifdef MUDLIB_ERROR_HANDLER
    struct mapping *m;
    struct svalue *mret;
    char *file;
    int line;
#else
    char *object_name;
#endif

    /* in case we're going to jump out of load_object */
    master_ob_is_loading = 0;
    restrict_destruct = 0;
    num_objects_this_thread = 0;/* reset the count */
    if (error_recovery_context_exists == CATCH_ERROR_CONTEXT) {	
        /* user catches this error */
	struct svalue v;
	
#ifdef LOG_CATCHES
	/* This is added so that catches generate messages in the log file. */
#ifdef MUDLIB_ERROR_HANDLER
	if (num_mudlib_error) {
	    debug_message("Error in error handler: ");
#endif
	    debug_message(emsg_buf);
	    if (current_object)
		debug_message("program: %s, object: %s, file: %s\n",
			      current_prog ? current_prog->name : "",
			      current_object->name,
			      get_line_number_if_any());
	    (void) dump_trace(0);
#ifdef MUDLIB_ERROR_HANDLER
	    num_mudlib_error = 0;
	} else {
	    num_mudlib_error++;
	    m = allocate_mapping(6);
	    add_mapping_string(m, "error", emsg_buf);
	    if (current_prog)
		add_mapping_string(m, "program", current_prog->name);
	    if (current_object)
		add_mapping_object(m, "object", current_object);
	    add_mapping_array(m, "trace", get_svalue_trace());
	    get_line_number_info(&file, &line);
	    add_mapping_string(m, "file", file);
	    add_mapping_pair(m, "line", line);
	    push_refed_mapping(m);
	    *(++sp) = const1;
	    mret = apply_master_ob(APPLY_ERROR_HANDLER,2);
	    if ((mret == (struct svalue *)-1) || !mret) {
		debug_message("No error handler for error: ");
		debug_message(emsg_buf);
		if (current_object)
		    debug_message("program: %s, object: %s, file: %s\n",
				  current_prog ? current_prog->name : "",
				  current_object->name,
				  get_line_number_if_any());
		(void) dump_trace(0);
	    } else if (mret->type == T_STRING) {
		debug_message("caught: %s", mret->u.string);
	    }
	    num_mudlib_error--;
	}
#endif
#endif
	v.type = T_STRING;
	v.u.string = emsg_buf;
	v.subtype = STRING_MALLOC;	/* Always reallocate */
	assign_svalue(&catch_value, &v);
	LONGJMP(error_recovery_context, 1);
	fatal("Catch() longjump failed");
    }
    too_deep_error = max_eval_error = 0;
    num_error++;
    if (num_error > 1)
	fatal("Too many simultaneous errors.\n");
#ifdef MUDLIB_ERROR_HANDLER
    if (num_mudlib_error || (error_recovery_context_exists & SAFE_APPLY_ERROR_CONTEXT)) {
        if (num_mudlib_error)
	    debug_message("Error in error handler: ");
	debug_message("%s", emsg_buf);
	if (current_object)
	    debug_message("program: %s, object: %s, file: %s\n",
			  current_prog ? current_prog->name : "",
			  current_object->name,
			  get_line_number_if_any());
	(void) dump_trace(0);
	num_mudlib_error = 0;
    } else {
	num_mudlib_error++;
	m = allocate_mapping(6);
	add_mapping_string(m, "error", emsg_buf);
	if (current_prog)
	    add_mapping_string(m, "program", current_prog->name);
	if (current_object)
	    add_mapping_object(m, "object", current_object);
	add_mapping_array(m, "trace", get_svalue_trace());
	get_line_number_info(&file, &line);
	add_mapping_string(m, "file", file);
	add_mapping_pair(m, "line", line);
	num_error--;
	reset_machine(0);
	push_mapping(m);
	m->ref--;
	mret = apply_master_ob(APPLY_ERROR_HANDLER, 1);
	if ((mret == (struct svalue *)-1) || !mret) {
	    debug_message("No error handler for error: ");
	    debug_message(emsg_buf);
	    if (current_object)
		debug_message("program: %s, object: %s, file: %s\n",
			      current_prog ? current_prog->name : "",
			      current_object->name,
			      get_line_number_if_any());
	} else if (mret->type == T_STRING)
	    debug_message("%s", mret->u.string);
	num_mudlib_error--;
	num_error++;
    }
    if (current_heart_beat) {
	set_heart_beat(current_heart_beat, 0);
	debug_message("Heart beat in %s turned off.\n", current_heart_beat->name);
	if (current_heart_beat->interactive) {
	    struct object *save_cmd = command_giver;

	    command_giver = current_heart_beat;
	    add_message("MudOS driver tells you: You have no heart beat!\n");
	    command_giver = save_cmd;
	}
	current_heart_beat = 0;
    }
    num_error--;
#else
    debug_message("%s", emsg_buf + 1);
    if (current_object) {
#ifndef NO_MUDLIB_STATS
	add_errors(&current_object->stats, 1);
#endif
	debug_message("program: %s, object: %s, file: %s\n",
		      current_prog ? current_prog->name : "",
		      current_object->name,
		      get_line_number_if_any());
    }
#if defined(DEBUG) && defined(TRACE_CODE)
    object_name = dump_trace(1);
#else
    object_name = dump_trace(0);
#endif
    if (object_name) {
	struct object *ob;

	ob = find_object2(object_name);
	if (!ob) {
	    if (command_giver)
		add_vmessage("error when executing program in destroyed object %s\n",
			    object_name);
	    debug_message("error when executing program in destroyed object %s\n",
			  object_name);
	}
    }
    if (command_giver && command_giver->interactive) {
	num_error--;
	/*
	 * The stack must be brought in a usable state. After the call to
	 * reset_machine(), all arguments to error() are invalid, and may not
	 * be used any more. The reason is that some strings may have been on
	 * the stack machine stack, and has been deallocated.
	 */
	/* In some cases we will allow the stack to clear afterwards - Sym */
	if (error_recovery_context_exists != SAFE_APPLY_ERROR_CONTEXT)
	  reset_machine(0);
	num_error++;
#ifndef NO_WIZARDS
	if ((command_giver->flags & O_IS_WIZARD) || !strlen(DEFAULT_ERROR_MESSAGE)) {
#endif
	    add_message(emsg_buf + 1);
	    if (current_object)
		add_vmessage("program: %s, object: %s, file: %s\n",
			    current_prog ? current_prog->name : "",
			    current_object->name,
			    get_line_number_if_any());
#ifndef NO_WIZARDS
	} else {
	    add_vmessage("%s\n", DEFAULT_ERROR_MESSAGE);
	}
#endif
    }
    if (current_heart_beat) {
	set_heart_beat(current_heart_beat, 0);
	debug_message("Heart beat in %s turned off.\n", current_heart_beat->name);
	if (current_heart_beat->interactive) {
	    struct object *save_cmd = command_giver;

	    command_giver = current_heart_beat;
	    add_message("MudOS driver tells you: You have no heart beat!\n");
	    command_giver = save_cmd;
	}
	current_heart_beat = 0;
    }
    num_error--;
#endif
    if (error_recovery_context_exists)
	LONGJMP(error_recovery_context, 1);
    abort();
}

void error_needs_free P1(char *, s)
{
    strncpy(emsg_buf + 1, s, 1998);
    emsg_buf[0] = '*';		/* all system errors get a * at the start */
    emsg_buf[1999] = '\0';
    FREE(s);

    error_handler();
}

void error PVARGS(va_alist)
{
    va_list args;
    char *fmt;

#ifdef HAS_STDARG_H
    va_start(args, va_alist);
    fmt = va_alist;
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif
    vsprintf(emsg_buf + 1, fmt, args);
    va_end(args);
    emsg_buf[0] = '*';		/* all system errors get a * at the start */

    error_handler();
}

/*
 * This one is called from HUP.
 */
int MudOS_is_being_shut_down;

#ifdef SIGNAL_FUNC_TAKES_INT
void startshutdownMudOS P1(int, sig)
#else
void startshutdownMudOS()
#endif
{
    MudOS_is_being_shut_down = 1;
}

/*
 * This one is called from the command "shutdown".
 * We don't call it directly from HUP, because it is dangerous when being
 * in an interrupt.
 */
void shutdownMudOS P1(int, exit_code)
{
    int i;
    shout_string("MudOS driver shouts: shutting down immediately.\n");
#ifndef NO_MUDLIB_STATS
    save_stat_files();
#endif
    ipc_remove();
#ifdef SOCKET_EFUNS
    for (i = 0; i < MAX_EFUN_SOCKS; i++) {
	if (lpc_socks[i].state == CLOSED) continue;
	while (close(lpc_socks[i].fd) == -1 && errno == EINTR)
	    ;
    }
#endif
#ifdef LATTICE
    signal(SIGUSR1, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
#endif
    unlink_swap_file();
#ifdef DEALLOCATE_MEMORY_AT_SHUTDOWN
    remove_all_objects();
    free_all_sent();
#ifndef NO_MUDLIB_STATS
    free_mudlib_stats();
#endif
    dump_malloc_data();
    find_alloced_data();
#endif
#ifdef PROFILING
    monitor(0, 0, 0, 0, 0);	/* cause gmon.out to be written */
#endif
    exit(exit_code);
}

/*
 * Call this one when there is only little memory left. It will start
 * Armageddon.
 */
void slow_shut_down P1(int, minutes)
{
    /*
     * Swap out objects, and free some memory.
     */
    struct svalue *amo;

    push_number(minutes);
    amo = apply_master_ob(APPLY_SLOW_SHUTDOWN, 1);
    /* in this case, approved means the mudlib will handle it */
    if (!MASTER_APPROVED(amo))
    {
	struct object *save_current = current_object, *save_command = command_giver;

	command_giver = 0;
	current_object = 0;
	shout_string("MudOS driver shouts: Out of memory.\n");
	command_giver = save_command;
	current_object = save_current;
#ifdef SIGNAL_FUNC_TAKES_INT
	startshutdownMudOS(1);
#else
	startshutdownMudOS();
#endif
	return;
    }
}

void do_message P5(struct svalue *, class, char *, msg, struct vector *, scope, struct vector *, exclude, int, recurse)
{
    int i, j, valid;
    struct object *ob;

    for (i = 0; i < scope->size; i++) {
	switch (scope->item[i].type) {
	case T_STRING:
	    ob = find_object(scope->item[i].u.string);
	    if (ob && !object_visible(ob))
		continue;
	    break;
	case T_OBJECT:
	    ob = scope->item[i].u.ob;
	    break;
	default:
	    continue;
	}
	if (ob->flags & O_LISTENER || ob->interactive) {
	    for (valid = 1, j = 0; j < exclude->size; j++) {
		if (exclude->item[j].type != T_OBJECT)
		    continue;
		if (exclude->item[j].u.ob == ob) {
		    valid = 0;
		    break;
		}
	    }
	    if (valid) {
		push_svalue(class);
		push_string(msg, STRING_CONSTANT);
		apply(APPLY_RECEIVE_MESSAGE, ob, 2, ORIGIN_DRIVER);
	    }
	} else if (recurse) {
	    struct vector *tmp;

	    tmp = all_inventory(ob, 1);
	    do_message(class, msg, tmp, exclude, 0);
	    free_vector(tmp);
	}
    }
}

#ifdef LAZY_RESETS
INLINE void try_reset P1(struct object *, ob)
{
    if ((ob->next_reset < current_time) && !(ob->flags & O_RESET_STATE)) {
#ifdef DEBUG
	if (d_flag) {
	    fprintf(stderr, "(lazy) RESET %s\n", ob->name);
	}
#endif
	/* need to set the flag here to prevent infinite loops in apply_low */
	ob->flags |= O_RESET_STATE;
	reset_object(ob);
    }
}
#endif

struct object *first_inventory P1(struct svalue *, arg)
{
    struct object *ob;

    if (arg->type == T_STRING) {
	ob = find_object(arg->u.string);
	if (ob && !object_visible(ob))
	    ob = 0;
    } else
	ob = arg->u.ob;
    if (ob == 0)
	bad_argument(arg, T_STRING | T_OBJECT, 1, F_FIRST_INVENTORY);
    ob = ob->contains;
    while (ob) {
	if (ob->flags & O_HIDDEN) {
	    if (object_visible(ob)) {
		return ob;
	    }
	} else
	    return ob;
	ob = ob->next_inv;
    }
    return 0;
}
