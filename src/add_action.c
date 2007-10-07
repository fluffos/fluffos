#include "std.h"
#include "comm.h"
#include "backend.h"
#include "add_action.h"

#ifndef NO_ADD_ACTION

#define MAX_VERB_BUFF 100

object_t *  hashed_living[CFG_LIVING_HASH_SIZE] = { 0 };

static int	    num_living_names;
static int	    num_searches = 1;
static int	    search_length = 1;
static int	    illegal_sentence_action;
static char *	    last_verb;
static object_t *   illegal_sentence_ob;

static void notify_no_command (void)
{
    union string_or_func p;
    svalue_t *v;

    if (!command_giver || !command_giver->interactive)
	return;
    p = command_giver->interactive->default_err_message;
    if (command_giver->interactive->iflags & NOTIFY_FAIL_FUNC) {
	save_command_giver(command_giver);
	v = call_function_pointer(p.f, 0);
	restore_command_giver();
	free_funp(p.f);
	if (command_giver && command_giver->interactive) {
	    if (v && v->type == T_STRING) {
		tell_object(command_giver, v->u.string, SVALUE_STRLEN(v));
	    }
	    command_giver->interactive->iflags &= ~NOTIFY_FAIL_FUNC;
	    command_giver->interactive->default_err_message.s = 0;
	}
    } else {
	if (p.s) {
	    tell_object(command_giver, p.s, strlen(p.s));
	    free_string(p.s);
	    command_giver->interactive->default_err_message.s = 0;
	} else {
	    tell_object(command_giver, default_fail_message, strlen(default_fail_message));
	}
    }
}

void clear_notify (object_t * ob)
{
    union string_or_func dem;
    interactive_t *ip = ob->interactive;

    dem = ip->default_err_message;
    if (ip->iflags & NOTIFY_FAIL_FUNC) {
	free_funp(dem.f);
	ip->iflags &= ~NOTIFY_FAIL_FUNC;
    }
    else if (dem.s)
	free_string(dem.s);
    ip->default_err_message.s = 0;
}

INLINE_STATIC int hash_living_name (const char *str)
{
    return whashstr(str) & (CFG_LIVING_HASH_SIZE - 1);
}

object_t *find_living_object (const char* str, int user)
{
    object_t **obp, *tmp;
    object_t **hl;

    if (!str)
	return 0;
    num_searches++;
    hl = &hashed_living[hash_living_name(str)];
    for (obp = hl; *obp; obp = &(*obp)->next_hashed_living) {
	search_length++;
#ifdef F_SET_HIDE
	if ((*obp)->flags & O_HIDDEN) {
	    if (!valid_hide(current_object))
		continue;
	}
#endif
	if (user && !((*obp)->flags & O_ONCE_INTERACTIVE))
	    continue;
	if (!((*obp)->flags & O_ENABLE_COMMANDS))
	    continue;
	if (strcmp((*obp)->living_name, str) == 0)
	    break;
    }
    if (*obp == 0)
	return 0;
    /* Move the found ob first. */
    if (obp == hl)
	return *obp;
    tmp = *obp;
    *obp = tmp->next_hashed_living;
    tmp->next_hashed_living = *hl;
    *hl = tmp;
    return tmp;
}

void remove_living_name (object_t * ob)
{
    object_t **hl;

    ob->flags &= ~O_ENABLE_COMMANDS;
    if (!ob->living_name)
	return;

    num_living_names--;
    DEBUG_CHECK(!ob->living_name, "remove_living_name: no living name set.\n");
    hl = &hashed_living[hash_living_name(ob->living_name)];
    while (*hl) {
	if (*hl == ob)
	    break;
	hl = &(*hl)->next_hashed_living;
    }
    DEBUG_CHECK1(*hl == 0, 
		 "remove_living_name: Object named %s no in hash list.\n",
		 ob->living_name);
    *hl = ob->next_hashed_living;
    free_string(ob->living_name);
    ob->next_hashed_living = 0;
    ob->living_name = 0;
}

static void set_living_name (object_t * ob, const char *str)
{
    int flags = ob->flags & O_ENABLE_COMMANDS;
    object_t **hl;

    if (ob->flags & O_DESTRUCTED)
	return;
    remove_living_name(ob);
    num_living_names++;
    hl = &hashed_living[hash_living_name(str)];
    ob->next_hashed_living = *hl;
    *hl = ob;
    ob->living_name = make_shared_string(str);
    ob->flags |= flags;
}

void stat_living_objects (outbuffer_t * out)
{
    outbuf_add(out, "Hash table of living objects:\n");
    outbuf_add(out, "-----------------------------\n");
    outbuf_addv(out, "%d living named objects, average search length: %4.2f\n\n",
		num_living_names, (double) search_length / num_searches);
}

void setup_new_commands (object_t * dest, object_t * item)
{
    object_t *next_ob, *ob;

    /*
     * Setup the new commands. The order is very important, as commands in
     * the room should override commands defined by the room. Beware that
     * init() in the room may have moved 'item' !
     * 
     * The call of init() should really be done by the object itself (except in
     * the -o mode). It might be too slow, though :-(
     */
    if (item->flags & O_ENABLE_COMMANDS) {
	save_command_giver(item);
	(void) apply(APPLY_INIT, dest, 0, ORIGIN_DRIVER);
	restore_command_giver();
	if (item->super != dest)
	    return;
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
	    save_command_giver(ob);
	    (void) apply(APPLY_INIT, item, 0, ORIGIN_DRIVER);
	    restore_command_giver();
	    if (dest != item->super)
		return;
	}
	if (item->flags & O_DESTRUCTED)	/* marion */
	    error("The object to be moved was destructed at call of " APPLY_INIT "()\n");
	if (ob->flags & O_DESTRUCTED) /* Alaron */
	    error("An object was destructed at call of " APPLY_INIT "()\n");
	if (item->flags & O_ENABLE_COMMANDS) {
	    save_command_giver(item);
	    (void) apply(APPLY_INIT, ob, 0, ORIGIN_DRIVER);
	    restore_command_giver();
	    if (dest != item->super)
		return;
	}
    }
    if (dest->flags & O_DESTRUCTED)	/* marion */
	error("The destination to move to was destructed at call of " APPLY_INIT "()\n");
    if (item->flags & O_DESTRUCTED)	/* Alaron */
	error("The object to be moved was destructed at call of " APPLY_INIT "()\n");
    if (dest->flags & O_ENABLE_COMMANDS) {
	save_command_giver(dest);
	(void) apply(APPLY_INIT, item, 0, ORIGIN_DRIVER);
	restore_command_giver();
    }
}

/*
 * This will enable an object to use commands normally only
 * accessible by interactive users.
 * Also check if the user is a wizard. Wizards must not affect the
 * value of the wizlist ranking.
 */
static void enable_commands (int num)
{
#ifndef NO_ENVIRONMENT
    object_t *pp;
#endif

    if (current_object->flags & O_DESTRUCTED)
	return;

    debug(d_flag, ("Enable commands /%s (ref %d)",
		   current_object->name, current_object->ref));

    if (num) {
	current_object->flags |= O_ENABLE_COMMANDS;
	set_command_giver(current_object);
    } else {
#ifndef NO_ENVIRONMENT
	/* Remove all sentences defined for the object */
	if (current_object->flags & O_ENABLE_COMMANDS) {
	    if (current_object->super) {
		remove_sent(current_object->super, current_object);
		for (pp = current_object->super->contains; pp; pp = pp->next_inv)
		    remove_sent(pp, current_object);
	    }
	    for (pp = current_object->contains; pp; pp = pp->next_inv)
		remove_sent(pp, current_object);
	}
#endif
	current_object->flags &= ~O_ENABLE_COMMANDS;
	if (current_object == command_giver)
	    set_command_giver(0);
    }
}

/*
 * Find the sentence for a command from the user.
 * Return success status.
 */

static int user_parser (char * buff)
{
    char verb_buff[MAX_VERB_BUFF];
    sentence_t *s;
    char *p;
    int length;
    char *user_verb = 0;
    int where;
    int save_illegal_sentence_action;
    
    debug(d_flag, ("cmd [/%s]: %s\n", command_giver->name, buff));

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
    for (s = command_giver->sent; s; s = s->next) {
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

	if (!(s->flags & V_FUNCTION))
	    debug(d_flag, ("Local command %s on /%s",
			   s->function.s, s->ob->obname));

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
	save_command_giver(command_giver);
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
	
	restore_command_giver();
	
	last_verb = 0;
	
	/* was this the right verb? */
	if (ret == 0) {
	    /* is it still around?  Otherwise, ignore this ...
	       it moved somewhere or dested itself */
	    if (s == command_giver->sent) {
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
		error("Illegal to call remove_action() [caller was /%s] from a verb returning zero.\n", illegal_sentence_ob->obname);
	    case 2:
		error("Illegal to move or destruct an object (/%s) defining actions from a verb function which returns zero.\n", illegal_sentence_ob->obname);
	    }
	}
    }
    notify_no_command();
    illegal_sentence_action = save_illegal_sentence_action;
    
    return 0;
}

/*
 * Take a user command and parse it.
 * The command can also come from a NPC.
 * Beware that 'str' can be modified and extended !
 */
int parse_command (char * str, object_t * ob)
{
    int res;

    /* disallow users to issue commands containing ansi escape codes */
#if defined(NO_ANSI) && !defined(STRIP_BEFORE_PROCESS_INPUT)
    char *c;

    for (c = str;  *c;  c++) {
	if (*c == 27) {
	    *c = ' ';		/* replace ESC with ' ' */
	}
    }
#endif

    save_command_giver(ob);
    res = user_parser(str);
    restore_command_giver();

    return res;
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
static void add_action (svalue_t * str, const char *cmd, int flag)
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
	debug(d_flag, ("--Add action %s", str->u.string));
	p->function.s = make_shared_string(str->u.string);
	p->flags = flag;
    } else {
	debug(d_flag, ("--Add action <function>"));

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
static int remove_action (const char *act, const char *verb)
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
		illegal_sentence_ob = current_object;
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
void remove_sent (object_t * ob, object_t * user)
{
    sentence_t **s;

    if (!(user->flags & O_ENABLE_COMMANDS))
	return;

    for (s = &user->sent; *s;) {
	sentence_t *tmp;

	if ((*s)->ob == ob) {
#ifdef DEBUG
	    if (!((*s)->flags & V_FUNCTION))
		debug(d_flag, ("--Unlinking sentence %s\n", (*s)->function.s));
#endif

	    tmp = *s;
	    *s = tmp->next;
	    free_sentence(tmp);
	    illegal_sentence_action = 2;
	    illegal_sentence_ob = ob;
	} else
	    s = &((*s)->next);
    }
}
#endif

#ifdef F_ADD_ACTION
void
f_add_action (void)
{
    long flag;

    if (st_num_arg == 3) {
	flag = (sp--)->u.number;
    } else flag = 0;

    if (sp->type == T_ARRAY) {
	int i, n = sp->u.arr->size;
	svalue_t *sv = sp->u.arr->item;

	for (i = 0; i < n; i++) {
	    if (sv[i].type == T_STRING) {
		add_action(sp-1, sv[i].u.string, flag & 3);
	    }
	}
	free_array((sp--)->u.arr);
    } else {
	add_action((sp-1), sp->u.string, flag & 3);
	free_string_svalue(sp--);
    }
    pop_stack();
}
#endif

#ifdef F_COMMAND
/*
 * Execute a command for an object. Copy the command into a
 * new buffer, because 'parse_command()' can modify the command.
 * If the object is not current object, static functions will not
 * be executed. This will prevent forcing users to do illegal things.
 *
 * Return cost of the command executed if success (> 0).
 * When failure, return 0.
 */
void f_command (void)
{
    long rc = 0;

    if (current_object && !(current_object->flags & O_DESTRUCTED))
    {
	char buff[1000];
	int save_eval_cost = get_eval();

	if (SVALUE_STRLEN(sp) > sizeof(buff) - 1)
	    error("Too long command.\n");

	strncpy(buff, sp->u.string, sizeof(buff));
	buff[sizeof(buff) - 1] = 0;

	if (parse_command(buff, current_object))
	  rc = save_eval_cost - get_eval();
    }

    free_string_svalue(sp);
    put_number(rc);
}
#endif

#ifdef F_COMMANDS
void f_commands (void)
{
    push_refed_array(commands(current_object));
}
#endif

#ifdef F_DISABLE_COMMANDS
void f_disable_commands (void)
{
    enable_commands(0);
}
#endif

#ifdef F_ENABLE_COMMANDS
void f_enable_commands (void)
{
    enable_commands(1);
}
#endif

#ifdef F_FIND_LIVING
void f_find_living (void)
{
    object_t *ob;

    ob = find_living_object(sp->u.string, 0);
    free_string_svalue(sp);
    /* safe b/c destructed objects have had their living names removed */
    if (ob) {
	put_unrefed_undested_object(ob, "find_living");
    } else {
	*sp = const0;
    }
}
#endif

#ifdef F_FIND_PLAYER
void f_find_player (void)
{
    object_t *ob;

    ob = find_living_object(sp->u.string, 1);
    free_string_svalue(sp);
    /* safe b/c destructed objects have had their living names removed */
    if (ob) {
	put_unrefed_undested_object(ob, "find_living");
    } else {
	*sp = const0;
    }
}
#endif

#ifdef F_LIVING
void f_living (void)
{
    if (sp->u.ob->flags & O_ENABLE_COMMANDS) {
	free_object(&sp->u.ob, "f_living:1");
	*sp = const1;
    } else {
	free_object(&sp->u.ob, "f_living:2");
	*sp = const0;
    }
}
#endif

#ifdef F_LIVINGS
void f_livings (void)
{
    push_refed_array(livings());
}
#endif

#ifdef F_NOTIFY_FAIL
void f_notify_fail (void)
{
    if (command_giver && command_giver->interactive) {
	clear_notify(command_giver);
	if (sp->type == T_STRING) {
	    command_giver->interactive->default_err_message.s = make_shared_string(sp->u.string);
	} else {
	    command_giver->interactive->iflags |= NOTIFY_FAIL_FUNC;
	    command_giver->interactive->default_err_message.f = sp->u.fp;
	    sp->u.fp->hdr.ref++;
	}
    }
    pop_stack();
}
#endif

#ifdef F_QUERY_VERB
void f_query_verb (void)
{
    if (!last_verb) {
	push_number(0);
	return;
    }
    share_and_push_string(last_verb);
}
#endif

#ifdef F_REMOVE_ACTION
void f_remove_action (void)
{
    long success;

    success = remove_action((sp - 1)->u.string, sp->u.string);
    free_string_svalue(sp--);
    free_string_svalue(sp);
    put_number(success);
}
#endif

#ifdef F_SET_LIVING_NAME
void f_set_living_name (void)
{
    set_living_name(current_object, sp->u.string);
    free_string_svalue(sp--);
}
#endif

#endif	/* ! NO_ADD_ACTION */
