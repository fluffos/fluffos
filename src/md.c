#include <stdio.h>
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "object.h"
#include "md.h"

/*
   note: do not use MALLOC() etc. in this module.  Unbridled recursion
   will occur.  (use malloc() etc. instead)

   This module introduces quite a lot of overhead but it can be useful
   for tracking down memory leaks or for catching the freeing on non-malloc'd
   data.  This module could easily be extended to allow the malloced memory
   chunks to be tagged with a string label.
*/

#ifdef DEBUGMALLOC

static totals[MAX_CATEGORY];

int malloc_mask = 0;

static node_t **table;
unsigned int total_malloced = 0L;
unsigned int hiwater = 0L;

void
MDinit()
{
	int j;

	table = (node_t **)calloc(TABLESIZE, sizeof(node_t *));
	for (j = 0; j < MAX_CATEGORY; j++) {
		totals[j] = 0;
	}
}

void
MDmalloc(node, size, tag, desc)
node_t *node;
int size;
int tag;
char *desc;
{
	unsigned int h;
	static int count = 0;

	total_malloced += size;
	if (total_malloced > hiwater) {
		hiwater = total_malloced;
	}
	h = (unsigned int)node % TABLESIZE;
	node->size = size;
	node->next = table[h];
#ifdef DEBUGMALLOC_EXTENSIONS
	if (tag < MAX_CATEGORY) {
		totals[tag] += size;
	}
	node->tag = tag;
	node->id = count++;
	node->desc = desc ? desc : "default";
	if (malloc_mask == node->tag) {
		fprintf(stderr,"MDmalloc: %5d, [%-25s], %8x:(%d)\n",
			node->tag, node->desc, (unsigned int)PTR(node), node->size);
		fflush(stderr);
	}
#endif
	table[h] = node;
}

int
MDfree(ptr)
void *ptr;
{
	unsigned int h;
	node_t *entry, **oentry;

	h = (unsigned int)ptr % TABLESIZE;
	oentry = &table[h];
	for (entry = *oentry; entry; oentry = &entry->next, entry = *oentry) {
		if (entry == ptr) {
			*oentry = entry->next;
			total_malloced -= entry->size;
			break;
		}
	}
	if (entry) {
#ifdef DEBUGMALLOC_EXTENSIONS
		if (entry->tag < MAX_CATEGORY) {
			totals[entry->tag] -= entry->size;
		}
		if (malloc_mask == entry->tag) {
			fprintf(stderr,"MDfree: %5d, [%-25s], %8x:(%d)\n",
				entry->tag, entry->desc, (unsigned int)PTR(entry), entry->size);
			fflush(stderr);
		}
#endif
	} else {
		fprintf(stderr,
	"md: debugmalloc: attempted to free non-malloc'd pointer %x\n",
		(unsigned int)ptr);
#ifdef DEBUG
		abort();
#endif
		return 0;
	}
	return 1;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void dump_debugmalloc(tfn, mask)
char *tfn;
int mask;
{
	int j, total = 0, chunks = 0, total2 = 0;
	char *fn;
	node_t *entry;
	FILE *fp;

	fn = check_valid_path(tfn, current_object, "debugmalloc", 1);
	if (!fn) {
		add_message("Invalid path '%s' for writing.\n", tfn);
		return;
	}
	fp = fopen(fn, "w");
	if (!fp) {
		add_message("Unable to open %s for writing.\n", fn);
		return;
	}
	add_message("Dumping to %s ...",fn);
	for (j = 0; j < TABLESIZE; j++) {
		for (entry = table[j]; entry; entry = entry->next) {
			if (!mask || (entry->tag == mask)) {
				fprintf(fp,"%-20s: sz %7d: id %6d: tag %8d, a %8x\n",
					entry->desc, entry->size, entry->id, entry->tag,
					(unsigned int)PTR(entry));
				total += entry->size;
				chunks++;
			}
		}
	}
	fprintf(fp, "total =    %8d\n", total);
	fprintf(fp, "# chunks = %8d\n", chunks);
	fprintf(fp, "ave. bytes per chunk = %7.2f\n\n", (float)total / chunks);
	fprintf(fp, "categories:\n\n");
	for (j = 0; j < MAX_CATEGORY; j++) {
		fprintf(fp, "%4d: %10d\n", j, totals[j]);
		total2 += totals[j];
	}
	fprintf(fp, "\ntotal = %11d\n", total2);
	fclose(fp);
	add_message(" done.\n");
	add_message("total =    %8d\n", total);
	add_message("# chunks = %8d\n", chunks);
	if (chunks) {
		add_message("ave. bytes per chunk = %7.2f\n", (float)total / chunks);
	}
}
#endif /* DEBUGMALLOC_EXTENSIONS */

void set_malloc_mask(mask)
int mask;
{
	malloc_mask = mask;
}

#endif /* DEBUGMALLOC */
