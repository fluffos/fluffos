#define SUPPRESS_COMPILER_INLINES
#include "../lpc_incl.h"
#include "../comm.h"
#include "../file_incl.h"
#include "../file.h"
#include "../backend.h"
#include "../compiler.h"
#include "../main.h"
#include "../eoperators.h"
#include "../efun_protos.h"
#include "../simul_efun.h"
#include "../add_action.h"
#include "../port.h"
#include "../applies.h"
#define MAX_COLOUR_STRING 200

/* should be done in configure */
#ifdef WIN32
#define strcasecmp(X, Y) stricmp(X, Y)
#endif

#ifdef F_REAL_TIME
void
f_real_time (void)
{
	push_number(time(NULL));
}
#endif

#ifdef F_COMPRESSEDP
void f_compressedp (void)
{
	int i;

	i = sp->u.ob->interactive && sp->u.ob->interactive->compressed_stream;
	free_object(&sp->u.ob, "f_compressedp");
	put_number(i != 0);
}
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
	int nob;
#ifdef F_SET_HIDE
	int apply_valid_hide, display_hidden = 0;
#endif
	object_t *ob, **obtab;
	array_t *vec;

	nob = 0;
#ifdef F_SET_HIDE
	apply_valid_hide = 1;
#endif

	obtab = CALLOCATE(max_array_size, object_t *, TAG_TEMPORARY, "named_livings");

	for (i = 0; i < CFG_LIVING_HASH_SIZE; i++) {
		for (ob = hashed_living[i]; ob; ob = ob->next_hashed_living) {
			if (!(ob->flags & O_ENABLE_COMMANDS))
				continue;
#ifdef F_SET_HIDE
			if (ob->flags & O_HIDDEN) {
				if (apply_valid_hide) {
					apply_valid_hide = 0;
					display_hidden = valid_hide(current_object);
				}
				if (!display_hidden)
					continue;
			}
#endif
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
f_remove_shadow (void)
{
	object_t *ob;

	ob = current_object;
	if (st_num_arg) {
		ob = sp->u.ob;
		pop_stack();
	}
	if (ob == 0 || (ob->shadowing == 0 && ob->shadowed == 0))
		push_number(0);
	else {
		if (ob->shadowed)
			ob->shadowed->shadowing = ob->shadowing;
		if (ob->shadowing)
			ob->shadowing->shadowed = ob->shadowed;
		ob->shadowing = ob->shadowed = 0;
		push_number(1);
	}
}
#endif

/* This was originally written my Malic for Demon.  I rewrote parts of it
   when I added it (added function support, etc) -Beek */
#ifdef F_QUERY_NOTIFY_FAIL
void
f_query_notify_fail (void) {
	char *p;

	if (command_giver && command_giver->interactive) {
		if (command_giver->interactive->iflags & NOTIFY_FAIL_FUNC) {
			push_funp(command_giver->interactive->default_err_message.f);
			return;
		} else if ((p = command_giver->interactive->default_err_message.s)) {
			STACK_INC;
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
f_store_variable (void) {
	int idx;
	svalue_t *sv;
	unsigned short type;

	idx = find_global_variable(current_object->prog, (sp-1)->u.string, &type, 0);
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
f_fetch_variable (void) {
	int idx;
	svalue_t *sv;
	unsigned short type;

	idx = find_global_variable(current_object->prog, sp->u.string, &type, 0);
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
f_set_prompt (void) {
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

static void deep_copy_svalue (svalue_t *, svalue_t *);

static array_t *deep_copy_array ( array_t * arg ) {
	array_t *vec;
	int i;

	vec = allocate_empty_array(arg->size);
	for (i = 0; i < arg->size; i++)
		deep_copy_svalue(&arg->item[i], &vec->item[i]);

	return vec;
}

static array_t *deep_copy_class (array_t * arg) {
	array_t *vec;
	int i;

	vec = allocate_empty_class_by_size(arg->size);
	for (i = 0; i < arg->size; i++)
		deep_copy_svalue(&arg->item[i], &vec->item[i]);

	return vec;
}

static int doCopy ( mapping_t * map, mapping_node_t * elt, void *dest) {
	svalue_t *sv;

	sv = find_for_insert((mapping_t *)dest, &elt->values[0], 1);
	if (!sv) {
		mapping_too_large();
		return 1;
	}

	deep_copy_svalue(&elt->values[1], sv);
	return 0;
}

static mapping_t *deep_copy_mapping ( mapping_t * arg ) {
	mapping_t *map;

	map = allocate_mapping( 0 ); /* this should be fixed.  -Beek */
	mapTraverse( arg, doCopy, map); /* Not horridly efficient either */
	return map;
}

static void deep_copy_svalue (svalue_t * from, svalue_t * to) {
	switch (from->type) {
	case T_ARRAY:
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
	case T_CLASS:
		depth++;
		if (depth > MAX_SAVE_SVALUE_DEPTH) {
			depth = 0;
			error("Mappings, arrays and/or classes nested too deep (%d) for copy()\n",
					MAX_SAVE_SVALUE_DEPTH);
		}
		*to = *from;
		to->u.arr = deep_copy_class( from->u.arr );
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
#ifndef NO_BUFFER_TYPE
	case T_BUFFER:
		*to = *from;
		to->u.buf = allocate_buffer(from->u.buf->size);
		memcpy(to->u.buf->item, from->u.buf->item, from->u.buf->size);
		break;
#endif
	default:
		assign_svalue_no_free( to, from );
	}
}

void f_copy (void)
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
void f_functions (void) {
	int i, j, num, ind;
	array_t *vec, *subvec;
	function_t *funp;
	program_t *prog;
	int flag = (sp--)->u.number;
	unsigned short *types;
	char buf[256];
	char *end = EndOf(buf);
	program_t *progp = sp->u.ob->prog;
	int offset = (flag&2)?progp->last_inherited:0;

	num = (flag&2)?progp->num_functions_defined:progp->num_functions_defined + progp->last_inherited;

	if (progp->num_functions_defined &&
			progp->function_table[progp->num_functions_defined-1].funcname[0]
			                                                               == APPLY___INIT_SPECIAL_CHAR)
		num--;

	vec = allocate_empty_array(num);
	i = num;

	while (i--) {
		unsigned short low, high, mid;

		prog = sp->u.ob->prog;
		ind = i+offset;

		/* Walk up the inheritance tree to the real definition */
		if (prog->function_flags[ind] & FUNC_ALIAS) {
			ind = prog->function_flags[ind] & ~FUNC_ALIAS;
		}

		while (prog->function_flags[ind] & FUNC_INHERITED) {
			low = 0;
			high = prog->num_inherited -1;

			while (high > low) {
				mid = (low + high + 1) >> 1;
				if (prog->inherit[mid].function_index_offset > ind)
					high = mid -1;
				else low = mid;
			}
			ind -= prog->inherit[low].function_index_offset;
			prog = prog->inherit[low].prog;
		}

		ind -= prog->last_inherited;

		funp = prog->function_table + ind;

		if (flag&1) {
			if (prog->type_start && prog->type_start[ind] != INDEX_START_NONE)
				types = &prog->argument_types[prog->type_start[ind]];
			else
				types = 0;

			vec->item[i].type = T_ARRAY;
			subvec = vec->item[i].u.arr = allocate_empty_array(3 + funp->num_arg);

			subvec->item[0].type = T_STRING;
			subvec->item[0].subtype = STRING_SHARED;
			subvec->item[0].u.string = make_shared_string(funp->funcname);

			subvec->item[1].type = T_NUMBER;
			subvec->item[1].subtype = 0;
			subvec->item[1].u.number = funp->num_arg;

			get_type_name(buf, end, funp->type);
			subvec->item[2].type = T_STRING;
			subvec->item[2].subtype = STRING_SHARED;
			subvec->item[2].u.string = make_shared_string(buf);

			for (j = 0; j < funp->num_arg; j++) {
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
			vec->item[i].u.string = make_shared_string(funp->funcname);
		}
	}

	pop_stack();
	push_refed_array(vec);
}
#endif

/* Beek */
#ifdef F_VARIABLES
static void fv_recurse (array_t * arr, int * idx, program_t * prog, int type, int flag) {
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

void f_variables (void) {
	int idx = 0;
	array_t *arr;
	int flag = (sp--)->u.number;
	program_t *prog = sp->u.ob->prog;

	arr = allocate_empty_array(prog->num_variables_total);
	fv_recurse(arr, &idx, prog, 0, flag);

	pop_stack();
	push_refed_array(arr);
}
#endif

/* also Beek */
#ifdef F_HEART_BEATS
void f_heart_beats (void) {
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

static int at_end(int i, int imax, int z, int *lens) {
	if (z + 1 != lens[i])
		return 0;
	for (i++; i < imax; i++) {
		if (lens[i] > 0)
			return 0;
	}
	return 1;
}

void
f_terminal_colour (void)
{
	const char *instr, *cp, **parts;
	char *savestr, *deststr, *ncp;
	char curcolour[MAX_COLOUR_STRING];
	char colouratstartword[MAX_COLOUR_STRING];
	int curcolourlen;
	int colourstartlen=0;
	const char *resetstr = 0;
	char *resetstrname;
	int resetstrlen = 0;
	int num, i, j, k, col, start, space, *lens, maybe_at_end;
	int space_garbage = 0;
	mapping_node_t *elt, **mtab;
	int buflen, max_buflen, space_buflen;
	int wrap = 0;
	int indent = 0;
	int fillout = 0;
	char *rep;
	int repused;

	if (st_num_arg >= 3) {
		if (st_num_arg == 4)
			indent = (sp--)->u.number;
		wrap = (sp--)->u.number;
		if (wrap < 0) {
			wrap = -wrap;
			fillout = 1;
		}
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
			parts = (const char **)CALLOCATE(1, char *, TAG_TEMPORARY, "f_terminal_colour: parts");
			parts[0] = instr;
			savestr = 0;
		} else {
			pop_stack(); /* no delimiter in string, so return the original */
			return;
		}
	} else {
		/* here we have something to parse */
		char *newstr = (char *) cp; //must be result of the string_copy above
		parts = (const char **) CALLOCATE(NSTRSEGS, char *, TAG_TEMPORARY, "f_terminal_colour: parts");
		if (newstr - instr) {       /* starting seg, if not delimiter */
			num = 1;
			parts[0] = instr;
			*newstr = 0;
		} else
			num = 0;
		// Search through for the %^...%^ combinations
		while (newstr) {
			newstr += 2;
			instr = newstr;
			do {
				newstr = strchr(newstr,TC_FIRST_CHAR);
				if (newstr) {
					if (newstr[1] == TC_SECOND_CHAR)
						break;
					newstr++;
				}
			} while (newstr);
			// Check and make sure we have an end marker.
			if (newstr) {
				*newstr = 0;
				// %^ at start of the text
				if (newstr > instr) {
					if (num && num % NSTRSEGS == 0) {
						// Increase the size of the parts array.
						parts = (const char **) RESIZE(parts, num + NSTRSEGS, char *,
								TAG_TEMPORARY, "f_terminal_colour: parts realloc");
					}
					// Put it in at the current location in the parts array.
					parts[num++] = instr;
				}
			}
		}
		if (*instr) {   /* trailing seg, if not delimiter */
			if (num && num % NSTRSEGS == 0) {
				// Increase the size of the parts array.
				parts = (const char **) RESIZE(parts, num + NSTRSEGS, char *,
						TAG_TEMPORARY, "f_terminal_colour: parts realloc");
			}
			// Put it in at the current location in the parts array.
			parts[num++] = instr;
		}
	}

	if (num == 0) {
		/* string consists entirely of %^'s */
		FREE(parts);
		if (savestr)
			FREE_MSTR(savestr);
		pop_stack();
		free_string_svalue(sp);
		sp->type = T_STRING;
		sp->subtype = STRING_CONSTANT;
		sp->u.string = "";
		return;
	}

	/* Could keep track of the lens as we create parts, removing the need
       for a strlen() below */
	lens = CALLOCATE(num, int, TAG_TEMPORARY, "f_terminal_colour: lens");
	mtab = sp->u.map->table;

	// First setup some little things.
	curcolour[0] = 0;
	curcolourlen = 0;

	// Find the reset colour string.
	resetstrname = findstring("RESET");
	k = sp->u.map->table_size;
	if (resetstrname) {
		int tmp;
		static svalue_t str = {T_STRING, STRING_SHARED};
		str.u.string = resetstrname;
		tmp = MAP_SVAL_HASH(str);
		for (elt = mtab[tmp & k]; elt; elt = elt->next) {
			if ( elt->values->type == T_STRING &&
					(elt->values + 1)->type == T_STRING &&
					resetstrname == elt->values->u.string) {
				resetstr = (elt->values + 1)->u.string;
				resetstrlen = strlen((elt->values + 1)->u.string);
				break;
			}
		}
	}

	if(!resetstrlen) {
		//we really really need one, so just default to ansi reset
		resetstr = "\e[49;49m\e[0;10m";
		resetstrlen = 15;
		add_mapping_string(sp->u.map, "RESET", resetstr);
	}

	/* Do the the pointer replacement and calculate the lengths */
	col = 0;
	start = -1;
	space = 0;
	maybe_at_end = 0;
	buflen = max_buflen = space_buflen = 0;
	for (j = i = 0, k = sp->u.map->table_size; i < num; i++) {
		// Look it up in the mapping.
		repused = 0;
		copy_and_push_string(parts[i]);
		svalue_t *tmp = apply(APPLY_TERMINAL_COLOUR_REPLACE, current_object, 1, ORIGIN_EFUN);
		if(tmp && tmp->type == T_STRING){
			rep = alloca(SVALUE_STRLEN(tmp)+1);
			strcpy(rep, tmp->u.string);
			repused = 1;
		}

		if((repused && (cp = findstring(rep))) || (!repused && (cp = findstring(parts[i])))) {
			int tmp;
			static svalue_t str = {T_STRING, STRING_SHARED};
			str.u.string = cp;
			tmp = MAP_SVAL_HASH(str);
			for (elt = mtab[tmp & k]; elt; elt = elt->next) {
				if ( elt->values->type == T_STRING &&
						(elt->values + 1)->type == T_STRING &&
						cp == elt->values->u.string) {
					parts[i] = (elt->values + 1)->u.string;
					/* Negative indicates don't count for wrapping */
					lens[i] = SVALUE_STRLEN(elt->values + 1);
					if (wrap) lens[i] = -lens[i];
					// Do stuff for continueing colour codes.
					if (!strcmp(resetstr, parts[i])) {
						curcolour[0] = 0;
						curcolourlen =0;
					} else {
						if (curcolourlen + strlen((elt->values + 1)->u.string) < MAX_COLOUR_STRING - 1) {
							strcat(curcolour, (elt->values + 1)->u.string);
							curcolourlen += strlen((elt->values + 1)->u.string);
						}
					}
					break;
				}
			}
			if (!elt) {
				lens[i] = SHARED_STRLEN(cp);
			}
		} else {
			if(repused){
				parts[i] = rep;
				lens[i] = wrap?-SVALUE_STRLEN(tmp):SVALUE_STRLEN(tmp);
			}
			else
				lens[i] = strlen(parts[i]);
		}

		if (lens[i] <= 0) {
			if (j + -lens[i] > max_string_length) {
				lens[i] = -(-(lens[i]) - (j + -lens[i] - max_string_length));
			}
			j += -lens[i];
			buflen += -lens[i];
			continue;
		}

		if (maybe_at_end) {
			if (j + indent > max_string_length) {
				/* this string no longer counts, so we are still in
                   a maybe_at_end condition.  This means we will end
                   up truncating the rest of the fragments too, since
                   the indent will never fit. */
				lens[i] = 0;
			} else {
				j += indent;
				col += indent;
				maybe_at_end = 0;
			}
		}

		j += lens[i];
		if (j > max_string_length) {
			lens[i] -= j - max_string_length;
			j = max_string_length;
		}

		if (wrap) {
			int z;
			const char *p = parts[i];
			// This is where we figure out the size of the lines and
			// the final output string.  j is the size of the final output
			// string and max_buflen is the size of the line.
			for (z = 0; z < lens[i]; z++) {
				char c = p[z];
				buflen++;
				if (c == '\n') {
					if (fillout) {
						j += wrap - col;
					}
					col = 0;
					space = space_buflen = 0;
					start = -1;
					j += resetstrlen + curcolourlen;
					buflen += resetstrlen + curcolourlen;
					max_buflen = (buflen > max_buflen ? buflen : max_buflen);
					buflen = 0;
				} else {
					if (col > start || (c != ' ' && c != '\t'))
						col++;
					else {
						j--;
						buflen--;
					}

					if (col > start && c == '\t')
						col += (8 - ((col - 1) % 8));
					if (c == ' ' || c == '\t') {
						space = col;
						space_buflen = buflen;
						strncpy(colouratstartword, curcolour, MAX_COLOUR_STRING-1);
						colourstartlen = curcolourlen;
					}
					if (col == wrap+1) {
						if (space) {
							if (fillout) {
								j += wrap - space;
							}
							col -= space;
							space = 0;
							j += resetstrlen + colourstartlen;
							buflen += resetstrlen + colourstartlen;
							max_buflen = (buflen > max_buflen ? buflen : max_buflen);
							buflen -= space_buflen;
							space_buflen = 0;
						} else {
							j++;
							col = 1;
							j += resetstrlen + curcolourlen;
							buflen += resetstrlen + curcolourlen;
							max_buflen = (buflen > max_buflen ? buflen : max_buflen);
							buflen = 1;
						}
						start = indent;
					} else
						continue;
				}

				/* If we get here, we ended a line by wrapping */
				if (z + 1 != lens[i] || col) {
					j += indent;
					col += indent;
				} else
					maybe_at_end = 1;

				if (j > max_string_length) {
					lens[i] -= (j - max_string_length);
					j = max_string_length;
					if (lens[i] < z) {
						/* must have been ok or we wouldn't be here */
						lens[i] = z;
						break;
					}
				}
			}
		}
	}

	if (wrap && buflen > max_buflen)
		max_buflen = buflen;

	/* now we have the final string in parts and length in j.
       let's compose it, wrapping if necessary */
	ncp = deststr = new_string(j, "f_terminal_colour: deststr");
	if (wrap) {
		char *tmp = new_string(max_buflen, "f_terminal_colour: wrap");
		char *pt = tmp;

		col = 0;
		start = -1;
		space = 0;
		buflen = space_buflen = 0;
		curcolour[0] = 0;
		curcolourlen = 0;
		for (i = 0; i < num; i++) {
			int kind;
			const char *p = parts[i];
			if (lens[i] < 0) {
				memcpy(pt, p, -lens[i]);
				pt += -lens[i];
				buflen += -lens[i];
				space_garbage += -lens[i]; /* Number of chars due to ignored junk
                                              since last space */
				// Do stuff for continueing colour codes.
				if (!strcmp(p, resetstr)) {
					curcolour[0] = 0;
					curcolourlen = 0;
				} else {
					if (curcolourlen + strlen(p) < MAX_COLOUR_STRING -1) {
						strcat(curcolour, p);
						curcolourlen += strlen(p);
					}
				}
				continue;
			}
			for (k = 0; k < lens[i]; k++) {
				int n;
				int endpad = wrap-col;
				char c = p[k];
				*pt++ = c;
				buflen++;
				if (c == '\n') {
					endpad = wrap - col;
					col = 0;
					kind = 0;
					space = space_garbage = 0;
					start = -1;
					buflen = 0;
					strncpy(colouratstartword, curcolour, MAX_COLOUR_STRING-1);
					colourstartlen = curcolourlen;
				} else {
					if (col > start || (c != ' ' && c != '\t'))
						col++;
					else {
						pt--;
						buflen--;
					}

					if (col > start && c == '\t')
						col += (8 - ((col - 1) % 8));
					if (c == ' ' || c == '\t') {
						space = col;
						space_garbage = 0;
						space_buflen = buflen;
						strncpy(colouratstartword, curcolour, MAX_COLOUR_STRING-1);
						colourstartlen = curcolourlen;
					}
					if (col == wrap+1) {
						if (space) {
							endpad = wrap - space;
							col -= space;
							space = 0;
							kind = 1;
							buflen -= space_buflen;
							space_buflen = 0;
						} else {
							col = 1;
							kind = 2;
							buflen = 1;
							strncpy(colouratstartword, curcolour, MAX_COLOUR_STRING-1);
							colourstartlen = curcolourlen;
						}
						start = indent;
					} else
						continue;
				}
				/* If we get here, we ended a line */
				n = (pt - tmp) - buflen;
				memcpy(ncp, tmp, n);
				ncp += n;
				if (kind == 1 || kind == 0) {
					/* replace the space */
					//ncp[-1] = '\n';
					ncp--;
				}
				if (kind == 2) {
					/* need to insert a newline */
					//*ncp++ = '\n';
				}
				// Insert the follow on colour codes.
				memcpy(ncp, resetstr, resetstrlen);
				ncp += resetstrlen;
				if (fillout) {
					// Fill in the remaining bits with spaces.
					memset(ncp, ' ', endpad);
					ncp += endpad;
				}
				*ncp++ = '\n';
				memcpy(ncp, colouratstartword, colourstartlen);
				ncp += colourstartlen;
				// Back to the normal code again.
				memmove(tmp, tmp + n, buflen);
				pt = tmp + buflen;
				if (col || !at_end(i, num, k, lens)) {
					memset(ncp, ' ', indent);
					ncp += indent;
					col += indent;
				}
			}
		}
		memcpy(ncp, tmp, pt - tmp);
		ncp += pt - tmp;
		FREE_MSTR(tmp);
	} else {
		for (i = 0; i < num; i++) {
			memcpy(ncp, parts[i], lens[i]);
			ncp += lens[i];
		}
	}
	*ncp = 0;
	FREE(lens);
	FREE(parts);
	if (savestr)
		FREE_MSTR(savestr);
	/* now we have what we want */
	pop_stack();
#ifndef DEBUG
	if (ncp - deststr != j) {
		fatal("Length miscalculated in terminal_colour()\n    Expected: %i Was: %i\n    String: %s\n    Indent: %i Wrap: %i\n", j, ncp - deststr, sp->u.string, indent, wrap);
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

static char *pluralize (const char * str) {
	char *pre;
	const char *p, *rel, *end;
	char *of_buf;
	int of_len = 0, plen, slen;
	int sz;

	/* default rule */
	int found = 0;
	const char * suffix = "s";

	sz = strlen(str);
	if (!sz) return 0;

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
			pre = (char *)DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
			strncpy(pre, str + 2, plen);
		} else if (sz > 2 && str[1] == 'n' && str[2] == ' ') {
			plen = sz - 3;
			pre = (char *)DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
			strncpy(pre, str + 3, plen);
		} else {
			plen = sz;
			pre = (char *)DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
			strncpy(pre, str, plen);
		}
	} else {
		plen = sz;
		pre = (char *)DXALLOC(plen + 1, TAG_TEMPORARY, "pluralize: pre");
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
			break;
		}
		if (!strcasecmp(rel + 1, "onus")) {
			found = PLURAL_SUFFIX;
			suffix = "es";
		}
		break;
	case 'C':
	case 'c':
		if (!strcasecmp(rel + 1, "hild")) {
			found = PLURAL_SUFFIX;
			suffix = "ren";
			break;
		}
		if (!strcasecmp(rel + 1, "liff")) {
			found = PLURAL_SUFFIX;
			suffix = "s";
		}
		break;
	case 'D':
	case 'd':
		if (!strcasecmp(rel + 1, "atum")) {
			found = PLURAL_CHOP + 2;
			suffix = "a";
			break;
		}
		if (!strcasecmp(rel + 1, "ie")) {
			found = PLURAL_CHOP + 1;
			suffix = "ce";
			break;
		}
		if (!strcasecmp(rel + 1, "eer")) {
			found = PLURAL_SAME;
			break;
		}
		if (!strcasecmp(rel + 1, "o")) {
			found = PLURAL_SUFFIX;
			suffix = "es";
			break;
		}
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
		if (!strcasecmp(rel + 1, "orum")) {
			found = PLURAL_CHOP + 2;
			suffix = "a";
			break;
		}
		if (!strcasecmp(rel + 1, "ife"))
			found = PLURAL_SUFFIX;
		break;
	case 'G':
	case 'g':
		if (!strcasecmp(rel + 1, "lasses")) {
			found = PLURAL_SAME;
			break;
		}
		if (!strcasecmp(rel + 1, "oose")) {
			found = PLURAL_CHOP + 4;
			suffix = "eese";
			break;
		}
		if (!strcasecmp(rel + 1, "o")) {
			found = PLURAL_SUFFIX;
			suffix = "es";
			break;
		}
		if (!strcasecmp(rel + 1, "um")) {
			found = PLURAL_SUFFIX;
			break;
		}
		if (!strcasecmp(rel + 1, "iraffe")) {
			found = PLURAL_SUFFIX;
			suffix = "s";
		}
		break;
	case 'H':
	case 'h':
		if (!strcasecmp(rel + 1, "uman")){
			found = PLURAL_SUFFIX;
			break;
		}
		if (!strcasecmp(rel + 1, "ave")) {
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
			break;
		}
		if (!strcasecmp(rel + 1, "otus")) {
			found = PLURAL_SUFFIX;
			break;
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
			break;
		}
		if (!strcasecmp(rel + 1, "ech")) {
		    found =  PLURAL_SUFFIX;
		    suffix = "s";
		}
		break;
	case 'O':
	case 'o':
		if (!strcasecmp(rel + 1, "x")) {
			found = PLURAL_SUFFIX;
			suffix = "en";
		}
		break;
	case 'P':
	case 'p':
		if (!strcasecmp(rel + 1, "ants"))
			found = PLURAL_SAME;
		break;
	case 'Q':
	case 'q':
		if (!strcasecmp(rel + 1, "uaff"))
			found = PLURAL_SUFFIX;
		break;
	case 'R':
	case 'r':
		if (!strcasecmp(rel + 1, "emains")) {
			found = PLURAL_SAME;
			break;
		}
		if (!strcasecmp(rel + 1, "oof"))
			found = PLURAL_SUFFIX;
		break;
	case 'S':
	case 's':
		if (!strcasecmp(rel + 1, "niff")) {
			found = PLURAL_SUFFIX;
			break;
		}
		if (!strcasecmp(rel + 1, "heep")) {
			found = PLURAL_SAME;
			break;
		}
		if (!strcasecmp(rel + 1, "phinx")) {
			found = PLURAL_CHOP + 1;
			suffix = "ges";
			break;
		}
		if (!strcasecmp(rel + 1, "taff")) {
			found = PLURAL_CHOP + 2;
			suffix = "ves";
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
			break;
		}
		if (!strcasecmp(rel + 1, "alisman")) {
			found = PLURAL_SUFFIX;
			suffix = "s";
		}
		break;
	case 'V':
	case 'v':
		if (!strcasecmp(rel + 1, "ax")) {
			found = PLURAL_SUFFIX;
			suffix = "en";
			break;
		}
		if (!strcasecmp(rel + 1, "irus")) {
			found = PLURAL_SUFFIX;
			suffix = "es";
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
	 * *us -> *i (cactus -> cacti)
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
	if (!found && (end != pre))
		switch (end[-1]) {
		case 'E': case 'e':
			if ((end-pre) > 1 && (end[-2] == 'f' || end[-2] == 'F')) {
				found = PLURAL_CHOP + 2;
				suffix = "ves";
			}
			break;
		case 'F': case 'f':
			if ((end-pre) > 1 && (end[-2] == 'e' || end[-2] == 'E'))
				break;
			found = PLURAL_CHOP + 1;
			if ((end-pre) > 1 && (end[-2] == 'f' || end[-2] == 'F'))
				found++;
			suffix = "ves";
			break;
		case 'H': case 'h':
			if ((end-pre) > 1 && (end[-2] == 'c' || end[-2]=='s'))
				suffix = "es";
			break;
#if 0
			/*
			 * This rule is causing more problems than not.  As such, I'm removing
			 * it in favour of adding exceptions for words above that should use
			 * this rule.  I'm aware that this rule is proper for Latin derived
			 * English words, however its use has fallen out of common speech and
			 * writing for the majority of cases.  Currently known common exceptions
			 * are forum (fora) and datum (data).
			 * -- Marius, 23-Jun-2000
			 */
		case 'M': case 'm':
			if ((end-pre) > 1 && end[-2] == 'u') {
				found = PLURAL_CHOP + 2;
				suffix = "a";
			}
			break;
#endif
		case 'N': case 'n':
			if ((end-pre) > 2 && end[-2] == 'a' && end[-3] == 'm') {
				found = PLURAL_CHOP + 3;
				suffix = "men";
			}
			break;
		case 'O': case 'o':
			if ((end-pre) > 1 && end[-2] != 'o')
				suffix = "es";
			break;
		case 'S': case 's':
			if ((end-pre) > 1 && end[-2] == 'i') {
				found = PLURAL_CHOP + 2;
				suffix = "es";
				break;
			}
			if ((end-pre) > 1 && end[-2] == 'u') {
				found = PLURAL_CHOP + 2;
				suffix = "i";
				break;
			}
			if ((end-pre) > 1 && (end[-2] == 'a' || end[-2] == 'e' || end[-2] == 'o'))
				suffix = "ses";
			else
				suffix = "es";
			break;
		case 'X': case 'x':
			suffix = "es";
			break;
		case 'Y': case 'y':
			if ((end-pre) > 1 && end[-2] != 'a' && end[-2] != 'e' && end[-2] != 'i'
					&& end[-2] != 'o' && end[-2] != 'u') {
				found = PLURAL_CHOP + 1;
				suffix = "ies";
			}
			break;
		case 'Z': case 'z':
			if ((end-pre) > 1 && (end[-2] == 'a' || end[-2] == 'e' || end[-2] == 'o'
					|| end[-2] == 'i' || end[-2] == 'u'))
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

	char *news = new_string(sz, "pluralize");
	news[sz] = 0;

	strncpy(news, pre, plen);
	if (slen)
		strncpy(news + plen, suffix, slen);
	if (of_len) {
		strcpy(news + plen + slen, of_buf);
		FREE(of_buf);
	}

	FREE(pre);
	return news;
} /* end of pluralize() */

void
f_pluralize (void)
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
static int file_length (const char * file)
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
		while ((newp = (char *)memchr(p + 1, '\n', num))) {
			num -= (newp - p);
			p = newp;
			ret++;
		}
	} while (!feof(f));

	fclose(f);
	return ret;
} /* end of file_length() */

void
f_file_length (void)
{
	int l;

	l = file_length(sp->u.string);
	pop_stack();
	push_number(l);
}
#endif

#ifdef F_UPPER_CASE
void
f_upper_case (void)
{
	const char *str;

	str = sp->u.string;
	/* find first upper case letter, if any */
	for (; *str; str++) {
		if (uislower(*str)) {
			char *newstr;
			int l = str - sp->u.string;
			unlink_string_svalue(sp);
			newstr = (char *) sp->u.string + l;
			*newstr = toupper((unsigned char)*newstr);
			for (newstr++; *newstr; newstr++) {
				if (uislower((unsigned char)*newstr))
					*newstr = toupper((unsigned char)*newstr);
			}
			return;
		}
	}
}
#endif

#ifdef F_REPLACEABLE
void f_replaceable (void) {
	object_t *obj;
	program_t *prog;
	int i, j, num, numignore, replaceable;
	char **ignore;

	if (st_num_arg == 2) {
		numignore = sp->u.arr->size;
		if (numignore)
			ignore = CALLOCATE(numignore + 2, char *, TAG_TEMPORARY, "replaceable");
		else
			ignore = 0;
		ignore[0] = findstring(APPLY_CREATE);
		ignore[1] = findstring(APPLY___INIT);
		for (i = 0; i < numignore; i++) {
			if (sp->u.arr->item[i].type == T_STRING)
				ignore[i + 2] = findstring(sp->u.arr->item[i].u.string);
			else
				ignore[i + 2] = 0;
		}
		numignore += 2;
		obj = (sp-1)->u.ob;
	} else {
		numignore = 2;
		ignore = CALLOCATE(2, char *, TAG_TEMPORARY, "replaceable");
		ignore[0] = findstring(APPLY_CREATE);
		ignore[1] = findstring(APPLY___INIT);
		obj = sp->u.ob;
	}

	prog = obj->prog;
	num = prog->num_functions_defined + prog->last_inherited;

	for (i = 0; i < num; i++) {
		if (prog->function_flags[i] & (FUNC_INHERITED | FUNC_NO_CODE)) continue;
		for (j = 0; j < numignore; j++)
			if (ignore[j] == find_func_entry(prog, i)->funcname)
				break;
		if (j == numignore)
			break;
	}

	replaceable = (i == num);
	if (obj == simul_efun_ob || prog->func_ref)
		replaceable = 0;

	if (st_num_arg == 2)
		free_array((sp--)->u.arr);
	FREE(ignore);
	free_svalue(sp, "f_replaceable");
	put_number(replaceable);
}
#endif

#ifdef F_PROGRAM_INFO
void f_program_info (void) {
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
	mapping_t *m;
	program_t *prog;
	int i, n;

	if (st_num_arg == 1) {
		ob = sp->u.ob;
		prog = ob->prog;
		if (!(ob->flags & O_CLONE)) {
			hdr_size += sizeof(program_t);
			prog_size += prog->program_size;

			/* function flags */
			func_size += (prog->last_inherited +
					prog->num_functions_defined) *sizeof(unsigned short);

			/* definitions */
			func_size += prog->num_functions_defined *
					sizeof(function_t);

			string_size += prog->num_strings * sizeof(char *);
			var_size += prog->num_variables_defined * (sizeof(char *) + sizeof(unsigned short));
			inherit_size += prog->num_inherited * sizeof(inherit_t);
			if (prog->num_classes)
				class_size += prog->num_classes * sizeof(class_def_t) + (prog->classes[prog->num_classes - 1].index + prog->classes[prog->num_classes - 1].size) * sizeof(class_member_entry_t);
			type_size += prog->num_functions_defined * sizeof(short);
			n = 0;
			if (prog->type_start) {
				unsigned short *ts = prog->type_start;
				int nfd = prog->num_functions_defined;

				for (i = 0; i < nfd; i++) {
					if (ts[i] == INDEX_START_NONE)
						continue;
					n += prog->function_table[i].num_arg;
				}
			}
			type_size += n * sizeof(short);
			total_size += prog->total_size;
		}
		pop_stack();
	} else {
		for (ob = obj_list; ob; ob = ob->next_all) {
			if (ob->flags & O_CLONE)
				continue;
			prog = ob->prog;
			hdr_size += sizeof(program_t);
			prog_size += prog->program_size;

			/* function flags */
			func_size += (prog->last_inherited +
					prog->num_functions_defined) << 1;

			/* definitions */
			func_size += prog->num_functions_defined *
					sizeof(function_t);


			string_size += prog->num_strings * sizeof(char *);
			var_size += prog->num_variables_defined * (sizeof(char *) + sizeof(unsigned short));
			inherit_size += prog->num_inherited * sizeof(inherit_t);
			if (prog->num_classes)
				class_size += prog->num_classes * sizeof(class_def_t) + (prog->classes[prog->num_classes - 1].index + prog->classes[prog->num_classes - 1].size) * sizeof(class_member_entry_t);
			type_size += prog->num_functions_defined * sizeof(short);
			n = 0;
			if (prog->type_start) {
				unsigned short *ts = prog->type_start;
				int nfd = prog->num_functions_defined;

				for (i = 0; i < nfd; i++) {
					if (ts[i] == INDEX_START_NONE)
						continue;
					n += prog->function_table[i].num_arg;
				}
			}
			type_size += n * sizeof(short);
			total_size += prog->total_size;
		}
	}

	m = allocate_mapping(0);
	add_mapping_pair(m, "header size", hdr_size);
	add_mapping_pair(m, "code size", prog_size);
	add_mapping_pair(m, "function size", func_size);
	add_mapping_pair(m, "string size", string_size);
	add_mapping_pair(m, "var size", var_size);
	add_mapping_pair(m, "class size", class_size);
	add_mapping_pair(m, "inherit size", inherit_size);
	add_mapping_pair(m, "saved type size", type_size);

	add_mapping_pair(m, "total size", total_size);

	push_refed_mapping(m);
}
#endif

/* Magician - 08May95
 * int remove_interactive(object ob)
 * If the object isn't destructed and is interactive, then remove it's
 * interactivity and disconnect it.  (useful for exec()ing to an already
 * interactive object, ie, Linkdead reconnection)
 */

#ifdef F_REMOVE_INTERACTIVE
void f_remove_interactive (void) {
	if( (sp->u.ob->flags & O_DESTRUCTED) || !(sp->u.ob->interactive) ) {
		free_object(&sp->u.ob, "f_remove_interactive");
		*sp = const0;
	} else {
		remove_interactive(sp->u.ob, 0);
		/* It may have been dested */
		if (sp->type == T_OBJECT)
			free_object(&sp->u.ob, "f_remove_interactive");
		*sp = const1;
	}
}
#endif

/* Zakk - August 23 1995
 * return the port number the interactive object used to connect to the
 * mud.
 */
#ifdef F_QUERY_IP_PORT
static int query_ip_port (object_t * ob)
{
	if (!ob || ob->interactive == 0)
		return 0;
	return ob->interactive->local_port;
}

void
f_query_ip_port (void)
{
	int tmp;

	if (st_num_arg) {
		tmp = query_ip_port(sp->u.ob);
		free_object(&sp->u.ob, "f_query_ip_port");
	} else {
		tmp = query_ip_port(command_giver);
		STACK_INC;
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

char *set_timezone (const char * timezone)
{
	static char put_tz[80];
	char *old_tz;

	old_tz = getenv("TZ");
	sprintf(put_tz, "TZ=%s", timezone);
	putenv(put_tz);
	tzset();
	return old_tz;
}

void reset_timezone (const char *old_tz)
{
	static char put_tz[80];
	if(old_tz){
		sprintf(put_tz, "TZ=%s", old_tz);
		putenv(put_tz);
	}else
#ifndef MINGW
		unsetenv("TZ");
#else
	putenv("TZ=");
#endif

	tzset ();
}

void f_zonetime (void)
{
	const char *timezone, *old_tz;
	char *retv;
	long time_val;
	int len;

	time_val = sp->u.number;
	pop_stack();
	timezone = sp->u.string;
	pop_stack();

	old_tz = set_timezone(timezone);
	retv = ctime((time_t *)&time_val);
	len = strlen(retv);
	retv[len-1] = '\0';
	reset_timezone(old_tz);
	push_malloced_string (string_copy(retv, "zonetime"));

}
#endif

#ifdef F_IS_DAYLIGHT_SAVINGS_TIME
void f_is_daylight_savings_time (void)
{
	struct tm *t;
	long time_to_check;
	const char *timezone;
	char *old_tz;

	time_to_check = sp->u.number;
	pop_stack();
	timezone = sp->u.string;
	pop_stack();

	old_tz = set_timezone(timezone);

	t = localtime((time_t *)&time_to_check);

	push_number((t->tm_isdst) > 0);

	reset_timezone(old_tz);
}
#endif

#ifdef F_DEBUG_MESSAGE
void f_debug_message (void) {
	debug_message("%s\n", sp->u.string);
	free_string_svalue(sp--);
}
#endif

#ifdef F_FUNCTION_OWNER
void f_function_owner (void) {
	object_t *owner = sp->u.fp->hdr.owner;

	free_funp(sp->u.fp);
	put_unrefed_object(owner, "f_function_owner");
}
#endif

#ifdef F_REPEAT_STRING
void f_repeat_string (void) {
	const char *str;
	int repeat, len, newlen;
	char *ret, *p;
	int i;

	repeat = (sp--)->u.number;
	if (repeat > 0) {
		str = sp->u.string;
		len = SVALUE_STRLEN(sp);
		if ((newlen = len * repeat) > max_string_length){
			repeat = max_string_length / len;
			newlen = len * repeat;
		}
	}
	if (repeat <= 0) {
		free_string_svalue(sp);
		sp->type = T_STRING;
		sp->subtype = STRING_CONSTANT;
		sp->u.string = "";
	} else if (repeat != 1) {
		p = ret = new_string(newlen, "f_repeat_string");
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
static int memory_share (svalue_t *);

static int node_share (mapping_t * m, mapping_node_t * elt, void * tp) {
	int *t = (int *)tp;

	*t += sizeof(mapping_node_t) - 2*sizeof(svalue_t);
	*t += memory_share(&elt->values[0]);
	*t += memory_share(&elt->values[1]);

	return 0;
}

static int memory_share (svalue_t * sv) {
	int i, total = sizeof(svalue_t);
	int subtotal;
	static int calldepth = 0;

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
			if (1+calldepth > 100)
				return 0;
			calldepth++;
			/* first svalue is stored inside the array struct, so sizeof(array_t)
			 * includes one svalue.
			 */
			 subtotal = sizeof(array_t) - sizeof(svalue_t);
			 for (i = 0; i < sv->u.arr->size; i++)
				 subtotal += memory_share(&sv->u.arr->item[i]);
			 calldepth--;
			 return total + subtotal/sv->u.arr->ref;
		case T_MAPPING:
			if (1+calldepth > 100)
				return 0;
			calldepth++;
			subtotal = sizeof(mapping_t);
			mapTraverse(sv->u.map, node_share, &subtotal);
			calldepth--;
			return total + subtotal/sv->u.map->ref;
		case T_FUNCTION:
		{
			svalue_t tmp;
			tmp.type = T_ARRAY;
			tmp.u.arr = sv->u.fp->hdr.args;

			if (1+calldepth > 100)
				return 0;
			calldepth++;
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
			calldepth--;
			return total + subtotal/sv->u.fp->hdr.ref;
		}
#ifndef NO_BUFFER_TYPE
		case T_BUFFER:
			/* first byte is stored inside the buffer struct */
			return total + (sizeof(buffer_t) + sv->u.buf->size - 1)/sv->u.buf->ref;
#endif
	}
	return total;
}


/*
 * The returned mapping is:
 *
 * map["program name"]["variable name"] = memory usage
 */
#ifdef F_MEMORY_SUMMARY
static void fms_recurse (mapping_t * map, object_t * ob,
		int * idx, program_t * prog) {
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

void f_memory_summary (void) {
	mapping_t *result = allocate_mapping(8);
	object_t *ob;
	int idx;
	svalue_t sv;

	sv.type = T_STRING;
	sv.subtype = STRING_SHARED;

	for (ob = obj_list; ob; ob = ob->next_all) {
		svalue_t *entry;

		sv.u.string = ob->prog->filename;
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

/* Marius */
#ifdef F_QUERY_REPLACED_PROGRAM
void f_query_replaced_program (void)
{
	char *res = 0;

	if (st_num_arg)
	{
		if (sp->u.ob->replaced_program)
			res = add_slash(sp->u.ob->replaced_program);
		free_object(&sp->u.ob, "f_query_replaced_program");
	}
	else
	{
		if (current_object->replaced_program)
			res = add_slash(sp->u.ob->replaced_program);
		STACK_INC;
	}

	if (res) {
		put_malloced_string(res);
	} else {
		put_number(0);
	}
}
#endif

/* Skullslayer@Realms of the Dragon */
#ifdef F_NETWORK_STATS
void f_network_stats (void)
{
	mapping_t *m;
	int i, ports = 0;

	for (i = 0;  i < 5;  i++)
		if (external_port[i].port)
			ports += 4;

#ifndef PACKAGE_SOCKETS
	m = allocate_mapping(ports + 4);
#else
	m = allocate_mapping(ports + 8);
#endif

	add_mapping_pair(m, "incoming packets total", inet_in_packets);
	add_mapping_pair(m, "incoming volume total", inet_in_volume);
	add_mapping_pair(m, "outgoing packets total", inet_out_packets);
	add_mapping_pair(m, "outgoing volume total", inet_out_volume);

#ifdef PACKAGE_SOCKETS
	add_mapping_pair(m, "incoming packets sockets", inet_socket_in_packets);
	add_mapping_pair(m, "incoming volume sockets", inet_socket_in_volume);
	add_mapping_pair(m, "outgoing packets sockets", inet_socket_out_packets);
	add_mapping_pair(m, "outgoing volume sockets", inet_socket_out_volume);
#endif

	if (ports) {
		for (i = 0;  i < 5;  i++) {
			if (external_port[i].port) {
				char buf[40];

				sprintf(buf, "incoming packets port %d", external_port[i].port);
				add_mapping_pair(m, buf, external_port[i].in_packets);
				sprintf(buf, "incoming volume port %d", external_port[i].port);
				add_mapping_pair(m, buf, external_port[i].in_volume);
				sprintf(buf, "outgoing packets port %d", external_port[i].port);
				add_mapping_pair(m, buf, external_port[i].out_packets);
				sprintf(buf, "outgoing volume port %d", external_port[i].port);
				add_mapping_pair(m, buf, external_port[i].out_volume);
			}
		}
	}

	push_refed_mapping(m);
}
#endif

#ifdef F_EVENT


/* EVENTS!
 * Okay. This is pretty simple.
 * Calls the function "event_"+event_name in the object specified.
 * If the object is an array, calls it in each.
 * If the object is a room, call it on its inventory.
 *  [Incorrect: actually calls it on all_inventory() of any object but
 *   only if addressed uniquely]
 * Passes all the parameters too.
 */

#define EVENT_PREFIX "event_"

void event(svalue_t * event_ob, const char * event_fun, int numparam,
		svalue_t * event_param) {

	object_t *ob, *origin;
	char *name;
	int i;

	origin = current_object;

	name = new_string(strlen (event_fun) + strlen (EVENT_PREFIX) + 1,
			"newmoon.c: au_event");
	push_malloced_string(name);

	strcpy(name, EVENT_PREFIX);
	strcat(name, event_fun);

	if (event_ob->type == T_ARRAY) {
		int ind;

		for (ind = 0; ind < event_ob->u.arr->size; ind++) {
			if (event_ob->u.arr->item[ind].type != T_OBJECT
					|| event_ob->u.arr->item[ind].u.ob->flags & O_DESTRUCTED)
				continue;

			push_object(origin);
			for (i = 0; i < numparam; i++)
				push_svalue (event_param + i);

			apply(name, event_ob->u.arr->item[ind].u.ob, numparam + 1,
					ORIGIN_EFUN);
		}
	} else if (event_ob->type == T_OBJECT) {
		int count = 0;
		/* First we call the event on the object itself */

		push_object(origin);
		for (i = 0; i < numparam; i++)
			push_svalue (event_param + i);

		apply(name, event_ob->u.ob, numparam + 1, ORIGIN_EFUN);

		/* And then call it on it's inventory..., if it's still around! */
		if (event_ob && event_ob->u.ob && !(event_ob->u.ob->flags
				& O_DESTRUCTED))
			for (ob = event_ob->u.ob->contains; ob; ob = ob->next_inv) {
				if (ob == origin)
					continue;

				if (ob->flags & O_DESTRUCTED)
					continue;
				push_object(ob);
				count++;
			}
		while (count--) {
			ob = sp->u.ob;
			pop_stack();
			if (!ob || ob->flags & O_DESTRUCTED)
				continue;
			else {
				push_object(origin);
				for (i = 0; i < numparam; i++)
					push_svalue (event_param + i);

				apply(name, ob, numparam + 1, ORIGIN_EFUN);
			}
		}
	}
	sp--;
	FREE_MSTR (name);
}

void f_event (void){

	int num;

	num = st_num_arg;

	event ((sp - num + 1), (sp - num + 2)->u.string, num - 2, (sp - num + 3));

	pop_n_elems(num);
}
#endif


#ifdef F_QUERY_NUM
void number_as_string (char * buf, long n){
	const char *low[] =  { "ten", "eleven", "twelve", "thirteen",
			"fourteen", "fifteen", "sixteen", "seventeen",
			"eighteen", "nineteen" };
	const char *hi[] =  { "", "", "twenty", "thirty", "forty", "fifty", "sixty",
			"seventy", "eighty", "ninety"};
	const char *single[] =  { "", "one", "two", "three", "four", "five", "six",
			"seven", "eight", "nine"};
	if(!n){
		strcat(buf, "zero");
		return;
	}

	if(n<20 && n>9){
		strcat(buf, low[n-10]);
		return;
	}

	strcat(buf,hi[n/10]);

	if ((n>20) && (n%10))
		strcat(buf, "-");
	n %= 10;

	strcat(buf, single[n]);
}

void f_query_num (void){
	char ret[100];
	int i;
	long n, limit;
	int changed = 0;
	char *res;

	ret[0] = 0;
	limit = sp->u.number;
	pop_stack();
	n = sp->u.number;
	//  pop_stack();

	if ((limit && n>limit) || (n<0) || (n>99999)) {
		strcpy(ret,  "many"); /* this is a little pointless ... */
		goto q_n_end;
	}

	if ((i = n/1000)) {
		n = n%1000;
		if (!n){
			number_as_string(ret, i);
			strcat(ret, " thousand");
			goto q_n_end;
		}

		number_as_string(ret, i);
		strcat(ret, " thousand");
		changed = 1;
	}

	if ((i = n/100)) {
		n = n%100;
		if (changed) {
			if (!n){
				strcat(ret, " and ");
				number_as_string(ret, i);
				strcat(ret, " hundred");
				goto q_n_end;
			}
			strcat(ret, ", ");
			number_as_string(ret, i);
			strcat(ret, " hundred");
		} else {
			if (!n){
				number_as_string(ret, i);
				strcat(ret, " hundred");
				goto q_n_end;
			}
			number_as_string(ret, i);
			strcat(ret, " hundred");
			changed = 1;
		}
	}
	if (changed)
		strcat(ret, " and ");

	number_as_string(ret, n);
	q_n_end:
	n = strlen(ret);
	res = new_string(n, "query_num");
	strcpy(res, ret);
	put_malloced_string(res);
}

#endif

#ifdef F_BASE_NAME
void f_base_name (void) {
	char *name, *tmp;
	int i;

	if( sp->type == T_OBJECT ) {
		if( sp->u.ob->flags & O_DESTRUCTED ) {
			free_object( &sp->u.ob, "f_base_name");
			*sp = const0;
			return;
		}
		name = (char *)add_slash(sp->u.ob->obname);
	} else {
		name = string_copy( sp->u.string, "f_base_name: name");
	}

	pop_stack();

	if( ( tmp = strchr( name, '#') ) != NULL ) {
		char *ret;
		i = tmp - name;
		ret = new_string( i, "f_base_name: ret");
		strncpy( ret, name, i );
		ret[i] = 0;
		FREE_MSTR(name);

		push_malloced_string(ret);
	} else {
		push_malloced_string(name);
	}

} /* f_base_name() */
#endif

#ifdef F_GET_GARBAGE
int garbage_check (object_t * ob, void * data){
	return (ob->ref == 1) && (ob->flags & O_CLONE) &&
			!(ob->super
#ifndef NO_SHADOWS
					|| ob->shadowing
#endif
			);
}

void f_get_garbage (void){
	int count, i;
	object_t **obs;
	array_t *ret;
	get_objects(&obs, &count, garbage_check, 0);

	if (count > max_array_size)
		count = max_array_size;
	ret = allocate_empty_array(count);
	for (i = 0;  i < count;  i++) {
		ret->item[i].type = T_OBJECT;
		ret->item[i].u.ob = obs[i];
		add_ref(obs[i], "f_get_garbage");
	}

	pop_n_elems(1);
	push_refed_array(ret);
}
#endif

#ifdef F_NUM_CLASSES

void
f_num_classes() {
	int i = sp->u.ob->prog->num_classes;
	pop_stack();
	push_number( i );
}

#endif

#ifdef F_ASSEMBLE_CLASS
void
f_assemble_class() {
	array_t *arr = copy_array( sp->u.arr );
	pop_stack();
	push_refed_array(arr);
	sp->type = T_CLASS;
}

#endif

#ifdef F_DISASSEMBLE_CLASS
void
f_disassemble_class() {
	array_t *arr;
	if( sp->type != T_CLASS )
		error( "Argument to disassemble_class() not a class.\n" );
	arr = copy_array( sp->u.arr );
	pop_stack();
	push_refed_array(arr);
}
#endif

#ifdef F_FETCH_CLASS_MEMBER

void f_fetch_class_member() {
	int pos = sp->u.number;
	array_t *arr;

	pos = sp->u.number;
	pop_stack();

	if( sp->type != T_CLASS )
		error( "Argument to fetch_class_member() not a class.\n" );

	arr = sp->u.arr;

	if( pos < 0 || pos >= arr->size )
		error( "Class index out of bounds.\n" );

	assign_svalue_no_free( sp, &arr->item[pos] );
	free_array( arr );
}
#endif

#ifdef F_STORE_CLASS_MEMBER

void f_store_class_member() {
	int pos = ( sp - 1 )->u.number;
	array_t *arr;

	if( ( sp - 2 )->type != T_CLASS )
		error( "Argument to store_class_member() not a class.\n" );

	arr = ( sp - 2 )->u.arr;

	if( pos < 0 || pos >= arr->size )
		error( "Class index out of bounds.\n" );

	assign_svalue(&arr->item[pos], sp);

	pop_2_elems();
}
#endif


#ifdef F_ELEMENT_OF
void f_element_of() {
	array_t *arr = sp->u.arr;
	if(!arr->size) {
		error("Can't take element from empty array.\n");
	}
	assign_svalue_no_free(sp, &arr->item[random_number(arr->size)]);
	free_array(arr);
}
#endif
#ifdef F_SHUFFLE

/* shuffle efun, based on LPC shuffle simul efun.
 * conversion by Taffyd.
 */

void shuffle(array_t * args) {
	int i, j;
	svalue_t temp;

	/* Hrm, if we have less than two elements, then the order isn't
	 * going to change! Let's just leave the old array on the stack.
	 */
	if ( args->size < 2 ) {
		return;
	}

	for ( i = 1; i < args->size; i++ ) {
		j = random_number( i + 1 );

		if ( i == j ) {
			continue;
		}

		temp = args->item[i];
		args->item[i] = args->item[j];
		args->item[j] = temp;
	}

	/* Well, that's it. We don't need to push or anything. */
}

void
f_shuffle()
{
	svalue_t *sv = sp - st_num_arg + 1;

	if (sv->type == T_ARRAY && sv->u.arr) {
		shuffle(sv->u.arr);
	}
	else {
		push_refed_array(&the_null_array);
	}
}
#endif

#ifdef F_MAX

void
f_max() {
	svalue_t *sarr = sp - 1;
	array_t *arr = sarr->u.arr;
	int max_index = 0;
	int i;

	if( !arr->size ) {
		error( "Can't find max of an empty array.\n" );
	}

	if( arr->item->type != T_NUMBER && arr->item->type != T_REAL &&
			arr->item->type != T_STRING ) {
		error( "Array must consist of ints, floats or strings.\n" );
	}

	for( i = 1; i < arr->size; i++ ) {
		// Check the type of this element.
		switch( arr->item[i].type ) {
		case T_NUMBER:
			switch( arr->item[max_index].type ) {
			case T_NUMBER:
				if( arr->item[i].u.number > arr->item[max_index].u.number )
					max_index = i;
				break;
			case T_REAL:
				if( arr->item[i].u.number > arr->item[max_index].u.real )
					max_index = i;
				break;
			default:
				error( "Inhomogeneous array.\n" );
			}
			break;
			case T_REAL:
				switch( arr->item[max_index].type ) {
				case T_NUMBER:
					if( arr->item[i].u.real > arr->item[max_index].u.number )
						max_index = i;
					break;
				case T_REAL:
					if( arr->item[i].u.real > arr->item[max_index].u.real )
						max_index = i;
					break;
				default:
					error( "Inhomogeneous array.\n" );
				}
				break;
				case T_STRING:
					if( arr->item[max_index].type != T_STRING ) {
						error( "Inhomogeneous array.\n" );
					}
					if( strcmp( arr->item[i].u.string,
							arr->item[max_index].u.string ) > 0 )
						max_index = i;
					break;
				default:
					error( "Array must consist of ints, floats or strings.\n" );
		}
	}

	if( sp->u.number != 0 ) {
		pop_2_elems();
		push_number( max_index );
		return;
	}

	pop_stack();

	assign_svalue_no_free( sp, &arr->item[max_index] );
	free_array( arr );
}

#endif

#ifdef F_MIN

void
f_min() {
	svalue_t *sarr = sp - 1;
	array_t *arr = sarr->u.arr;
	int min_index = 0;
	int i;

	if( !arr->size ) {
		error( "Can't find min of an empty array.\n" );
	}

	if( arr->item->type != T_NUMBER && arr->item->type != T_REAL &&
			arr->item->type != T_STRING ) {
		error( "Array must consist of ints, floats or strings.\n" );
	}

	for( i = 1; i < arr->size; i++ ) {
		// Check the type of this element.
		switch( arr->item[i].type ) {
		case T_NUMBER:
			switch( arr->item[min_index].type ) {
			case T_NUMBER:
				if( arr->item[i].u.number < arr->item[min_index].u.number )
					min_index = i;
				break;
			case T_REAL:
				if( arr->item[i].u.number < arr->item[min_index].u.real )
					min_index = i;
				break;
			default:
				error( "Inhomogeneous array.\n" );
			}
			break;
			case T_REAL:
				switch( arr->item[min_index].type ) {
				case T_NUMBER:
					if( arr->item[i].u.real < arr->item[min_index].u.number )
						min_index = i;
					break;
				case T_REAL:
					if( arr->item[i].u.real < arr->item[min_index].u.real )
						min_index = i;
					break;
				default:
					error( "Inhomogeneous array.\n" );
				}
				break;
				case T_STRING:
					if( arr->item[min_index].type != T_STRING ) {
						error( "Inhomogeneous array.\n" );
					}
					if( strcmp( arr->item[i].u.string,
							arr->item[min_index].u.string ) < 0 )
						min_index = i;
					break;
				default:
					error( "Array must consist of ints, floats or strings.\n" );
		}
	}

	if( sp->u.number != 0 ) {
		pop_2_elems();
		push_number( min_index );
		return;
	}

	pop_stack();

	assign_svalue_no_free( sp, &arr->item[min_index] );
	free_array( arr );
}
#endif

#ifdef F_ABS

void
f_abs() {
	if( sp->type == T_REAL && sp->u.real < 0.0 )
		sp->u.real = -sp->u.real;
	else if( sp->type == T_NUMBER && sp->u.number < 0 )
		sp->u.number = -sp->u.number;
}
#endif

#ifdef F_ROLL_MDN

void
f_roll_MdN() {
	long roll = 0;

	if ( (sp - 1)->u.number > 0 && sp->u.number > 0 ) {
		while( (sp - 1)->u.number-- )
			roll += 1 + random_number( sp->u.number );
	}

	pop_stack(); // Pop one...
	sp->u.number = roll; // And change the other!
}

#endif

#ifdef F_STRING_DIFFERENCE
int min3( int a, int b, int c ) {
	if( a < b ) {
		if( a < c ) {
			return a;
		}
	}
	else if( b < c ) {
		return b;
	}
	return c;
}

int levenshtein( char *a, int as, char *b, int bs ) {
	int *table, skew, nskew, i, j;
	// Strip common pre- and suffix.  This doesn't change the result.
	while( as > 0 && a[0] == b[0] ) {
		a++;
		b++;
		as--;
		bs--;
	}

	while( as > 0 && a[as - 1] == b[bs - 1] ) {
		as--;
		bs--;
	}

	if( !as ) {                  // Empty string needs bs insertions.
		return bs;
	}

	table = CALLOCATE( bs + 1, int, TAG_TEMPORARY, "levenshtein" );
	for( i = 1; i <= bs; i++ ) {
		table[i] = i;
	}

	for( i = 0; i < as; i++ ) {
		table[0] = i + 1;
		skew = i;
		for( j = 1; j <= bs; j++ ) {
			if( a[i] != b[j - 1] ) {
				skew++;
			}

			nskew = table[j];
			table[j] = min3( table[j - 1] + 1, nskew + 1, skew );
			skew = nskew;
		}
	}

	i = table[bs];
	FREE( table );
	return i;
} /* levenshtein() */

void f_string_difference() {
	int diff, as, bs;
	char *a, *b;

	a = (char *)sp->u.string;
	b = (char *)( sp - 1 )->u.string;

	if( !strcmp( a, b ) ) {
		diff = 0;
	}
	else {
		as = strlen( a );
		bs = strlen( b );

		// Algorithm is quicker if the shorter string is passed first.
		if( as < bs ) {
			diff = levenshtein( a, as, b, bs );
		}
		else {
			diff = levenshtein( b, bs, a, as );
		}
	}

	free_string_svalue( sp-- );
	free_string_svalue( sp );
	put_number( diff );
}
#endif

#ifdef F_QUERY_CHARMODE
static int query_charmode (object_t * ob){
	int ret;
	if (!ob || ob->interactive == 0){
		ret = -2;
	}
	else {
		ret = (ob->interactive->iflags & I_SINGLE_CHAR);
	}
	return ret;
}

void f_query_charmode (void){
	int tmp;

	if (st_num_arg){
		tmp = query_charmode(sp->u.ob);
		free_object(&sp->u.ob, "f_query_charmode");
	}
	else {
		tmp = -1;
	}
	put_number(tmp);
}

#ifdef F_REMOVE_CHARMODE
static int remove_charmode (object_t * ob){
	int ret;
	if (!ob || ob->interactive == 0){
		ret = -2;
	}
	else {
		ret = (ob->interactive->iflags &= ~I_SINGLE_CHAR);
	}
	return ret;
}

void f_remove_charmode (void){
	int tmp;

	if (st_num_arg){
		tmp = remove_charmode(sp->u.ob);
		free_object(&sp->u.ob, "f_remove_charmode");
	}
	else {
		tmp = -1;
	}
	//if(tmp > 0 && !(tmp & I_SINGLE_CHAR)) tmp = 1;
	//else tmp = 0;
	put_number(tmp);
}
#endif
#endif
#ifdef F_REMOVE_GET_CHAR
static int remove_get_char (object_t * ob){
	int ret;
	if (!ob || ob->interactive == 0){
		ret = -2;
	}
	else ret = 0;

	if (ob->interactive->input_to) {
		ret = 1;
		free_sentence(ob->interactive->input_to);
		if (ob->interactive->num_carry > 0)
			free_some_svalues(ob->interactive->carryover, ob->interactive->num_carry);
		ob->interactive->carryover = NULL;
		ob->interactive->num_carry = 0;
		ob->interactive->input_to = 0;
	}
	else {
		ret = -1;
	}
	return ret;
}

void f_remove_get_char (void){
	int tmp;

	if (st_num_arg){
		tmp = remove_get_char(sp->u.ob);
		free_object(&sp->u.ob, "f_remove_get_char");
	}
	else {
		tmp = -3;
	}
	put_number(tmp);
}

#endif
#ifdef F_SEND_NULLBYTE
void f_send_nullbyte (void){
	int tmp;
	object_t *who;
	tmp = 0;

	who = sp->u.ob;
	if (!who || (who->flags & O_DESTRUCTED) || !who->interactive ||
			(who->interactive->iflags & (NET_DEAD | CLOSING))) {
		tmp = -2;
	}
	else {
		tmp = 1;
		//""is only the end-of-string zero byte.
		add_message(who,"",1);
		flush_message(who->interactive);
	}
	free_object(&sp->u.ob, "f_send_nullbyte");
	put_number(tmp);
}

#endif

#ifdef F_RESTORE_FROM_STRING
void f_restore_from_string(){
	const char *buf;
	long noclear;

	buf = (sp-1)->u.string;
	noclear = sp->u.number;
	if (!noclear) {
		clear_non_statics(current_object);
	}
	copy_and_push_string(buf); //restore_object_from_buff modifies the string in place, which is ok, copied strings aren't shared
	restore_object_from_buff(current_object, (char *)sp->u.string, noclear);
	pop_3_elems();
}
#endif

#ifdef F_CLASSES
void f_classes() {
   int i, j, num, size, offset, flag;
   array_t *vec, *subvec, *subsubvec;
   unsigned short *types;
   char buf[256];
   char *end;
   program_t *prog;

   flag = (sp--)->u.number;
   end = EndOf( buf );

   prog = sp->u.ob->prog;
   num = prog->num_classes;
   vec = allocate_empty_array( num );

   // Pull out data for each class.
   for( i = 0; i < num; i++ ) {
      // Do we want additional info on each class?
      if( flag ) {
         size = prog->classes[i].size;

         vec->item[i].type = T_ARRAY;
         subvec = vec->item[i].u.arr = allocate_empty_array( 1 + size );

         // First item of return array: the class's name.
         subvec->item[0].type = T_STRING;
         subvec->item[0].subtype = STRING_SHARED;
         subvec->item[0].u.string = make_shared_string(
                  prog->strings[prog->classes[i].classname] );

         offset = prog->classes[i].index;

         // Find the name and type of each class member.
         for( j = 0; j < size; j++, offset++ ) {
            subvec->item[j + 1].type = T_ARRAY;
            subsubvec = subvec->item[j + 1].u.arr = allocate_empty_array( 2 );

            // Each subarray contains the member's name...
            subsubvec->item[0].type = T_STRING;
            subsubvec->item[0].subtype = STRING_SHARED;
            subsubvec->item[0].u.string = make_shared_string(
                     prog->strings[prog->class_members[offset].membername] );

            // ...and type.
            get_type_name( buf, end, prog->class_members[offset].type );
            subsubvec->item[1].type = T_STRING;
            subsubvec->item[1].subtype = STRING_SHARED;
            subsubvec->item[1].u.string = make_shared_string( buf );
         }
      }
      else {
         // No additional info. Just pull out the class name.
         vec->item[i].type = T_STRING;
         vec->item[i].subtype = STRING_SHARED;
         vec->item[i].u.string = make_shared_string(
                  prog->strings[prog->classes[i].classname] );
      }
   }

   pop_stack();
   push_refed_array( vec );
}

#endif
