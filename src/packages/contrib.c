#define SUPRESS_COMPILER_INLINES
#ifdef LATTICE
#include "/lpc_incl.h"
#include "/comm.h"
#include "/file_incl.h"
#include "/file.h"
#include "/backend.h"
#include "/swap.h"
#include "/compiler.h"
#include "/main.h"
#include "/eoperators.h"
#else
#include "../lpc_incl.h"
#include "../comm.h"
#include "../file_incl.h"
#include "../file.h"
#include "../backend.h"
#include "../swap.h"
#include "../compiler.h"
#include "../main.h"
#include "../eoperators.h"
#endif

/* should be done in configure */
#ifdef WIN32
#define strcasecmp(X, Y) stricmp(X, Y)
#endif

/*
 * This differs from the livings() efun in that this efun only returns
 * objects which have had set_living_name() called as well as 
 * enable_commands().  The other major difference is that this is
 * substantially faster.
 */
#ifdef F_NAMED_LIVINGS
void f_named_livings() {
    int i;
    int nob, apply_valid_hide, hide_is_valid = 0;
    object_t *ob, **obtab;
    array_t *vec;

    nob = 0;
    apply_valid_hide = 1;

    obtab = CALLOCATE(max_array_size, object_t *, TAG_TEMPORARY, "named_livings");

    for (i = 0; i < CFG_LIVING_HASH_SIZE; i++) {
	for (ob = hashed_living[i]; ob; ob = ob->next_hashed_living) {
	    if (!(ob->flags & O_ENABLE_COMMANDS))
		continue;
	    if (ob->flags & O_HIDDEN) {
		if (apply_valid_hide) {
		    apply_valid_hide = 0;
		    hide_is_valid = valid_hide(current_object);
		}
		if (hide_is_valid)
		    continue;
	    }
	    if (nob == max_array_size)
		break;
	    obtab[nob++] = ob;
	}
    }

    vec = allocate_empty_array(nob);
    while (--nob >= 0) {
	vec->item[nob].type = T_OBJECT;
	vec->item[nob].u.ob = obtab[nob];
	add_ref(obtab[nob], "livings");
    }

    FREE(obtab);

    push_refed_array(vec);
}    
#endif

/* I forgot who wrote this, please claim it :) */
#ifdef F_REMOVE_SHADOW
void
f_remove_shadow PROT((void))
{
    object_t *ob;
    
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
	} else if ((p = command_giver->interactive->default_err_message.s)) {
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

/* Beek again */
#ifdef F_STORE_VARIABLE
void
f_store_variable PROT((void)) {
    int idx;
    svalue_t *sv;
    unsigned short type;
    
    idx = find_global_variable(current_object->prog, (sp-1)->u.string, &type);
    if (idx == -1)
	error("No variable named '%s'!\n", (sp-1)->u.string);
    sv = &current_object->variables[idx];
    free_svalue(sv, "f_store_variable");
    *sv = *sp--;
    free_string_svalue(sp--);
}
#endif

#ifdef F_FETCH_VARIABLE
void
f_fetch_variable PROT((void)) {
    int idx;
    svalue_t *sv;
    unsigned short type;
    
    idx = find_global_variable(current_object->prog, sp->u.string, &type);
    if (idx == -1)
	error("No variable named '%s'!\n", sp->u.string);
    sv = &current_object->variables[idx];
    free_string_svalue(sp--);
    push_svalue(sv);
}
#endif

/* Beek */
#ifdef F_SET_PROMPT
void
f_set_prompt PROT((void)) {
    object_t *who;
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

void deep_copy_svalue PROT((svalue_t *, svalue_t *));

array_t *deep_copy_array P1( array_t *, arg ) {
    array_t *vec;
    int i;
    
    vec = allocate_empty_array(arg->size);
    for (i = 0; i < arg->size; i++)
	deep_copy_svalue(&arg->item[i], &vec->item[i]);
    return vec;
}

int doCopy P3( mapping_t *, map, mapping_node_t *, elt, mapping_t *, dest) {
    svalue_t *sp;
    
    sp = find_for_insert(dest, &elt->values[0], 1);
    if (!sp) {
	mapping_too_large();
	return 1;
    }
    
    deep_copy_svalue(&elt->values[1], sp);
    return 0;
}

mapping_t *deep_copy_mapping P1( mapping_t *, arg ) {
    mapping_t *map;
    
    map = allocate_mapping( 0 ); /* this should be fixed.  -Beek */
    mapTraverse( arg, (int (*)()) doCopy, map);
    return map;
}

void deep_copy_svalue P2(svalue_t *, from, svalue_t *, to) {
    switch (from->type) {
    case T_ARRAY:
    case T_CLASS:
	depth++;
	if (depth > MAX_SAVE_SVALUE_DEPTH) {
	    depth = 0;
	    error("Mappings, arrays and/or classes nested too deep (%d) for copy()\n",
		  MAX_SAVE_SVALUE_DEPTH);
	}
	*to = *from;
	to->u.arr = deep_copy_array( from->u.arr );
	depth--;
	break;
    case T_MAPPING:
	depth++;
	if (depth > MAX_SAVE_SVALUE_DEPTH) {
	    depth = 0;
	    error("Mappings, arrays and/or classes nested too deep (%d) for copy()\n",
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
    svalue_t ret;
    
    depth = 0;
    deep_copy_svalue(sp, &ret);
    free_svalue(sp, "f_copy");
    *sp = ret;
}
#endif    

/* Gudu@VR */    
/* flag and extra info by Beek */
#ifdef F_FUNCTIONS
void f_functions PROT((void)) {
    int i, j, num, index;
    array_t *vec, *subvec;
    runtime_function_u *func_entry;
    compiler_function_t *funp;
    program_t *prog;
    int flag = (sp--)->u.number;
    unsigned short *types;
    char buf[256];
    char *end = EndOf(buf);
    program_t *progp;
    
    if (sp->u.ob->flags & O_SWAPPED) 
	load_ob_from_swap(sp->u.ob);

    progp = sp->u.ob->prog;
    num = progp->num_functions_total;
    if (num && progp->function_table[progp->num_functions_defined-1].name[0]
	== APPLY___INIT_SPECIAL_CHAR)
	num--;
	
    vec = allocate_empty_array(num);
    i = num;
    
    while (i--) {
	prog = sp->u.ob->prog;
	index = i;
	func_entry = FIND_FUNC_ENTRY(prog, index);

	/* Walk up the inheritance tree to the real definition */
	while (prog->function_flags[index] & NAME_INHERITED) {
	    prog = prog->inherit[func_entry->inh.offset].prog;
	    index = func_entry->inh.function_index_offset;
	    func_entry = FIND_FUNC_ENTRY(prog, index);
	}

	funp = prog->function_table + func_entry->def.f_index;

	if (flag) {
	    if (prog->type_start && prog->type_start[index] != INDEX_START_NONE)
		types = &prog->argument_types[prog->type_start[index]];
	    else
		types = 0;

	    vec->item[i].type = T_ARRAY;
	    subvec = vec->item[i].u.arr = allocate_empty_array(3 + func_entry->def.num_arg);
	    
	    subvec->item[0].type = T_STRING;
	    subvec->item[0].subtype = STRING_SHARED;
	    subvec->item[0].u.string = ref_string(funp->name);

	    subvec->item[1].type = T_NUMBER;
	    subvec->item[1].subtype = 0;
	    subvec->item[1].u.number = func_entry->def.num_arg;

	    get_type_name(buf, end, funp->type);
	    subvec->item[2].type = T_STRING;
	    subvec->item[2].subtype = STRING_SHARED;
	    subvec->item[2].u.string = make_shared_string(buf);

	    for (j = 0; j < func_entry->def.num_arg; j++) {
		if (types) {
		    get_type_name(buf, end, types[j]);
		    subvec->item[3 + j].type = T_STRING;
		    subvec->item[3 + j].subtype = STRING_SHARED;
		    subvec->item[3 + j].u.string = make_shared_string(buf);
		} else {
		    subvec->item[3 + j].type = T_NUMBER;
		    subvec->item[3 + j].u.number = 0;
		}
	    }
	} else {
	    vec->item[i].type = T_STRING;
	    vec->item[i].subtype = STRING_SHARED;
	    vec->item[i].u.string = ref_string(funp->name);
	}
    }
    
    pop_stack();
    push_refed_array(vec);
}
#endif

/* Beek */
#ifdef F_VARIABLES
static void fv_recurse P5(array_t *, arr, int *, idx, program_t *, prog, int, type, int, flag) {
    int i;
    array_t *subarr;
    char buf[256];
    char *end = EndOf(buf);
    
    for (i = 0; i < prog->num_inherited; i++) {
	fv_recurse(arr, idx, prog->inherit[i].prog, 
		   type | prog->inherit[i].type_mod, flag);
    }
    for (i = 0; i < prog->num_variables_defined; i++) {
	if (flag) {
	    arr->item[*idx + i].type = T_ARRAY;
	    subarr = arr->item[*idx + i].u.arr = allocate_empty_array(2);
	    subarr->item[0].type = T_STRING;
	    subarr->item[0].subtype = STRING_SHARED;
	    subarr->item[0].u.string = ref_string(prog->variable_table[i]);
	    get_type_name(buf, end, prog->variable_types[i]);
	    subarr->item[1].type = T_STRING;
	    subarr->item[1].subtype = STRING_SHARED;
	    subarr->item[1].u.string = make_shared_string(buf);
	} else {
	    arr->item[*idx + i].type = T_STRING;
	    arr->item[*idx + i].subtype = STRING_SHARED;
	    arr->item[*idx + i].u.string = ref_string(prog->variable_table[i]);
	}
    }
    *idx += prog->num_variables_defined;
}

void f_variables PROT((void)) {
    int idx = 0;
    array_t *arr;
    int flag = (sp--)->u.number;
    program_t *prog = sp->u.ob->prog;
    
    if (sp->u.ob->flags & O_SWAPPED)
	load_ob_from_swap(sp->u.ob);
    
    arr = allocate_empty_array(prog->num_variables_total);
    fv_recurse(arr, &idx, prog, 0, flag);
    
    pop_stack();
    push_refed_array(arr);
}
#endif

/* also Beek */
#ifdef F_HEART_BEATS
void f_heart_beats PROT((void)) {
    push_refed_array(get_heart_beats());
}
#endif

/*Aleas@Nightmare */
#ifdef F_TERMINAL_COLOUR
/* A fast implementation of the Nightmare color support.

   [Ed note: These codes were actually used on Discworld
   before Nightmare]

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

/* number of input string segments, if more, it still works, but a
   _slow_ realloc is required */
#define NSTRSEGS 32
#define TC_FIRST_CHAR '%'
#define TC_SECOND_CHAR '^'

int at_end(int i, int imax, int z, int *lens) {
    if (z + 1 != lens[i])
	return 0;
    for (i++; i < imax; i++) {
	if (lens[i] > 0)
	    return 0;
    }
    return 1;
}

void 
f_terminal_colour P2( int, num_arg, int, instruction)
{
    char *instr, *cp, *savestr, *deststr, **parts;
    int num, i, j, k, col, space, *lens, maybe_at_end;
    int space_garbage;
    mapping_node_t *elt, **mtab;
    int tmp;
    int wrap = 0;
    int indent = 0;

    if (st_num_arg >= 3) {
	if (st_num_arg == 4)
	    indent = (sp--)->u.number;
	wrap = (sp--)->u.number;
	if (wrap < 2 && wrap != 0) wrap = 2;
	if (indent < 0 || indent >= wrap - 1)
	    indent = wrap - 2;
    }

    cp = instr = (sp-1)->u.string;
    do {
	cp = strchr(cp, TC_FIRST_CHAR);
	if (cp) 
	{
	    if (cp[1] == TC_SECOND_CHAR)
	    {
		savestr = string_copy(instr, "f_terminal_colour");
		cp = savestr + ( cp - instr );
		instr = savestr;
		break;
	    }
	    cp++;
	}
    } while (cp);
    if (cp == NULL) {
	if (wrap) {
	    num = 1;
	    parts = CALLOCATE(1, char *, TAG_TEMPORARY, "f_terminal_colour: parts");
	    parts[0] = instr;
	    savestr = 0;
	} else {
	    pop_stack(); /* no delimiter in string, so return the original */
	    return;
	}
    } else {
        /* here we have something to parse */

	parts = CALLOCATE(NSTRSEGS, char *, TAG_TEMPORARY, "f_terminal_colour: parts");
	if (cp - instr) {	/* starting seg, if not delimiter */
	    num = 1;
	    parts[0] = instr;
	    *cp = 0;
	} else
	    num = 0;
	while (cp) {
	    cp += 2;
	    instr = cp;
	    do {
		cp = strchr(cp,TC_FIRST_CHAR);
		if (cp) {
		    if (cp[1] == TC_SECOND_CHAR)
			break;
		    cp++;
		}
	    } while (cp);
	    if (cp) {
		*cp = 0;
		if (cp > instr) {
		    parts[num] = instr;
		    num++;
		    if (num % NSTRSEGS == 0)
			parts = RESIZE(parts, num + NSTRSEGS, char *, 
				       TAG_TEMPORARY, "f_terminal_colour: parts realloc");
		}
	    }
	}
	if (*instr)	/* trailing seg, if not delimiter */
	    parts[num++] = instr;
    }

    /* Could keep track of the lens as we create parts, removing the need
       for a strlen() below */
    lens = CALLOCATE(num, int, TAG_TEMPORARY, "f_terminal_colour: lens");
    mtab = sp->u.map->table;

    /* Do the the pointer replacement and calculate the lengths */
    col = 0;
    space = 0;
    maybe_at_end = 0;
    for (j = i = 0, k = sp->u.map->table_size; i < num; i++) {
	int len;
	    
	if ((cp = findstring(parts[i]))) {
	    tmp = MAP_POINTER_HASH(cp);
	    for (elt = mtab[tmp & k]; elt; elt = elt->next)
		if ( elt->values->type == T_STRING && 
		     (elt->values + 1)->type == T_STRING &&
		     cp == elt->values->u.string) {
		    parts[i] = (elt->values + 1)->u.string;
		    /* Negative indicates don't count for wrapping */
		    len = SVALUE_STRLEN(elt->values + 1);
		    if (wrap) len = -len;
		    break;
		}
	    if (!elt)
		len = SHARED_STRLEN(cp);
	} else {
	    len = strlen(parts[i]);
	}
	lens[i] = len;
	if (len > 0) {
	    if (maybe_at_end) {
		if (j + indent > max_string_length) {
		    /* this string no longer counts, so we are still in
		       a maybe_at_end condition.  This means we will end
		       up truncating the rest of the fragments too, since
		       the indent will never fit. */
		    lens[i] = 0;
		    len = 0;
		} else {
		    j += indent;
		    col += indent;
		    maybe_at_end = 0;
		}
	    }
	    j += len;
	    if (j > max_string_length) {
		lens[i] -= j - max_string_length;
		j = max_string_length;
	    }
	    if (wrap) {
		int z;
		char *p = parts[i];
		for (z = 0; z < lens[i]; z++) {
		    char c = p[z];
		    if (c == '\n') {
			col = 0;
		    } else {
			col++;
			if (c == ' ')
			    space = col;
			if (col == wrap+1) {
			    if (space) {
				col -= space;
				space = 0;
			    } else {
				j++;
				col = 1;
			    }
			} else
			    continue;
		    }
		    /* If we get here, we ended a line */
		    if (z + 1 != lens[i] || col) {
			j += indent;
			col += indent;
		    } else
			maybe_at_end = 1;

		    if (j > max_string_length) {
			lens[i] -= (j - max_string_length);
			j = max_string_length;
			if (lens[i] < z) {
			    /* must have been ok 
			       or we wouldn't be here */
			    lens[i] = z;
			    break;
			}
		    }
		}
	    }
	} else {
	    j += -len;
	    if (j > max_string_length) {
		lens[i] = -(-(lens[i]) - (j - max_string_length));
		j = max_string_length;
	    }
	}
    }
    
    /* now we have the final string in parts and length in j. 
       let's compose it, wrapping if necessary */
    cp = deststr = new_string(j, "f_terminal_colour: deststr");
    if (wrap) {
	/* FIXME */
	char *tmp = new_string(8192, "f_terminal_colour: wrap");
	char *pt = tmp;
	
	col = 0;
	space = 0;
	for (i = 0; i < num; i++) {
	    int kind;
	    int len;
	    int l = lens[i];
	    char *p = parts[i];
	    if (l < 0) {
		memcpy(pt, p, -l);
		pt += -l;
		space_garbage += -l; /* Number of chars due to ignored junk
					since last space */
		continue;
	    }
	    for (k = 0; k < lens[i]; k++) {
		int n;
		char c = p[k];
		*pt++ = c;
		if (c == '\n') {
		    col = 0;
		    kind = 0;
		} else {
		    col++;
		    if (c == ' ') {
			space = col;
			space_garbage = 0;
		    }
		    if (col == wrap+1) {
			if (space) {
			    col -= space;
			    space = 0;
			    kind = 1;
			} else {
			    col = 1;
			    kind = 2;
			}
		    } else
			continue;
		}
		/* If we get here, we ended a line */
		len = (kind == 1 ? col + space_garbage : col);
		n = (pt - tmp) - len;
		memcpy(cp, tmp, n);
		cp += n;
		if (kind == 1) {
		    /* replace the space */
		    cp[-1] = '\n';
		}
		if (kind == 2) {
		    /* need to insert a newline */
		    *cp++ = '\n';
		}
		memmove(tmp, tmp + n, len);
		pt = tmp + len;
		if (col || !at_end(i, num, k, lens)) {
		    memset(cp, ' ', indent);
		    cp += indent;
		    col += indent;
		}
	    }
	}
	memcpy(cp, tmp, pt - tmp);
	cp += pt - tmp;
	FREE_MSTR(tmp);
    } else {
	for (i = 0; i < num; i++) {
	    memcpy(cp, parts[i], lens[i]);
	    cp += lens[i];
	}
    }
    *cp = 0;
    FREE(lens);
    FREE(parts);
    if (savestr)
	FREE_MSTR(savestr);
    /* now we have what we want */
    pop_stack();
#ifdef DEBUG
    if (cp - deststr != j) {
	fatal("Length miscalculated in terminal_colour()\n    Expected: %i Was: %i\n    String: %s\n    Indent: %i Wrap: %i\n", j, cp - deststr, sp->u.string, indent, wrap);
    }
#endif
    free_string_svalue(sp);
    sp->type = T_STRING;
    sp->subtype = STRING_MALLOC;
    sp->u.string = deststr;
}
#endif

#ifdef F_PLURALIZE

#define PLURAL_SUFFIX  1
#define PLURAL_SAME    2
/* number to chop is added */
#define PLURAL_CHOP    2

char *pluralize P1(char *, str) {
    char *pre, *rel, *end;
    char *p, *of_buf;
    int of_len = 0, plen, slen;
    int sz;

    /* default rule */
    int found = 0;
    char *suffix = "s";
    
    sz = strlen(str);
    if (sz == 0) return 0;

    /* if it is of the form 'X of Y', pluralize the 'X' part */
    if ((p = strstr(str, " of "))) {
	of_buf = alloc_cstring(p, "pluralize: of");
	of_len = strlen(of_buf);
	sz = p - str;
    }

    /*
     * first, get rid of determiners.  pluralized forms never have them ;)
     * They can have 'the' so don't remove that 
     */  
    if (str[0] == 'a' || str[0] == 'A') {
	if (str[1] == ' ') {
	    plen = sz - 2;
	    pre = DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
	    strncpy(pre, str + 2, plen);
	} else if (str[1] == 'n' && str[2] == ' ') {
	    plen = sz - 3;
	    pre = DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
	    strncpy(pre, str + 3, plen);
	} else {
	    plen = sz;
	    pre = DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
	    strncpy(pre, str, plen);
	}
    } else {
	plen = sz;
	pre = DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
	strncpy(pre, str, plen);
    }
    pre[plen] = 0;

    /*
     * only pluralize the last word, ie: lose adjectives.
     */
    if ((p = strrchr(pre, ' ')))
	rel = p + 1;
    else
	rel = pre;
	
    end = rel + strlen(rel);

    /*
     * trap the exceptions to the rules below and special cases.
     *
     * Hmm, maybe this should be a prebuilt hash table to make maintenance
     * a bit easier.  Possibly gperf based; or is that overkill? :-)
     */
    switch (rel[0]) {
    case 'A':
    case 'a':
	if (!strcasecmp(rel + 1, "re")) {
	    found = PLURAL_CHOP + 3;
	    suffix = "is";
	}
	break;
    case 'B':
    case 'b':
	if (!strcasecmp(rel + 1, "us")) {
	    found = PLURAL_SUFFIX;
	    suffix = "es";
	}
	break;
    case 'C':
    case 'c':
	if (!strcasecmp(rel + 1, "hild")) {
	    found = PLURAL_SUFFIX;
	    suffix = "ren";
	}
	break;
    case 'D':
    case 'd':
	if (!strcasecmp(rel + 1, "ie")) {
	    found = PLURAL_CHOP + 1;
	    suffix = "ce";
	} else
	if (!strcasecmp(rel + 1, "eer")) {
	    found = PLURAL_SAME;
	} else
	if (!strcasecmp(rel + 1, "o")) {
	    found = PLURAL_SUFFIX;
	    suffix = "es";
        } else
	if (!strcasecmp(rel + 1, "ynamo"))
	    found = PLURAL_SUFFIX;
	break;
    case 'F':
    case 'f':
	if (!strcasecmp(rel + 1, "oot")) {
	    found = PLURAL_CHOP + 3;
	    suffix = "eet";
	    break;
	}
	if (!strcasecmp(rel + 1, "ish")) {
	    found = PLURAL_SAME;
	    break;
	}
	if (!strcasecmp(rel + 1, "ife"))
	    found = PLURAL_SUFFIX;
	break;
    case 'G':
    case 'g':
	if (!strcasecmp(rel + 1, "oose")) {
	    found = PLURAL_CHOP + 4;
	    suffix = "eese";
	} else
	if (!strcasecmp(rel + 1, "o")) {
	    found = PLURAL_SUFFIX;
	    suffix = "es";
	} else
	if (!strcasecmp(rel + 1, "um")) {
	    found = PLURAL_SUFFIX;
	}
	break;
    case 'H':
    case 'h':
	if (!strcasecmp(rel + 1, "uman"))
	    found = PLURAL_SUFFIX;
	else if (!strcasecmp(rel + 1, "ave")) {
	    found = PLURAL_CHOP + 2;
	    suffix = "s";
	}	    
	break;
    case 'I':
    case 'i':
	if (!strcasecmp(rel + 1, "ndex")) {
	    found = PLURAL_CHOP + 2;
	    suffix = "ices";
	}
	break;
    case 'L':
    case 'l':
	if (!strcasecmp(rel + 1, "ouse")) {
	    found = PLURAL_CHOP + 4;
	    suffix = "ice";
	}
	break;
    case 'M':
    case 'm':
	if (!strcasecmp(rel + 1, "ackerel")) {
	    found = PLURAL_SAME;
	    break;
	}
	if (!strcasecmp(rel + 1, "oose")) {
	    found = PLURAL_SAME;
	    break;
	}
	if (!strcasecmp(rel + 1, "ouse")) {
	    found = PLURAL_CHOP + 4;
	    suffix = "ice";
	    break;
	}
	if (!strcasecmp(rel + 1, "atrix")) {
	    found = PLURAL_CHOP + 1;
	    suffix = "ces";
	}
	break;
    case 'O':
    case 'o':
	if (!strcasecmp(rel + 1, "x")) {
	    found = PLURAL_SUFFIX;
	    suffix = "en";
	}
	break;
    case 'S':
    case 's':
	if (!strcasecmp(rel + 1, "heep")) {
	    found = PLURAL_SAME;
	    break;
	}
	if (!strcasecmp(rel + 1, "phinx")) {
	    found = PLURAL_CHOP + 1;
	    suffix = "ges";
	    break;
	}
	if (!strcasecmp(rel + 1, "afe")) {
	    found = PLURAL_SUFFIX;
	    break;
	}
	if (!strcasecmp(rel + 1, "haman")) 
	    found = PLURAL_SUFFIX;
	break;
    case 'T':
    case 't':
	if (!strcasecmp(rel + 1, "hief")) {
	    found = PLURAL_CHOP + 1;
	    suffix = "ves";
	    break;
	}
	if (!strcasecmp(rel + 1, "ooth")) {
	    found = PLURAL_CHOP + 4;
	    suffix = "eeth";
	}
	break;
    case 'V':
    case 'v':
	if (!strcasecmp(rel + 1, "ax")) {
	    found = PLURAL_SUFFIX;
	    suffix = "en";
	}
	break;
    case 'W':
    case 'w':
	if (!strcasecmp(rel + 1, "as")) {
	    found = PLURAL_CHOP + 2;
	    suffix = "ere";
	}
	break;
    }
    /*
     * now handle "rules" ... god I hate english!!
     */
    /*
     * *x -> *xes (fox -> foxes)
     * *s -> *ses (pass -> passes)
     * *ch -> *ches (church -> churches)
     * *sh -> *shes (brush -> brushes)
     */
    /*
     * *ff -> *ves (staff -> staves)
     * *fe -> *ves (knife -> knives)
     */
    /*
     * *f -> *ves (half -> halves)
     * *ef -> *efs (chef -> chefs) (really a rule for a special case)
     */
    /*
     * *y -> *ies (gumby -> gumbies)
     */
    /*
     * *us -> *i (virus -> viri)
     */
    /*
     * *man -> *men (foreman -> foremen)
     */
    /*
     * *is -> *es (this is from gordons pluralize ... )
     */
    /*
     * *o -> *s (also from gordon)
     */

    /* don't have to set found to PLURAL_SUFFIX in these rules b/c
       found == 0 is interpreted as PLURAL_SUFFIX */
    if (!found)
	switch (end[-1]) {
	case 'E': case 'e':
	    if (end[-2] == 'f' || end[-2] == 'F') {
		found = PLURAL_CHOP + 2;
		suffix = "ves";
	    }
	    break;
	case 'F': case 'f':
	    if (end[-2] == 'e' || end[-2] == 'E')
		break;
	    if (end[-2] == 'f' || end[-2] == 'F') {
		found = PLURAL_CHOP + 2;
		suffix = "ves";
		break;
	    }
	    found = PLURAL_CHOP + 1;
	    suffix = "ves";
	    break;
	case 'H': case 'h':
	    if (end[-2] == 'c' || end[-2]=='s')
		suffix = "es";
	    break;
	case 'M': case 'm':
	    if (end[-2] == 'u') {
		found = PLURAL_CHOP + 2;
		suffix = "a";
	    }
	    break;
	case 'N': case 'n':
	    if (end[-2] == 'a' && end[-3] == 'm') {
		found = PLURAL_CHOP + 3;
		suffix = "men";
	    }
	    break;
	case 'O': case 'o':
	    suffix = "es";
	    break;
	case 'S': case 's':
	    if (end[-2] == 'i') {
		found = PLURAL_CHOP + 2;
		suffix = "es";
		break;
	    }
	    if (end[-2] == 'u') {
		found = PLURAL_CHOP + 2;
		suffix = "i";
		break;
	    }
	    if (end[-2] == 'a' || end[-2] == 'e' || end[-2] == 'o')
		suffix = "ses";
	    else
		suffix = "es";
	    break;
	case 'X': case 'x':
	    suffix = "es";
	    break;
	case 'Y': case 'y':
	    if (end[-2] != 'a' && end[-2] != 'e' && end[-2] != 'i'
		&& end[-2] != 'o' && end[-2] != 'u') {
		found = PLURAL_CHOP + 1;    
		suffix = "ies";
	    }
	    break;
	case 'Z': case 'z':
	    if (end[-2] == 'a' || end[-2] == 'e' || end[-2] == 'o'
		|| end[-2] == 'i' || end[-2] == 'u')
		suffix = "zes";
	    else
		suffix = "es";
    }

    switch (found) {
    case PLURAL_SAME:
	slen = 0;
	sz = plen + of_len;
	break;
    default:
	plen -= (found - PLURAL_CHOP);
	/* fallthrough */
    case 0:
    case PLURAL_SUFFIX:
	slen = strlen(suffix);
	sz = plen + slen + of_len;
	break;
    }

    p = new_string(sz, "pluralize");
    p[sz] = 0;
    
    strncpy(p, pre, plen);
    if (slen) 
	strncpy(p + plen, suffix, slen);
    if (of_len) {
	strcpy(p + plen + slen, of_buf);
	FREE(of_buf);
    }

    FREE(pre);
    return p;
} /* end of pluralize() */

void 
f_pluralize PROT((void))
{
   char *s;

   s = pluralize(sp->u.string);
   pop_stack();
   if (!s)
      push_number(0);
   else
      push_malloced_string(s);
}
#endif

#ifdef F_FILE_LENGTH
/*
 * file_length() efun, returns the number of lines in a file.
 * Returns -1 if no privs or file doesn't exist.
 */
int file_length P1(char *, file)
{
  struct stat st;
  FILE *f;
  int ret = 0;
  int num;
  char buf[2049];
  char *p, *newp;

  file = check_valid_path(file, current_object, "file_size", 0);
  
  if (!file) return -1;
  if (stat(file, &st) == -1)
      return -1;
  if (st.st_mode & S_IFDIR)
      return -2;
  if (!(f = fopen(file, "r")))
      return -1;
  
  do {
      num = fread(buf, 1, 2048, f);
      p = buf - 1;
      while ((newp = memchr(p + 1, '\n', num))) {
	  num -= (newp - p);
	  p = newp;
	  ret++;
      }
  } while (!feof(f));

  fclose(f);
  return ret;
} /* end of file_length() */

void 
f_file_length PROT((void))
{
    int l;
    
    l = file_length(sp->u.string);
    pop_stack();
    push_number(l);
}
#endif

#ifdef F_UPPER_CASE
void
f_upper_case PROT((void))
{
    register char *str;

    str = sp->u.string;
    /* find first upper case letter, if any */
    for (; *str; str++) {
	if (islower(*str)) {
	    int l = str - sp->u.string;
	    unlink_string_svalue(sp);
	    str = sp->u.string + l;
	    *str -= 'a' - 'A';
	    for (str++; *str; str++) {
		if (islower(*str))
		    *str -= 'a' - 'A';
	    }
	    return;
	}
    }
}
#endif

#ifdef F_REPLACEABLE
void f_replaceable PROT((void)) {
    program_t *prog;
    int i, j, num, numignore;
    char **ignore;
    
    if (st_num_arg == 2) {
	numignore = sp->u.arr->size;
	if (numignore)
	    ignore = CALLOCATE(numignore, char *, TAG_TEMPORARY, "replaceable");
	else
	    ignore = 0;
	for (i = 0; i < numignore; i++) {
	    if (sp->u.arr->item[i].type == T_STRING)
		ignore[i] = findstring(sp->u.arr->item[i].u.string);
	    else
		ignore[i] = 0;
	}
	prog = (sp-1)->u.ob->prog;
    } else {
	numignore = 1;
	ignore = CALLOCATE(1, char *, TAG_TEMPORARY, "replaceable");
	ignore[0] = findstring(APPLY_CREATE);
	prog = sp->u.ob->prog;
    }
    
    num = prog->num_functions_total - 1; /* ignore #global_init# */
    
    for (i = 0; i < num; i++) {
	if (prog->function_flags[i] & (NAME_INHERITED | NAME_NO_CODE)) continue;
	for (j = 0; j < numignore; j++)
	    if (ignore[j] == prog->function_table[FIND_FUNC_ENTRY(prog, i)->def.f_index].name)
		break;
	if (j == numignore)
	    break;
    }
    if (st_num_arg == 2)
	free_array((sp--)->u.arr);
    FREE(ignore);
    free_svalue(sp, "f_replaceable");
    put_number(i == num);
}
#endif

#ifdef F_PROGRAM_INFO
void f_program_info PROT((void)) {
    int func_size = 0;
    int string_size = 0;
    int var_size = 0;
    int inherit_size = 0;
    int prog_size = 0;
    int hdr_size = 0;
    int class_size = 0;
    int type_size = 0;
    int total_size = 0;
    object_t *ob;
    program_t *prog;
    outbuffer_t out;
    int i, n;

    if (st_num_arg == 1) {
	ob = sp->u.ob;
	prog = ob->prog;
	if (!(ob->flags & (O_CLONE|O_SWAPPED))) {
	    hdr_size += sizeof(program_t);
	    prog_size += prog->program_size;
	    func_size += 2 * prog->num_functions_total; /* function flags */
#ifdef COMPRESS_FUNCTION_TABLES
	    /* compressed table header */
	    func_size += sizeof(compressed_offset_table_t) - 1;
	    /* it's entries */
	    func_size += (prog->function_compressed->first_defined - prog->function_compressed->num_compressed);
	    /* offset table */
	    func_size += sizeof(runtime_function_u) * (prog->num_functions_total - prog->function_compressed->num_deleted);
#else
	    /* offset table */
	    func_size += prog->num_functions_total * sizeof(runtime_function_u);
#endif
	    /* definitions */
	    func_size += prog->num_functions_defined * sizeof(compiler_function_t);
	    string_size += prog->num_strings * sizeof(char *);
	    var_size += prog->num_variables_defined * (sizeof(char *) + sizeof(unsigned short));
	    inherit_size += prog->num_inherited * sizeof(inherit_t);
	    if (prog->num_classes)
		class_size += prog->num_classes * sizeof(class_def_t) + (prog->classes[prog->num_classes - 1].index + prog->classes[prog->num_classes - 1].size) * sizeof(class_member_entry_t);
	    type_size += prog->num_functions_defined * sizeof(short);
	    n = 0;
	    for (i = 0; i < prog->num_functions_defined; i++) {
		int start;
		unsigned short *ts = prog->type_start;
		int ri;
		
		if (!ts) continue;
		start = ts[i];
		if (start == INDEX_START_NONE)
		    continue;
		ri = prog->function_table[i].runtime_index;
		start += FIND_FUNC_ENTRY(prog, ri)->def.num_arg;
		if (start > n)
		    n = start;
	    }
	    type_size += n * sizeof(short);
	    total_size += prog->total_size;
	}
	pop_stack();
    } else {
	for (ob = obj_list; ob; ob = ob->next_all) {
	    if (ob->flags & (O_CLONE|O_SWAPPED)) continue;
	    prog = ob->prog;
	    hdr_size += sizeof(program_t);
	    prog_size += prog->program_size;
	    func_size += prog->num_functions_total; /* function flags */
#ifdef COMPRESS_FUNCTION_TABLES
	    /* compressed table header */
	    func_size += sizeof(compressed_offset_table_t) - 1;
	    /* it's entries */
	    func_size += (prog->function_compressed->first_defined - prog->function_compressed->num_compressed);
	    /* offset table */
	    func_size += sizeof(runtime_function_u) * (prog->num_functions_total - prog->function_compressed->num_deleted);
#else
	    /* offset table */
	    func_size += prog->num_functions_total * sizeof(runtime_function_u);
#endif
	    /* definitions */
	    func_size += prog->num_functions_defined * sizeof(compiler_function_t);
	    string_size += prog->num_strings * sizeof(char *);
	    var_size += prog->num_variables_defined * (sizeof(char *) + sizeof(unsigned short));
	    inherit_size += prog->num_inherited * sizeof(inherit_t);
	    if (prog->num_classes)
		class_size += prog->num_classes * sizeof(class_def_t) + (prog->classes[prog->num_classes - 1].index + prog->classes[prog->num_classes - 1].size) * sizeof(class_member_entry_t);
	    type_size += prog->num_functions_defined * sizeof(short);
	    n = 0;
	    for (i = 0; i < prog->num_functions_defined; i++) {
		int start;
		int ri;
		
		unsigned short *ts = prog->type_start;
		if (!ts) continue;
		start = ts[i];
		if (start == INDEX_START_NONE)
		    continue;
		ri = prog->function_table[i].runtime_index;
		start += FIND_FUNC_ENTRY(prog, ri)->def.num_arg;
		if (start > n)
		    n = start;
	    }
	    type_size += n * sizeof(short);
	    total_size += prog->total_size;
	}
    }

    outbuf_zero(&out);
    
    outbuf_addv(&out, "\nheader size: %i\n", hdr_size);
    outbuf_addv(&out, "code size: %i\n", prog_size);
    outbuf_addv(&out, "function size: %i\n", func_size);
    outbuf_addv(&out, "string size: %i\n", string_size);
    outbuf_addv(&out, "var size: %i\n", var_size);
    outbuf_addv(&out, "class size: %i\n", class_size);
    outbuf_addv(&out, "inherit size: %i\n", inherit_size);
    outbuf_addv(&out, "saved type size: %i\n\n", type_size);

    outbuf_addv(&out, "total size: %i\n", total_size);
    
    outbuf_push(&out);
}
#endif

/* Magician - 08May95
 * int remove_interactive(object ob)
 * If the object isn't destructed and is interactive, then remove it's
 * interactivity and disconnect it.  (useful for exec()ing to an already
 * interactive object, ie, Linkdead reconnection)
 */

#ifdef F_REMOVE_INTERACTIVE
void f_remove_interactive PROT((void)) {
    if( (sp->u.ob->flags & O_DESTRUCTED) || !(sp->u.ob->interactive) ) {
	free_object(sp->u.ob, "f_remove_interactive");
	*sp = const0;
    } else {
        remove_interactive(sp->u.ob, 0);
	/* It may have been dested */
	if (sp->type == T_OBJECT)
	    free_object(sp->u.ob, "f_remove_interactive");
	*sp = const1;
    }
}
#endif

/* Zakk - August 23 1995
 * return the port number the interactive object used to connect to the
 * mud.
 */
#ifdef F_QUERY_IP_PORT
int query_ip_port P1(object_t *, ob)
{
    if (!ob || ob->interactive == 0)
	return 0;
    return ob->interactive->local_port;
}    

void
f_query_ip_port PROT((void))
{
    int tmp;
    
    if (st_num_arg) {
	tmp = query_ip_port(sp->u.ob);
	free_object(sp->u.ob, "f_query_ip_port");
    } else {
	tmp = query_ip_port(command_giver);
	sp++;
    }
    put_number(tmp);
}
#endif

/*
** John Viega (rust@lima.imaginary.com) Jan, 1996
** efuns for doing time zone conversions.  Much friendlier 
** than doing all the lookup tables in LPC.
** most muds have traditionally just used an offset of the 
** mud time or GMT, and this isn't always correct.
*/

#ifdef F_ZONETIME

char *
set_timezone (char * timezone)
{
  char put_tz[20];
  char *old_tz;

  old_tz = getenv("TZ");
  sprintf (put_tz, "TZ=%s", timezone);
  putenv (put_tz);
  tzset ();
  return old_tz;
}

void 
reset_timezone (char *old_tz)
{
  int  i = 0;
  int  env_size = 0;
  char put_tz[20];

  if (!old_tz)
    {
      while (environ[env_size] != NULL)
        {
          if (strlen (environ[env_size]) > 3 && environ[env_size][2] == '='
             && environ[env_size][1] == 'Z' && environ[env_size][0] == 'T')
            {
              i = env_size;
            }
          env_size++;
        }
      if ((i+1) == env_size)
        {
          environ[i] = NULL;
        }
      else
        {
          environ[i] = environ[env_size-1];
          environ[env_size-1] = NULL;
        }
    }
  else
    {
      sprintf (put_tz, "TZ=%s", old_tz);
      putenv (put_tz);
    }
  tzset ();
}

void 
f_zonetime PROT((void))
{
  char *timezone, *old_tz;
  char *retv;
  int  time_val;
  int  len;
  
  time_val   = sp->u.number;
  pop_stack ();
  timezone   = sp->u.string;
  pop_stack ();

  old_tz = set_timezone (timezone);
  retv = ctime ((time_t *)&time_val);
  len  = strlen (retv);
  retv[len-1] = '\0';
  reset_timezone (old_tz);
  push_malloced_string (string_copy(retv, "zonetime"));
  
}
#endif

#ifdef F_IS_DAYLIGHT_SAVINGS_TIME
void
f_is_daylight_savings_time PROT((void))
{
  struct tm *t;
  int       time_to_check;
  char      *timezone;
  char      *old_tz;

  time_to_check = sp->u.number;
  pop_stack ();
  timezone = sp->u.string;
  pop_stack ();

  old_tz = set_timezone (timezone);
 
  t = localtime ((time_t *)&time_to_check);

  push_number ((t->tm_isdst) > 0);

  reset_timezone (old_tz);
}
#endif

#ifdef F_DEBUG_MESSAGE
void f_debug_message PROT((void)) {
    debug_message("%s\n", sp->u.string);
    free_string_svalue(sp--);
}
#endif

#ifdef F_FUNCTION_OWNER
void f_function_owner PROT((void)) {
    object_t *owner = sp->u.fp->hdr.owner;
    
    free_funp(sp->u.fp);
    put_unrefed_object(owner, "f_function_owner");
}
#endif

#ifdef F_REPEAT_STRING
void f_repeat_string PROT((void)) {
    char *str;
    int repeat, len;
    char *ret, *p;
    int i;
    
    repeat = (sp--)->u.number;    
    if (repeat <= 0) {
	free_string_svalue(sp);
	sp->type = T_STRING;
	sp->subtype = STRING_CONSTANT;
	sp->u.string = "";
    } else if (repeat != 1) {
	str = sp->u.string;
	len = SVALUE_STRLEN(sp);
	p = ret = new_string(len * repeat, "f_repeat_string");
	for (i = 0; i < repeat; i++) {
	    memcpy(p, str, len);
	    p += len;
	}
	*p = 0;
	free_string_svalue(sp);
	sp->type = T_STRING;
	sp->subtype = STRING_MALLOC;
	sp->u.string = ret;
    }
}
#endif

#ifdef F_MEMORY_SUMMARY
static int memory_share PROT((svalue_t *));

static int node_share P3(mapping_t *, m, mapping_node_t *, elt, void *, tp) {
    int *t = (int *)tp;
    
    *t += sizeof(mapping_node_t) - 2*sizeof(svalue_t);
    *t += memory_share(&elt->values[0]);
    *t += memory_share(&elt->values[1]);

    return 0;
}

static int memory_share P1(svalue_t *, sv) {
    int i, total = sizeof(svalue_t);
    int subtotal;
    
    switch (sv->type) {
    case T_STRING:
	switch (sv->subtype) {
	case STRING_MALLOC:
	    return total + 
		(1 + COUNTED_STRLEN(sv->u.string) + sizeof(malloc_block_t))/
		(COUNTED_REF(sv->u.string));
	case STRING_SHARED:
	    return total + 
		(1 + COUNTED_STRLEN(sv->u.string) + sizeof(block_t))/
		(COUNTED_REF(sv->u.string));
	}
	break;
    case T_ARRAY:
    case T_CLASS:
	/* first svalue is stored inside the array struct, so sizeof(array_t)
	 * includes one svalue.
	 */
	subtotal = sizeof(array_t) - sizeof(svalue_t);
	for (i = 0; i < sv->u.arr->size; i++)
	    subtotal += memory_share(&sv->u.arr->item[i]);
	return total + subtotal/sv->u.arr->ref;
    case T_MAPPING:
	subtotal = sizeof(mapping_t);
	mapTraverse(sv->u.map, node_share, &subtotal);
	return total + subtotal/sv->u.map->ref;
    case T_FUNCTION:
    {
	svalue_t tmp;
	tmp.type = T_ARRAY;
	tmp.u.arr = sv->u.fp->hdr.args;

	if (tmp.u.arr)
	    subtotal = sizeof(funptr_hdr_t) + memory_share(&tmp) - sizeof(svalue_t);
	else
	    subtotal = sizeof(funptr_hdr_t);
	switch (sv->u.fp->hdr.type) {
	case FP_EFUN:
	    subtotal += sizeof(efun_ptr_t);
	    break;
	case FP_LOCAL | FP_NOT_BINDABLE:
	    subtotal += sizeof(local_ptr_t);
	    break;
	case FP_SIMUL:
	    subtotal += sizeof(simul_ptr_t);
	    break;
	case FP_FUNCTIONAL:
	case FP_FUNCTIONAL | FP_NOT_BINDABLE:
	    subtotal += sizeof(functional_t);
	    break;
	}
	return total + subtotal/sv->u.fp->hdr.ref;
    }
    case T_BUFFER:
	/* first byte is stored inside the buffer struct */
	return total + (sizeof(buffer_t) + sv->u.buf->size - 1)/sv->u.buf->ref;
    }
    return total;
}


/*
 * The returned mapping is:
 * 
 * map["program name"]["variable name"] = memory usage
 */
#ifdef F_MEMORY_SUMMARY
static void fms_recurse P4(mapping_t *, map, object_t *, ob, 
			   int *, idx, program_t *, prog) {
    int i;
    svalue_t *entry;
    svalue_t sv;
    
    sv.type = T_STRING;
    sv.subtype = STRING_SHARED;

    for (i = 0; i < prog->num_inherited; i++)
	fms_recurse(map, ob, idx, prog->inherit[i].prog);

    for (i = 0; i < prog->num_variables_defined; i++) {
	int size = memory_share(ob->variables + *idx + i);
	
	sv.u.string = prog->variable_table[i];
	entry = find_for_insert(map, &sv, 0);
	entry->u.number += size;
    }
    *idx += prog->num_variables_defined;
}

void f_memory_summary PROT((void)) {
    mapping_t *result = allocate_mapping(8);
    object_t *ob;
    int idx;
    svalue_t sv;
    
    sv.type = T_STRING;
    sv.subtype = STRING_SHARED;
    
    for (ob = obj_list; ob; ob = ob->next_all) {
	svalue_t *entry;
	
	if (ob->flags & O_SWAPPED) 
	    load_ob_from_swap(ob);

	sv.u.string = ob->prog->name;
	entry = find_for_insert(result, &sv, 0);
	if (entry->type == T_NUMBER) {
	    entry->type = T_MAPPING;
	    entry->u.map = allocate_mapping(8);
	}
	idx = 0;
	fms_recurse(entry->u.map, ob, &idx, ob->prog);
    }
    push_refed_mapping(result);
}
#endif

#endif
