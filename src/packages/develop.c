#ifdef LATTICE
#include "/lpc_incl.h"
#include "/comm.h"
#include "/md.h"
#else
#include "../lpc_incl.h"
#include "../comm.h"
#include "../md.h"
#endif

static object_t *ob;

#ifdef F_DEBUG_INFO
void
f_debug_info PROT((void))
{
    svalue_t *arg;
    outbuffer_t out;

    outbuf_zero(&out);
    arg = sp - 1;
    switch (arg[0].u.number) {
    case 0:
	{
	    int i, flags;
	    object_t *obj2;

	    ob = arg[1].u.ob;
	    flags = ob->flags;
	   outbuf_addv(&out, "O_HEART_BEAT      : %s\n",
			flags & O_HEART_BEAT ? "TRUE" : "FALSE");
#ifndef NO_WIZARDS
	   outbuf_addv(&out, "O_IS_WIZARD       : %s\n",
			flags & O_IS_WIZARD ? "TRUE" : "FALSE");
#endif
#ifdef NO_ADD_ACTION
	   outbuf_addv(&out, "O_LISTENER : %s\n",
			flags & O_LISTENER ? "TRUE" : "FALSE");
#else
	   outbuf_addv(&out, "O_ENABLE_COMMANDS : %s\n",
			flags & O_ENABLE_COMMANDS ? "TRUE" : "FALSE");
#endif
	   outbuf_addv(&out, "O_CLONE           : %s\n",
			flags & O_CLONE ? "TRUE" : "FALSE");
	   outbuf_addv(&out, "O_VIRTUAL         : %s\n",
			flags & O_VIRTUAL ? "TRUE" : "FALSE");
	   outbuf_addv(&out, "O_DESTRUCTED      : %s\n",
			flags & O_DESTRUCTED ? "TRUE" : "FALSE");
	   outbuf_addv(&out, "O_SWAPPED         : %s\n",
			flags & O_SWAPPED ? "TRUE" : "FALSE");
	   outbuf_addv(&out, "O_ONCE_INTERACTIVE: %s\n",
			flags & O_ONCE_INTERACTIVE ? "TRUE" : "FALSE");
	   outbuf_addv(&out, "O_RESET_STATE     : %s\n",
			flags & O_RESET_STATE ? "TRUE" : "FALSE");
	   outbuf_addv(&out, "O_WILL_CLEAN_UP   : %s\n",
			flags & O_WILL_CLEAN_UP ? "TRUE" : "FALSE");
	   outbuf_addv(&out, "O_WILL_RESET: %s\n",
			flags & O_WILL_RESET ? "TRUE" : "FALSE");
#ifndef NO_LIGHT
	   outbuf_addv(&out, "total light : %d\n", ob->total_light);
#endif
	   outbuf_addv(&out, "next_reset  : %d\n", ob->next_reset);
	   outbuf_addv(&out, "time_of_ref : %d\n", ob->time_of_ref);
	   outbuf_addv(&out, "ref         : %d\n", ob->ref);
#ifdef DEBUG
	   outbuf_addv(&out, "extra_ref   : %d\n", ob->extra_ref);
#endif
	   outbuf_addv(&out, "swap_num    : %d\n", ob->swap_num);
	   outbuf_addv(&out, "name        : '%s'\n", ob->name);
	   outbuf_addv(&out, "next_all    : OBJ(%s)\n",
			ob->next_all ? ob->next_all->name : "NULL");
	    if (obj_list == ob)
		outbuf_add(&out, "This object is the head of the object list.\n");
	    for (obj2 = obj_list, i = 1; obj2; obj2 = obj2->next_all, i++)
		if (obj2->next_all == ob) {
		   outbuf_addv(&out, "Previous object in object list: OBJ(%s)\n",
				obj2->name);
		   outbuf_addv(&out, "position in object list:%d\n", i);
		}
	    break;
	}
    case 1:
	ob = arg[1].u.ob;
	if (ob->flags & O_SWAPPED) {
	    outbuf_add(&out, "Swapped\n");
	    break;
	}
	outbuf_addv(&out, "program ref's %d\n", ob->prog->ref);
	outbuf_addv(&out, "Name %s\n", ob->prog->name);
	outbuf_addv(&out, "program size %d\n",
		    ob->prog->program_size);
	outbuf_addv(&out, "num func's %d (%d) \n", ob->prog->num_functions,
		    ob->prog->num_functions * sizeof(function_t));
	outbuf_addv(&out, "num strings %d\n", ob->prog->num_strings);
	outbuf_addv(&out, "num vars %d (%d)\n", ob->prog->num_variables,
		    ob->prog->num_variables * sizeof(variable_t));
	outbuf_addv(&out, "num inherits %d (%d)\n", ob->prog->num_inherited,
		    ob->prog->num_inherited * sizeof(inherit_t));
	outbuf_addv(&out, "total size %d\n", ob->prog->total_size);
	break;
    case 2:
	{
	    int i;
	    ob = arg[1].u.ob;
	    for (i=0; i<ob->prog->num_variables; i++) {
		outbuf_addv(&out, "%s: ", ob->prog->variable_names[i].name);
		print_svalue(&ob->variables[i]);
		outbuf_add(&out, "\n");
	    }
	    break;
	}	
    default:
	bad_arg(1, F_DEBUG_INFO);
    }
    pop_stack();
    pop_stack();
    outbuf_push(&out);
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
    case T_CLASS:
    case T_ARRAY:
	r = sp->u.arr->ref;
	break;
    case T_OBJECT:
	r = sp->u.ob->ref;
	break;
    case T_FUNCTION:
	r = sp->u.fp->hdr.ref;
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
    char *res;
    
    res = dump_debugmalloc((sp - 1)->u.string, sp->u.number);
    free_string_svalue(--sp);
    sp->subtype = STRING_MALLOC;
    sp->u.string = res;
}
#endif

#ifdef F_SET_MALLOC_MASK
void
f_set_malloc_mask PROT((void))
{
    set_malloc_mask((sp--)->u.number);
}
#endif

#ifdef F_CHECK_MEMORY
void
f_check_memory PROT((void))
{
    check_all_blocks((sp--)->u.number);
}
#endif
#endif				/* (defined(DEBUGMALLOC) &&
				 * defined(DEBUGMALLOC_EXTENSIONS)) */

#ifdef F_TRACE
void
f_trace PROT((void))
{
    int ot = -1;

    if (command_giver && command_giver->interactive) {
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

    if (command_giver && command_giver->interactive) {
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

