#ifdef LATTICE
#include "/config.h"
#include "/efuns.h"
#include "/lint.h"
#else
#include "../config.h"
#include "../efuns.h"
#include "../lint.h"
#endif

#ifdef F_REMOVE_SHADOW
void
f_remove_shadow P2(int, num_arg, int, instruction)
{
    struct object *ob;
    
    ob = current_object;
    if ( num_arg )
	{
	    ob = sp->u.ob;
	    pop_stack();
	}
    if ( ! ob || ! ob->shadowing )
	push_number( 0 );
    else
	{
	    if ( ob->shadowed )
		ob->shadowed->shadowing = ob->shadowing;
	    if ( ob->shadowing )
		ob->shadowing->shadowed = ob->shadowed;
	    ob->shadowing = ob->shadowed = 0;
	    push_number( 1 );
	}
}
#endif

#ifdef F_QUERY_NOTIFY_FAIL
void
f_query_notify_fail P2(int, num_arg, int, instruction) {
    char *p;

    if (command_giver && command_giver->interactive) {
	if (command_giver->interactive->iflags & NOTIFY_FAIL_FUNC) {
	    push_funp(command_giver->interactive->default_err_message.f);
	    return;
	} else if (p = command_giver->interactive->default_err_message.s) {
	    sp++;
	    sp->type = T_STRING;
	    sp->subtype = STRING_SHARED;
	    sp->u.string = p;
	    ref_string(p);
	    return;
	}
    }
    push_number(0);
}
#endif

#ifdef F_SET_PROMPT
void
f_set_prompt P2(int, num_arg, int, instruction) {
    struct object *who;
    if (num_arg == 2) {
	who = sp->u.ob;
	pop_stack();
    } else who = command_giver;
    
    if (!who || who->flags & O_DESTRUCTED || !who->interactive)
	error("Prompts can only be set for interactives.\n");
    
    /* Future work */
    /* ed() will nuke this; also we have to make sure the string will get
     * freed */
}
#endif
