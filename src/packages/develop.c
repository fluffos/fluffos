#ifdef LATTICE
#include "/lpc_incl.h"
#include "/comm.h"
#else
#include "../lpc_incl.h"
#include "../comm.h"
#endif

static struct object *ob;

#ifdef F_DEBUG_INFO
void
f_debug_info PROT((void))
{
    struct svalue *arg, res;

    arg = sp - 1;
    switch (arg[0].u.number) {
    case 0:
	{
	    int i, flags;
	    struct object *obj2;

	    ob = arg[1].u.ob;
	    flags = ob->flags;
	    add_vmessage("O_HEART_BEAT      : %s\n",
			flags & O_HEART_BEAT ? "TRUE" : "FALSE");
#ifndef NO_WIZARDS
	    add_vmessage("O_IS_WIZARD       : %s\n",
			flags & O_IS_WIZARD ? "TRUE" : "FALSE");
#endif
#ifdef NO_ADD_ACTION
	    add_vmessage("O_LISTENER : %s\n",
			flags & O_LISTENER ? "TRUE" : "FALSE");
#else
	    add_vmessage("O_ENABLE_COMMANDS : %s\n",
			flags & O_ENABLE_COMMANDS ? "TRUE" : "FALSE");
#endif
	    add_vmessage("O_CLONE           : %s\n",
			flags & O_CLONE ? "TRUE" : "FALSE");
	    add_vmessage("O_VIRTUAL         : %s\n",
			flags & O_VIRTUAL ? "TRUE" : "FALSE");
	    add_vmessage("O_DESTRUCTED      : %s\n",
			flags & O_DESTRUCTED ? "TRUE" : "FALSE");
	    add_vmessage("O_SWAPPED         : %s\n",
			flags & O_SWAPPED ? "TRUE" : "FALSE");
	    add_vmessage("O_ONCE_INTERACTIVE: %s\n",
			flags & O_ONCE_INTERACTIVE ? "TRUE" : "FALSE");
	    add_vmessage("O_RESET_STATE     : %s\n",
			flags & O_RESET_STATE ? "TRUE" : "FALSE");
	    add_vmessage("O_WILL_CLEAN_UP   : %s\n",
			flags & O_WILL_CLEAN_UP ? "TRUE" : "FALSE");
	    add_vmessage("O_WILL_RESET: %s\n",
			flags & O_WILL_RESET ? "TRUE" : "FALSE");
#ifndef NO_LIGHT
	    add_vmessage("total light : %d\n", ob->total_light);
#endif
	    add_vmessage("next_reset  : %d\n", ob->next_reset);
	    add_vmessage("time_of_ref : %d\n", ob->time_of_ref);
	    add_vmessage("ref         : %d\n", ob->ref);
#ifdef DEBUG
	    add_vmessage("extra_ref   : %d\n", ob->extra_ref);
#endif
	    add_vmessage("swap_num    : %d\n", ob->swap_num);
	    add_vmessage("name        : '%s'\n", ob->name);
	    add_vmessage("next_all    : OBJ(%s)\n",
			ob->next_all ? ob->next_all->name : "NULL");
	    if (obj_list == ob)
		add_message("This object is the head of the object list.\n");
	    for (obj2 = obj_list, i = 1; obj2; obj2 = obj2->next_all, i++)
		if (obj2->next_all == ob) {
		    add_vmessage("Previous object in object list: OBJ(%s)\n",
				obj2->name);
		    add_vmessage("position in object list:%d\n", i);
		}
	    assign_svalue_no_free(&res, &const0);
	    break;
	}
    case 1:
	ob = arg[1].u.ob;
	if (ob->flags & O_SWAPPED) {
	    add_message("Swapped\n");
	    break;
	}
	add_vmessage("program ref's %d\n", ob->prog->p.i.ref);
	add_vmessage("Name %s\n", ob->prog->name);
	add_vmessage("program size %d\n",
		    ob->prog->p.i.program_size);
	add_vmessage("num func's %d (%d) \n", ob->prog->p.i.num_functions,
		    ob->prog->p.i.num_functions * sizeof(struct function));
	add_vmessage("num strings %d\n", ob->prog->p.i.num_strings);
	add_vmessage("num vars %d (%d)\n", ob->prog->p.i.num_variables,
		    ob->prog->p.i.num_variables * sizeof(struct variable));
	add_vmessage("num inherits %d (%d)\n", ob->prog->p.i.num_inherited,
		    ob->prog->p.i.num_inherited * sizeof(struct inherit));
	add_vmessage("total size %d\n", ob->prog->p.i.total_size);
	assign_svalue_no_free(&res, &const0);
	break;
    case 2:
	{
	    int i;
	    ob = arg[1].u.ob;
	    for (i=0; i<ob->prog->p.i.num_variables; i++) {
		add_vmessage("%s: ", ob->prog->p.i.variable_names[i].name);
		print_svalue(&ob->variables[i]);
		add_message("\n");
	    }
	    break;
	}	
    default:
	bad_arg(1, F_DEBUG_INFO);
    }
    pop_stack();
    free_svalue(sp,"debug_info");
    *sp = res;
}
#endif

#ifdef F_REFS
void
f_refs PROT((void))
{
    int r;

    switch (sp->type) {
    case T_MAPPING:
	r = sp->u.map->ref;
	break;
    case T_POINTER:
	r = sp->u.vec->ref;
	break;
    case T_OBJECT:
	r = sp->u.ob->ref;
	break;
    case T_FUNCTION:
	r = sp->u.fp->ref;
	break;
    case T_BUFFER:
	r = sp->u.buf->ref;
	break;
    default:
	r = 0;
	break;
    }
    free_svalue(sp, "f_refs");
    put_number(r - 1);		/* minus 1 to compensate for being arg of
				 * refs() */
}
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
#ifdef F_DEBUGMALLOC
void
f_debugmalloc PROT((void))
{
    dump_debugmalloc((sp - 1)->u.string, sp->u.number);
    free_string_svalue(--sp);
    *sp = const0;
}
#endif

#ifdef F_SET_MALLOC_MASK
void
f_set_malloc_mask PROT((void))
{
    set_malloc_mask(sp->u.number);
    sp->u.number = 0;
}
#endif

#ifdef F_CHECK_MEMORY
void
f_check_memory PROT((void))
{
    check_all_blocks(sp->u.number);
    sp->u.number = 0;
}
#endif
#endif				/* (defined(DEBUGMALLOC) &&
				 * defined(DEBUGMALLOC_EXTENSIONS)) */

#ifdef F_TRACE
void
f_trace PROT((void))
{
    int ot = -1;

    if (command_giver && command_giver->interactive &&
        command_giver->flags & O_IS_WIZARD) {
        ot = command_giver->interactive->trace_level;
        command_giver->interactive->trace_level = sp->u.number;
    }
    sp->u.number = ot;
}
#endif

#ifdef F_TRACEPREFIX
void
f_traceprefix PROT((void))
{
    char *old = 0;

    if (command_giver && command_giver->interactive &&
        command_giver->flags & O_IS_WIZARD) {
        old = command_giver->interactive->trace_prefix;
        if (sp->type & T_STRING) {
            command_giver->interactive->trace_prefix =
                make_shared_string(sp->u.string);
            free_string_svalue(sp);
	} else
            command_giver->interactive->trace_prefix = 0;
    }
    if (old) {
        put_shared_string(old);
    } else *sp = const0;
}
#endif

