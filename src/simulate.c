#define SUPPRESS_COMPILER_INLINES
#include "std.h"
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
#include "md.h"
#include "eoperators.h"
#include "ed.h"
#include "file.h"
#include "packages/parser.h"

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

static object_t *restrict_destruct;

char *last_verb = 0;

int illegal_sentence_action;
object_t *obj_list, *obj_list_destruct;
object_t *current_object;	/* The object interpreting a function. */
object_t *command_giver;	/* Where the current command came from. */
object_t *current_interactive;	/* The user who caused this execution */

#ifdef PRIVS
static void init_privs_for_object PROT((object_t *));
#endif
#ifdef PACKAGE_UIDS
static int give_uid_to_object PROT((object_t *));
#endif
static int init_object PROT((object_t *));
static svalue_t *load_virtual_object PROT((char *));
static char *make_new_name PROT((char *));
#ifndef NO_ENVIRONMENT
static void send_say PROT((object_t *, char *, array_t *));
#endif
static sentence_t *alloc_sentence PROT((void));
#if !defined(NO_ADD_ACTION) && !defined(NO_ENVIRONMENT)
static void remove_sent PROT((object_t *, object_t *));
#endif

INLINE void check_legal_string P1(char *, s)
{
    if (strlen(s) > LARGEST_PRINTABLE_STRING) {
	error("Printable strings limited to length of %d.\n",
	      LARGEST_PRINTABLE_STRING);
    }
}

/* equivalent to strcpy(x, y); return x + strlen(y), but faster and safer */
/* Code like:
 * 
 * char buf[256];
 * strcpy(buf, ...);
 * strcat(buf, ...);
 * strcat(buf, ...);
 *
 * Should be replaced with:
 *
 * char buf[256];
 * char *p, *end = EndOf(buf);
 * p = strput(buf, end, ...);
 * p = strput(p, end, ...);
 * p = strput(p, end, ...);
 */
char *strput P3(char *, x, char *, limit, char *, y) {
    while ((*x++ = *y++)) {
	if (x == limit) {
	    *(x-1) = 0;
	    break;
	}
    }
    return x - 1;
}

char *strput_int P3(char *, x, char *, limit, int, num) {
    char buf[20];
    sprintf(buf, "%d", num);
    return strput(x, limit, buf);
}

#ifdef PRIVS
static char *privs_file_fname = (char *) 0;

static void
init_privs_for_object P1(object_t *, ob)
{
    object_t *tmp_ob;
    svalue_t *value;

    if (!master_ob)
	tmp_ob = ob;
    else
	tmp_ob = master_ob;

    if (!current_object
#ifdef PACKAGE_UIDS
	|| !current_object->uid
#endif
    ) {
	ob->privs = NULL;
	return;
    }
    push_malloced_string(add_slash(ob->name));
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
#ifdef PACKAGE_UIDS
static char *creator_file_fname = (char *) 0;

static int give_uid_to_object P1(object_t *, ob)
{
    svalue_t *ret;
    char *creator_name;

    if (!master_ob) {
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
    push_malloced_string(add_slash(ob->name));
    if (!creator_file_fname)
	creator_file_fname = make_shared_string(APPLY_CREATOR_FILE);
    ret = apply_master_ob(creator_file_fname, 1);
    if (!ret)
	error("master object: No function " APPLY_CREATOR_FILE "() defined!\n");
    if (!ret || ret == (svalue_t *)-1 || ret->type != T_STRING) {
	svalue_t arg;

	destruct_object(ob);
	if (!ret) error("Master object has no function " APPLY_CREATOR_FILE "().\n");
	if (ret == (svalue_t *)-1) error("Can't load objects without a master object.");
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

static int init_object P1(object_t *, ob)
{
#ifdef PACKAGE_MUDLIB_STATS
    init_stats_for_object(ob);
#endif
#ifdef PRIVS
    init_privs_for_object(ob);
#endif				/* PRIVS */
#ifdef PACKAGE_MUDLIB_STATS
    add_objects(&ob->stats, 1);
#endif
#ifdef NO_ADD_ACTION
    if (function_exists(APPLY_CATCH_TELL, ob, 1) ||
	function_exists(APPLY_RECEIVE_MESSAGE, ob, 1))
	ob->flags |= O_LISTENER;
#endif
#ifdef PACKAGE_UIDS
    return give_uid_to_object(ob);
#else
    return 1;
#endif
}


static svalue_t *
       load_virtual_object P1(char *, name)
{
    svalue_t *v;

    if (!master_ob) return 0;
    push_malloced_string(add_slash(name));
    v = apply_master_ob(APPLY_COMPILE_OBJECT, 1);
    if (!v || (v->type != T_OBJECT))
	return 0;
    return v;
}

void set_master P1(object_t *, ob) {
#if defined(PACKAGE_UIDS) || defined(PACKAGE_MUDLIB_STATS)
    int first_load = (!master_ob);
#endif
#ifdef PACKAGE_UIDS
    svalue_t *ret;
#endif

    master_ob = ob;
    /* Make sure master_ob is never made a dangling pointer. */
    add_ref(master_ob, "set_master");
#ifndef PACKAGE_UIDS
#  ifdef PACKAGE_MUDLIB_STATS
    if (first_load) {
      set_backbone_domain("BACKBONE");
      set_master_author("NONAME");
    }
#  endif
#else
    ret = apply_master_ob(APPLY_GET_ROOT_UID, 0);
    /* can't be -1 or we wouldn't be here */
    if (!ret) {
        debug_message("No function %s() in master object; possibly the mudlib doesn't want PACKAGE_UIDS to be defined.\n",
		    APPLY_GET_ROOT_UID);
	exit(-1);
    }
    if (ret->type != T_STRING) {
        debug_message("%s() in master object does not work.\n",
		    APPLY_GET_ROOT_UID);
	exit(-1);
    }
    if (first_load) {
      master_ob->uid = set_root_uid(ret->u.string);
      master_ob->euid = master_ob->uid;
#  ifdef PACKAGE_MUDLIB_STATS
      set_master_author(ret->u.string);
#  endif
      ret = apply_master_ob(APPLY_GET_BACKBONE_UID, 0);
      if (ret == 0 || ret->type != T_STRING) {
	  debug_message("%s() in the master file does not work\n",
		  APPLY_GET_BACKBONE_UID);
	  exit(-1);
      }
      set_backbone_uid(ret->u.string);
#  ifdef PACKAGE_MUDLIB_STATS
      set_backbone_domain(ret->u.string);
#  endif
    } else {
      master_ob->uid = add_uid(ret->u.string);
      master_ob->euid = master_ob->uid;
    }
#endif
 }

char *check_name P1(char *, src) {
    char *p;
    while (*src == '/') src++;
    p = src;
    while (*p) {
	if (*p == '/' && *(p + 1) == '/')
	    return 0;
	p++;
    }
    return src;
}

int strip_name P3(char *, src, char *, dest, int, size) {
    char last_c = 0;
    char *p = dest;
    char *end = dest + size - 1;

    while (*src == '/') src++;

    while (*src && p < end) {
	if (last_c == '/' && *src == '/') return 0;
	last_c = (*p++ = *src++);
    }

    /* In some cases, (for example, object loading) this currently gets
     * run twice, once in find_object, and once in load object.  The
     * net effect of this is:
     * /foo.c -> /foo [no such exists, try to load] -> /foo created
     * /foo.c.c -> /foo.c [no such exists, try to load] -> /foo created
     *
     * causing a duplicate object crash.  There are two ways to fix this:
     * (1) strip multiple .c's so that the output of this routine is something
     *     that doesn't change if this is run again.
     * (2) make sure this routine is only called once on any name.
     *
     * The first solution is the one currently in use.
     */
    while (p - dest > 2 && p[-1] == 'c' && p[-2] == '.') 
	p -= 2;

    *p = 0;
    return 1;
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
#ifdef LPC_TO_C
object_t *int_load_object P2(char *, lname, lpc_object_t *, lpc_obj)
#else
object_t *int_load_object P1(char *, lname)
#endif
{
    int f;
    program_t *prog;
    object_t *ob, *save_command_giver = command_giver;
    svalue_t *mret;
    struct stat c_st;
    char real_name[200], name[200];
#ifdef LPC_TO_C
    char out_name[200];
    char *out_ptr = out_name;
    int save_compile_to_c = compile_to_c;
#endif

    if (++num_objects_this_thread > INHERIT_CHAIN_SIZE)
	error("Inherit chain too deep: > %d when trying to load '%s'.\n", INHERIT_CHAIN_SIZE, lname);
#ifdef PACKAGE_UIDS
    if (current_object && current_object->euid == NULL)
	error("Can't load objects when no effective user.\n");
#endif
    if (!strip_name(lname, name, sizeof name))
	error("Filenames with consecutive /'s in them aren't allowed (%s).\n",
	      lname);

    /*
     * First check that the c-file exists.
     */
    (void) strcpy(real_name, name);
    (void) strcat(real_name, ".c");

    if (stat(real_name, &c_st) == -1) {
	svalue_t *v;

	if (!(v = load_virtual_object(name))) {
	    num_objects_this_thread--;
	    return 0;
	}
	/* Now set the file name of the specified object correctly... */
	ob = v->u.ob;
	remove_object_hash(ob);
	if (ob->name)
	    FREE(ob->name);
	ob->name = alloc_cstring(name, "load_object");
	SET_TAG(ob->name, TAG_OBJ_NAME);
	enter_object_hash(ob);
	ob->flags |= O_VIRTUAL;
	ob->load_time = current_time;
#ifdef LPC_TO_C
	compile_to_c = save_compile_to_c;
#endif
#ifdef PRIVS
	if (ob->privs) free_string(ob->privs);
	init_privs_for_object(ob);
#endif
	num_objects_this_thread--;
	return ob;
    }
    /*
     * Check if it's a legal name.
     */
    if (!legal_path(real_name)) {
	debug_message("Illegal pathname: /%s\n", real_name);
	error("Illegal path name '/%s'.\n", real_name);
	return 0;
    }
#ifdef BINARIES
    if (!(prog = load_binary(real_name, lpc_obj)) && !inherit_file) {
#endif
	/* maybe move this section into compile_file? */
	if (comp_flag) {
	    debug_message(" compiling /%s ...", real_name);
#ifdef LATTICE
	    fflush(stderr);
#endif
	}
	f = open(real_name, O_RDONLY);
	if (f == -1) {
	    debug_perror("compile_file", real_name);
	    error("Could not read the file '/%s'.\n", real_name);
	}
#ifdef LPC_TO_C
	if (compile_to_c) {
	    compilation_output_file = crdir_fopen(out_ptr);
	    if (compilation_output_file == 0) {
		debug_perror("compile_to_c", out_ptr);
		error("Could not open output file '/%s'.\n", out_ptr);
	    }
	    compilation_ident = 0;
	    prog = compile_file(f, real_name);
	    fclose(compilation_output_file);
	    if (prog) {
		free_prog(prog, 1);
		prog = 0;
	    }
	} else {
#endif
	    prog = compile_file(f, real_name);
#ifdef LPC_TO_C
	}
#endif
	if (comp_flag)
	    debug_message(" done\n");
	update_compile_av(total_lines);
	total_lines = 0;
	close(f);
#ifdef BINARIES
    }
#endif

    /* Sorry, can't handle objects without programs yet. */
    if (inherit_file == 0 && (num_parse_error > 0 || prog == 0)) {
	if (prog)
	    free_prog(prog, 1);
	if (num_parse_error == 0 && prog == 0)
	    error("No program in object '/%s'!\n", name);
	error("Error in loading object '/%s'\n", name);
    }
    /*
     * This is an iterative process. If this object wants to inherit an
     * unloaded object, then discard current object, load the object to be
     * inherited and reload the current object again. The global variable
     * "inherit_file" will be set by lang.y to point to a file name.
     */
    if (inherit_file) {
	object_t *inh_obj;
	char inhbuf[MAX_OBJECT_NAME_SIZE];

	if (!strip_name(inherit_file, inhbuf, sizeof inhbuf))
	    strcpy(inhbuf, inherit_file);
	FREE(inherit_file);
	inherit_file = 0;
	
	if (prog) {
	    free_prog(prog, 1);
	    prog = 0;
	}
	if (strcmp(inhbuf, name) == 0) {
	    error("Illegal to inherit self.\n");
	}

	if ((inh_obj = lookup_object_hash(inhbuf))) {
#ifdef LPC_TO_C
	    DEBUG_CHECK(!(inh_obj->flags & O_COMPILED_PROGRAM), "Inherited object is already loaded!\n");
	    inh_obj = load_object(inhbuf, (lpc_object_t *)inh_obj);
#else
	    IF_DEBUG(fatal("Inherited object is already loaded!"));
#endif
	} else {
	    inh_obj = load_object(inhbuf, 0);
	}
	if (!inh_obj) error("Inherited file '/%s' does not exist!\n",
			    inhbuf);

	/*
	 * Yes, the following is necessary.  It is possible that when we
	 * loaded the inherited object, it loaded this object from it's
	 * create function. Without this check, that would crash the driver.
	 * -Beek
	 */
	if (!(ob = lookup_object_hash(name))) {
	    ob = load_object(name, 0);
	    /* sigh, loading the inherited file removed us */
	    if (!ob) { 
		num_objects_this_thread--;
		return 0;
	    }
	    ob->load_time = current_time;
	}
#ifdef LPC_TO_C
	else if (ob->flags & O_COMPILED_PROGRAM) {
	    /* I don't think this is possible, but ... */
	    ob = load_object(name, (lpc_object_t *)ob);
	    /* sigh, loading the inherited file removed us */
	    if (!ob) { num_objects_this_thread--; return 0; }
	    ob->load_time = current_time;
	}
#endif
	num_objects_this_thread--;
#ifdef LPC_TO_C
	compile_to_c = save_compile_to_c;
#endif
	return ob;
    }
    ob = get_empty_object(prog->num_variables_total);
    /* Shared string is no good here */
    ob->name = alloc_cstring(name, "load_object");
    SET_TAG(ob->name, TAG_OBJ_NAME);
    ob->prog = prog;
    ob->flags |= O_WILL_RESET;	/* must be before reset is first called */
    ob->next_all = obj_list;
    obj_list = ob;
    enter_object_hash(ob);	/* add name to fast object lookup table */
    push_object(ob);
    mret = apply_master_ob(APPLY_VALID_OBJECT, 1);
    if (mret && !MASTER_APPROVED(mret)) {
	destruct_object(ob);
	error("master object: " APPLY_VALID_OBJECT "() denied permission to load '/%s'.\n", name);
    }

    if (init_object(ob))
	call_create(ob, 0);
    if (!(ob->flags & O_DESTRUCTED) &&
	function_exists(APPLY_CLEAN_UP, ob, 1)) {
	ob->flags |= O_WILL_CLEAN_UP;
    }
    command_giver = save_command_giver;
#ifdef DEBUG
    if (d_flag > 1 && ob)
	debug_message("--/%s loaded\n", ob->name);
#endif
    ob->load_time = current_time;
    num_objects_this_thread--;
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
object_t *clone_object P2(char *, str1, int, num_arg)
{
    object_t *ob, *new_ob;
    object_t *save_command_giver = command_giver;

#ifdef PACKAGE_UIDS
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
    if (ob->flags & O_CLONE)
	if (!(ob->flags & O_VIRTUAL) || strrchr(str1, '#'))
	    error("Cannot clone from a clone\n");
	else {
	    /*
	     * well... it's a virtual object.  So now we're going to "clone"
	     * it.
	     */
	    svalue_t *v;

	    pop_n_elems(num_arg); /* possibly this should be smarter */
	                          /* but then, this whole section is a
				     kludge and should be looked at.

				     Note that create() never gets called
				     in clones of virtual objects.
				     -Beek */

	    if (!(str1 = check_name(str1))) 
		error("Filenames with consecutive /'s in them aren't allowed (%s).\n", str1);

	    if (ob->ref == 1
#ifndef NO_ENVIRONMENT
		&& !ob->super && !ob->contains
#endif
		) {
		/*
		 * ob unused so reuse it instead to save space. (possibly
		 * loaded just for cloning)
		 */
		new_ob = ob;
	    } else {
		/* can't reuse, so load another */
		if (!(v = load_virtual_object(str1)))
		    return 0;
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
    new_ob = get_empty_object(ob->prog->num_variables_total);
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

#ifndef NO_ENVIRONMENT
object_t *environment P1(svalue_t *, arg)
{
    object_t *ob = current_object;

    if (arg && arg->type == T_OBJECT)
	ob = arg->u.ob;
    if (ob == 0 || ob->super == 0 || (ob->flags & O_DESTRUCTED))
	return 0;
    if (ob->flags & O_DESTRUCTED)
	error("environment() of destructed object.\n");
    return ob->super;
}
#endif

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
int command_for_object P1(char *, str)
{
    char buff[1000];
    int save_eval_cost = eval_cost;

    if (strlen(str) > sizeof(buff) - 1)
	error("Too long command.\n");
    else if (current_object->flags & O_DESTRUCTED)
	return 0;
    strncpy(buff, str, sizeof buff);
    buff[sizeof buff - 1] = '\0';
    if (parse_command(buff, current_object))
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


#ifndef NO_ENVIRONMENT
static object_t *object_present2 PROT((char *, object_t *));

object_t *object_present P2(svalue_t *, v, object_t *, ob)
{
    svalue_t *ret;
    object_t *ret_ob;
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
	push_svalue(v);
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

static object_t *object_present2 P2(char *, str, object_t *, ob)
{
    svalue_t *ret;
    char *p;
    int count = 0, length;

    if ((length = strlen(str))) {
	p = str + length - 1;
	if (isdigit(*p)) {
	    do {
		p--;
	    } while (p > str && isdigit(*p));
	    
	    if (*p == ' ') {
		count = atoi(p + 1) - 1;
		length = p - str;
	    }
	}
    }
    for (; ob; ob = ob->next_inv) {
	p = new_string(length, "object_present2");
	memcpy(p, str, length);
	p[length] = 0;
	push_malloced_string(p);
	ret = apply(APPLY_ID, ob, 1, ORIGIN_DRIVER);
	if (ob->flags & O_DESTRUCTED)
	    return 0;
	if (IS_ZERO(ret))
	    continue;
	if (count-- > 0)
	    continue;
	return ob;
    }
    return 0;
}
#endif

void init_master P1(char *, file) {
    char buf[512];
#ifdef LPC_TO_C
    lpc_object_t *compiled_version;
#endif
    object_t *new_ob;
    
    if (!file || !file[0]) {
	fprintf(stderr, "No master object specified in config file\n");
	exit(-1);
    }
	
    if (!strip_name(file, buf, sizeof buf))
	error("Illegal master file name '%s'\n", file);
    
#ifdef LPC_TO_C
    compiled_version = (lpc_object_t *)lookup_object_hash(buf);
#endif

    if (file[strlen(file) - 2] != '.')
	strcat(buf, ".c");
    
    new_ob = load_object(file, compiled_version);
    if (new_ob == 0) {
	fprintf(stderr, "The master file %s was not loaded.\n",
		file);
	exit(-1);
    }
    set_master(new_ob);
}

static char *saved_master_name;
static char *saved_simul_name;

static void fix_object_names() {
    master_ob->name = saved_master_name;
    simul_efun_ob->name = saved_simul_name;
}

/*
 * Remove an object. It is first moved into the destruct list, and
 * not really destructed until later. (see destruct2()).
 */
void destruct_object P1(object_t *, ob)
{
    object_t **pp;
    int removed;
#ifndef NO_ENVIRONMENT
    object_t *super;
    object_t *save_restrict_destruct = restrict_destruct;
#endif

    if (restrict_destruct && restrict_destruct != ob)
	error("Only this_object() can be destructed from move_or_destruct.\n");

#ifdef PACKAGE_SOCKETS
    /*
     * check if object has an efun socket referencing it for a callback. if
     * so, close the efun socket.
     */
    if (ob->flags & O_EFUN_SOCKET) {
	close_referencing_sockets(ob);
    }
#endif
#ifdef PACKAGE_PARSER
    if (ob->pinfo) {
	parse_free(ob->pinfo);
	ob->pinfo = 0;
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
	object_t *otmp;
	object_t *ob2;

	/*
	 * move from bottom to top of shadow chain
	 */
	for (ob2 = ob->shadowed; ob2->shadowed; ob2 = ob2->shadowed);
	/*
	 * remove shadows top...down being careful to unlink shadow being
	 * destructed from chain
	 */
	for (; ob2;) {
	    otmp = ob2;
	    ob2 = ob2->shadowing;
	    if (ob2)
		ob2->shadowed = 0;
	    otmp->shadowing = 0;
	    destruct_object(otmp);
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
	debug_message("Deobject_t /%s (ref %d)\n", ob->name, ob->ref);
#endif

#ifndef NO_ENVIRONMENT
    /* try to move our contents somewhere */
    super = ob->super;

    while (ob->contains) {
	object_t *otmp;

	otmp = ob->contains;
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
	if (otmp == ob->contains)
	    destruct_object(otmp);
    }
#endif
    
#ifdef PACKAGE_MUDLIB_STATS
    add_objects(&ob->stats, -1);
#endif
#ifdef OLD_ED
    if (ob->interactive && ob->interactive->ed_buffer)
	save_ed_buffer(ob);
#else
    if (ob->flags & O_IN_EDIT) {
	object_save_ed_buffer(ob);
	ob->flags &= ~O_IN_EDIT;
    }
#endif
#ifndef NO_ENVIRONMENT
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
#endif

    /* At this point, we can still back out, but this is the very last
     * minute we can do so.  Make sure we have a new object to replace
     * us if this is a vital object.
     */
    if (ob == master_ob || ob == simul_efun_ob) {
	object_t *new_ob;
	char *tmp = ob->name;
#ifdef LPC_TO_C
	lpc_object_t *compiled_version;
#endif

	(++sp)->type = T_ERROR_HANDLER;
	sp->u.error_handler = fix_object_names;
	saved_master_name = master_ob->name;
	saved_simul_name = simul_efun_ob->name;
	
	/* hack to make sure we don't find ourselves at several points
	   in the following process */
	ob->name = "";

#ifdef LPC_TO_C
	compiled_version = (lpc_object_t *)lookup_object_hash(tmp);
#endif

	/* handle these two carefully, since they are rather vital */
	new_ob = load_object(tmp, compiled_version);
	if (!new_ob) {
	    ob->name = tmp;
	    sp--;
	    error("Destruct on vital object failed: new copy failed to reload.");
	}

	free_object(ob, "vital object reference");
	if (ob == master_ob)
	    set_master(new_ob);
	if (ob == simul_efun_ob)
	    set_simul_efun(new_ob);
	
	/* Set the name back so we can remove it from the hash table.
	   Also be careful not to remove the new object, which has
	   the same name. */
	sp--; /* error handler */
	ob->name = tmp;
	tmp = new_ob->name;
	new_ob->name = "";
	remove_object_hash(ob);
	new_ob->name = tmp;
    } else
	remove_object_hash(ob);

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
	break;
    }
    DEBUG_CHECK(!removed, "Failed to delete object.\n");

#ifndef NO_ADD_ACTION
    if (ob->living_name)
	remove_living_name(ob);
    ob->flags &= ~O_ENABLE_COMMANDS;
#endif
#ifndef NO_ENVIRONMENT
    ob->super = 0;
    ob->next_inv = 0;
    ob->contains = 0;
#endif
    ob->next_all = obj_list_destruct;
    obj_list_destruct = ob;
    set_heart_beat(ob, 0);
    ob->flags |= O_DESTRUCTED;
    /* moved this here from destruct2() -- see comments in destruct2() */
    if (ob->interactive)
	remove_interactive(ob, 1);
}

/*
 * This one is called when no program is executing from the main loop.
 */
void destruct2 P1(object_t *, ob)
{
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("Destruct-2 object /%s (ref %d)\n", ob->name, ob->ref);
    }
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
    if (ob->prog->num_variables_total > 0) {
	/*
	 * Deallocate variables in this object. The space of the variables
	 * are not deallocated until the object structure is freed in
	 * free_object().
	 */
	int i;

	for (i = 0; i < (int) ob->prog->num_variables_total; i++) {
	    free_svalue(&ob->variables[i], "destruct2");
	    ob->variables[i] = const0u;
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
 * message never goes to objects in the avoid array, or the source itself.
 *
 * rewritten, bobf@metronet.com (Blackthorn) 9/6/93
 */

#ifndef NO_ENVIRONMENT
static void send_say P3(object_t *, ob, char *, text, array_t *, avoid)
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
#endif

#ifndef NO_ENVIRONMENT
void say P2(svalue_t *, v, array_t *, avoid)
{
    object_t *ob, *origin, *save_command_giver = command_giver;
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
#ifdef F_TELL_ROOM
void tell_room P3(object_t *, room, svalue_t *, v, array_t *, avoid)
{
    object_t *ob;
    char *buff;
    int valid, j;
    char txt_buf[LARGEST_PRINTABLE_STRING + 1];

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
	IF_DEBUG(buff = 0);
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
#endif
#endif

void shout_string P1(char *, str)
{
    object_t *ob;

    check_legal_string(str);

    for (ob = obj_list; ob; ob = ob->next_all) {
	if (!(ob->flags & O_LISTENER) || (ob == command_giver)
#ifndef NO_ENVIRONMENT
	    || !ob->super
#endif
	    )
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
#ifndef NO_ENVIRONMENT
    object_t *pp;
#endif

    if (current_object->flags & O_DESTRUCTED)
	return;
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("Enable commands /%s (ref %d)\n",
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
#ifndef NO_ENVIRONMENT
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
#endif
	current_object->flags &= ~O_ENABLE_COMMANDS;
	command_giver = 0;
    }
}
#endif

/*
 * Set up a function in this object to be called with the next
 * user input string.
 */
int input_to P4(svalue_t *, fun, int, flag, int, num_arg, svalue_t *, args)
{
    sentence_t *s;
    svalue_t *x;
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
	    i = num_arg * sizeof(svalue_t);
	    if ((x = (svalue_t *)
		 DMALLOC(i, TAG_INPUT_TO, "input_to: 1")) == NULL)
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
	    fun->u.fp->hdr.ref++;
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
int get_char P4(svalue_t *, fun, int, flag, int, num_arg, svalue_t *, args)
{
    sentence_t *s;
    svalue_t *x;
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
	    i = num_arg * sizeof(svalue_t);
	    if ((x = (svalue_t *)
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
	    fun->u.fp->hdr.ref++;
	    s->flags = V_FUNCTION;
	}
	s->ob = current_object;
	add_ref(current_object, "get_char");
	return 1;
    }
    free_sentence(s);
    return 0;
}

void print_svalue P1(svalue_t *, arg)
{
    char tbuf[2048];

    if (arg == 0) {
	tell_object(command_giver, "<NULL>");
    } else
	switch (arg->type) {
	case T_STRING:
	    check_legal_string(arg->u.string);
	    tell_object(command_giver, arg->u.string);
	    break;
	case T_OBJECT:
	    sprintf(tbuf, "OBJ(/%s)", arg->u.ob->name);
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
	case T_ARRAY:
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

void do_write P1(svalue_t *, arg)
{
    object_t *save_command_giver = command_giver;

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

object_t *find_object P1(char *, str)
{
    object_t *ob;
    char tmpbuf[MAX_OBJECT_NAME_SIZE];

    if (!strip_name(str, tmpbuf, sizeof tmpbuf))
	return 0;

    if ((ob = lookup_object_hash(tmpbuf))) {
#ifdef LPC_TO_C
	if (ob->flags & O_COMPILED_PROGRAM) {
	    ob = load_object(tmpbuf, (lpc_object_t *)ob);
	    if (!ob || (ob->flags & O_DESTRUCTED))	/* *sigh* */
		return 0;
	}
#endif
	if (ob->flags & O_SWAPPED)
	    load_ob_from_swap(ob);
	return ob;
    }
    ob = load_object(tmpbuf, 0);
    if (!ob || (ob->flags & O_DESTRUCTED))	/* *sigh* */
	return 0;
    if (ob && ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    return ob;
}

/* Look for a loaded object. Return 0 if non found. */
object_t *find_object2 P1(char *, str)
{
    register object_t *ob;
    char p[MAX_OBJECT_NAME_SIZE];

    if (!strip_name(str, p, sizeof p))
	return 0;

    if ((ob = lookup_object_hash(p))) {
#ifdef LPC_TO_C
	if (ob->flags & O_COMPILED_PROGRAM)
	    return 0;
#endif
	if (ob->flags & O_SWAPPED)
	    load_ob_from_swap(ob);
	return ob;
    }
    return 0;
}

#ifndef NO_ENVIRONMENT
/*
 * Transfer an object.
 * The object has to be taken from one inventory list and added to another.
 * The main work is to update all command definitions, depending on what is
 * living or not. Note that all objects in the same inventory are affected.
 */
void move_object P2(object_t *, item, object_t *, dest)
{
    object_t **pp, *ob;
#ifndef NO_ADD_ACTION
    object_t *next_ob;
    object_t *save_cmd = command_giver;
#endif

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
#ifdef DEBUG
	if (!okey)
	    fatal("Failed to find object /%s in super list of /%s.\n",
		  item->name, item->super->name);
#endif
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
#endif

#ifndef NO_LIGHT
/*
 * Every object has a count of the number of light sources it contains.
 * Update this.
 */

void add_light P2(object_t *, p, int, n)
{
    if (n == 0)
	return;
    p->total_light += n;
#ifndef NO_ENVIRONMENT
    while ((p = p->super))
	p->total_light += n;
#endif
}
#endif

static sentence_t *sent_free = 0;
int tot_alloc_sentence;

static sentence_t *alloc_sentence()
{
    sentence_t *p;

    if (sent_free == 0) {
	p = ALLOCATE(sentence_t, TAG_SENTENCE, "alloc_sentence");
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

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_free_sentences() {
    sentence_t *sent = sent_free;

#ifdef PACKAGE_UIDS
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

void free_sentence P1(sentence_t *, p)
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
    char verb_buff[MAX_VERB_BUFF];
#ifndef NO_ENVIRONMENT
    object_t *super;
#endif
    sentence_t *s;
    char *p;
    int length;
    object_t *save_command_giver = command_giver;
    char *user_verb = 0;
    int where;
    int save_illegal_sentence_action;
    
#ifdef DEBUG
    if (d_flag > 1)
	debug_message("cmd [/%s]: %s\n", command_giver->name, buff);
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

    save_illegal_sentence_action = illegal_sentence_action;
    illegal_sentence_action = 0;
    for (s = save_command_giver->sent; s; s = s->next) {
	svalue_t *ret;
	object_t *command_object;
	    
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
	if (d_flag > 1 && !(s->flags & V_FUNCTION))
	    debug_message("Local command %s on /%s\n",
			  s->function, s->ob->name);
#endif
	if (s->flags & V_NOSPACE) {
	    int l1 = strlen(s->verb);
	    int l2 = strlen(verb_buff);
	    
	    if (l1 < l2)
		last_verb = verb_buff + l1;
	    else
		last_verb = "";
	} else {
	    if (!s->verb[0] || (s->flags & V_SHORT))
		last_verb = verb_buff;
	    else
		last_verb = s->verb;
	}
	/*
	 * If the function is static and not defined by current object, then
	 * it will fail. If this is called directly from user input, then
	 * the origin is the driver and it will be allowed.
	 */
	where = (current_object ? ORIGIN_EFUN : ORIGIN_DRIVER);
	
	/*
	 * Remember the object, to update moves.
	 */
	command_object = s->ob;
#ifndef NO_ENVIRONMENT
	super = command_object->super;
#endif
	if (s->flags & V_NOSPACE) {
	    copy_and_push_string(&buff[strlen(s->verb)]);
	} else if (buff[length] == ' ') {
	    copy_and_push_string(&buff[length + 1]);
	} else {
	    push_undefined();
	}
	if (s->flags & V_FUNCTION) {
	    ret = call_function_pointer(s->function.f, 1);
	} else {
	    if (s->function.s[0] == APPLY___INIT_SPECIAL_CHAR)
		error("Illegal function name.\n");
	    ret = apply(s->function.s, s->ob, 1, where);
	}
	/* s may be dangling at this point */
	
	command_giver = save_command_giver;
	
	last_verb = 0;
	
	/* was this the right verb? */
	if (ret == 0) {
	    /* is it still around?  Otherwise, ignore this ...
	       it moved somewhere or dested itself */
	    if (s == save_command_giver->sent) {
		char buf[256];
		if (s->flags & V_FUNCTION) {
		    sprintf(buf, "Verb '%s' bound to uncallable function pointer.\n", s->verb);
		    error(buf);
		} else {
		    sprintf(buf, "Function for verb '%s' not found.\n",
			    s->verb);
		    error(buf);
		}
	    }
	}
	
	if (ret && (ret->type != T_NUMBER || ret->u.number != 0)) {
#ifdef PACKAGE_MUDLIB_STATS
	    if (command_giver && command_giver->interactive
#ifndef NO_WIZARDS
		&& !(command_giver->flags & O_IS_WIZARD)
#endif
		)
		add_moves(&command_object->stats, 1);
#endif
	    if (!illegal_sentence_action)
		illegal_sentence_action = save_illegal_sentence_action;
	    return 1;
	}
	if (illegal_sentence_action) {
	    switch (illegal_sentence_action) {
	    case 1:
		error("Illegal to call remove_action() from a verb returning zero.\n");
	    case 2:
		error("Illegal to move or destruct an object defining actions from a verb function which returns zero.\n");
	    }
	}
    }
    notify_no_command();
    illegal_sentence_action = save_illegal_sentence_action;
    
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
void add_action P3(svalue_t *, str, char *, cmd, int, flag)
{
    sentence_t *p;
    object_t *ob;

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
    if (ob != command_giver
#ifndef NO_ENVIRONMENT
	&& ob->super != command_giver &&
	ob->super != command_giver->super && ob != command_giver->super
#endif
	)
	return;			/* No need for an error, they know what they
				 * did wrong. */
    p = alloc_sentence();
    if (str->type == T_STRING) {
#ifdef DEBUG
	if (d_flag > 1)
	    debug_message("--Add action %s\n", str->u.string);
#endif
      p->function.s = make_shared_string(str->u.string);
      p->flags = flag;
    } else {
#ifdef DEBUG
	if (d_flag > 1)
	    debug_message("--Add action <function>\n");
#endif
      p->function.f = str->u.fp;
      str->u.fp->hdr.ref++;
      p->flags = flag | V_FUNCTION;
    }
    p->ob = ob;
    p->verb = make_shared_string(cmd);
    /* This is ok; adding to the top of the list doesn't harm anything */
    p->next = command_giver->sent;
    command_giver->sent = p;
}

/*
 * Remove sentence with specified verb and action.  Return 1
 * if success.  If command_giver, remove his action, otherwise
 * remove current_object's action.
 */
int remove_action P2(char *, act, char *, verb)
{
    object_t *ob;
    sentence_t **s;

    if (command_giver)
	ob = command_giver;
    else
	ob = current_object;

    if (ob) {
	for (s = &ob->sent; *s; s = &((*s)->next)) {
	    sentence_t *tmp;
	    
	    if (((*s)->ob == current_object) && (!((*s)->flags & V_FUNCTION))
		&& !strcmp((*s)->function.s, act)
		&& !strcmp((*s)->verb, verb)) {
		tmp = *s;
		*s = tmp->next;
		free_sentence(tmp);
		illegal_sentence_action = 1;
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
#ifndef NO_ENVIRONMENT
static void remove_sent P2(object_t *, ob, object_t *, user)
{
    sentence_t **s;

    for (s = &user->sent; *s;) {
	sentence_t *tmp;

	if ((*s)->ob == ob) {
#ifdef DEBUG
	    if (d_flag > 1 && !((*s)->flags & V_FUNCTION))
		debug_message("--Unlinking sentence %s\n", (*s)->function);
#endif
	    tmp = *s;
	    *s = tmp->next;
	    free_sentence(tmp);
	    illegal_sentence_action = 2;
	} else
	    s = &((*s)->next);
    }
}
#endif
#endif /* NO_ADD_ACTION */

void fatal P1V(char *, fmt)
{
    static int in_fatal = 0;
    char msg_buf[2049];
    va_list args;
    V_DCL(char *fmt);

    if (in_fatal) {
	debug_message("Fatal error while shutting down.  Aborting.\n");
    } else {
	in_fatal = 1;
	V_START(args, fmt);
	V_VAR(char *, fmt, args);
	vsprintf(msg_buf, fmt, args);
	va_end(args);
	debug_message("******** FATAL ERROR: %s\nMudOS driver attempting to exit gracefully.\n", msg_buf);
	if (current_file)
	    debug_message("(occured during compilation of %s at line %d)\n", current_file, current_line);
	if (current_object)
	    debug_message("(current object was /%s)\n", current_object->name);
	
	dump_trace(1);
	
#ifdef PACKAGE_MUDLIB_STATS
	save_stat_files();
#endif
	copy_and_push_string(msg_buf);
	if (command_giver) {
	    push_object(command_giver);
	} else {
	    push_undefined();
	}
	if (current_object) {
	    push_object(current_object);
	} else {
	    push_undefined();
	}
	apply_master_ob(APPLY_CRASH, 3);
	debug_message("crash() in master called successfully.  Aborting.\n");
    }
    /* Make sure we don't trap our abort() */
#ifdef SIGABRT
    signal(SIGABRT, SIG_DFL);
#endif
#ifdef SIGILL
    signal(SIGILL, SIG_DFL);
#endif
#ifdef SIGIOT
    signal(SIGIOT, SIG_DFL);
#endif
    
#if !defined(DEBUG_NON_FATAL) || !defined(DEBUG)
    abort();
#endif
    in_fatal = 0;
}

static int num_error = 0;

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
    if (((current_error_context->save_csp + 1)->framekind & FRAME_MASK) == FRAME_CATCH) {
	LONGJMP(current_error_context->context, 1);
	fatal("Throw_error failed!");
    }
    error("Throw with no catch.\n");
}

static void debug_message_with_location P1(char *, err) {
    if (current_object && current_prog) {
	debug_message("%sprogram: /%s, object: /%s, file: %s\n",
		      err,
		      current_prog->name,
		      current_object->name,
		      get_line_number(pc, current_prog));
    } else if (current_object) {
	debug_message("%sprogram: (none), object: /%s, file: (none)\n",
		      err,
		      current_object->name);
    } else {
	debug_message("%sprogram: (none), object: (none), file: (none)\n",
		      err);
    }
}

#ifndef MUDLIB_ERROR_HANDLER
static void add_message_with_location P1(char *, err) {
    if (current_object && current_prog) {
	add_vmessage(command_giver, "%sprogram: /%s, object: /%s, file: %s\n",
		     err,
		     current_prog->name,
		     current_object->name,
		     get_line_number(pc, current_prog));
    } else if (current_object) {
	add_vmessage(command_giver, "%sprogram: (none), object: /%s, file: (none)\n",
		     err,
		     current_object->name);
    } else {
	add_vmessage(command_giver, "%sprogram: (none), object: (none), file: (none)\n",
		     err);
    }
}
#endif

#ifdef MUDLIB_ERROR_HANDLER
static void mudlib_error_handler P2(char *, err, int, catch) {
    mapping_t *m;
    char *file;
    int line;
    svalue_t *mret;

    m = allocate_mapping(6);
    add_mapping_string(m, "error", err);
    if (current_prog)
	add_mapping_string(m, "program", current_prog->name);
    if (current_object)
	add_mapping_object(m, "object", current_object);
    add_mapping_array(m, "trace", get_svalue_trace());
    get_line_number_info(&file, &line);
    add_mapping_string(m, "file", file);
    add_mapping_pair(m, "line", line);

    push_refed_mapping(m);
    if (catch) {
	*(++sp) = const1;
	mret = apply_master_ob(APPLY_ERROR_HANDLER,2);
    } else {
	mret = apply_master_ob(APPLY_ERROR_HANDLER,1);
    }
    if ((mret == (svalue_t *)-1) || !mret) {
	debug_message("No error handler for error: ");
	debug_message_with_location(err);
	dump_trace(0);
    } else if (mret->type == T_STRING) {
	debug_message("%s", mret->u.string);
    }
}
#endif

void error_handler P1(char *, err)
{
#ifndef MUDLIB_ERROR_HANDLER
    char *object_name;
#endif

    /* in case we're going to jump out of load_object */
    restrict_destruct = 0;
    num_objects_this_thread = 0;/* reset the count */

    if (((current_error_context->save_csp + 1)->framekind & FRAME_MASK) == FRAME_CATCH) {
        /* user catches this error */
#ifdef LOG_CATCHES
	/* This is added so that catches generate messages in the log file. */
#ifdef MUDLIB_ERROR_HANDLER
	if (num_mudlib_error) {
	    debug_message("Error in error handler: ");
#endif
	    debug_message_with_location(err);
	    (void) dump_trace(0);
#ifdef MUDLIB_ERROR_HANDLER
	    num_mudlib_error = 0;
	} else {
	    num_mudlib_error++;
	    mudlib_error_handler(err, 1);
	    num_mudlib_error--;
	}
#endif
#endif
	free_svalue(&catch_value, "caught error");
	catch_value.type = T_STRING;
	catch_value.subtype = STRING_MALLOC;
	catch_value.u.string = string_copy(err, "caught error");
	LONGJMP(current_error_context->context, 1);
	fatal("Catch() longjump failed");
    }
    too_deep_error = max_eval_error = 0;
    if (num_error > 0) {
	/* This can happen via errors in the object_name() apply. */
	debug_message("Error '%s' while trying to print error trace -- trace suppressed.\n", err);
	if (current_error_context)
	    LONGJMP(current_error_context->context, 1);
	fatal("LONGJMP failed or no error context for error.\n");
    }
    num_error++;
#ifdef MUDLIB_ERROR_HANDLER
    if (num_mudlib_error) {
	debug_message("Error in error handler: ");
	debug_message_with_location(err);
	(void) dump_trace(0);
	num_mudlib_error = 0;
    } else {
	num_mudlib_error++;
	num_error--;
	mudlib_error_handler(err, 0);
	num_mudlib_error--;
	num_error++;
    }
    if (current_heart_beat) {
	set_heart_beat(current_heart_beat, 0);
	debug_message("Heart beat in %s turned off.\n", current_heart_beat->name);
	if (current_heart_beat->interactive)
	    add_message(current_heart_beat, "MudOS driver tells you: You have no heart beat!\n");
	
	current_heart_beat = 0;
    }
#else
    if (current_object) {
#ifdef PACKAGE_MUDLIB_STATS
	add_errors(&current_object->stats, 1);
#endif
    }
    debug_message_with_location(err + 1);
#if defined(DEBUG) && defined(TRACE_CODE)
    object_name = dump_trace(1);
#else
    object_name = dump_trace(0);
#endif
    if (object_name) {
	object_t *ob;

	ob = find_object2(object_name);
	if (!ob) {
	    if (command_giver)
		add_vmessage(command_giver,
			     "error when executing program in destroyed object /%s\n",
			    object_name);
	    debug_message("error when executing program in destroyed object /%s\n",
			  object_name);
	}
    }
    if (command_giver && command_giver->interactive) {
#ifndef NO_WIZARDS
	if ((command_giver->flags & O_IS_WIZARD) || !strlen(DEFAULT_ERROR_MESSAGE)) {
#endif
	    add_message_with_location(err + 1);
#ifndef NO_WIZARDS
	} else {
	    add_vmessage(command_giver, "%s\n", DEFAULT_ERROR_MESSAGE);
	}
#endif
    }
    if (current_heart_beat) {
	set_heart_beat(current_heart_beat, 0);
	debug_message("Heart beat in /%s turned off.\n", current_heart_beat->name);
	if (current_heart_beat->interactive)
	    add_message(current_heart_beat, "MudOS driver tells you: You have no heart beat!\n");

	current_heart_beat = 0;
    }
#endif
    num_error--;
    if (current_error_context)
	LONGJMP(current_error_context->context, 1);
    fatal("LONGJMP failed or no error context for error.\n");
}

void error_needs_free P1(char *, s)
{
    char err_buf[2048];
    strncpy(err_buf + 1, s, 2047);
    err_buf[0] = '*';		/* all system errors get a * at the start */
    err_buf[2047] = '\0';
    FREE_MSTR(s);

    error_handler(err_buf);
}

void error P1V(char *, fmt)
{
    char err_buf[2048];
    va_list args;
    V_DCL(char *fmt);

    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vsprintf(err_buf + 1, fmt, args);
    va_end(args);
    err_buf[0] = '*';		/* all system errors get a * at the start */

    error_handler(err_buf);
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
#ifdef PACKAGE_MUDLIB_STATS
    save_stat_files();
#endif
    ipc_remove();
#ifdef PACKAGE_SOCKETS
    for (i = 0; i < max_lpc_socks; i++) {
	if (lpc_socks[i].state == CLOSED) continue;
	while (close(lpc_socks[i].fd) == -1 && errno == EINTR)
	    ;
    }
#endif
    for (i = 0; i < max_users; i++) {
	if (all_users[i] && !(all_users[i]->iflags & CLOSING))
	    flush_message(all_users[i]);
    }
#ifdef LATTICE
    signal(SIGUSR1, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
#endif
    unlink_swap_file();
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
    svalue_t *amo;

    push_number(minutes);
    amo = apply_master_ob(APPLY_SLOW_SHUTDOWN, 1);
    /* in this case, approved means the mudlib will handle it */
    if (!MASTER_APPROVED(amo))
    {
	object_t *save_current = current_object, *save_command = command_giver;

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

void do_message P5(svalue_t *, class, svalue_t *, msg, array_t *, scope, array_t *, exclude, int, recurse)
{
    int i, j, valid;
    object_t *ob;

    for (i = 0; i < scope->size; i++) {
	switch (scope->item[i].type) {
	case T_STRING:
	    ob = find_object(scope->item[i].u.string);
	    if (!ob || !object_visible(ob))
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
		push_svalue(msg);
		apply(APPLY_RECEIVE_MESSAGE, ob, 2, ORIGIN_DRIVER);
	    }
	}
#ifndef NO_ENVIRONMENT
	else if (recurse) {
	    array_t *tmp;

	    tmp = all_inventory(ob, 1);
	    do_message(class, msg, tmp, exclude, 0);
	    free_array(tmp);
	}
#endif
    }
}

#ifndef NO_RESETS
#ifdef LAZY_RESETS
void try_reset P1(object_t *, ob)
{
    if ((ob->next_reset < current_time) && !(ob->flags & O_RESET_STATE)) {
#ifdef DEBUG
	if (d_flag) {
	    debug_message("(lazy) RESET /%s\n", ob->name);
	}
#endif
	/* need to set the flag here to prevent infinite loops in apply_low */
	ob->flags |= O_RESET_STATE;
	reset_object(ob);
    }
}
#endif
#endif

#ifndef NO_ENVIRONMENT
#ifdef F_FIRST_INVENTORY
object_t *first_inventory P1(svalue_t *, arg)
{
    object_t *ob;

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
#endif
#endif
