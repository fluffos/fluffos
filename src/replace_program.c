#include "std.h"
#include "replace_program.h"
#include "simul_efun.h"
#include "efun_protos.h"

/*
 * replace_program.c
 * replaces the program in a running object with one of the programs
 * it inherits, in order to save memory.
 * Ported from Amylaars LP 3.2 driver
 */

replace_ob_t *obj_list_replace = 0;

static program_t *search_inherited (char *, program_t *, int *);
static replace_ob_t *retrieve_replace_program_entry (void);

int replace_program_pending (object_t * ob)
{
    replace_ob_t *r_ob;

    for (r_ob = obj_list_replace;  r_ob;  r_ob = r_ob->next) {
        if (r_ob->ob == ob)
            return 1;
    }

    return 0;
}

void replace_programs (void)
{
    replace_ob_t *r_ob, *r_next;
    int i, num_fewer, offset;
    svalue_t *svp;

    debug(d_flag, ("start of replace_programs"));

    for (r_ob = obj_list_replace; r_ob; r_ob = r_next) {
        program_t *old_prog;

        num_fewer = r_ob->ob->prog->num_variables_total - r_ob->new_prog->num_variables_total;

        debug(d_flag, ("%d less variables\n", num_fewer));

        tot_alloc_object_size -= num_fewer * sizeof(svalue_t[1]);
        if ((offset = r_ob->var_offset)) {
            svp = r_ob->ob->variables;
            /* move our variables up to the top */
            for (i = 0; i < r_ob->new_prog->num_variables_total; i++) {
                free_svalue(svp, "replace_programs");
                *svp = *(svp + offset);
                *(svp + offset) = const0u;
                svp++;
            }
            /* free the rest */
            for (i = 0; i < num_fewer; i++) {
                free_svalue(svp, "replace_programs");
                *svp++ = const0u;
            }
        } else {
            /* We just need to remove the last num_fewer variables */
            svp = &r_ob->ob->variables[r_ob->new_prog->num_variables_total];
            for (i = 0; i < num_fewer; i++) {
                free_svalue(svp, "replace_programs");
                *svp++ = const0u;
            }
        }

        if (r_ob->ob->replaced_program) {
            FREE_MSTR(r_ob->ob->replaced_program);
            r_ob->ob->replaced_program = 0;
        }
        r_ob->ob->replaced_program = string_copy(r_ob->new_prog->filename, "replace_programs");

        reference_prog(r_ob->new_prog, "replace_programs");
        old_prog = r_ob->ob->prog;
        r_ob->ob->prog = r_ob->new_prog;
        r_next = r_ob->next;
        free_prog(&old_prog);

        debug(d_flag, ("program freed."));
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
    debug(d_flag, ("end of replace_programs"));
}

#ifdef F_REPLACE_PROGRAM
static program_t *search_inherited (char * str, program_t * prg, int * offpnt)
{
    program_t *tmp;
    int i;

    debug(d_flag, ("search_inherited started"));
    debug(d_flag, ("searching for PRG(/%s) in PRG(/%s)", str, prg->filename));
    debug(d_flag, ("num_inherited=%d\n", prg->num_inherited));

    for (i = 0; i < prg->num_inherited; i++) {
        debug(d_flag, ("index %d:", i));
        debug(d_flag, ("checking PRG(/%s)", prg->inherit[i].prog->filename));

        if (strcmp(str, prg->inherit[i].prog->filename) == 0) {
            debug(d_flag, ("match found"));

            *offpnt = prg->inherit[i].variable_index_offset;
            return prg->inherit[i].prog;
        } else if ((tmp = search_inherited(str, prg->inherit[i].prog,
                                          offpnt))) {
            debug(d_flag, ("deferred match found"));

            *offpnt += prg->inherit[i].variable_index_offset;
            return tmp;
        }
    }
    debug(d_flag, ("search_inherited failed"));

    return (program_t *) 0;
}

static replace_ob_t *retrieve_replace_program_entry (void)
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
f_replace_program (void)
{
    replace_ob_t *tmp;
    int name_len;
    char *name, *xname;
    program_t *new_prog;
    int var_offset;

    if (sp->type != T_STRING)
        bad_arg(1, F_REPLACE_PROGRAM);
    debug(d_flag, ("replace_program called"));

    if (!current_object)
        error("replace_program called with no current object\n");
    if (current_object == simul_efun_ob)
        error("replace_program on simul_efun object\n");

    if (current_object->prog->func_ref)
        error("cannot replace a program with function references.\n");

    name_len = SVALUE_STRLEN(sp);
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

    debug(d_flag, ("replace_program finished"));

    free_string_svalue(sp--);
}

#endif
