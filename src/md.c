#include "std.h"
#include "md.h"
#ifdef DEBUGMALLOC_EXTENSIONS
#include "program.h"
#include "lpc_incl.h"
#include "comm.h"
#include "lex.h"
#include "simul_efun.h"
#endif

/*
   note: do not use MALLOC() etc. in this module.  Unbridled recursion
   will occur.  (use malloc() etc. instead)

   This module introduces quite a lot of overhead but it can be useful
   for tracking down memory leaks or for catching the freeing on non-malloc'd
   data.
*/

#ifdef DEBUGMALLOC

#define LEFT_MAGIC(node) ((node)->magic)
#define RIGHT_MAGIC_ADDR(node) ((char *)(node) + sizeof(node_t) + (node)->size)
#define STORE_RIGHT_MAGIC(node) \
     *(RIGHT_MAGIC_ADDR(node)) = (char)(MD_MAGIC >> 24) & 0xff; \
     *(RIGHT_MAGIC_ADDR(node)+1) = (char)(MD_MAGIC >> 16) & 0xff; \
     *(RIGHT_MAGIC_ADDR(node)+2) = (char)(MD_MAGIC >> 8) & 0xff; \
     *(RIGHT_MAGIC_ADDR(node)+3) = (char)MD_MAGIC & 0xff
                               

static totals[MAX_CATEGORY];
static blocks[MAX_CATEGORY];

static char *sources[] = { 
    "*", "temporary blocks", "permanent blocks", "compiler blocks", 
    "data blocks", "miscellaneous blocks", "<#6>", "<#7>", "<#8>", "<#9>",
    "<#10>", "program blocks", "call_out blocks", "interactives", "ed blocks", 
    "error_contexts", "include list", "permanent identifiers", 
    "identifier hash table", "reserved block", "mudlib stats", "objects",
    "object table", "config table", "simul_efuns", "sentences", "string table",
    "free swap blocks", "uids", "object names", "predefines", "line numbers",
    "compiler local blocks",
    "<#33>", "<#34>", "<#35>", "<#36>", "<#37>", "<#38>", "<#39>", 
    "malloc'ed strings", "shared strings", "function pointers", "arrays",
    "mappings", "mapping nodes", "mapping tables"
};

int malloc_mask = 121;

static node_t **table;
unsigned int total_malloced = 0L;
unsigned int hiwater = 0L;

void MDinit()
{
    int j;

    table = (node_t **) calloc(TABLESIZE, sizeof(node_t *));
    for (j = 0; j < MAX_CATEGORY; j++) {
	totals[j] = 0;
    }
}

void
MDmalloc P4(node_t *, node, int, size, int, tag, char *, desc)
{
    unsigned int h;
    static int count = 0;

    total_malloced += size;
    if (total_malloced > hiwater) {
	hiwater = total_malloced;
    }
    h = (unsigned int) node % TABLESIZE;
    node->size = size;
    node->next = table[h];
#ifdef CHECK_MEMORY
    LEFT_MAGIC(node) = MD_MAGIC;
    STORE_RIGHT_MAGIC(node);
#endif
#ifdef DEBUGMALLOC_EXTENSIONS
    if ((tag & 0xff) < MAX_CATEGORY) {
	totals[tag & 0xff] += size;
	blocks[tag & 0xff]++;
    }
    if (((tag >> 8) & 0xff) < MAX_CATEGORY) {
	totals[(tag >> 8) & 0xff] += size;
	blocks[(tag >> 8) & 0xff]++;
    }
    node->tag = tag;
    node->id = count++;
    node->desc = desc ? desc : "default";
    if (malloc_mask == node->tag) {
	fprintf(stderr, "MDmalloc: %5d, [%-25s], %8x:(%d)\n",
		node->tag, node->desc, (unsigned int) PTR(node), node->size);
	fflush(stderr);
    }
#endif
    table[h] = node;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void set_tag P2(void *, ptr, int, tag) {
    node_t *node = PTR_TO_NODET(ptr);
    
    if ((node->tag & 0xff) < MAX_CATEGORY) {
	totals[node->tag & 0xff] -= node->size;
	blocks[node->tag & 0xff]--;
    }
    if (((node->tag >> 8) & 0xff) < MAX_CATEGORY) {
	totals[(node->tag >> 8) & 0xff] -= node->size;
	blocks[(node->tag >> 8) & 0xff]--;
    }
    node->tag = tag;
    if ((node->tag & 0xff) < MAX_CATEGORY) {
	totals[node->tag & 0xff] += node->size;
	blocks[node->tag & 0xff]++;
    }
    if (((node->tag >> 8) & 0xff) < MAX_CATEGORY) {
	totals[(node->tag >> 8) & 0xff] += node->size;
	blocks[(node->tag >> 8) & 0xff]++;
    }
}
#endif

int
MDfree P1(void *, ptr)
{
    unsigned int h;
    int tmp;
    node_t *entry, **oentry;

    h = (unsigned int) ptr % TABLESIZE;
    oentry = &table[h];
    for (entry = *oentry; entry; oentry = &entry->next, entry = *oentry) {
	if (entry == ptr) {
	    *oentry = entry->next;
	    total_malloced -= entry->size;
	    break;
	}
    }
    if (entry) {
#ifdef CHECK_MEMORY
	if (LEFT_MAGIC(entry) != MD_MAGIC) {
	    fprintf(stderr, "MDfree: left side of entry corrupt: %s %04x\n", entry->desc, (int)entry->tag);
	}
	COPY_INT(&tmp, RIGHT_MAGIC_ADDR(entry));
	if (tmp != MD_MAGIC) {
	    fprintf(stderr, "MDfree: right side of entry corrupt: %s %04x\n", entry->desc, (int)entry->tag);
	}
#endif
#ifdef DEBUGMALLOC_EXTENSIONS
	if ((entry->tag & 0xff) < MAX_CATEGORY) {
	    totals[entry->tag & 0xff] -= entry->size;
	    blocks[entry->tag & 0xff]--;
	}
	if (((entry->tag >> 8) & 0xff) < MAX_CATEGORY) {
	    totals[(entry->tag >> 8) & 0xff] -= entry->size;
	    blocks[(entry->tag >> 8) & 0xff]--;
	}
	if (malloc_mask == entry->tag) {
	    fprintf(stderr, "MDfree: %5d, [%-25s], %8x:(%d)\n",
	    entry->tag, entry->desc, (unsigned int) PTR(entry), entry->size);
	    fflush(stderr);
	}
#endif
    } else {
	fprintf(stderr,
	     "md: debugmalloc: attempted to free non-malloc'd pointer %04x\n",
		(unsigned int) ptr);
#ifdef DEBUG
	abort();
#endif
	return 0;
    }
    return 1;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void dump_debugmalloc P2(char *, tfn, int, mask)
{
    int j, total = 0, chunks = 0, total2 = 0;
    char *fn;
    node_t *entry;
    FILE *fp;

    fn = check_valid_path(tfn, current_object, "debugmalloc", 1);
    if (!fn) {
	add_vmessage("Invalid path '%s' for writing.\n", tfn);
	return;
    }
    fp = fopen(fn, "w");
    if (!fp) {
	add_vmessage("Unable to open %s for writing.\n", fn);
	return;
    }
    add_vmessage("Dumping to %s ...", fn);
    for (j = 0; j < TABLESIZE; j++) {
	for (entry = table[j]; entry; entry = entry->next) {
	    if (!mask || (entry->tag == mask)) {
		fprintf(fp, "%-30s: sz %7d: id %6d: tag %08x, a %8x\n",
			entry->desc, entry->size, entry->id, entry->tag,
			(unsigned int) PTR(entry));
		total += entry->size;
		chunks++;
	    }
	}
    }
    fprintf(fp, "total =    %8d\n", total);
    fprintf(fp, "# chunks = %8d\n", chunks);
    fprintf(fp, "ave. bytes per chunk = %7.2f\n\n", (float) total / chunks);
    fprintf(fp, "categories:\n\n");
    for (j = 0; j < MAX_CATEGORY; j++) {
	fprintf(fp, "%4d: %10d\n", j, totals[j]);
	total2 += totals[j];
    }
    fprintf(fp, "\ntotal = %11d\n", total2);
    fclose(fp);
    add_message(" done.\n");
    add_vmessage("total =    %8d\n", total);
    add_vmessage("# chunks = %8d\n", chunks);
    if (chunks) {
	add_vmessage("ave. bytes per chunk = %7.2f\n", (float) total / chunks);
    }
}
#endif				/* DEBUGMALLOC_EXTENSIONS */

void set_malloc_mask P1(int, mask)
{
    malloc_mask = mask;
}

#ifdef DEBUGMALLOC_EXTENSIONS
static void mark_object P1(struct object *, ob) {
    struct sentence *sent;
    int i;

    if (ob->prog)
	ob->prog->p.i.extra_ref++;

    if (ob->name) {
	DO_MARK(ob->name, TAG_OBJ_NAME);
    }

#ifdef PRIVS
    if (ob->privs)
	EXTRA_REF(BLOCK(ob->privs))++;
#endif

#ifndef NO_ADD_ACTION
    if (ob->living_name)
	EXTRA_REF(BLOCK(ob->living_name))++;

    sent = ob->sent;
    
    while (sent) {
	DO_MARK(sent, TAG_SENTENCE);
	if (sent->flags & V_FUNCTION)
	    sent->function.f->extra_ref++;
	else {
	    if (sent->function.s)
		EXTRA_REF(BLOCK(sent->function.s))++;
	}
	if (sent->verb)
	    EXTRA_REF(BLOCK(sent->verb))++;
	sent = sent->next;
    }
#endif
    
    if (ob->prog)
	for (i = 0; i < ob->prog->p.i.num_variables; i++)
	    mark_svalue(&ob->variables[i]);
    else
	add_vmessage("can't mark variables; %s is swapped.\n",
		    ob->name);
}

void mark_svalue P1(struct svalue *, sv) {
    switch (sv->type) {
    case T_OBJECT:
	sv->u.ob->extra_ref++;
	break;
    case T_POINTER:
	sv->u.vec->extra_ref++;
	break;
    case T_MAPPING:
	sv->u.map->extra_ref++;
	break;
    case T_FUNCTION:
	sv->u.fp->extra_ref++;
	break;
    case T_BUFFER:
	sv->u.buf->extra_ref++;
	break;
    case T_STRING:
	switch (sv->subtype) {
	case STRING_MALLOC:
	    DO_MARK(sv->u.string, TAG_STRING);
	    break;
	case STRING_SHARED:
	    EXTRA_REF(BLOCK(sv->u.string))++;
	    break;
	}
    }    
}

static void mark_funp P1(struct funp*, fp) {
#ifdef NEW_FUNCTIONS
    fp->owner->extra_ref++;
    if (fp->type == FP_CALL_OTHER)
      mark_svalue(&fp->f.obj);
    else if ((fp->type & 0x0f) == FP_FUNCTIONAL) {
      fp->f.a.prog->p.i.extra_func_ref++;
    }
    mark_svalue(&fp->args);
#else
    mark_svalue(&fp->obj);
    mark_svalue(&fp->fun);
#endif
}

static void mark_sentence P1(struct sentence *, sent) {
    if (sent->flags & V_FUNCTION) {
      if (sent->function.f)
          mark_funp(sent->function.f);
    } else {
      if (sent->function.s)
          EXTRA_REF(BLOCK(sent->function.s))++;
    }
    if (sent->verb)
      EXTRA_REF(BLOCK(sent->verb))++;
}

static int print_depth = 0;

static void md_print_vector  P1(struct vector *, vec) {
    int i;

    add_message("({ ");
    for (i=0; i < vec->size; i++) {
      switch (vec->item[i].type) {
      case T_INVALID:
          add_message("INVALID");
          break;
      case T_NUMBER:
          add_vmessage("%d", vec->item[i].u.number);
          break;
      case T_REAL:
          add_vmessage("%f", vec->item[i].u.real);
          break;
      case T_STRING:
          add_vmessage("\"%s\"", vec->item[i].u.string);
          break;
      case T_POINTER:
          if (print_depth < 2) {
              print_depth++;
              md_print_vector(vec->item[i].u.vec);
          } else {
              add_message("({ ... })");
          }
          break;
      case T_BUFFER:
          add_message("<buffer>");
          break;
      case T_FUNCTION:
          add_message("<function>");
          break;
      case T_MAPPING:
          add_message("<mapping>");
          break;
      case T_OBJECT:
          add_vmessage("OBJ(%s)", vec->item[i].u.ob->name);
          break;
      }
      if (i != vec->size - 1) add_message(", ");
    }
    add_message(" })\n");
    print_depth--;
}

/* The following types of blocks aren't checked for leaks yet:
   TAG_SWAP
 */

void check_all_blocks P1(int, flag) {
    int i, j, hsh;
    int tmp;
    node_t *entry;
    struct object *ob;
    struct svalue *sv;
    struct vector *vec;
    struct mapping *map;
    struct buffer *buf;
    struct funp *fp;
    struct node *node;
    struct program *prog;
    struct sentence *sent;
    char *ptr;
    block_t *ssbl;
    extern struct svalue apply_ret_value;

    int num = 0, total = 0;
    
    /* need to unswap everything first */
    for (ob = obj_list; ob; ob = ob->next_all) {
      if (ob->flags & O_SWAPPED)
          load_ob_from_swap(ob);
    }

    add_message("Performing memory tests ...\n");
    
#if 0
    for (hsh = 0; hsh < TABLESIZE; hsh++) {
	for (entry = table[hsh]; entry; entry = entry->next) {
	    if (strcmp(entry->desc, "apply_low")==0) {
		if (findstring(PTR(entry)))
		    num++;
		total++;
	    }
	}
    }
    add_vmessage("fraction: %d/%d\n", num, total);
#endif
    
    for (hsh = 0; hsh < TABLESIZE; hsh++) {
	for (entry = table[hsh]; entry; entry = entry->next) {
	    entry->tag &= ~TAG_MARKED;
#ifdef CHECK_MEMORY
	    if (LEFT_MAGIC(entry) != MD_MAGIC) {
		add_vmessage("WARNING: left side of entry corrupt: %s %04x\n", entry->desc, (int)entry->tag);
	    }
	    COPY_INT(&tmp, RIGHT_MAGIC_ADDR(entry));
	    if (tmp != MD_MAGIC) {
		add_vmessage("WARNING: right side of entry corrupt: %s %04x\n", entry->desc, (int)entry->tag);
	    }
#endif
	    switch (entry->tag & 0xff00) {
	    case TAG_TEMPORARY:
		add_vmessage("WARNING: Found temporary block: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_COMPILER:
		add_vmessage("Found compiler block: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_MISC:
		add_vmessage("Found miscellaneous block: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    }
	    switch (entry->tag) {
	    case TAG_OBJECT:
		ob = NODET_TO_PTR(entry, struct object *);
		ob->extra_ref = 0;
		break;
	    case TAG_PROGRAM:
		prog = NODET_TO_PTR(entry, struct program *);
		prog->p.i.extra_ref = 0;
		prog->p.i.extra_func_ref = 0;
		break;
	    case TAG_SHARED_STRING:
		ssbl = NODET_TO_PTR(entry, block_t *);
		EXTRA_REF(ssbl) = 0;
		break;
	    case TAG_VECTOR: 
		vec = NODET_TO_PTR(entry, struct vector *);
		vec->extra_ref = 0;
		break;
	    case TAG_MAPPING:
		map = NODET_TO_PTR(entry, struct mapping *);
		map->extra_ref = 0;
		break;
	    case TAG_FUNP:
		fp = NODET_TO_PTR(entry, struct funp *);
		fp->extra_ref = 0;
		break;
	    case TAG_BUFFER:
		buf = NODET_TO_PTR(entry, struct buffer *);
		buf->extra_ref = 0;
		break;
	    }
#if 0
	    if (entry->tag == TAG_STRING
		&& strcmp(entry->desc, "assign_svalue_no_free")
		&& strcmp(entry->desc, "restore_string")) {
		if (findstring(PTR(entry))) {
		    add_vmessage("Malloc'ed string is also shared: %s %04x:\n\"%s\"\n", entry->desc, (int)entry->tag, PTR(entry));
		}
	    }
#endif
	}
    }
    
    /* the easy ones to find */
    if (blocks[TAG_SIMULS & 0xff] > 2)
	add_message("WARNING: more than two simul_efun tables allocated.\n");
    if (blocks[TAG_INC_LIST & 0xff] > 1)
	add_message("WARNING: more than one include list allocated.\n");
    if (blocks[TAG_IDENT_TABLE & 0xff] > 1)
	add_message("WARNING: more than one identifier hash table allocated.\n");
    if (blocks[TAG_RESERVED & 0xff] > 1)
	add_message("WARNING: more than one reserved block allocated.\n");
    if (blocks[TAG_OBJ_TBL & 0xff] > 1)
	add_message("WARNING: more than object table allocated.\n");
    if (blocks[TAG_CONFIG & 0xff] > 1)
	add_message("WARNING: more than config file table allocated.\n");
    if (blocks[TAG_STR_TBL & 0xff] > 1)
	add_message("WARNING: more than string table allocated.\n");
    if (totals[TAG_CALL_OUT & 0xff] != print_call_out_usage(-1))
	add_message("WARNING: wrong number of call_out blocks allocated.\n");
    if (blocks[TAG_LOCALS & 0xff] > 3)
	add_message("WARNING: more than 3 local blocks allocated.\n");

    if (blocks[TAG_SENTENCE & 0xff] != tot_alloc_sentence)
	add_vmessage("WARNING: tot_alloc_sentence is: %i should be: %i\n",
		     tot_alloc_sentence, blocks[TAG_SENTENCE & 0xff]);
    if (blocks[TAG_OBJECT & 0xff] != tot_alloc_object)
	add_vmessage("WARNING: tot_alloc_object is: %i should be: %i\n",
		     tot_alloc_object, blocks[TAG_OBJECT & 0xff]);
    if (blocks[TAG_PROGRAM & 0xff] != total_num_prog_blocks)
	add_vmessage("WARNING: total_num_prog_blocks is: %i should be: %i\n",
		     total_num_prog_blocks, blocks[TAG_PROGRAM & 0xff]);
    if (blocks[TAG_VECTOR & 0xff] != num_arrays)
	add_vmessage("WARNING: num_arrays is: %i should be: %i\n",
		     num_arrays, blocks[TAG_VECTOR & 0xff]);
    if (blocks[TAG_MAPPING & 0xff] != num_mappings)
	add_vmessage("WARNING: num_mappings is: %i should be: %i\n",
		     num_mappings, blocks[TAG_MAPPING & 0xff]);
    if (blocks[TAG_MAP_TBL & 0xff] != num_mappings)
	add_vmessage("WARNING: %i tables for %i mappings\n",
		     blocks[TAG_MAP_TBL & 0xff], num_mappings);
    if (blocks[TAG_MAP_NODE & 0xff] != total_mapping_nodes)
	add_vmessage("WARNING: total_mapping_nodes is: %i should be: %i\n",
		     total_mapping_nodes, blocks[TAG_MAP_NODE & 0xff]);
    if (blocks[TAG_INTERACTIVE & 0xff] != total_users)
	add_vmessage("WATNING: total_users is: %i should be: %i\n",
		     total_users, blocks[TAG_INTERACTIVE & 0xff]);
    if (blocks[TAG_SHARED_STRING & 0xff] != num_distinct_strings)
	add_vmessage("WARNING: num_distinct_strings is: %i should be: %i\n",
		     num_distinct_strings, blocks[TAG_SHARED_STRING & 0xff]);
    
    /* now do a mark and sweep check to see what should be alloc'd */
    for (i = 0; i < MAX_USERS; i++)
	if (all_users[i]) {
	    DO_MARK(all_users[i], TAG_INTERACTIVE);
	    all_users[i]->ob->extra_ref++;
	    if (all_users[i]->input_to) {
		all_users[i]->input_to->ob->extra_ref++;
		mark_sentence(all_users[i]->input_to);
		if (all_users[i]->num_carry) {
		    for (j = 0; j < all_users[i]->num_carry; j++)
			mark_svalue(all_users[i]->carryover + j);
		}
	    }
	    
#ifndef NO_ADD_ACTION
	    if (all_users[i]->iflags & NOTIFY_FAIL_FUNC)
		all_users[i]->default_err_message.f->extra_ref++;
	    else if (all_users[i]->default_err_message.s)
		EXTRA_REF(BLOCK(all_users[i]->default_err_message.s))++;
#endif
	}
    
#ifndef NO_UIDS
    mark_all_uid_nodes();
#endif
#ifndef NO_MUDLIB_STATS
    mark_mudlib_stats();
#endif
    mark_all_defines();
    mark_free_sentences();
    mark_iptable();
    mark_stack();
    mark_call_outs();
    mark_simuls();
    mark_apply_low_cache();

    mark_svalue(&apply_ret_value);

    master_ob->extra_ref++;
    simul_efun_ob->extra_ref++;
    for (ob = obj_list; ob; ob = ob->next_all) {
	ob->extra_ref++;
    }
    
    for (hsh = 0; hsh < TABLESIZE; hsh++) {
	for (entry = table[hsh]; entry; entry = entry->next) {
	    switch (entry->tag & ~TAG_MARKED) {
	    case TAG_IDENT_TABLE: {
		struct ident_hash_elem *hptr, *first;
		struct ident_hash_elem **table;
		int size;
		
		table = NODET_TO_PTR(entry, struct ident_hash_elem **);
		size = (entry->size / 3) / sizeof(struct ident_hash_elem *);
		for (i = 0; i < size; i++) {
		    first = table[i];
		    if (first) {
			hptr = first;
			do {
			    if (!(hptr->token & IHE_RESWORD)) {
				DO_MARK(hptr, TAG_PERM_IDENT);
			    }
			    hptr = hptr->next;
			} while (hptr != first);
		    }
		}
		break;
	    }
	    case TAG_FUNP:
		fp = NODET_TO_PTR(entry, struct funp *);
		mark_funp(fp);
		break;
	    case TAG_VECTOR:		
		vec = NODET_TO_PTR(entry, struct vector *);
		if (entry->size != sizeof(struct vector) + sizeof(struct svalue[1]) * (vec->size - 1))
		    add_vmessage("vector size doesn't match block size: %s %04x\n", entry->desc, (int)entry->tag);
		for (i = 0; i < vec->size; i++) mark_svalue(&vec->item[i]);
		break;
	    case TAG_MAP_NODE:
		node = NODET_TO_PTR(entry, struct node *);
		mark_svalue(node->values);
		mark_svalue(node->values + 1);
		break;
	    case TAG_MAPPING:		
		map = NODET_TO_PTR(entry, struct mapping *);
		DO_MARK(map->table, TAG_MAP_TBL);
		
		i = map->table_size;
		do {
		    for (node = map->table[i]; node; node = node->next) {
			DO_MARK(node, TAG_MAP_NODE);
		    }
		} while (i--);
		break;
	    case TAG_OBJECT:
		ob = NODET_TO_PTR(entry, struct object *);
		mark_object(ob);
		break;
	    case TAG_PROGRAM:
		prog = NODET_TO_PTR(entry, struct program *);
		
		if (prog->p.i.line_info)
		    DO_MARK(prog->p.i.file_info, TAG_LINENUMBERS);
		
		for (i = 0; i < (int) prog->p.i.num_inherited; i++)
		    prog->p.i.inherit[i].prog->p.i.extra_ref++;
		
		for (i = 0; i < (int) prog->p.i.num_functions; i++)
		    if (prog->p.i.functions[i].name)
			EXTRA_REF(BLOCK(prog->p.i.functions[i].name))++;
		
		for (i = 0; i < (int) prog->p.i.num_strings; i++)
		    EXTRA_REF(BLOCK(prog->p.i.strings[i]))++;

		for (i = 0; i < (int) prog->p.i.num_variables; i++)
		    EXTRA_REF(BLOCK(prog->p.i.variable_names[i].name))++;
		
		EXTRA_REF(BLOCK(prog->name))++;
	    }
	}
    }
	
	/* now check */
    for (hsh = 0; hsh < TABLESIZE; hsh++) {
	for (entry = table[hsh]; entry; entry = entry->next) {
	    switch (entry->tag) {
	    case TAG_ERROR_CONTEXT: {
		struct error_context_stack *ec, *tmp;
		
		ec = NODET_TO_PTR(entry, struct error_context_stack *);
		tmp = ecsp;
		while (tmp) {
		    if (tmp == ec) break;
		    tmp = tmp->next;
		}
		if (!tmp) 
		    add_vmessage("WARNING: Found orphan error context: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    }
	    case TAG_MUDLIB_STATS:
		add_vmessage("WARNING: Found orphan mudlib stat block: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_PROGRAM:
		prog = NODET_TO_PTR(entry, struct program *);
		if (prog->p.i.ref != prog->p.i.extra_ref)
		    add_vmessage("Bad ref count for program %s, is %d - should be %d\n", prog->name, prog->p.i.ref, prog->p.i.extra_ref);
		if (prog->p.i.func_ref != prog->p.i.extra_func_ref)
		    add_vmessage("Bad function ref count for program %s, is %d - should be %d\n", prog->name, prog->p.i.func_ref, prog->p.i.extra_func_ref);
		break;
	    case TAG_OBJECT:
		ob = NODET_TO_PTR(entry, struct object *);
		if (ob->ref != ob->extra_ref)
		    add_vmessage("Bad ref count for object %s, is %d - should be %d\n", ob->name, ob->ref, ob->extra_ref);
		break;
	    case TAG_VECTOR:
		vec = NODET_TO_PTR(entry, struct vector *);
		if (vec->ref != vec->extra_ref) {
		    add_vmessage("Bad ref count for array, is %d - should be %d\n", vec->ref, vec->extra_ref);
		    print_depth = 0;
		    md_print_vector(vec);
		}
		break;
	    case TAG_MAPPING:
		map = NODET_TO_PTR(entry, struct mapping *);
		if (map->ref != map->extra_ref)
		    add_vmessage("Bad ref count for mapping, is %d - should be %d\n", map->ref, map->extra_ref);
		break;
	    case TAG_FUNP:
		fp = NODET_TO_PTR(entry, struct funp *);
		if (fp->ref != fp->extra_ref)
		    add_vmessage("Bad ref count for function pointer, is %d - should be %d\n", fp->ref, fp->extra_ref);
		break;
	    case TAG_BUFFER:
		buf = NODET_TO_PTR(entry, struct buffer *);
		if (buf->ref != buf->extra_ref)
		    add_vmessage("Bad ref count for buffer, is %d - should be %d\n", buf->ref, buf->extra_ref);
		break;
	    case TAG_PREDEFINES:
		add_vmessage("WARNING: Found orphan predefine: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_LINENUMBERS:
		add_vmessage("WARNING: Found orphan line number block: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_OBJ_NAME:
		add_vmessage("WARNING: Found orphan object name: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_INTERACTIVE:
		add_vmessage("WARNING: Found orphan interactive: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_UID:
		add_vmessage("WARNING: Found orphan uid node: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_SENTENCE:
		sent = NODET_TO_PTR(entry, struct sentence *);
		add_vmessage("WARNING: Found orphan sentence: %s:%s - %s %04x\n", sent->ob->name, sent->function, entry->desc, (int)entry->tag);
		break;
	    case TAG_PERM_IDENT:
		add_vmessage("WARNING: Found orphan permanent identifier: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_STRING: 
		ptr = NODET_TO_PTR(entry, char *);
		add_vmessage("WARNING: Found orphan malloc'ed string: \"%s\" - %s %04x\n", ptr, entry->desc, (int)entry->tag);
		break;
	    case TAG_SHARED_STRING:
		ssbl = NODET_TO_PTR(entry, block_t *);
		if (REFS(ssbl) != EXTRA_REF(ssbl))
		    add_vmessage("Bad ref count for shared string \"%s\", is %d - should be %d\n", STRING(ssbl), REFS(ssbl), EXTRA_REF(ssbl));
		break;
	    case TAG_ED:
		add_vmessage("Found allocated ed block: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_MAP_TBL:
		add_vmessage("WARNING: Found orphan mapping table: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    case TAG_MAP_NODE:
		add_vmessage("WARNING: Found orphan mapping table: %s %04x\n", entry->desc, (int)entry->tag);
		break;
	    }
	    entry->tag &= ~TAG_MARKED;
	}
    }

    if (flag) {
	add_message("\n\n");
	add_message("      source         blks  total\n");
	add_message("-------------------- ---- --------\n");
	for (i = 1; i < MAX_CATEGORY; i++) {
	    if (totals[i])
		add_vmessage("%30s %4d %8d\n", sources[i], blocks[i], totals[i]);
	    if (i == 5) add_message("\n");
	}
    }
}
#endif                          /* DEBUGMALLOC_EXTENSIONS */
#endif				/* DEBUGMALLOC */
