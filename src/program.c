#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "program.h"
#include "swap.h"

int total_num_prog_blocks, total_prog_block_size;

void reference_prog P2(struct program *, progp, char *, from)
{
    progp->p.i.ref++;
#ifdef DEBUG
    if (d_flag)
	printf("reference_prog: %s ref %d (%s)\n",
	       progp->name, progp->p.i.ref, from);
#endif
}

void deallocate_program P1(struct program *, progp)
{
    int i;

#ifdef DEBUG
    if (d_flag)
	printf("free_prog: %s\n", progp->name);
#endif
    
    total_prog_block_size -= progp->p.i.total_size;
    total_num_prog_blocks -= 1;

    /* Free all function names. */
    for (i = 0; i < (int) progp->p.i.num_functions; i++)
	if (progp->p.i.functions[i].name)
	    free_string(progp->p.i.functions[i].name);
    /* Free all strings */
    for (i = 0; i < (int) progp->p.i.num_strings; i++)
	free_string(progp->p.i.strings[i]);
    /* Free all variable names */
    for (i = 0; i < (int) progp->p.i.num_variables; i++)
	free_string(progp->p.i.variable_names[i].name);
    /* Free all inherited objects */
    for (i = 0; i < (int) progp->p.i.num_inherited; i++)
	free_prog(progp->p.i.inherit[i].prog, 1);
    free_string(progp->name);

    /*
     * We're going away for good, not just being swapped, so free up
     * line_number stuff.
     */
    if (progp->p.i.line_swap_index != -1)
	remove_line_swap(progp);
    if (progp->p.i.file_info)
	FREE(progp->p.i.file_info);
    
#ifdef LPC_TO_C
    /*
     * more stuff for compiled objects; will swapping a compiled object
     * crash?  -Beek
     */
    if (!progp->p.i.program_size)
	FREE(progp->p.i.program);
#endif
    FREE((char *) progp);
}

/*
 * Decrement reference count for a program. If it is 0, then free the prgram.
 * The flag free_sub_strings tells if the propgram plus all used strings
 * should be freed. They normally are, except when objects are swapped,
 * as we want to be able to read the program in again from the swap area.
 * That means that strings are not swapped.
 */
void free_prog P2(struct program *, progp, int, free_sub_strings)
{
    progp->p.i.ref--;
    if (progp->p.i.ref > 0)
	return;
    if (progp->p.i.func_ref > 0)
	return;

#ifdef DEBUG
    if (progp->p.i.ref < 0)
	fatal("Negative ref count for prog ref.\n");
#endif

    if (free_sub_strings) 
	deallocate_program(progp);
    else {
	total_prog_block_size -= progp->p.i.total_size;
	total_num_prog_blocks -= 1;
	FREE((char *) progp);
    }
}

