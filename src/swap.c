#include "std.h"
#include "swap.h"
#include "file_incl.h"
#include "simul_efun.h"
#include "master.h"
#include "comm.h"
#include "md.h"
#include "file.h"
#include "port.h"

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

/* The swap file is opened once */
#ifdef SWAP_USE_FD
static int swap_file;
#else
static FILE *swap_file;
#endif

#define DIFF(x, y) ((char *)(x) - (char *)(y))
#define ADD(x, y) (&(((char *)(y))[(POINTER_INT)x]))

typedef struct sw_block_s {
    int start;
    int length;
    struct sw_block_s *next;
} sw_block_t;

static sw_block_t *swap_free;

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
#ifdef SWAP_USE_FD
	char host[50];

	gethostname(host, sizeof host);
	sprintf(file_name_buf, "%s.%s.%d", SWAP_FILE, host, 
		external_port[0].port);
	file_name = file_name_buf;
	if (file_name[0] == '/')
	    file_name++;
        swap_file = open(file_name, O_RDWR | O_CREAT | O_TRUNC);
#else
	char host[50];

	gethostname(host, sizeof host);
	sprintf(file_name_buf, "%s.%s.%d", SWAP_FILE, host, 
		external_port[0].port);
	file_name = file_name_buf;
	if (file_name[0] == '/')
	    file_name++;
	swap_file = fopen(file_name, "w+b");
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
 *  Seek the swap file
 */
static void
swap_seek P2(long, offset, int, flag) {
    int ret;
    
    do {
#ifdef SWAP_USE_FD
	ret = lseek(swap_file, offset, flag);
#else
	ret = fseek(swap_file, offset, flag);
#endif
    } while (ret == -1 && errno == EINTR);
    if (ret == -1)
	fatal("Couldn't seek the swap file, error %s, offset %d.\n",
	      port_strerror(errno), offset);
}

/*
 * Find a position to swap to, using free blocks if possible.
 * 'length' is the size we need.
 *
 * Todo - think about better free block allocation methods
 */
static int alloc_swap P1(int, length)
{
    sw_block_t *ptr, *prev;
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
    sw_block_t *m, *ptr, *prev;

    length += sizeof(int);	/* extend with size of hidden information */

    /*
     * Construct and insert new free block
     */
    m = (sw_block_t *) DXALLOC(sizeof(sw_block_t), TAG_SWAP, "free_swap");
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
    if (!block || time_to_swap == 0)
	return 0;
    if (!assert_swap_file())
	return 0;

    if (*locp == -1) {		/* needs data written out */
	*locp = alloc_swap(size + sizeof size);
	swap_seek(*locp, 0);
#ifdef SWAP_USE_FD
        if ((write(swap_file, &size, sizeof size) != sizeof size) ||
	    write(swap_file, block, size) != size) {
	    debug_perror("swap_out", swap_file);
	    *locp = -1;
	    return 0;
	}
#else
	if (fwrite((char *) &size, sizeof size, 1, swap_file) != 1 ||
	    fwrite(block, size, 1, swap_file) != 1) {
	    debug_perror("swap_out:swap file", 0);
	    *locp = -1;
	    return 0;
	}
#endif
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
    int size;

    DEBUG_CHECK(!blockp, "blockp null in swap_in()\n");
    
    if (loc == -1)
	return 0;
    swap_seek(loc, 0);
#ifdef SWAP_USE_FD
    /* find out size */
    if (read(swap_file, &size, sizeof size) == -1)
        fatal("Couldn't read the swap file.\n");
    DEBUG_CHECK(size <= 0, "Illegal size read from swap file.\n");
    *blockp = DXALLOC(size, TAG_SWAP, "swap_in");
    if (read(swap_file, *blockp, size) == -1)
        fatal("Couldn't read the swap file.\n");
#else
    /* find out size */
    if (fread((char *) &size, sizeof size, 1, swap_file) == -1)
	fatal("Couldn't read the swap file.\n");
    DEBUG_CHECK(size <= 0, "Illegal size read from swap file.\n");
    *blockp = DXALLOC(size, TAG_SWAP, "swap_in");
    if (fread(*blockp, size, 1, swap_file) == -1)
	fatal("Couldn't read the swap file.\n");
#endif
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
locate_out P1(program_t *, prog)
{
    if (!prog)
	return 0;
    debug(d_flag, ("locate_out: %p %p %p %p %p %p %p\n",
		      prog->program, prog->function_table,
	     prog->strings, prog->variable_table, prog->inherit,
		      prog->argument_types, prog->type_start));

    prog->program = (char *)DIFF(prog->program, prog);
    prog->function_table = (function_t *)DIFF(prog->function_table, prog);
    prog->function_flags = (unsigned short *)DIFF(prog->function_flags, prog);
    prog->strings = (char **)DIFF(prog->strings, prog);
    prog->variable_table = (char **)DIFF(prog->variable_table, prog);
    prog->variable_types = (unsigned short *)DIFF(prog->variable_types, prog);
    prog->inherit = (inherit_t *)DIFF(prog->inherit, prog);
    prog->classes = (class_def_t *)DIFF(prog->classes, prog);
    prog->class_members = (class_member_entry_t *)DIFF(prog->class_members, prog);
    if (prog->type_start) {
	prog->argument_types = (unsigned short *)DIFF(prog->argument_types, prog);
	prog->type_start = (unsigned short *)DIFF(prog->type_start, prog);
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
locate_in P1(program_t *, prog)
{
    if (!prog)
	return 0;
    prog->program = ADD(prog->program, prog);
    prog->function_table = (function_t *)ADD(prog->function_table, prog);
    prog->function_flags = (unsigned short *)ADD(prog->function_flags, prog);
    prog->strings = (char **)ADD(prog->strings, prog);
    prog->variable_table = (char **)ADD(prog->variable_table, prog);
    prog->variable_types = (unsigned short *)ADD(prog->variable_types, prog);
    prog->inherit = (inherit_t *)ADD(prog->inherit, prog);
    prog->classes = (class_def_t *)ADD(prog->classes, prog);
    prog->class_members = (class_member_entry_t *)ADD(prog->class_members, prog);
    if (prog->type_start) {
	prog->argument_types = (unsigned short *)ADD(prog->argument_types, prog);
	prog->type_start = (unsigned short *)ADD(prog->type_start, prog);
    }
    debug(d_flag, ("locate_in: %p %p %p %p %p %p %p\n",
		      prog->program, prog->function_table,
	     prog->strings, prog->variable_table, prog->inherit,
		      prog->argument_types, prog->type_start));

    return 1;
}

/*
 * Swap out an object. Only the program is swapped, not the 'object_t'.
 *
 * marion - the swap seems to corrupt the function table
 */
int swap P1(object_t *, ob)
{
    /* the simuls[] table uses pointers to the functions so the simul_efun
     * program cannot be relocated.  locate_in() could be changed to
     * correct this or simuls[] could use offsets, but it doesn't seem
     * worth it just to get the simul_efun object to swap.  Maybe later.
     *
     * Ditto the master object and master_applies[].  Mudlibs that have
     * a period TIME_TO_SWAP between successive master applies must be
     * extremely rare ...
     */
    if (ob == simul_efun_ob || ob == master_ob) return 0;
    if (ob->flags & O_DESTRUCTED)
	return 0;
    debug(d_flag, ("Swap object /%s (ref %d)", ob->name, ob->ref));

    if (ob->prog->line_info)
	swap_line_numbers(ob->prog);	/* not always done before we get here */
    if ((ob->flags & O_HEART_BEAT) || (ob->flags & O_CLONE)) {
	debug(d_flag, ("  object not swapped - heart beat or cloned."));
	return 0;
    }
    if (ob->prog->ref > 1 || ob->interactive) {
	debug(d_flag, ("  object not swapped - inherited or interactive or in apply_low() cache."));

	return 0;
    }
    if (ob->prog->func_ref > 0) {
	debug(d_flag, ("  object not swapped - referenced by functions."));

	return 0;
    }
    locate_out(ob->prog);	/* relocate the internal pointers */
    if (swap_out((char *) ob->prog, ob->prog->total_size, (int *) &ob->swap_num)) {
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

void load_ob_from_swap P1(object_t *, ob)
{
    if (ob->swap_num == -1)
	fatal("Loading not swapped object.\n");

    debug(d_flag, ("Unswap object /%s (ref %d)", ob->name, ob->ref));

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
    total_prog_block_size += ob->prog->total_size;
    total_num_prog_blocks += 1;
}

/*
 * Swap out line number information.
 */
int
swap_line_numbers P1(program_t *, prog)
{
    int size;

    if (!prog || !prog->line_info)
	return 0;

    debug(d_flag, ("Swap line numbers for /%s", prog->name));

    size = prog->file_info[0];
    if (swap_out((char *) prog->file_info, size,
		 &prog->line_swap_index)) {
	line_num_bytes_swapped += size;
	FREE(prog->file_info);
	prog->file_info = 0;
	prog->line_info = 0;
	return 1;
    }
    return 0;
}

/*
 * Reload line number information from swap.
 */
void load_line_numbers P1(program_t *, prog)
{
    int size;

    if (prog->line_info)
	return;

    debug(d_flag, ("Unswap line numbers for /%s\n", prog->name));

    size = swap_in((char **) &prog->file_info, prog->line_swap_index);
    SET_TAG(prog->file_info, TAG_LINENUMBERS);
    prog->line_info = (unsigned char *)&prog->file_info[prog->file_info[1]];
    line_num_bytes_swapped -= size;
}

/**
 **  Misc. routines.
 **/

/*
 * Remove the swap space associated with this object.
 */
void remove_swap_file P1(object_t *, ob)
{
    if (!ob)
	return;

    /* may be swapped out, so swap in to get size, update stats, etc */
    if (ob->flags & O_SWAPPED)
	load_ob_from_swap(ob);
    if (ob->prog)
	free_swap(ob->swap_num, ob->prog->total_size);
    ob->swap_num = -1;
}

/*
 * Same as above, but to remove line_number swap space.
 */
void
remove_line_swap P1(program_t *, prog)
{
    if (!prog->line_info)
	load_line_numbers(prog);
    if (prog->line_swap_index != -1 && prog->line_info)
	free_swap(prog->line_swap_index,
		  prog->file_info[0]);
    prog->line_swap_index = -1;
}

void print_swap_stats P1(outbuffer_t *, out)
{
    int size, cnt, end;
    sw_block_t *m;

    outbuf_add(out, "Swap information:\n");
    outbuf_add(out, "-------------------------\n");
    outbuf_addv(out, "Progs swapped:       %10lu\n", num_swapped);
    outbuf_addv(out, "Linenum bytes:       %10lu\n", line_num_bytes_swapped);
    outbuf_addv(out, "Total bytes swapped: %10lu\n", total_bytes_swapped);
    if (!swap_file) {
	outbuf_add(out, "No swap file\n");
	return;
    }
    size = cnt = 0;
    for (m = swap_free; m; size += m->length, cnt++, m = m->next);
    swap_seek(0, 2);
#ifdef SWAP_USE_FD
    end = tell(swap_file) - last_data;
#else
    end = ftell(swap_file) - last_data;
#endif
    if (end) {
	size += end;
	cnt++;
    }
    outbuf_addv(out, "Freed bytes:         %10lu (%d chunks)\n", size, cnt);
}

/*
 * This one is called at shutdown. Remove the swap file.
 */
void unlink_swap_file()
{
    if (swap_file == 0)
	return;
#ifdef SWAP_USE_FD
    close(swap_file);
    unlink(file_name);
#else
    unlink(file_name);  /* why is this backwards ? */
    fclose(swap_file);
#endif
}



