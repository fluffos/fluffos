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
/* This should have flag and other function info etc -Beek */
#ifdef F_FUNCTIONS
void f_functions PROT((void)) {
    int i, num;
    array_t *vec;
    function_t *functions;
    
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
    push_refed_array(vec);
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

#ifdef F_PLURALIZE

#define PLURAL_IS(x) { x; return rel; }

/* This should handle 'of' */
char *pluralize P1(char *, str) {
    char *pre, *rel, *end;
    char *p, of_buf[256];
    int has_of = 0;
    int i, sz;
    
    sz = strlen(str);
    if (sz <= 1) return 0;

    /* if it is of the form 'X of Y', pluralize the 'X' part */
    if (p = strstr(str, " of ")) {
	strcpy(p, of_buf);
	has_of = 1;
	sz = p - str;
    }

    /* currently can add up to 3 chars on the end (child -> children) */
    pre = DXALLOC(sz+4, TAG_STRING, "pluralize: pre");
    
    /*
     * first, get rid of determiners.  pluralized forms never have them ;)
     * They can have 'the' so don't remove that 
     */  
    if (str[0] == 'a' || str[0] == 'A') {
	if (str[1] == ' ') {
	    strncpy(pre, str + 2, sz - 2);
	    pre[sz - 2] = 0;
	} else if (str[1] == 'n' && str[2] == ' ') {
	    strncpy(pre, str + 3, sz - 3);
	    pre[sz - 3] = 0;
	} else {
	    strncpy(pre, str, sz);
	    pre[sz] = 0;
	}
    } else {
	strcpy(pre, str, sz);
	pre[sz] = 0;
    }

    /*
     * only pluralize the last word, ie: lose adjectives.
     */
    if (p = strrchr(pre, ' '))
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
	if (!strcasecmp(rel + 1, "us"))
	    PLURAL_IS(strcpy(end, "es"));
	break;
    case 'C':
    case 'c':
	if (!strcasecmp(rel + 1, "hild"))
	    PLURAL_IS(strcpy(end, "ren"));
	break;
    case 'D':
    case 'd':
	if (!strcasecmp(rel + 1, "ie"))
	    PLURAL_IS(strcpy(rel + 2, "ce"));
	if (!strcasecmp(rel + 1, "eer"))
	    PLURAL_IS();
	if (!strcasecmp(rel + 1, "ynamo"))
	    PLURAL_IS(strcpy(end, "s"));
	break;
    case 'F':
    case 'f':
	if (!strcasecmp(rel + 1, "oot"))
	    PLURAL_IS( rel[1] = 'e'; rel[2] = 'e'; );
	break;
    case 'G':
    case 'g':
	if (!strcasecmp(rel + 1, "oose"))
	    PLURAL_IS( rel[1] = 'e'; rel[2] = 'e'; );
	break;
    case 'H':
    case 'h':
	if (!strcasecmp(rel + 1, "uman"))
	    PLURAL_IS(strcpy(end, "s"));
	break;
    case 'I':
    case 'i':
	if (!strcasecmp(rel + 1, "ndex"))
	    PLURAL_IS(strcpy(rel + 3, "ices"));
	break;
    case 'F':
    case 'f':
	if (!strcasecmp(rel + 1, "ish"))
	    PLURAL_IS();
	if (!strcasecmp(rel + 1, "ife"))
	    PLURAL_IS(strcpy(end, "s"));
	break;
    case 'L':
    case 'l':
	if (!strcasecmp(rel + 1, "ouse"))
	    PLURAL_IS(strcpy(rel + 1, "ice"));
	break;
    case 'M':
    case 'm':
	if (!strcasecmp(rel + 1, "oose"))
	    PLURAL_IS();
	if (!strcasecmp(rel + 1, "ouse"))
	    PLURAL_IS(strcpy(rel + 1, "ice"));
	if (!strcasecmp(rel + 1, "atrix"))
	    PLURAL_IS(strcpy(end - 1, "ces"));
	break;
    case 'O':
    case 'o':
	if (!strcasecmp(rel + 1, "x"))
	    PLURAL_IS(strcpy(end, "en"));
	break;
    case 'S':
    case 's':
	if (!strcasecmp(rel + 1, "heep"))
	    PLURAL_IS();
	if (!strcasecmp(rel + 1, "phinx"))
	    PLURAL_IS(strcpy(end - 1, "ges"));
	break;
    case 'T':
    case 't':
	if (!strcasecmp(rel + 1, "ooth"))
	    PLURAL_IS( rel[1] = 'e'; rel[2] = 'e'; );
	break;
    case 'V':
    case 'v':
	if (!strcasecmp(rel + 1, "ax"))
	    PLURAL_IS(strcpy(end, "en"));
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
    switch (end[-1]) {
    case 'E': case 'e':
	if (end[-2] == 'f' || end[-2] == 'F')
	    PLURAL_IS(strcpy(end - 2, "ves"));
	break;
    case 'F': case 'f':
	if (end[-2] == 'e' || end[-2] == 'E')
	    PLURAL_IS(strcpy(end, "s"));
	if (end[-2] == 'f' || end[-2] == 'F')
	    PLURAL_IS(strcpy(end - 2, "ves"));
	PLURAL_IS(strcpy(end - 1, "ves"));
	break;
    case 'H': case 'h':
	if (end[-2] == 'c' || end[-2]=='s')
	    PLURAL_IS(strcpy(end, "es"));
	break;
    case 'M': case 'm':
	if (end[-2] == 'u')
	    PLURAL_IS(strcpy(end - 2, "a"));
	break;
    case 'N': case 'n':
	if (end[-2] == 'a' && end[-3] == 'm')
	    PLURAL_IS(end[-2] = 'e');
	break;
    case 'O': case 'o':
	PLURAL_IS(strcpy(end, "es"));
    case 'S': case 's':
	if (end[-2] == 'i')
	    PLURAL_IS(end[-2] = 'e');
	if (end[-2] == 'u')
	    PLURAL_IS(strcpy(end-2, "i"));
	if (end[-2] == 'a' || end[-2] == 'e' || end[-2] == 'o')
	    PLURAL_IS(strcpy(end, "ses"));
	PLURAL_IS(strcpy(end, "es"));
    case 'X': case 'x':
	PLURAL_IS(strcpy(end, "es"));
    case 'Y': case 'y':
	if (end[-2] == 'a' || end[-2] == 'e' || end[-2] == 'i'
	    || end[-2] == 'o' || end[-2] == 'u')
	    PLURAL_IS(strcpy(end, "s"));
	PLURAL_IS(strcpy(end - 1, "ies"));
    case 'Z': case 'z':
	if (end[-2] == 'a' || end[-2] == 'e' || end[-2] == 'o'
	    || end[-2] == 'i' || end[-2] == 'u')
	    PLURAL_IS(strcpy(end, "zes"));
	PLURAL_IS(strcpy(end, "es"));
    }
    
    /*
     * default: (* -> *s)
     */
    PLURAL_IS(strcpy(end, "s"));
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
      while (p = strchr(p + 1, ' ') && p < n)
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

    if (sp->subtype == STRING_MALLOC) {
	str = sp->u.string;

	for (; *str; str++)
	    if (islower(*str))
		*str -= 'a' - 'A';
    } else {
	char *result;

	result = str = string_copy(sp->u.string, "upper_case");
	for (; str; str++)
	    if (isupper(*str))
		*str += 'a' - 'A';
	free_string_svalue(sp);
	sp->subtype = STRING_MALLOC;
	sp->u.string = result;
    }
}
#endif

#ifdef F_REPLACEABLE
void f_replaceable PROT((void)) {
    struct program *prog = sp->u.ob->prog;
    int i, num;
    struct function *functions;
    
    num = prog->p.i.num_functions;
    functions = prog->p.i.functions;
    
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
    struct object *ob;
    
    for (ob = obj_list; ob; ob = ob->next_all) {
	if (ob->flags & O_CLONE) continue;
	hdr_size += sizeof(struct program);
	prog_size += ob->prog->p.i.program_size;
	func_size += ob->prog->p.i.num_functions * sizeof(struct function);
	string_size += ob->prog->p.i.num_strings * sizeof(char *);
	var_size += ob->prog->p.i.num_variables * sizeof(struct variable);
	inherit_size += ob->prog->p.i.num_inherited * sizeof(struct inherit);
    }
    
    add_vmessage("header size: %i\n", hdr_size);
    add_vmessage("code size: %i\n", prog_size);
    add_vmessage("function size: %i\n", func_size);
    add_vmessage("string size: %i\n", string_size);
    add_vmessage("var size: %i\n", var_size);
    add_vmessage("inherit size: %i\n", inherit_size);
}
#endif
