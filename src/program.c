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
	debug_message("reference_prog: /%s ref %d (%s)\n",
	       progp->name, progp->ref, from);
#endif
}

void deallocate_program P1(program_t *, progp)
{
    int i;

#ifdef DEBUG
    if (d_flag)
	debug_message("free_prog: /%s\n", progp->name);
#endif
    
    total_prog_block_size -= progp->total_size;
    total_num_prog_blocks -= 1;

    /* Free all function names. */
    for (i = 0; i < (int) progp->num_functions_defined; i++)
	if (progp->function_table[i].name)
	    free_string(progp->function_table[i].name);
    /* Free all strings */
    for (i = 0; i < (int) progp->num_strings; i++)
	free_string(progp->strings[i]);
    /* Free all variable names */
    for (i = 0; i < (int) progp->num_variables_defined; i++)
	free_string(progp->variable_table[i]);
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

char *variable_name P2(program_t *, prog, int, idx) {
    int i = prog->num_inherited - 1;
    int first;

    if (i > -1)
	first = prog->inherit[i].variable_index_offset + prog->inherit[i].prog->num_variables_total;
    else
	return prog->variable_table[idx];
    if (idx >= first)
	return prog->variable_table[idx - first];
    while (idx < prog->inherit[i].variable_index_offset)
	i--;
    return variable_name(prog->inherit[i].prog, idx - prog->inherit[i].variable_index_offset);
}

#ifdef COMPRESS_FUNCTION_TABLES
/* Warning: sometimes returns a pointer to a static object.  So don't
   hold the returned pointers too long; it will be invalidated on the
   next call to this function */
runtime_function_u *find_func_entry P2(program_t *, prog, int, index) {
    static runtime_function_u ret;
    
    int f_ov = prog->function_compressed->first_overload;
    int n_ov = prog->function_compressed->first_defined - prog->function_compressed->num_compressed;
    int idx;
    int fidx;
    
    if ((index < f_ov) || (idx = index - f_ov) >= n_ov ||
	(fidx = prog->function_compressed->index[idx]) == 255) {
	int first = 0, last = prog->num_inherited - 1;
	/* The entry was omitted.  Remake it */
	while (last > first) {
	    int mid = (last + first + 1) / 2;
	    if (prog->inherit[mid].function_index_offset > index)
		last = mid - 1;
	    else
		first = mid;
	}
	ret.inh.offset = first;
	ret.inh.function_index_offset = index - prog->inherit[first].function_index_offset;
	return &ret;
    } else {
	return prog->function_offsets + fidx;
    }
}
#endif
