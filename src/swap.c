#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "swap.h"
#include "simul_efun.h"
#include "comm.h"
#include "md.h"

/*
 * Swap out programs from objects.
 *
 * Todo:
 *   Separate objects/programs, so that they can swapped out
 *   independently.  This way even inherited programs could
 *   be swapped, at the expense of not swapping entire program.
 */

static int num_swapped;
static int total_bytes_swapped;
static int line_num_bytes_swapped;
static char file_name_buf[100];
static char *file_name = file_name_buf;

static FILE *swap_file;		/* The swap file is opened once */

static struct sw_block {
    int start;
    int length;
    struct sw_block *next;
}       *swap_free;

static int last_data;

static int assert_swap_file PROT((void));
static int alloc_swap PROT((int));
static void free_swap PROT((int start, int length));
static int swap_in PROT((char **, int));
static int swap_out PROT((char *, int, int *));

/**
 ** General swapping routines.
 **/

/*
 * Make sure swap file is opened.
 */
static int assert_swap_file()
{
    if (swap_file == NULL) {
#ifndef MSDOS
	char host[50];

	gethostname(host, sizeof host);
	sprintf(file_name_buf, "%s.%s.%d", SWAP_FILE, host, port_number);
	file_name = file_name_buf;
	if (file_name[0] == '/')
	    file_name++;
#ifdef OS2
	swap_file = fopen(file_name, "w+b");
#else
	swap_file = fopen(file_name, "w+");
#endif
#else
	     swap_file = fopen(strcpy(file_name, "LPMUD.SWAP"), "w+b");
#endif
	swap_free = 0;
	last_data = 0;
	/* Leave this file pointer open ! */
	if (swap_file == 0)
	    return 0;
    }
    return 1;
}

/*
 * Find a position to swap to, using free blocks if possible.
 * 'length' is the size we need.
 *
 * Todo - think about better free block allocation methods
 */
static int alloc_swap P1(int, length)
{
    struct sw_block *ptr, *prev;
    int ret;

    /*
     * Doing first fit.  (next fit might work nicely if next pointer is reset
     * when a block is freed, anticipating a new allocation of the same size)
     */
    for (ptr = swap_free, prev = 0; ptr; prev = ptr, ptr = ptr->next) {
	if (ptr->length < length)
	    continue;
	/*
	 * found a block, update free list
	 */
	ret = ptr->start;
	ptr->start += length;
	ptr->length -= length;
	if (ptr->length == 0) {
	    /*
	     * exact fit, remove free block from list
	     */
	    if (!prev)
		swap_free = ptr->next;
	    else
		prev->next = ptr->next;
	    FREE(ptr);
	}
	return ret;
    }

    /*
     * no appropriate blocks found, go to end of file
     */
    return last_data;
}

/*
 * Free up a chunk of swap space, coalescing free blocks if necessary.
 *
 * Todo - think about tradeoff of storing the free block
 * info in the swap file itself.
 */
static void free_swap P2(int, start, int, length)
{
    struct sw_block *m, *ptr, *prev;

    length += sizeof(int);	/* extend with size of hidden information */

    /*
     * Construct and insert new free block
     */
    m = (struct sw_block *) DXALLOC(sizeof(struct sw_block), TAG_SWAP, "free_swap");
    m->start = start;
    m->length = length;

    for (ptr = swap_free, prev = 0; ptr; prev = ptr, ptr = ptr->next) {
	if (start < ptr->start)
	    break;
    }
    if (!prev) {
	swap_free = m;
    } else {
	prev->next = m;
    }
    m->next = ptr;

    /*
     * Combine adjacent blocks
     */
    if (ptr && m->start + m->length == ptr->start) {
	m->length += ptr->length;
	m->next = ptr->next;
	FREE(ptr);
    }
    if (prev && prev->start + prev->length == m->start) {
	prev->length += m->length;
	prev->next = m->next;
	FREE(m);
	m = prev;
    }
    /*
     * There is an implicit infinite block at the end making life hard Can't
     * do this earlier, since m and prev could have combined, so prev must be
     * found again (or use doubly linked list, etc).
     */
    if (m->start + m->length == last_data) {
	DEBUG_CHECK(m->next, "extraneous free swap blocks!\n");
	/* find prev pointer again *sigh* */
	for (ptr = swap_free, prev = 0; ptr != m; prev = ptr, ptr = ptr->next);
	last_data = m->start;
	FREE(m);
	if (!prev)
	    swap_free = 0;
	else
	    prev->next = 0;
    }
}

/*
 * Actually swap something out.
 *   block   - the memory to swap out
 *   size    - how big it is
 *   locp    - the swap location is written to the int this points to
 */
static int
swap_out P3(char *, block, int, size, int *, locp)
{
    extern int errno;

    if (!block || time_to_swap == 0)
	return 0;
    if (!assert_swap_file())
	return 0;

    if (*locp == -1) {		/* needs data written out */
	*locp = alloc_swap(size + sizeof size);
	if (fseek(swap_file, *locp, 0) == -1)
	    fatal("Couldn't seek the swap file, errno %d, offset %d.\n",
		  errno, *locp);
	if (fwrite((char *) &size, sizeof size, 1, swap_file) != 1 ||
	    fwrite(block, size, 1, swap_file) != 1) {
	    debug_message("I/O error in swap.\n");
	    perror("FOO: ");
	    *locp = -1;
	    return 0;
	}
	if (*locp >= last_data)
	    last_data = *locp + sizeof size + size;
    }
    total_bytes_swapped += size;/* also count sizeof int?? */
    return 1;
}

/*
 * Read something back in from swap.  Return the size.
 *   blockp    - a pointer to what will hold the block read in
 *   loc       - position in the swap file to read from
 */
static int
swap_in P2(char **, blockp, int, loc)
{
    extern int errno;
    int size;

    if (loc == -1)
	return 0;
    if (fseek(swap_file, loc, 0) == -1)
	fatal("Couldn't seek the swap file, errno %d, offset %d.\n",
	      errno, loc);
    /* find out size */
    if (fread((char *) &size, sizeof size, 1, swap_file) == -1)
	fatal("Couldn't read the swap file.\n");
    *blockp = DXALLOC(size, TAG_SWAP, "swap_in");
    if (fread(*blockp, size, 1, swap_file) == -1)
	fatal("Couldn't read the swap file.\n");
    total_bytes_swapped -= size;
    return size;
}

/**
 ** Routines to swap/load specific things.
 **/

/*
 * marion - adjust pointers for swap out and later relocate on swap in
 *   program
 *   functions
 *   strings
 *   variable_names
 *   inherit
 *   argument_types
 *   type_start
 */
int
locate_out P1(struct program *, prog)
{
    char *p = 0;		/* keep cc happy */

    if (!prog)
	return 0;
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("locate_out: %lX %lX %lX %lX %lX %lX %lX\n",
		      prog->p.i.program, prog->p.i.functions,
	     prog->p.i.strings, prog->p.i.variable_names, prog->p.i.inherit,
		      prog->p.i.argument_types, prog->p.i.type_start);
    }
#endif
    prog->p.i.program = &p[prog->p.i.program - (char *) prog];
    prog->p.i.functions = (struct function *)
	& p[(char *) prog->p.i.functions - (char *) prog];
    prog->p.i.strings = (char **)
	&p[(char *) prog->p.i.strings - (char *) prog];
    prog->p.i.variable_names = (struct variable *)
	& p[(char *) prog->p.i.variable_names - (char *) prog];
    prog->p.i.inherit = (struct inherit *)
	& p[(char *) prog->p.i.inherit - (char *) prog];
    if (prog->p.i.type_start) {
	prog->p.i.argument_types = (unsigned short *)
	    &p[(char *) prog->p.i.argument_types - (char *) prog];
	prog->p.i.type_start = (unsigned short *)
	    &p[(char *) prog->p.i.type_start - (char *) prog];
    }
    return 1;
}


/*
 * marion - relocate pointers after swap in
 *   program
 *   functions
 *   strings
 *   variable_names
 *   inherit
 *   argument_types
 *   type_start
 */
int
locate_in P1(struct program *, prog)
{
    char *p = (char *) prog;

    if (!prog)
	return 0;
    prog->p.i.program = &p[prog->p.i.program - (char *) 0];
    prog->p.i.functions = (struct function *)
	& p[(char *) prog->p.i.functions - (char *) 0];
    prog->p.i.strings = (char **)
	&p[(char *) prog->p.i.strings - (char *) 0];
    prog->p.i.variable_names = (struct variable *)
	& p[(char *) prog->p.i.variable_names - (char *) 0];
    prog->p.i.inherit = (struct inherit *)
	& p[(char *) prog->p.i.inherit - (char *) 0];
    if (prog->p.i.type_start) {
	prog->p.i.argument_types = (unsigned short *)
	    &p[(char *) prog->p.i.argument_types - (char *) 0];
	prog->p.i.type_start = (unsigned short *)
	    &p[(char *) prog->p.i.type_start - (char *) 0];
    }
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("locate_in: %lX %lX %lX %lX %lX %lX\n",
		      prog->p.i.program, prog->p.i.functions,
	     prog->p.i.strings, prog->p.i.variable_names, prog->p.i.inherit,
		      prog->p.i.argument_types, prog->p.i.type_start);
    }
#endif
    return 1;
}

/*
 * Swap out an object. Only the program is swapped, not the 'struct object'.
 *
 * marion - the swap seems to corrupt the function table
 */
int swap P1(struct object *, ob)
{
    /* the simuls[] table uses pointers to the functions so the simul_efun
     * program cannot be relocated.  locate_in() could be changed to
     * correct this or simuls[] could use offsets, but it doesn't seem
     * worth it just to get the simul_efun object to swap.  Maybe later.
     */
    if (ob == simul_efun_ob) return 0;
    if (ob->flags & O_DESTRUCTED)
	return 0;
#ifdef DEBUG
    if (d_flag > 1) {		/* marion */
	debug_message("Swap object %s (ref %d)\n", ob->name, ob->ref);
    }
#endif
    if (ob->prog->p.i.line_info)
	swap_line_numbers(ob->prog);	/* not always done before we get here */
    if ((ob->flags & O_HEART_BEAT) || (ob->flags & O_CLONE)) {
#ifdef DEBUG
	if (d_flag > 1) {
	    debug_message("  object not swapped - heart beat or cloned.\n");
	}
#endif
	return 0;
    }
    if (ob->prog->p.i.ref > 1 || ob->interactive) {
#ifdef DEBUG
	if (d_flag > 1) {
	    debug_message("  object not swapped - inherited or interactive.\n");
	}
#endif
	return 0;
    }
    if (ob->prog->p.i.func_ref > 0) {
#ifdef DEBUG
	if (d_flag > 1) {
	    debug_message("  object not swapped - referenced by functions.\n");
	}
#endif
	return 0;
    }
    locate_out(ob->prog);	/* relocate the internal pointers */
    if (swap_out((char *) ob->prog, ob->prog->p.i.total_size, (int *) &ob->swap_num)) {
	num_swapped++;
	free_prog(ob->prog, 0);	/* Do not free the strings */
	ob->prog = 0;
	ob->flags |= O_SWAPPED;
	return 1;
    } else {
	locate_in(ob->prog);
	return 0;
    }
}

void load_ob_from_swap P1(struct object *, ob)
{
    if (ob->swap_num == -1)
	fatal("Loading not swapped object.\n");
#ifdef DEBUG
    if (d_flag > 1) {		/* marion */
	debug_message("Unswap object %s (ref %d)\n", ob->name, ob->ref);
    }
#endif
    swap_in((char **) &ob->prog, ob->swap_num);
    SET_TAG(ob->prog, TAG_PROGRAM);
    /*
     * to be relocated: program functions strings variable_names inherit
     * argument_types type_start
     */
    locate_in(ob->prog);	/* relocate the internal pointers */

    /* The reference count will already be 1 ! */
    ob->flags &= ~O_SWAPPED;
    num_swapped--;
    total_prog_block_size += ob->prog->p.i.total_size;
    total_num_prog_blocks += 1;
}

/*
 * Swap out line number information.
 */
int
swap_line_numbers P1(struct program *, prog)
{
    int size;

    if (!prog || !prog->p.i.line_info)
	return 0;
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("Swap line numbers for %s\n", prog->name);
    }
#endif
    size = prog->p.i.file_info[0];
    if (swap_out((char *) prog->p.i.file_info, size,
		 &prog->p.i.line_swap_index)) {
	line_num_bytes_swapped += size;
	FREE(prog->p.i.file_info);
	prog->p.i.file_info = 0;
	prog->p.i.line_info = 0;
	return 1;
    }
    return 0;
}

/*
 * Reload line number information from swap.
 */
void load_line_numbers P1(struct program *, prog)
{
    int size;

    if (prog->p.i.line_info)
	return;
#ifdef DEBUG
    if (d_flag > 1) {
	debug_message("Unswap line numbers for %s\n", prog->name);
    }
#endif
    size = swap_in((char **) &prog->p.i.file_info, prog->p.i.line_swap_index);
    SET_TAG(prog->p.i.file_info, TAG_LINENUMBERS);
    prog->p.i.line_info = (unsigned char *)&prog->p.i.file_info[prog->p.i.file_info[1]];
    line_num_bytes_swapped -= size;
}

/**
 **  Misc. routines.
 **/

/*
 * Remove the swap space associated with this object.
 */
void remove_swap_file P1(struct object *, ob)
{
    if (!ob)
	return;

    /* may be swapped out, so swap in to get size, update stats, etc */
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    if (ob->prog)
	free_swap(ob->swap_num, ob->prog->p.i.total_size);
    ob->swap_num = -1;
}

/*
 * Same as above, but to remove line_number swap space.
 */
void
remove_line_swap P1(struct program *, prog)
{
    if (!prog->p.i.line_info)
	load_line_numbers(prog);
    if (prog->p.i.line_swap_index != -1 && prog->p.i.line_info)
	free_swap(prog->p.i.line_swap_index,
		  prog->p.i.file_info[0]);
    prog->p.i.line_swap_index = -1;
}

void print_swap_stats()
{
    extern int errno;
    int size, cnt, end;
    struct sw_block *m;

    add_message("Swap information:\n");
    add_message("-------------------------\n");
    add_vmessage("Progs swapped:       %10lu\n", num_swapped);
    add_vmessage("Linenum bytes:       %10lu\n", line_num_bytes_swapped);
    add_vmessage("Total bytes swapped: %10lu\n", total_bytes_swapped);
    if (!swap_file) {
	add_message("No swap file\n");
	return;
    }
    size = cnt = 0;
    for (m = swap_free; m; size += m->length, cnt++, m = m->next);
    if (fseek(swap_file, 0L, 2) == -1)
	fatal("Couldn't seek end of the swap file, errno %d\n", errno);
    end = ftell(swap_file) - last_data;
    if (end) {
	size += end;
	cnt++;
    }
    add_vmessage("Freed bytes:         %10lu (%d chunks)\n", size, cnt);
}

/*
 * This one is called at shutdown. Remove the swap file.
 */
void unlink_swap_file()
{
    if (swap_file == 0)
	return;
#ifndef MSDOS
    unlink(file_name);
    fclose(swap_file);
#else
    fclose(swap_file);
    unlink(file_name);
#endif
}
