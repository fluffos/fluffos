#include "std.h"
#include "lpc_incl.h"
#include "replace_program.h"
#include "simul_efun.h"
#include "swap.h"

/*
 * replace_program.c
 * replaces the program in a running object with one of the programs
 * it inherits, in order to save memory.
 * Ported from Amylaars LP 3.2 driver
 */

replace_ob_t *obj_list_replace = 0;

static program_t *search_inherited PROT((char *, program_t *, int *));
static replace_ob_t *retrieve_replace_program_entry PROT((void));

void replace_programs()
{
    replace_ob_t *r_ob, *r_next;
    int i, j;
    svalue_t *svp;

#ifdef DEBUG
    if (d_flag)
	debug_message("start of replace_programs\n");
#endif
    for (r_ob = obj_list_replace; r_ob; r_ob = r_next) {
	program_t *old_prog;

	if (r_ob->ob->flags & O_SWAPPED)
	    load_ob_from_swap(r_ob->ob);
	i = r_ob->ob->prog->num_variables - r_ob->new_prog->num_variables;
#ifdef DEBUG
	if (d_flag)
	    debug_message("%d less variables\n", i);
#endif
	tot_alloc_object_size -= i * sizeof(svalue_t[1]);
	svp = r_ob->ob->variables;
	j = r_ob->var_offset;
	i -= j;
#ifdef DEBUG
	if (d_flag)
	    debug_message("freeing %d variables:\n", j);
#endif
	while (--j >= 0) {
	    free_svalue(svp, "replace_programs");
	    *(svp++) = const0n;
	}
#ifdef DEBUG
	if (d_flag)
	    debug_message("freed.\n");
#endif
	if (r_ob->var_offset)
	    for (j = 0; j < (int) r_ob->new_prog->num_variables; j++)
		r_ob->ob->variables[j] = svp[j];
	svp += j;
#ifdef DEBUG
	if (d_flag)
	    debug_message("freeing %d variables:\n", i);
#endif
	while (--i >= 0) {
	    free_svalue(svp, "replace_programs");
	    *(svp++) = const0n;
	}
#ifdef DEBUG
	if (d_flag)
	    debug_message("freed.\n");
#endif
	r_ob->new_prog->ref++;
	old_prog = r_ob->ob->prog;
	r_ob->ob->prog = r_ob->new_prog;
	r_next = r_ob->next;
	free_prog(old_prog, 1);
#ifdef DEBUG
	if (d_flag)
	    debug_message("program freed.\n");
#endif
#ifndef NO_SHADOWS
	if (r_ob->ob->shadowing) {
	    /*
	     * The master couldn't decide if it's a legal shadowing before
	     * the program was actually replaced. It is possible that the
	     * blueprint to the replacing program is already destructed, and
	     * it's source changed. On the other hand, if we called the
	     * master now, all kind of volatile data structures could result,
	     * even new entries for obj_list_replace. This would eventually
	     * require to reference it, and all the lrpp's , in
	     * check_a_lot_ref_counts() and garbage_collection() . Being able
	     * to use replace_program() in shadows is hardly worth this
	     * effort. Thus, we simply stop the shadowing.
	     */
	    r_ob->ob->shadowing->shadowed = r_ob->ob->shadowed;
	    if (r_ob->ob->shadowed) {
		r_ob->ob->shadowed->shadowing = r_ob->ob->shadowing;
		r_ob->ob->shadowed = 0;
	    }
	    r_ob->ob->shadowing = 0;
	}
#endif
	FREE((char *) r_ob);
    }
    obj_list_replace = (replace_ob_t *) 0;
#ifdef DEBUG
    if (d_flag)
	debug_message("end of replace_programs\n");
#endif
}

#ifdef F_REPLACE_PROGRAM
static program_t *search_inherited P3(char *, str, program_t *, prg, int *, offpnt)
{
    program_t *tmp;
    int i;

#ifdef DEBUG
    if (d_flag) {
	debug_message("search_inherited started\n");
	debug_message("searching for PRG(%s) in PRG(%s)\n", str, prg->name);
	debug_message("num_inherited=%d\n", prg->num_inherited);
    }
#endif
    for (i = 0; i < (int) prg->num_inherited; i++) {
#ifdef DEBUG
	if (d_flag) {
	    debug_message("index %d:\n", i);
	    debug_message("checking PRG(%s)\n", prg->inherit[i].prog->name);
	}
#endif
	if (strcmp(str, prg->inherit[i].prog->name) == 0) {
#ifdef DEBUG
	    if (d_flag)
		debug_message("match found\n");
#endif
	    *offpnt = prg->inherit[i].variable_index_offset;
	    return prg->inherit[i].prog;
	} else if ((tmp = search_inherited(str, prg->inherit[i].prog,
					  offpnt))) {
#ifdef DEBUG
	    if (d_flag)
		debug_message("deferred match found\n");
#endif
	    *offpnt += prg->inherit[i].variable_index_offset;
	    return tmp;
	}
    }
#ifdef DEBUG
    if (d_flag)
	debug_message("search_inherited failed\n");
#endif
    return (program_t *) 0;
}

static replace_ob_t *retrieve_replace_program_entry()
{
    replace_ob_t *r_ob;

    for (r_ob = obj_list_replace; r_ob; r_ob = r_ob->next) {
	if (r_ob->ob == current_object) {
	    return r_ob;
	}
    }
    return 0;
}

void
f_replace_program P2(int, num_arg, int, instruction)
{
    replace_ob_t *tmp;
    int name_len;
    char *name, *xname;
    program_t *new_prog;
    int var_offset;

    if (sp->type != T_STRING)
	bad_arg(1, instruction);
#ifdef DEBUG
    if (d_flag)
	debug_message("replace_program called\n");
#endif
    if (!current_object)
	error("replace_program called with no current object\n");
    if (current_object == simul_efun_ob)
	error("replace_program on simul_efun object\n");

    if (current_object->prog->func_ref)
	error("cannot replace a program with function references.\n");

    name_len = strlen(sp->u.string);
    name = (char *) DMALLOC(name_len + 3, TAG_TEMPORARY, "replace_program");
    xname = name;
    strcpy(name, sp->u.string);
    if (name[name_len - 2] != '.' || name[name_len - 1] != 'c')
	strcat(name, ".c");
    if (*name == '/')
	name++;
    new_prog = search_inherited(name, current_object->prog, &var_offset);
    FREE(xname);
    if (!new_prog) {
	error("program to replace the current with has to be inherited\n");
    }
    if (!(tmp = retrieve_replace_program_entry())) {
	tmp = ALLOCATE(replace_ob_t, TAG_TEMPORARY, "replace_program");
	tmp->ob = current_object;
	tmp->next = obj_list_replace;
	obj_list_replace = tmp;
    }
    tmp->new_prog = new_prog;
    tmp->var_offset = var_offset;
#ifdef DEBUG
    if (d_flag)
	debug_message("replace_program finished\n");
#endif
    free_string_svalue(sp--);
}

#endif
