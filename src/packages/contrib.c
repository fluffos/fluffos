#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#endif

/* I forgot who wrote this, please claim it :) */
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

/* This was originally written my Malic for Demon.  I rewrote parts of it
   when I added it (added function support, etc) -Beek */
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

/* Beek */
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
    
    vec = allocate_array(arg->size);
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

void f_copy P2(int, num_arg, int, instruction) {
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
void f_functions P2(int, num_arg, int, instruction) {
    int i, num;
    struct vector *vec;
    struct function *functions;
    
    num = sp->u.ob->prog->p.i.num_functions;
    functions = sp->u.ob->prog->p.i.functions;
    
    vec = allocate_array(num);
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

/* Gudu@VR */
/*
 * efun::map_string() -- Replace all occurrences of mapping keys in a string
 *                       with the corresponding mapping values.  It is similar
 *                       to efun::replace_string() except that it acomplishes
 *                       multiple different replacements in one pass.
 *
 * This scheme first extracts, from the input mapping, arrays of the mapping
 * keys, mapping values, first chars of the mapping keys, and sizes of the
 * mapping keys and values.  The string to map is then iterated over char
 * by char and compared to the array of mapping key first chars.  When a
 * mapping first char is found, the set of mapping keys is iterated over
 * and compared to the proper length of the string to be mapped.  If one of
 * the mapping keys matches, it is copied into the return string in place
 * of the corresponding char from the input string.
 *
 * Notes: It would be more efficient to use a pair of vectors as input
 *        instead of accepting a mapping and then extracting the vectors.
 *        I chose the mapping scheme because I thought it provided a better
 *        interface to the LPC coder.
 *        At present the routine will error if the mapping contains any keys
 *        or values that are not of type T_STRING.  This is somewhat of a
 *        kludge.  A more elegant solution might convert these non-strings
 *        to a string or somehow exclude them.  However, I don't really
 *        know if its worth the extra effort and or overhead.
 *
 * Gudu@VR 2/10/94
 */
#ifdef F_MAP_STRING
/* this should come from elsewhere */
#define MAX_STRING_SIZE 8096

/* This uses malloc() directly.  That's a no-no */
void f_map_string P2( int, num_arg, int, instruction)
{
    int i, *map_key_sizes, *map_val_sizes;
    struct vector *map_keys, *map_vals;
    char *search_keys, *ptr1, *ptr2, *ptr3, str[MAX_STRING_SIZE];
    char *overflow_ptr;
    
    /* Setup a workspace. */
    /* This is to speed and simplify the processing below. */
    map_keys = mapping_indices( sp->u.map);
    map_vals = mapping_values( sp->u.map);
    i = map_keys->size;
    search_keys = (char *)malloc( i+1);
    map_key_sizes = (int *)malloc( sizeof( int)*i);
    map_val_sizes = (int *)malloc( sizeof( int)*i);
    search_keys[i] = '\0'; /* Null terminate the string */
    while( i--) {
	if( map_keys->item[i].type != T_STRING)
	    /* For now, just bomb. */
	    error("Mapping index not of type string in map_string().\n");
	if( map_vals->item[i].type != T_STRING)
	    /* For now, just bomb. */
	    error("Mapping value not of type string in map_string().\n");
	search_keys[i] = map_keys->item[i].u.string[0];
	map_key_sizes[i] = strlen( map_keys->item[i].u.string);
	map_val_sizes[i] = strlen( map_vals->item[i].u.string);
    }
    
    /* Doit! */
    ptr1 = (sp-1)->u.string;
    ptr2 = str;
    overflow_ptr = str + MAX_STRING_SIZE - 1;
    while( *ptr1) {
	ptr3 = strchr( search_keys, *ptr1);
	if( !ptr3) {
	    /* Char not a key; just copy it and go on. */
	    /* Check for overflow. */
	    if( ptr2 == overflow_ptr)
		error("Max string size exceeded in map_string().\n");
	    *ptr2 = *ptr1;
	    ptr1++;
	    ptr2++;
	    continue;
	}
	/* We have a key!  Find the full key and copy it. */
	i = map_keys->size;
	while( i--) {
	    if( strncmp( ptr1, map_keys->item[i].u.string, map_key_sizes[i]))
		continue;
	    /* Check for overflow */
	    if( ptr2 + map_val_sizes[i] >= overflow_ptr)
		error("Max string size exceeded in map_string().\n");
	    memcpy( ptr2, map_vals->item[i].u.string, map_val_sizes[i]);
	    ptr1 += map_key_sizes[i] - 1;
	    ptr2 += map_val_sizes[i] - 1;
	    break;
	}
	ptr1++;
	ptr2++;
    }
    /* Null terminate the new string */
    *ptr2 = '\0';
    
    pop_n_elems( 2);
    push_string( str, STRING_MALLOC);
}
#endif
