#define SUPRESS_COMPILER_INLINES
#ifdef LATTICE
#include "/lpc_incl.h"
#include "/mapping.h"
#include "/comm.h"
#include "/file_incl.h"
#include "/file.h"
#include "/object.h"
#include "/eoperators.h"
#include "/backend.h"
#include "/swap.h"
#include "/compiler.h"
#else
#include "../lpc_incl.h"
#include "../mapping.h"
#include "../comm.h"
#include "../file_incl.h"
#include "../file.h"
#include "../object.h"
#include "../eoperators.h"
#include "../backend.h"
#include "../swap.h"
#include "../compiler.h"
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
    variable_t *var;
    svalue_t *sv;

    var = find_status((sp-1)->u.string);
    if (!var)
	error("No variable named '%s'!\n", (sp-1)->u.string);
    sv = &current_object->variables[var - current_object->prog->variable_names];
    free_svalue(sv, "f_store_variable");
    *sv = *sp--;
    free_string_svalue(sp--);
}
#endif

#ifdef F_FETCH_VARIABLE
void
f_fetch_variable PROT((void)) {
    variable_t *var;
    svalue_t *sv;

    var = find_status(sp->u.string);
    if (!var)
	error("No variable named '%s'!\n", sp->u.string);
    sv = &current_object->variables[var - current_object->prog->variable_names];
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
	depth++;
	if (depth > MAX_SAVE_SVALUE_DEPTH) {
	    depth = 0;
	    error("Mappings and/or arrays nested too deep (%d) for copy()\n",
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
    int i, j, num;
    array_t *vec, *subvec;
    function_t *functions, *funp;
    program_t *prog;
    int flag = (sp--)->u.number;
    unsigned short *types;

    if (sp->u.ob->flags & O_SWAPPED) 
	load_ob_from_swap(sp->u.ob);
    
    num = sp->u.ob->prog->num_functions;
    functions = sp->u.ob->prog->functions;

    vec = allocate_empty_array(num);
    i = num;
    
    while (i--) {
	if (flag) {
	    funp = functions + i;
	    prog = sp->u.ob->prog;

	    while (funp->flags & NAME_INHERITED) {
		prog = prog->inherit[funp->offset].prog;
		funp = &prog->functions[funp->function_index_offset];
	    }

	    if (prog->type_start && prog->type_start[i] != INDEX_START_NONE)
		types = &prog->argument_types[prog->type_start[i]];
	    else
		types = 0;

	    vec->item[i].type = T_ARRAY;
	    subvec = vec->item[i].u.arr = allocate_empty_array(3 + funp->num_arg);
	    
	    subvec->item[0].type = T_STRING;
	    subvec->item[0].subtype = STRING_SHARED;
	    subvec->item[0].u.string = ref_string(funp->name);

	    subvec->item[1].type = T_NUMBER;
	    subvec->item[1].subtype = 0;
	    subvec->item[1].u.number = funp->num_arg;

	    subvec->item[2].type = T_STRING;
	    subvec->item[2].subtype = STRING_SHARED;
	    subvec->item[2].u.string = make_shared_string(get_type_name(funp->type));

	    for (j = 0; j < funp->num_arg; j++) {
		if (types) {
		    subvec->item[3 + j].type = T_STRING;
		    subvec->item[3 + j].subtype = STRING_SHARED;
		    subvec->item[3 + j].u.string = make_shared_string(get_type_name(types[j]));
		} else {
		    subvec->item[3 + j].type = T_NUMBER;
		    subvec->item[3 + j].u.number = 0;
		}
	    }
	} else {
	    vec->item[i].type = T_STRING;
	    vec->item[i].subtype = STRING_SHARED;
	    vec->item[i].u.string = ref_string(functions[i].name);
	}
    }
    
    pop_stack();
    push_refed_array(vec);
}
#endif

/* Beek */
#ifdef F_VARIABLES
void f_variables PROT((void)) {
    int i, num;
    array_t *vec, *subvec;
    variable_t *variables;
    int flag = (sp--)->u.number;
    program_t *prog = sp->u.ob->prog;
    
    if (sp->u.ob->flags & O_SWAPPED)
	load_ob_from_swap(sp->u.ob);
    
    num = prog->num_variables;
    variables = prog->variable_names;

    vec = allocate_empty_array(num);
    i = num;
    
    while (i--) {
	if (flag) {
	    vec->item[i].type = T_ARRAY;
	    subvec = vec->item[i].u.arr = allocate_empty_array(2);
	    subvec->item[0].type = T_STRING;
	    subvec->item[0].subtype = STRING_SHARED;
	    subvec->item[0].u.string = ref_string(variables[i].name);
	    subvec->item[1].type = T_STRING;
	    subvec->item[1].subtype = STRING_SHARED;
	    subvec->item[1].u.string = make_shared_string(get_type_name(variables[i].type));
	} else {
	    vec->item[i].type = T_STRING;
	    vec->item[i].subtype = STRING_SHARED;
	    vec->item[i].u.string = ref_string(variables[i].name);
	}
    }
    
    pop_stack();
    push_refed_array(vec);
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
    mapping_node_t *elt, **mtab;

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

    parts = CALLOCATE(NSTRSEGS, char *, TAG_TEMPORARY, "f_terminal_colour: parts");
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
		    parts = RESIZE(parts, num + NSTRSEGS, char *, 
				   TAG_TEMPORARY, "f_terminal_colour: parts realloc");
	    }
	}
    }
    if (strlen(instr))	/* trailing seg, if not delimiter */
	parts[num++] = instr;

    lens = CALLOCATE(num, int, TAG_TEMPORARY, "f_terminal_colour: lens");

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
	cp = deststr = new_string(j, "f_terminal_colour: deststr");
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
	cp = deststr = new_string(j, "f_terminal_colour: deststr");
	for (i = 0; i < num; i++)
	{
	    strcpy(cp,parts[i]);
	    cp += lens[i];
	}
    }
    FREE(lens);
    FREE(parts);
    FREE_MSTR(savestr);
    /* now we have what we want */
    pop_stack();
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
    if (sz <= 1) return 0;

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
     */
    switch (rel[0]) {
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
	}
	break;
    case 'H':
    case 'h':
	if (!strcasecmp(rel + 1, "uman"))
	    found = PLURAL_SUFFIX;
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
	if (!strcasecmp(rel + 1, "afe")) 
	    found = PLURAL_SUFFIX;
	break;
    case 'T':
    case 't':
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
	    && end[-2] != 'o' && end[-2] != 'u')
		suffix = "ies";
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
  char buf[2049];
  char *p, *end;

  file = check_valid_path(file, current_object, "file_size", 0);
  
  if (!file) return -1;
  if (stat(file, &st) == -1)
      return -1;
  if (st.st_mode == S_IFDIR)
      return -2;
  if (!(f = fopen(file, "r")))
      return -1;
  
  do {
      end = buf + fread(buf, 2048, 1, f);
      *end = 0;
      p = buf - 1;
      while ((p = strchr(p + 1, '\n')) && p < end)
	  ret++;
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
    program_t *prog = sp->u.ob->prog;
    int i, num;
    function_t *functions;
    
    num = prog->num_functions;
    functions = prog->functions;
    
    for (i = 0; i < num; i++) {
	if (functions[i].flags & (NAME_INHERITED | NAME_NO_CODE)) continue;
	if (strcmp(functions[i].name, "create")==0) continue;
	break;
    }
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
    int hdr_size;
    object_t *ob;
    int num_pushes[100];
    int i;
    outbuffer_t out;

    for (i=0; i < 10; i++)
	num_pushes[i]=0;
    for (ob = obj_list; ob; ob = ob->next_all) {
	if (ob->flags & O_CLONE) continue;
	hdr_size += sizeof(program_t);
	prog_size += ob->prog->program_size;
	func_size += ob->prog->num_functions * sizeof(function_t);
	string_size += ob->prog->num_strings * sizeof(char *);
	var_size += ob->prog->num_variables * sizeof(variable_t);
	inherit_size += ob->prog->num_inherited * sizeof(inherit_t);
#if 0
	walk_program_code(num_pushes, ob->prog);
#endif
    }
    outbuf_zero(&out);
    for (i=0; i <10; i++)
	outbuf_addv(&out, "%i ", num_pushes[i]);
    
    outbuf_addv(&out, "\nheader size: %i\n", hdr_size);
    outbuf_addv(&out, "code size: %i\n", prog_size);
    outbuf_addv(&out, "function size: %i\n", func_size);
    outbuf_addv(&out, "string size: %i\n", string_size);
    outbuf_addv(&out, "var size: %i\n", var_size);
    outbuf_addv(&out, "inherit size: %i\n", inherit_size);
    
    outbuf_push(&out);
}
#endif

/* Magician - 08May95
 * int remove_interactive(object ob)
 * If the object isn't destructed and is interactive, then remove it's
 * interactivity and disconnect it.  (useful for exec()ing to an already
 * interactive object, ie, Linkdead reconnection)
 */

void f_remove_interactive PROT((void)) {
    if( (sp->u.ob->flags & O_DESTRUCTED) || !(sp->u.ob->interactive) ) {
	free_object(sp->u.ob, "f_remove_interactive");
	*sp = const0;
    } else {
        remove_interactive(sp->u.ob);
	/* It may have been dested */
	if (sp->type == T_OBJECT)
	    free_object(sp->u.ob, "f_remove_interactive");
	*sp = const1;
    }
}
