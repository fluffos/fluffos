#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "program.h"
#include "swap.h"

int total_num_prog_blocks, total_prog_block_size;

void reference_prog P2(program_t *, progp, char *, from)
{
    progp->ref++;
#ifdef DEBUG
    if (d_flag)
	debug_message("reference_prog: %s ref %d (%s)\n",
	       progp->name, progp->ref, from);
#endif
}

void deallocate_program P1(program_t *, progp)
{
    int i;

#ifdef DEBUG
    if (d_flag)
	debug_message("free_prog: %s\n", progp->name);
#endif
    
    total_prog_block_size -= progp->total_size;
    total_num_prog_blocks -= 1;

    /* Free all function names. */
    for (i = 0; i < (int) progp->num_functions; i++)
	if (progp->functions[i].name)
	    free_string(progp->functions[i].name);
    /* Free all strings */
    for (i = 0; i < (int) progp->num_strings; i++)
	free_string(progp->strings[i]);
    /* Free all variable names */
    for (i = 0; i < (int) progp->num_variables; i++)
	free_string(progp->variable_names[i].name);
    /* Free all inherited objects */
    for (i = 0; i < (int) progp->num_inherited; i++)
	free_prog(progp->inherit[i].prog, 1);
    free_string(progp->name);

    /*
     * We're going away for good, not just being swapped, so free up
     * line_number stuff.
     */
    if (progp->line_swap_index != -1)
	remove_line_swap(progp);
    if (progp->file_info)
	FREE(progp->file_info);
    
    FREE((char *) progp);
}

/*
 * Decrement reference count for a program. If it is 0, then free the prgram.
 * The flag free_sub_strings tells if the propgram plus all used strings
 * should be freed. They normally are, except when objects are swapped,
 * as we want to be able to read the program in again from the swap area.
 * That means that strings are not swapped.
 */
void free_prog P2(program_t *, progp, int, free_sub_strings)
{
    progp->ref--;
    if (progp->ref > 0)
	return;
    if (progp->func_ref > 0)
	return;

    if (free_sub_strings) 
	deallocate_program(progp);
    else {
	total_prog_block_size -= progp->total_size;
	total_num_prog_blocks -= 1;
	FREE((char *) progp);
    }
}

