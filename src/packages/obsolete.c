#define SUPPRESS_COMPILER_INLINES
#ifdef LATTICE
#include "/lpc_incl.h"
#include "/sprintf.h"
#else
#include "../lpc_incl.h"
#include "../sprintf.h"
#endif

#if defined(F_MESSAGE) || defined(F_SAY) || defined(F_TELL_ROOM) || defined(F_WRITE)
INLINE void check_legal_string P1(svalue_t *, s)
{
    /* s->type is assumed to be T_STRING */
    if (SVALUE_STRLEN(s) > LARGEST_PRINTABLE_STRING)
	error("Printable strings limited to length of %d.\n", LARGEST_PRINTABLE_STRING);
}
#endif

#ifdef F_MESSAGE
static void do_message P5(svalue_t *, class, svalue_t *, msg, array_t *, scope, array_t *, exclude, int, recurse)
{
    int i, j, valid;
    object_t *ob;

    for (i = 0;  i < scope->size;  i++) {
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
	    for (valid = 1, j = 0;  j < exclude->size;  j++) {
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
#ifndef NO_ENIRONMENT
	else if (recurse) {
	    array_t *tmp;

	    tmp = all_inventory(ob, 1);
	    do_message(class, msg, tmp, exclude, 0);
	    free_array(tmp);
	}
#endif
    }
}

void f_message PROT((void))
{
    array_t *use, *avoid;
    int num_arg = st_num_arg;
    svalue_t *args;

    args = sp - num_arg + 1;
    switch (args[2].type) {
	case T_OBJECT:
	case T_STRING:
	    use = allocate_empty_array(1);
	    use->item[0] = args[2];
	    args[2].type = T_ARRAY;
	    args[2].u.arr = use;
	    break;
	case T_ARRAY:
	    use = args[2].u.arr;
	    break;
	case T_NUMBER:
	    if (args[2].u.number == 0) {
		/* this is really bad and probably should be rm'ed - Beek;
		 * on the other hand, we don't have a debug_message() efun yet.
		 * Well, there is one in contrib now ...
		 */
		/* for compatibility (write() simul_efuns, etc)  -bobf */
		check_legal_string(&args[1]);

		add_message(command_giver, args[1].u.string, SVALUE_STRLEN(args + 1));
		pop_n_elems(num_arg);
		return;
	    }
	default:
	    bad_argument(&args[2], T_OBJECT | T_STRING | T_ARRAY | T_NUMBER, 3, F_MESSAGE);
    }

    if (num_arg == 4) {
	switch (args[3].type) {
	    case T_OBJECT:
		avoid = allocate_empty_array(1);
		avoid->item[0] = args[3];
		args[3].type = T_ARRAY;
		args[3].u.arr = avoid;
		break;
	    case T_ARRAY:
		avoid = args[3].u.arr;
		break;
	    default:
		avoid = &the_null_array;
	}
    } else {
	avoid = &the_null_array;
    }

    do_message(&args[0], &args[1], use, avoid, 1);
    pop_n_elems(num_arg);
}
#endif

#ifdef F_PRINTF
void f_printf PROT((void))
{
    int num_arg = st_num_arg;
    char *ret;

    if (command_giver) {
	ret = string_print_formatted((sp - num_arg + 1)->u.string,
				     num_arg - 1, sp - num_arg + 2);
	if (ret) {
	    tell_object(command_giver, ret, COUNTED_STRLEN(ret));
	    FREE_MSTR(ret);
	}
    }

    pop_n_elems(num_arg);
}
#endif

#ifdef F_SAY
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
 * note that if catch_tell() causes any recipient object to destruct or move,
 * there's a good chance that successive recipient objects that should receive
 * the message will not receive it.  since this is obsolete and unsupported
 * code, i don't care anymore, but just so it's known.  i did at least add a
 * quick optimization into send_say() tho -Marius
 *
 * rewritten, bobf@metronet.com (Blackthorn) 9/6/93
 */
static void send_say P3(object_t *, ob, svalue_t *, text, array_t *, avoid)
{
    int valid, j;

    for (valid = 1, j = 0;  j < avoid->size;  j++) {
	if (avoid->item[j].type != T_OBJECT)
	    continue;
	if (avoid->item[j].u.ob == ob) {
	    valid = 0;
	}
    }

    if (valid)
	tell_object(ob, text->u.string, SVALUE_STRLEN(text));
}

static void say P2(svalue_t *, v, array_t *, avoid)
{
    object_t *ob, *origin;

    check_legal_string(v);

    if (current_object->flags & O_LISTENER || current_object->interactive)
	save_command_giver(current_object);
    else
	save_command_giver(command_giver);
    if (command_giver)
	origin = command_giver;
    else
	origin = current_object;

    /* To our surrounding object... */
    if ((ob = origin->super)) {
	if (ob->flags & O_LISTENER || ob->interactive)
	    send_say(ob, v, avoid);

	/* And its iunventory... */
	for (ob = origin->super->contains;  ob;  ob = ob->next_inv) {
	    if (ob != origin && (ob->flags & O_LISTENER || ob->interactive)) {
		send_say(ob, v, avoid);
		if (ob->flags & O_DESTRUCTED)
		    break;
	    }
	}
    }
    /* Our inventory... */
    for (ob = origin->contains;  ob;  ob = ob->next_inv) {
	if (ob->flags & O_LISTENER || ob->interactive) {
	    send_say(ob, v, avoid);
	    if (ob->flags & O_DESTRUCTED)
		break;
	}
    }

    restore_command_giver();
}

void f_say PROT((void))
{
    if (st_num_arg == 1) {
	say(sp, &the_null_array);
	pop_stack();
    } else {
	if (sp->type == T_ARRAY) {
	    say(sp - 1, sp->u.arr);
	} else {
	    array_t *avoid = allocate_empty_array(1);

	    avoid->item[0] = *sp;
	    say(sp - 1, avoid);
	    free_array(avoid);
	}
	pop_2_elems();
    }
}
#endif

#ifdef F_SHOUT
void f_shout PROT((void))
{
    shout_string(sp->u.string);
    free_string_svalue(sp--);
}
#endif

#ifdef F_TELL_OBJECT
void f_tell_object PROT((void))
{
    tell_object((sp - 1)->u.ob, sp->u.string, SVALUE_STRLEN(sp));
    pop_n_elems(2);
}
#endif

#ifdef F_TELL_ROOM
/*
 * Sends a string to all objects inside of a specific object.
 * Revised, bobf@metronet.com 9/6/93
 */
static void tell_room P3(object_t *, room, svalue_t *, v, array_t *, avoid)
{
    object_t *ob;
    char *buff;
    int valid, j, len;
    char txt_buf[LARGEST_PRINTABLE_STRING + 1];

    switch (v->type) {
	case T_STRING:
	    check_legal_string(v);
	    buff = v->u.string;
	    len = SVALUE_STRLEN(v);
	    break;
	case T_OBJECT:
	    buff = v->u.ob->name;
	    len = strlen(buff);
	    break;
	case T_NUMBER:
	    buff = txt_buf;
	    sprintf(buff, "%d", v->u.number);
	    len = strlen(buff);
	    break;
	case T_REAL:
	    buff = txt_buf;
	    sprintf(buff, "%f", v->u.real);
	    len = strlen(buff);
	    break;
	default:
	    bad_argument(v, T_OBJECT | T_NUMBER | T_REAL | T_STRING, 2, F_TELL_ROOM);
	    IF_DEBUG(buff = 0);
    }

    /* as with say(), catch_tell() causing a move or a destruct can have unpredictable
     * results -Marius
     */
    for (ob = room->contains;  ob;  ob = ob->next_inv) {
	if (!ob->interactive &&(!ob->flags & O_LISTENER))
	    continue;

	for (valid = 1, j = 0;  j < avoid->size;  j++) {
	    if (avoid->item[j].type != T_OBJECT)
		continue;
	    if (avoid->item[j].u.ob == ob) {
		valid = 0;
		break;
	    }
	}

	if (valid) {
	    tell_object(ob, buff, len);
	    if (ob->flags & O_DESTRUCTED)
		break;
	}
    }
}

void f_tell_room PROT((void))
{
    int num_arg = st_num_arg;
    svalue_t *arg = sp - num_arg + 1;
    object_t *ob;
    array_t *avoid;

    if (arg->type == T_OBJECT) {
	ob = arg[0].u.ob;
    } else {			/* must be a string... */
	ob = find_object(arg[0].u.string);
	if (!ob || !object_visible(ob))
	    error("Bad argument 1 to tell_room()\n");
    }

    if (num_arg == 2) {
	avoid = &the_null_array;
    } else {
	if (arg[2].type == T_OBJECT) {
	    avoid = allocate_empty_array(1);
	    avoid->item[0] = arg[2];
	} else {
	    avoid = arg[2].u.arr;
	}
    }

    tell_room(ob, &arg[1], avoid);
    if (num_arg > 2 && arg[2].type == T_OBJECT)
	free_array(avoid);
    pop_n_elems(num_arg);
}
#endif

#ifdef F_WRITE
static void print_svalue P1(svalue_t *, arg)
{
    char tbuf[2048];

    if (arg == 0) {
	tell_object(command_giver, "<NULL>", 6);
    } else {
	switch (arg->type) {
	    case T_STRING:
		check_legal_string(arg);
		tell_object(command_giver, arg->u.string, SVALUE_STRLEN(arg));
		break;
	    case T_OBJECT:
		sprintf(tbuf, "OBJ(/%s)", arg->u.ob->name);
		tell_object(command_giver, tbuf, strlen(tbuf));
		break;
	    case T_NUMBER:
		sprintf(tbuf, "%d", arg->u.number);
		tell_object(command_giver, tbuf, strlen(tbuf));
		break;
	    case T_REAL:
		sprintf(tbuf, "%d", arg->u.real);
		tell_object(command_giver, tbuf, strlen(tbuf));
		break;
	    case T_ARRAY:
		tell_object(command_giver, "<ARRAY>", 7);
		break;
	    case T_MAPPING:
		tell_object(command_giver, "<MAPPING>", 9);
		break;
	    case T_FUNCTION:
		tell_object(command_giver, "<FUNCTION>", 10);
		break;
#ifndef NO_BUFFER_TYPE
	    case T_BUFFER:
		tell_object(command_giver, "<BUFFER>", 8);
		break;
#endif
	    default:
		tell_object(command_giver, "<UNKNOWN>", 9);
		break;
	}
    }
}

void f_write PROT((void))
{
    object_t *ob = command_giver;

#ifndef NO_SHADOWS
    if (ob == 0 && current_object->shadowing)
	ob = current_object;
    if (ob) {
	/* Send the message to the first object in the shadow list */
	while (ob->shadowing)
	    ob = ob->shadowing;
    }
#else
    if (!ob)
	ob = current_object;
#endif				/* NO_SHADOWS */
    save_command_giver(ob);
    print_svalue(sp);
    restore_command_giver();
    pop_stack();
}
#endif
