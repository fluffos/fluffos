#ifdef LATTICE
#include "/lpc_incl.h"
#include "/mapping.h"
#include "/comm.h"
#else
#include "../lpc_incl.h"
#include "../mapping.h"
#include "../comm.h"
#endif

/* I forgot who wrote this, please claim it :) */
#ifdef F_REMOVE_SHADOW
void
f_remove_shadow PROT((void))
{
    struct object *ob;
    
    ob = current_object;
    if ( st_num_arg )
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

/* This was originally written my Malic for Demon.  I rewrote parts of it
   when I added it (added function support, etc) -Beek */
#ifdef F_QUERY_NOTIFY_FAIL
void
f_query_notify_fail PROT((void)) {
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

/* Beek */
#ifdef F_SET_PROMPT
void
f_set_prompt PROT((void)) {
    struct object *who;
    if (st_num_arg == 2) {
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

/* Gudu@VR wrote copy_array() and copy_mapping() which this is heavily
 * based on.  I made it into a general copy() efun which incorporates
 * both. -Beek
 */
#ifdef F_COPY
static int depth;

void deep_copy_svalue PROT((struct svalue *, struct svalue *));

struct vector *deep_copy_vector P1( struct vector *, arg ) {
    struct vector *vec;
    int i;
    
    vec = allocate_empty_array(arg->size);
    for (i = 0; i < arg->size; i++)
	deep_copy_svalue(&arg->item[i], &vec->item[i]);
    return vec;
}

int doCopy P3( struct mapping *, map, struct node *, elt, struct mapping *, dest) {
    struct svalue *sp;
    
    sp = find_for_insert(dest, &elt->values[0], 1);
    if (!sp) {
	mapping_too_large();
	return 1;
    }
    
    deep_copy_svalue(&elt->values[1], sp);
    return 0;
}

struct mapping *deep_copy_mapping P1( struct mapping *, arg ) {
    struct mapping *map;
    
    map = allocate_mapping( 0 ); /* this should be fixed.  -Beek */
    mapTraverse( arg, (int (*)()) doCopy, map);
    return map;
}

void deep_copy_svalue P2(struct svalue *, from, struct svalue *, to) {
    switch (from->type) {
    case T_POINTER:
	depth++;
	if (depth > MAX_SAVE_SVALUE_DEPTH) {
	    depth = 0;
	    error("Mappings and/or arrays nested too deep (%d) for copy()\n",
		  MAX_SAVE_SVALUE_DEPTH);
	}
	*to = *from;
	to->u.vec = deep_copy_vector( from->u.vec );
	depth--;
	break;
    case T_MAPPING:
	depth++;
	if (depth > MAX_SAVE_SVALUE_DEPTH) {
	    depth = 0;
	    error("Mappings and/or arrays nested too deep (%d) for copy()\n",
		  MAX_SAVE_SVALUE_DEPTH);
	}
	*to = *from;
	to->u.map = deep_copy_mapping( from->u.map );
	depth--;
	break;
    default:
	assign_svalue_no_free( to, from );
    }
}

void f_copy PROT((void))
{
    struct svalue ret;
    
    depth = 0;
    deep_copy_svalue(sp, &ret);
    free_svalue(sp, "f_copy");
    *sp = ret;
}
#endif    

/* Gudu@VR */    
/* This should have flag and other function info etc -Beek */
#ifdef F_FUNCTIONS
void f_functions PROT((void)) {
    int i, num;
    struct vector *vec;
    struct function *functions;
    
    num = sp->u.ob->prog->p.i.num_functions;
    functions = sp->u.ob->prog->p.i.functions;
    
    vec = allocate_empty_array(num);
    i = num;
    
    while (i--) {
	vec->item[i].type = T_STRING;
	vec->item[i].subtype = STRING_SHARED;
	vec->item[i].u.string = ref_string(functions[i].name);
    }
    
    pop_stack();
    push_refed_vector(vec);
}
#endif

/*Aleas@Nightmare */
#ifdef F_TERMINAL_COLOUR
/* A fast implementation of the Nightmare color support.

   Rewritten several times, since Beek wants it to be
   perfect :)

   Takes a string and a mapping as args. The string is
   exploded using "%^" as delimiter, then all keys of
   the mapping found in the resulting array are replaced
   by their values. Afterwards a string imploded from
   the array is returned.

   No actual string copying is done except for the
   creation of the final string and a temporary copy of
   the input string to avoid destruction of shared
   input strings. An array of pointers to the segments
   of the string is compared against the mapping keys
   and replaced with a pointer to the value belonging to
   that key where matches are found.

   After the replacement pass the result string is created
   from the pointer array.

   Further speed is gained by the fact that no parsing is
   done if the input string does not contain any "%^" 
   delimiter sequence.

   by Aleas@Nightmare, dec-94 */

/* number of input string segments, if more, it still works, but a _slow_ realloc is required */
#define NSTRSEGS 32
void 
f_terminal_colour P2( int, num_arg, int, instruction)
{
    char *instr, *cp, *savestr, *deststr, **parts;
    int num, i, j, k, *lens;
    struct node *elt, **mtab;

    cp = instr = (sp-1)->u.string;
    do {
	cp = strchr(cp,'%');
	if (cp) 
	{
	    if (cp[1] == '^')
	    {
		savestr = string_copy(instr, "f_terminal_colour");
		cp = savestr + ( cp - instr );
		instr = savestr;
		break;
	    }
	    cp++;
	}
    } while (cp);
    if (cp == NULL)
    {
	pop_stack();	/* no delimiter in string, so return the original */
	return;
    }
    /* here we have something to parse */

    parts = ( char ** ) DMALLOC(sizeof( char * ) * NSTRSEGS, TAG_TEMPORARY, "f_terminal_colour: parts");
    if (cp - instr) 	/* starting seg, if not delimiter */
    {
	num = 1;
	parts[0] = instr;
	*cp = 0;
    } else
	num = 0;
    while (cp) 
    {
	cp += 2;
	instr = cp;
	do {
	    cp = strchr(cp,'%');
	    if (cp) 
	    {
		if (cp[1] == '^')
		    break;
		cp++;
	    }
	} while (cp);
	if (cp)
	{
	    *cp = 0;
	    if (cp > instr)
	    {
		parts[num] = instr;
		num++;
		if (num % NSTRSEGS == 0)
		    parts = ( char ** ) DREALLOC(( char *) parts,
			sizeof( char * ) * (num + NSTRSEGS), TAG_TEMPORARY, "f_terminal_colour: parts realloc");
	    }
	}
    }
    if (strlen(instr))	/* trailing seg, if not delimiter */
	parts[num++] = instr;

    lens = ( int * ) DMALLOC(sizeof( int ) * num, TAG_TEMPORARY, "f_terminal_colour: lens");

    /* Do the the pointer replacement and calculate the lengths */
    if ( ( mtab = sp->u.map->table ) ) /* a mapping with values */
    {
	for (j = i = 0, k = sp->u.map->table_size; i < num; i++)
	{
	    cp = parts[i];
	    for (elt = mtab[mapHashstr(cp) & k]; elt; elt = elt->next)
		if ( elt->values->type == T_STRING && 
		     (elt->values + 1)->type == T_STRING &&
		     strcmp(cp,elt->values->u.string) == 0 )
		     {
			cp = parts[i] = (elt->values + 1)->u.string;
			break;
		     }
	    lens[i] = strlen(cp);
	    j += lens[i];
	}
    } else {
	for (j = i = 0; i < num; i++)
	{
	    lens[i] = strlen(parts[i]);
	    j += lens[i];
	}
    }

    /* now we have the final string in parts and length in j. let's compose it */
    if (j > max_string_length) {
	j = max_string_length;
	cp = deststr = DXALLOC(j + 1, TAG_STRING, "f_terminal_colour: deststr");
	for (j = i = 0; i < num; i++)
	{
	    k = lens[i];
	    if ( (j+k) >= max_string_length )
	    {
		strncpy(cp,parts[i], max_string_length-j-k);
		deststr[max_string_length] = 0;
		break;
	    } else {
		strcpy(cp,parts[i]);
		j += k;
		cp += k;
	    }
	}
    } else {
	cp = deststr = DXALLOC(j + 1, TAG_STRING, "f_terminal_colour: deststr");
	for (i = 0; i < num; i++)
	{
	    strcpy(cp,parts[i]);
	    cp += lens[i];
	}
    }
    FREE(lens);
    FREE(parts);
    FREE(savestr);
    /* now we have what we want */
    pop_stack();
    switch(sp->subtype)
    {
	case STRING_MALLOC:
	    FREE(sp->u.string);
	    break;
	case STRING_SHARED:
	    free_string(sp->u.string);
	    break;
	default:
	    break;
    }
    sp->subtype = STRING_MALLOC;
    sp->u.string = deststr;
}
#endif
