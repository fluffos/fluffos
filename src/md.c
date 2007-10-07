#include "std.h"
#include "lpc_incl.h"
#include "file.h"
#include "md.h"
#ifdef DEBUGMALLOC_EXTENSIONS
#include "comm.h"
#include "lex.h"
#include "simul_efun.h"
#include "call_out.h"
#include "mapping.h"
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
#include "socket_efuns.h"
#endif
#endif
#include "master.h"

#ifdef PACKAGE_PARSER
#include "packages/parser.h"
#endif

/*
   note: do not use MALLOC() etc. in this module.  Unbridled recursion
   will occur.  (use malloc() etc. instead)

   This module introduces quite a lot of overhead but it can be useful
   for tracking down memory leaks or for catching the freeing on non-malloc'd
   data.  This module could easily be extended to allow the malloced memory
   chunks to be tagged with a string label.
*/

#ifdef DEBUGMALLOC

#define LEFT_MAGIC(node) ((node)->magic)
#define RIGHT_MAGIC_ADDR(node) ((unsigned char *)(node) + sizeof(md_node_t) + (node)->size)
#define STORE_RIGHT_MAGIC(node) \
     *(RIGHT_MAGIC_ADDR(node)) = (char)(MD_MAGIC >> 24) & 0xff; \
     *(RIGHT_MAGIC_ADDR(node)+1) = (char)(MD_MAGIC >> 16) & 0xff; \
     *(RIGHT_MAGIC_ADDR(node)+2) = (char)(MD_MAGIC >> 8) & 0xff; \
     *(RIGHT_MAGIC_ADDR(node)+3) = (char)MD_MAGIC & 0xff
#define FETCH_RIGHT_MAGIC(l, node) \
     l = (*(RIGHT_MAGIC_ADDR(node)) << 24) + \
         (*(RIGHT_MAGIC_ADDR(node) + 1) << 16) + \
         (*(RIGHT_MAGIC_ADDR(node) + 2) << 8) + \
         (*(RIGHT_MAGIC_ADDR(node) + 3))

int totals[MAX_CATEGORY];
int blocks[MAX_CATEGORY];

static char *sources[] = { 
    "*", "temporary blocks", "permanent blocks", "compiler blocks", 
    "data blocks", "miscellaneous blocks", "<#6>", "<#7>", "<#8>", "<#9>",
    "<#10>", "program blocks", "call_out blocks", "interactives", "ed blocks", 
    "<#15>", "include list", "permanent identifiers", 
    "identifier hash table", "reserved block", "mudlib stats", "objects",
    "object table", "config table", "simul_efuns", "sentences", "string table",
    "free swap blocks", "uids", "object names", "predefines", "line numbers",
    "compiler local blocks", "compiled program", "users", "debugmalloc overhead",
    "heart_beat list", "parser", "input_to", "sockets", 
    "strings", "malloc strings", "shared strings", "function pointers", "arrays",
    "mappings", "mapping nodes", "mapping tables", "buffers", "classes"
};

int malloc_mask = 121;

static md_node_t **table;
unsigned int total_malloced = 0L;
unsigned int hiwater = 0L;

#ifdef DEBUGMALLOC_EXTENSIONS
void check_all_blocks (int);

outbuffer_t out;

void MDmemcheck();

void MDmemcheck() {
    check_all_blocks(2);
    if (out.buffer)
        fatal("Internal memory check failed: %s\n", out.buffer);
}
#endif

void MDinit()
{
    int j;

    table = (md_node_t **) calloc(MD_TABLE_SIZE, sizeof(md_node_t *));
    for (j = 0; j < MAX_CATEGORY; j++) {
        totals[j] = 0;
    }
}

void
MDmalloc (md_node_t * node, int size, int tag, char * desc)
{
    unsigned long h;
    static int count = 0;

    if (!size) {
        debug_message("md: debugmalloc: attempted to allocate zero bytes\n");
#ifdef DEBUG
        abort();
#endif
        return;
    }
    total_malloced += size;
    if (total_malloced > hiwater) {
        hiwater = total_malloced;
    }
    h = MD_HASH(node);
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
        debug_message("MDmalloc: %5d, [%-25s], %8lx:(%d)\n",
                node->tag, node->desc, PTR(node), node->size);
    }
#endif
    table[h] = node;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void set_tag (const void * ptr, int tag) {
    md_node_t *node = PTR_TO_NODET(ptr);
    
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
MDfree (void * ptr)
{
    unsigned long h;
    int tmp;
    md_node_t *entry, **oentry;

    h = MD_HASH(ptr);
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
            debug_message("MDfree: left side of entry corrupt: %s %04x at %lx\n", entry->desc, entry->tag, entry);
        }
        FETCH_RIGHT_MAGIC(tmp, entry);
        if (tmp != MD_MAGIC) {
            debug_message("MDfree: right side of entry corrupt: %s %04x at %lx\n", entry->desc, entry->tag, entry);
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
            debug_message("MDfree: %5d, [%-25s], %8lx:(%d)\n",
            entry->tag, entry->desc, (unsigned long) PTR(entry), entry->size);
        }
#endif
    } else {
        debug_message("md: debugmalloc: attempted to free non-malloc'd pointer %08lx\n",
                (unsigned long) ptr);
#ifdef DEBUG
        abort();
#endif
        return 0;
    }
    return 1;
}

#ifdef DEBUGMALLOC_EXTENSIONS
char *dump_debugmalloc (char * tfn, int mask)
{
    int j, total = 0, chunks = 0, total2 = 0;
    const char *fn;
    md_node_t *entry;
    FILE *fp;

    outbuf_zero(&out);
    fn = check_valid_path(tfn, current_object, "debugmalloc", 1);
    if (!fn) {
        error("Invalid path '%s' for writing.\n", tfn);
    }
    fp = fopen(fn, "w");
    if (!fp) {
        error("Unable to open %s for writing.\n", fn);
    }
    for (j = 0; j < MD_TABLE_SIZE; j++) {
        for (entry = table[j]; entry; entry = entry->next) {
            if (!mask || (entry->tag == mask)) {
                fprintf(fp, "%-30s: sz %7d: id %6d: tag %08x, a %8lx\n",
                        entry->desc, entry->size, entry->id, entry->tag,
                        (unsigned long) PTR(entry));
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
    outbuf_addv(&out, "total =    %8d\n", total);
    outbuf_addv(&out, "# chunks = %8d\n", chunks);
    if (chunks) {
        outbuf_addv(&out, "ave. bytes per chunk = %7.2f\n", (float) total / chunks);
    }
    outbuf_fix(&out);
    return out.buffer;
}
#endif                          /* DEBUGMALLOC_EXTENSIONS */

void set_malloc_mask (int mask)
{
    malloc_mask = mask;
}

#ifdef DEBUGMALLOC_EXTENSIONS
static void mark_object (object_t * ob) {
#ifndef NO_ADD_ACTION
    sentence_t *sent;
#endif
    int i;

    if (ob->prog)
        ob->prog->extra_ref++;

    if (ob->obname) {
        DO_MARK(ob->obname, TAG_OBJ_NAME);
    }

    if (ob->replaced_program)
        EXTRA_REF(BLOCK(ob->replaced_program))++;

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
            sent->function.f->hdr.extra_ref++;
        else {
            if (sent->function.s)
                EXTRA_REF(BLOCK(sent->function.s))++;
        }
        if (sent->verb)
            EXTRA_REF(BLOCK(sent->verb))++;
        sent = sent->next;
    }
#endif

#ifdef PACKAGE_PARSER
    if (ob->pinfo)
        parser_mark(ob->pinfo);
#endif    

    if (ob->prog)
        for (i = 0; i < ob->prog->num_variables_total; i++)
            mark_svalue(&ob->variables[i]);
    else
        outbuf_addv(&out, "can't mark variables; %s is swapped.\n",
                    ob->obname);
}

void mark_svalue (svalue_t * sv) {
    switch (sv->type) {
    case T_OBJECT:
        sv->u.ob->extra_ref++;
        break;
    case T_ARRAY:
        sv->u.arr->extra_ref++;
        break;
    case T_CLASS:
        sv->u.arr->extra_ref++;
        break;
    case T_MAPPING:
        sv->u.map->extra_ref++;
        break;
    case T_FUNCTION:
        sv->u.fp->hdr.extra_ref++;
        break;
#ifndef NO_BUFFER_TYPE
    case T_BUFFER:
        sv->u.buf->extra_ref++;
        break;
#endif
    case T_STRING:
        switch (sv->subtype) {
        case STRING_MALLOC:
            MSTR_EXTRA_REF(sv->u.string)++;
            break;
        case STRING_SHARED:
            EXTRA_REF(BLOCK(sv->u.string))++;
            break;
        }
    }    
}

static void mark_funp (funptr_t* fp) {
    if (fp->hdr.args)
        fp->hdr.args->extra_ref++;

    if (fp->hdr.owner)
        fp->hdr.owner->extra_ref++;
    switch (fp->hdr.type) {
        case FP_LOCAL | FP_NOT_BINDABLE:
            if (fp->hdr.owner)
                fp->hdr.owner->prog->extra_func_ref++;
            break;
        case FP_FUNCTIONAL:
        case FP_FUNCTIONAL | FP_NOT_BINDABLE:
            fp->f.functional.prog->extra_func_ref++;
            break;
    }
}

static void mark_sentence (sentence_t * sent) {
    if (sent->flags & V_FUNCTION) {
      if (sent->function.f)
          sent->function.f->hdr.extra_ref++;
    } else {
      if (sent->function.s)
          EXTRA_REF(BLOCK(sent->function.s))++;
    }
#ifndef NO_ADD_ACTION
    if (sent->verb)
      EXTRA_REF(BLOCK(sent->verb))++;
#endif
}

static int print_depth = 0;

static void md_print_array  (array_t * vec) {
    int i;

    outbuf_add(&out, "({ ");
    for (i=0; i < vec->size; i++) {
      switch (vec->item[i].type) {
      case T_INVALID:
          outbuf_add(&out, "INVALID");
          break;
      case T_NUMBER:
          outbuf_addv(&out, "%ld", vec->item[i].u.number);
          break;
      case T_REAL:
          outbuf_addv(&out, "%f", vec->item[i].u.real);
          break;
      case T_STRING:
          outbuf_addv(&out, "\"%s\"", vec->item[i].u.string);
          break;
      case T_ARRAY:
          if (print_depth < 2) {
              print_depth++;
              md_print_array(vec->item[i].u.arr);
          } else {
              outbuf_add(&out, "({ ... })");
          }
          break;
      case T_CLASS:
          outbuf_add(&out, "<class>");
          break;
#ifndef NO_BUFFER_TYPE
      case T_BUFFER:
          outbuf_add(&out, "<buffer>");
          break;
#endif
      case T_FUNCTION:
          outbuf_add(&out, "<function>");
          break;
      case T_MAPPING:
          outbuf_add(&out, "<mapping>");
          break;
      case T_OBJECT:
          outbuf_addv(&out, "OBJ(%s)", vec->item[i].u.ob->obname);
          break;
      }
      if (i != vec->size - 1) outbuf_add(&out, ", ");
    }
    outbuf_add(&out, " })\n");
    print_depth--;
}

static void mark_config (void) {
    int i;

    for (i = 0; i < NUM_CONFIG_STRS; i++) {
        DO_MARK(config_str[i], TAG_STRING);
    }
}

#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
void mark_sockets (void) {
    int i;
    char *s;

    for (i = 0; i < max_lpc_socks; i++) {
        if (lpc_socks[i].flags & S_READ_FP) {
            lpc_socks[i].read_callback.f->hdr.extra_ref++;
        } else 
        if ((s = lpc_socks[i].read_callback.s)) {
            EXTRA_REF(BLOCK(s))++;
        }
        if (lpc_socks[i].flags & S_WRITE_FP) {
            lpc_socks[i].write_callback.f->hdr.extra_ref++;
        } else 
        if ((s = lpc_socks[i].write_callback.s)) {
            EXTRA_REF(BLOCK(s))++;
        }
        if (lpc_socks[i].flags & S_CLOSE_FP) {
            lpc_socks[i].close_callback.f->hdr.extra_ref++;
        } else 
        if ((s = lpc_socks[i].close_callback.s)) {
            EXTRA_REF(BLOCK(s))++;
        }
    }
}
#endif

#ifdef STRING_STATS
static int base_overhead = 0;

/* Compute the correct values of allocd_strings, allocd_bytes, and
 * bytes_distinct_strings based on blocks that are actually allocated.
 */
void compute_string_totals (int * asp, int * abp, int * bp) {
    int hsh;
    md_node_t *entry;
    malloc_block_t *msbl;
    block_t *ssbl;

    *asp = 0;
    *abp = 0;
    *bp = 0;
    
    for (hsh = 0; hsh < MD_TABLE_SIZE; hsh++) {
        for (entry = table[hsh]; entry; entry = entry->next) {
            if (entry->tag == TAG_MALLOC_STRING) {
                msbl = NODET_TO_PTR(entry, malloc_block_t*);
                *bp += msbl->size + 1;
                *asp += msbl->ref;
                *abp += msbl->ref * (msbl->size + 1);
            }
            if (entry->tag == TAG_SHARED_STRING) {
                ssbl = NODET_TO_PTR(entry, block_t*);
                *bp += ssbl->size + 1;
                *asp += ssbl->refs;
                *abp += ssbl->refs * (ssbl->size + 1);
            }
        }
    }
}

/*
 * Verify string statistics.  out can be zero, in which case any errors
 * are printed to stdout and abort() is called.  Otherwise the error messages
 * are added to the outbuffer.
 */
void check_string_stats (outbuffer_t * out) {
    int overhead = blocks[TAG_SHARED_STRING & 0xff] * sizeof(block_t)
        + blocks[TAG_MALLOC_STRING & 0xff] * sizeof(malloc_block_t);
    int num = blocks[TAG_SHARED_STRING & 0xff] + blocks[TAG_MALLOC_STRING & 0xff];
    int bytes, as, ab;
    
    compute_string_totals(&as, &ab, &bytes);
    
    if (!base_overhead)
        base_overhead = overhead_bytes - overhead;
    overhead += base_overhead;

    if (num != num_distinct_strings) {
        if (out) {
            outbuf_addv(out, "WARNING: num_distinct_strings is: %i should be: %i\n",
                        num_distinct_strings, num);
        } else {
            printf("WARNING: num_distinct_strings is: %i should be: %i\n",
                   num_distinct_strings, num);
            abort();
        }
    }
    if (overhead != overhead_bytes) {
        if (out) {
            outbuf_addv(out, "WARNING: overhead_bytes is: %i should be: %i\n",
               overhead_bytes, overhead);
        } else {
            printf("WARNING: overhead_bytes is: %i should be: %i\n",
                   overhead_bytes, overhead);
            abort();
        }
    }
    if (bytes != bytes_distinct_strings) {
        if (out) {
            outbuf_addv(out, "WARNING: bytes_distinct_strings is: %i should be: %i\n",
                        bytes_distinct_strings, bytes - (overhead - base_overhead));
        } else {
            printf("WARNING: bytes_distinct_strings is: %i should be: %i\n",
                   bytes_distinct_strings, bytes - (overhead - base_overhead));
            abort();
        }
    }
    if (allocd_strings != as) {
        if (out) {
            outbuf_addv(out, "WARNING: allocd_strings is: %i should be: %i\n",
                        allocd_strings, as);
        } else {
            printf("WARNING: allocd_strings is: %i should be: %i\n",
                   allocd_strings, as);
            abort();
        }
    }
    if (allocd_bytes != ab) {
        if (out) {
            outbuf_addv(out, "WARNING: allocd_bytes is: %i should be: %i\n",
                        allocd_bytes, ab);
        } else {
            printf("WARNING: allocd_bytes is: %i should be: %i\n",
                   allocd_bytes, ab);
            abort();
        }
    }
}
#endif

/* currently: 1 - debug, 2 - suppress leak checks */
void check_all_blocks (int flag) {
    int i, j, hsh;
    int tmp;
    md_node_t *entry;
    object_t *ob;
    array_t *vec;
    mapping_t *map;
#ifndef NO_BUFFER_TYPE
    buffer_t *buf;
#endif
    funptr_t *fp;
    mapping_node_t *node;
    program_t *prog;
    sentence_t *sent;
    char *ptr;
    block_t *ssbl;
    malloc_block_t *msbl;
    extern svalue_t apply_ret_value;

#if 0
    int num = 0, total = 0;
#endif
    
    outbuf_zero(&out);
    if (!(flag & 2))
        outbuf_add(&out, "Performing memory tests ...\n");
    
    for (hsh = 0; hsh < MD_TABLE_SIZE; hsh++) {
        for (entry = table[hsh]; entry; entry = entry->next) {
            entry->tag &= ~TAG_MARKED;
#ifdef CHECK_MEMORY
            if (LEFT_MAGIC(entry) != MD_MAGIC) {
                outbuf_addv(&out, "WARNING: left side of entry corrupt: %s %08lx at %lx\n", entry->desc, entry->tag, entry);
            }
            FETCH_RIGHT_MAGIC(tmp, entry);
            if (tmp != MD_MAGIC) {
                outbuf_addv(&out, "WARNING: right side of entry corrupt: %s %08lx at %lx\n", entry->desc, entry->tag, entry);
            }
#endif
            switch (entry->tag & 0xff00) {
            case TAG_TEMPORARY:
                if (!(flag & 2))
                    outbuf_addv(&out, "WARNING: Found temporary block: %s %04x\n", entry->desc, entry->tag);
                break;
            case TAG_COMPILER:
                if (!(flag & 2))
                    outbuf_addv(&out, "Found compiler block: %s %04x\n", entry->desc, entry->tag);
                break;
            case TAG_MISC:
                outbuf_addv(&out, "Found miscellaneous block: %s %04x\n", entry->desc, entry->tag);
                break;
            }
            switch (entry->tag) {
            case TAG_OBJECT:
                ob = NODET_TO_PTR(entry, object_t *);
                ob->extra_ref = 0;
                break;
            case TAG_PROGRAM:
                prog = NODET_TO_PTR(entry, program_t *);
                prog->extra_ref = 0;
                prog->extra_func_ref = 0;
                break;
            case TAG_MALLOC_STRING:
                {
                    char *str;
                
                    msbl = NODET_TO_PTR(entry, malloc_block_t *);
                    /* don't give an error for the return value we are 
                       constructing :) */
                    if (msbl == MSTR_BLOCK(out.buffer))
                        break;

                    str = (char *)(msbl + 1);
                    msbl->extra_ref = 0;
                    if (msbl->size != USHRT_MAX && msbl->size != strlen(str)) {
                        outbuf_addv(&out, "Malloc'ed string length is incorrect: %s %04x '%s': is: %i should be: %i\n", entry->desc, entry->tag, str, msbl->size, strlen(str));
                    }
                    break;
                }
            case TAG_SHARED_STRING:
                ssbl = NODET_TO_PTR(entry, block_t *);
                EXTRA_REF(ssbl) = 0;
                break;
            case TAG_ARRAY: 
                vec = NODET_TO_PTR(entry, array_t *);
                vec->extra_ref = 0;
                break;
            case TAG_CLASS: 
                vec = NODET_TO_PTR(entry, array_t *);
                vec->extra_ref = 0;
                break;
            case TAG_MAPPING:
                map = NODET_TO_PTR(entry, mapping_t *);
                map->extra_ref = 0;
                break;
            case TAG_FUNP:
                fp = NODET_TO_PTR(entry, funptr_t *);
                fp->hdr.extra_ref = 0;
                break;
#ifndef NO_BUFFER_TYPE
            case TAG_BUFFER:
                buf = NODET_TO_PTR(entry, buffer_t *);
                buf->extra_ref = 0;
                break;
#endif
            }
        }
    }
    
    if (!(flag & 2)) {
        /* the easy ones to find */
        if (blocks[TAG_SIMULS & 0xff] > 3)
            outbuf_add(&out, "WARNING: more than three simul_efun tables allocated.\n");
        if (blocks[TAG_INC_LIST & 0xff] > 1)
            outbuf_add(&out, "WARNING: more than one include list allocated.\n");
        if (blocks[TAG_IDENT_TABLE & 0xff] > 1)
            outbuf_add(&out, "WARNING: more than one identifier hash table allocated.\n");
        if (blocks[TAG_RESERVED & 0xff] > 1)
            outbuf_add(&out, "WARNING: more than one reserved block allocated.\n");
        if (blocks[TAG_OBJ_TBL & 0xff] > 1)
            outbuf_add(&out, "WARNING: more than object table allocated.\n");
        if (blocks[TAG_CONFIG & 0xff] > 1)
            outbuf_add(&out, "WARNING: more than config file table allocated.\n");
        if (blocks[TAG_STR_TBL & 0xff] > 1)
            outbuf_add(&out, "WARNING: more than string table allocated.\n");
        if (totals[TAG_CALL_OUT & 0xff] != print_call_out_usage(&out, -1))
            outbuf_add(&out, "WARNING: wrong number of call_out blocks allocated.\n");
        if (blocks[TAG_LOCALS & 0xff] > 3)
            outbuf_add(&out, "WARNING: more than 3 local blocks allocated.\n");
        
        if (blocks[TAG_SENTENCE & 0xff] != tot_alloc_sentence)
            outbuf_addv(&out, "WARNING: tot_alloc_sentence is: %i should be: %i\n",
                        tot_alloc_sentence, blocks[TAG_SENTENCE & 0xff]);
        if (blocks[TAG_OBJECT & 0xff] != tot_alloc_object)
            outbuf_addv(&out, "WARNING: tot_alloc_object is: %i should be: %i\n",
                        tot_alloc_object, blocks[TAG_OBJECT & 0xff]);
        if (blocks[TAG_PROGRAM & 0xff] != total_num_prog_blocks)
            outbuf_addv(&out, "WARNING: total_num_prog_blocks is: %i should be: %i\n",
                        total_num_prog_blocks, blocks[TAG_PROGRAM & 0xff]);
#ifdef ARRAY_STATS
        if (blocks[TAG_ARRAY & 0xff] != num_arrays)
            outbuf_addv(&out, "WARNING: num_arrays is: %i should be: %i\n",
                        num_arrays, blocks[TAG_ARRAY & 0xff]);
        if (totals[TAG_ARRAY & 0xff] != total_array_size)
            outbuf_addv(&out, "WARNING: total_array_size is: %i should be: %i\n",
                        total_array_size, totals[TAG_ARRAY & 0xff]);
#endif
        if (blocks[TAG_MAPPING & 0xff] != num_mappings)
            outbuf_addv(&out, "WARNING: num_mappings is: %i should be: %i\n",
                        num_mappings, blocks[TAG_MAPPING & 0xff]);
        if (blocks[TAG_MAP_TBL & 0xff] != num_mappings)
            outbuf_addv(&out, "WARNING: %i tables for %i mappings\n",
                        blocks[TAG_MAP_TBL & 0xff], num_mappings);
        if (blocks[TAG_INTERACTIVE & 0xff] != num_user)
            outbuf_addv(&out, "WATNING: num_user is: %i should be: %i\n",
                        num_user, blocks[TAG_INTERACTIVE & 0xff]);
#ifdef STRING_STATS
        check_string_stats(&out);
#endif
        
#ifdef PACKAGE_EXTERNAL
        for (i = 0; i < NUM_EXTERNAL_CMDS; i++) {
            if (external_cmd[i]) {
                DO_MARK(external_cmd[i], TAG_STRING);
            }
        }
#endif
    
        /* now do a mark and sweep check to see what should be alloc'd */
        for (i = 0; i < max_users; i++)
            if (all_users[i]) {
                DO_MARK(all_users[i], TAG_INTERACTIVE);
                all_users[i]->ob->extra_ref++;
                if (all_users[i]->input_to) {
                    all_users[i]->input_to->ob->extra_ref++;
                    DO_MARK(all_users[i]->input_to, TAG_SENTENCE);
                    mark_sentence(all_users[i]->input_to);
                    if (all_users[i]->num_carry) {
                        for (j = 0; j < all_users[i]->num_carry; j++)
                            mark_svalue(all_users[i]->carryover + j);
                    }
                }
                
#ifndef NO_ADD_ACTION
                if (all_users[i]->iflags & NOTIFY_FAIL_FUNC)
                    all_users[i]->default_err_message.f->hdr.extra_ref++;
                else if (all_users[i]->default_err_message.s)
                    EXTRA_REF(BLOCK(all_users[i]->default_err_message.s))++;
#endif
            }

        if (*(DEFAULT_FAIL_MESSAGE)) {
          char buf[8192];
          
          strcpy(buf, DEFAULT_FAIL_MESSAGE);
          strcat(buf, "\n");
          EXTRA_REF(BLOCK(findstring(buf)))++;
        }
        
#ifdef PACKAGE_UIDS
        mark_all_uid_nodes();
#endif
#ifdef PACKAGE_MUDLIB_STATS
        mark_mudlib_stats();
#endif
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
        mark_sockets();
#endif
#ifdef PACKAGE_PARSER
        parser_mark_verbs();
#endif
        mark_file_sv();
        mark_all_defines();
        mark_free_sentences();
        mark_iptable();
        mark_stack();
        mark_command_giver_stack();
        mark_call_outs();
        mark_simuls();
        mark_apply_low_cache();
        mark_mapping_node_blocks();
        mark_config();
        
        mark_svalue(&apply_ret_value);
        
        if (master_ob)
            master_ob->extra_ref++;
        if (simul_efun_ob)
            simul_efun_ob->extra_ref++;
        for (ob = obj_list; ob; ob = ob->next_all) {
            ob->extra_ref++;
        }
        /* objects on obj_list_destruct still have a ref too */
        for (ob = obj_list_destruct; ob; ob = ob->next_all) {
            ob->extra_ref++;
        }
    
        for (hsh = 0; hsh < MD_TABLE_SIZE; hsh++) {
            for (entry = table[hsh]; entry; entry = entry->next) {
                switch (entry->tag & ~TAG_MARKED) {
                case TAG_IDENT_TABLE: {
                    ident_hash_elem_t *hptr, *first;
                    ident_hash_elem_t **table;
                    int size;
                    
                    table = NODET_TO_PTR(entry, ident_hash_elem_t **);
                    size = (entry->size / 3) / sizeof(ident_hash_elem_t *);
                    for (i = 0; i < size; i++) {
                        first = table[i];
                        if (first) {
                            hptr = first;
                            do {
                                if (hptr->token & (IHE_SIMUL | IHE_EFUN)) {
                                    DO_MARK(hptr, TAG_PERM_IDENT);
                                }
                                hptr = hptr->next;
                            } while (hptr != first);
                        }
                    }
                    break;
                }
                case TAG_FUNP:
                    fp = NODET_TO_PTR(entry, funptr_t *);
                    mark_funp(fp);
                    break;
                case TAG_ARRAY:
                    vec = NODET_TO_PTR(entry, array_t *);
                    if (entry->size != sizeof(array_t) + sizeof(svalue_t[1]) * (vec->size - 1))
                        outbuf_addv(&out, "array size doesn't match block size: %s %04x\n", entry->desc, entry->tag);
                    for (i = 0; i < vec->size; i++) mark_svalue(&vec->item[i]);
                    break;
                case TAG_CLASS:
                    vec = NODET_TO_PTR(entry, array_t *);
                    if (entry->size != sizeof(array_t) + sizeof(svalue_t[1]) * (vec->size - 1))
                        outbuf_addv(&out, "class size doesn't match block size: %s %04x\n", entry->desc, entry->tag);
                    for (i = 0; i < vec->size; i++) mark_svalue(&vec->item[i]);
                    break;
                case TAG_MAPPING:               
                    map = NODET_TO_PTR(entry, mapping_t *);
                    DO_MARK(map->table, TAG_MAP_TBL);
                    
                    i = map->table_size;
                    do {
                        for (node = map->table[i]; node; node = node->next) {
                            mark_svalue(node->values);
                            mark_svalue(node->values + 1);
                        }
                    } while (i--);
                    break;
                case TAG_OBJECT:
                    ob = NODET_TO_PTR(entry, object_t *);
                    mark_object(ob);
                    {
                        object_t *tmp = obj_list;
                        while (tmp && tmp != ob)
                            tmp = tmp->next_all;
                        if (!tmp) {
                            tmp = obj_list_destruct;
                            while (tmp && tmp != ob)
                                tmp = tmp->next_all;
                        }
                        if (!tmp) {
                            tmp = obj_list_dangling;
                            while (tmp && tmp != ob)
                                tmp = tmp->next_all;
                            if (tmp)
                                outbuf_addv(&out,
                                        "WARNING: %s is dangling.\n",
                                        ob->obname);
                        }
                        if (!tmp)
                            outbuf_addv(&out, 
                                        "WARNING: %s not in object list.\n",
                                        ob->obname);
                    }
                    break;
                case TAG_LPC_OBJECT:
                    ob = NODET_TO_PTR(entry, object_t *);
                    if (ob->obname) {
                        DO_MARK(ob->obname, TAG_OBJ_NAME);
                    }
                    break;
                case TAG_PROGRAM:
                    prog = NODET_TO_PTR(entry, program_t *);
                    
                    if (prog->line_info) {
                        DO_MARK(prog->file_info, TAG_LINENUMBERS);
                    }
                    
                    for (i = 0; i < prog->num_inherited; i++)
                        prog->inherit[i].prog->extra_ref++;
                    
                    for (i = 0; i < prog->num_functions_defined; i++)
                        if (prog->function_table[i].funcname)
                            EXTRA_REF(BLOCK(prog->function_table[i].funcname))++;
                    
                    for (i = 0; i < prog->num_strings; i++)
                        EXTRA_REF(BLOCK(prog->strings[i]))++;
                    
                    for (i = 0; i < prog->num_variables_defined; i++)
                        EXTRA_REF(BLOCK(prog->variable_table[i]))++;
                    
                    EXTRA_REF(BLOCK(prog->filename))++;
                }
            }
        }
        
        /* now check */
        for (hsh = 0; hsh < MD_TABLE_SIZE; hsh++) {
            for (entry = table[hsh]; entry; entry = entry->next) {
                switch (entry->tag) {
                case TAG_MUDLIB_STATS:
                    outbuf_addv(&out, "WARNING: Found orphan mudlib stat block: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_PROGRAM:
                    prog = NODET_TO_PTR(entry, program_t *);
                    if (prog->ref != prog->extra_ref)
                        outbuf_addv(&out, "Bad ref count for program %s, is %d - should be %d\n", prog->filename, prog->ref, prog->extra_ref);
                    if (prog->func_ref != prog->extra_func_ref)
                        outbuf_addv(&out, "Bad function ref count for program %s, is %d - should be %d\n", prog->filename, prog->func_ref, prog->extra_func_ref);
                    break;
                case TAG_OBJECT:
                    ob = NODET_TO_PTR(entry, object_t *);
                    if (ob->ref != ob->extra_ref)
                        outbuf_addv(&out, "Bad ref count for object %s, is %d - should be %d\n", ob->obname, ob->ref, ob->extra_ref);
                    break;
                case TAG_ARRAY:
                    vec = NODET_TO_PTR(entry, array_t *);
                    if (vec->ref != vec->extra_ref) {
                        outbuf_addv(&out, "Bad ref count for array, is %d - should be %d\n", vec->ref, vec->extra_ref);
                        print_depth = 0;
                        md_print_array(vec);
                    }
                    break;
                case TAG_CLASS:
                    vec = NODET_TO_PTR(entry, array_t *);
                    if (vec->ref != vec->extra_ref)
                        outbuf_addv(&out, "Bad ref count for class, is %d - should be %d\n", vec->ref, vec->extra_ref);
                    break;
                case TAG_MAPPING:
                    map = NODET_TO_PTR(entry, mapping_t *);
                    if (map->ref != map->extra_ref)
                        outbuf_addv(&out, "Bad ref count for mapping, is %d - should be %d\n", map->ref, map->extra_ref);
                    break;
                case TAG_FUNP:
                    fp = NODET_TO_PTR(entry, funptr_t *);
                    if (fp->hdr.ref != fp->hdr.extra_ref)
                        outbuf_addv(&out, "Bad ref count for function pointer (owned by %s), is %d - should be %d\n", (fp->hdr.owner ? fp->hdr.owner->obname : "(null)"), fp->hdr.ref, fp->hdr.extra_ref);
                    break;
#ifndef NO_BUFFER_TYPE
                case TAG_BUFFER:
                    buf = NODET_TO_PTR(entry, buffer_t *);
                    if (buf->ref != buf->extra_ref)
                        outbuf_addv(&out, "Bad ref count for buffer, is %d - should be %d\n", buf->ref, buf->extra_ref);
                    break;
#endif
                case TAG_PREDEFINES:
                    outbuf_addv(&out, "WARNING: Found orphan predefine: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_LINENUMBERS:
                    outbuf_addv(&out, "WARNING: Found orphan line number block: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_OBJ_NAME:
                    outbuf_addv(&out, "WARNING: Found orphan object name: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_INTERACTIVE:
                    outbuf_addv(&out, "WARNING: Found orphan interactive: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_UID:
                    outbuf_addv(&out, "WARNING: Found orphan uid node: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_SENTENCE:
                    sent = NODET_TO_PTR(entry, sentence_t *);
                    outbuf_addv(&out, "WARNING: Found orphan sentence: %s:%s - %s %04x\n", sent->ob->obname, sent->function, entry->desc, entry->tag);
                    break;
                case TAG_PERM_IDENT:
                    outbuf_addv(&out, "WARNING: Found orphan permanent identifier: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_STRING: 
                    ptr = NODET_TO_PTR(entry, char *);
                    outbuf_addv(&out, "WARNING: Found orphan malloc'ed string: \"%s\" - %s %04x\n", ptr, entry->desc, entry->tag);
                    break;
                case TAG_MALLOC_STRING:
                    msbl = NODET_TO_PTR(entry, malloc_block_t *);
                    /* don't give an error for the return value we are 
                       constructing :) */
                    if (msbl == MSTR_BLOCK(out.buffer))
                        break;
                    
                    if (msbl->ref != msbl->extra_ref)
                        outbuf_addv(&out, "Bad ref count for malloc string \"%s\" %s %04x, is %d - should be %d\n", (char *)(msbl + 1), entry->desc, entry->tag, msbl->ref, msbl->extra_ref);
                    break;
                case TAG_SHARED_STRING:
                    ssbl = NODET_TO_PTR(entry, block_t *);
                    if (REFS(ssbl) != EXTRA_REF(ssbl))
                        outbuf_addv(&out, "Bad ref count for shared string \"%s\", is %d - should be %d\n", STRING(ssbl), REFS(ssbl), EXTRA_REF(ssbl));
                    break;
                case TAG_ED:
                    outbuf_addv(&out, "Found allocated ed block: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_MAP_TBL:
                    outbuf_addv(&out, "WARNING: Found orphan mapping table: %s %04x\n", entry->desc, entry->tag);
                    break;
                case TAG_MAP_NODE_BLOCK:
                    outbuf_addv(&out, "WARNING: Found orphan mapping node block: %s %04x\n", entry->desc, entry->tag);
                    break;
                }
                entry->tag &= ~TAG_MARKED;
            }
        }
    }
    
    if (flag & 1) {
        outbuf_add(&out, "\n\n");
        outbuf_add(&out, "      source                    blks   total\n");
        outbuf_add(&out, "------------------------------ ------ --------\n");
        for (i = 1; i < MAX_CATEGORY; i++) {
            if (totals[i])
                outbuf_addv(&out, "%-30s %6d %8d\n", sources[i], blocks[i], totals[i]);
            if (i == 5) outbuf_add(&out, "\n");
        }
    }
    if (!(flag & 2))
        outbuf_push(&out);
    else {
      FREE_MSTR(out.buffer);
      push_number(0);
    }
}
#endif                          /* DEBUGMALLOC_EXTENSIONS */
#endif                          /* DEBUGMALLOC */
