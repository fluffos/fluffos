#include "std.h"
#include "lpc_incl.h"

int total_num_prog_blocks, total_prog_block_size;

void reference_prog (program_t * progp, const char * from)
{
    progp->ref++;
    debug(d_flag, ("reference_prog: /%s ref %d (%s)\n",
               progp->filename, progp->ref, from));
}

void deallocate_program (program_t * progp)
{
    int i;

    debug(d_flag, ("free_prog: /%s\n", progp->filename));
    
    total_prog_block_size -= progp->total_size;
    total_num_prog_blocks -= 1;

    /* Free all function names. */
    for (i = 0; i < progp->num_functions_defined; i++)
        if (progp->function_table[i].funcname)
            free_string(progp->function_table[i].funcname);
    /* Free all strings */
    for (i = 0; i < progp->num_strings; i++)
        free_string(progp->strings[i]);
    /* Free all variable names */
    for (i = 0; i < progp->num_variables_defined; i++)
        free_string(progp->variable_table[i]);
    /* Free all inherited objects */
    for (i = 0; i < progp->num_inherited; i++)
        free_prog(&progp->inherit[i].prog);
    free_string(progp->filename);

    /*
     * We're going away for good, not just being swapped, so free up
     * line_number stuff.
     */
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
void free_prog (program_t **progp)
{
    (*progp)->ref--;
    if ((*progp)->ref > 0) {
      *progp = (program_t *)2;//NULL;
        return;
    }
    if ((*progp)->func_ref > 0) {
      *progp = (program_t *)3;//NULL;
        return;
    }

    deallocate_program(*progp);
    *progp = (program_t *)4;//NULL;
}

char *variable_name (program_t * prog, int idx) {
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

function_t *find_func_entry (program_t * prog, int index) {
    register int low, mid, high;
    

    /* Walk up the inheritance tree to the real definition */   
    if (prog->function_flags[index] & FUNC_ALIAS) {
        index = prog->function_flags[index] & ~FUNC_ALIAS;
    }
    
    while (prog->function_flags[index] & FUNC_INHERITED) {
        low = 0;
        high = prog->num_inherited -1;
        
        while (high > low) {
            mid = (low + high + 1) >> 1;
            if (prog->inherit[mid].function_index_offset > index)
                high = mid -1;
            else low = mid;
        }
        index -= prog->inherit[low].function_index_offset;
        prog = prog->inherit[low].prog;
    }
    
    index -= prog->last_inherited;

    return prog->function_table + index;
}

