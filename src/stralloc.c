#include <stdio.h>
#include <string.h>

#include "config.h"
#include "lint.h"

/*
   this code is not the same as the original code.  I cleaned it up to
   use structs to: 1) make it easier to check the driver for memory leaks
   (using MallocDebug on a NeXT) and 2) because it looks cleaner this way.
   --Truilkan@TMI 92/04/19
*/

/*
 * stralloc.c - string management.
 *
 * All strings are stored in an extensible hash table, with reference counts.
 * free_string decreases the reference count; if it gets to zero, the string
 * will be deallocated.  add_string increases the ref count if it finds a
 * matching string, or allocates it if it cant.  There is no way to allocate
 * a string of a particular size to fill later (hash wont work!), so you'll
 * have to copy things from a static (or malloced and later freed) buffer -
 * that is, if you want to avoid space leaks...
 *
 * Current overhead:
 *	sizeof(block_t) per string (string pointer, next pointer, and a short
 *  for refs). Strings are nearly all fairly short, so this is a significant
 *  overhead - there is also the 4 byte malloc overhead and the fact that
 *  malloc generally allocates blocks which are a power of 2 (should write my
 *	own best-fit malloc specialised to strings); then again, GNU malloc
 *	is bug free...
 */

/*
 * there is also generic hash table management code, but strings can be shared
 * (that was the point of this code), will be unique in the table,
 * require a reference count, and are malloced, copied and freed at
 * will by the string manager.  Besides, I wrote this code first :-).
 * Look at htable.c for the other code.  It uses the Hash() function
 * defined here, and requires hashed objects to have a pointer to the
 * next element in the chain (which you specify when you call the functions).
 */

#define	MAXSHORT	(1 << (sizeof(short)*8 - 2))

char * xalloc();
#ifndef _AIX
char * strcpy();
#endif

static int num_distinct_strings = 0;
int bytes_distinct_strings = 0;
static int allocd_strings = 0;
static int allocd_bytes = 0;
int overhead_bytes = 0;
static int search_len = 0;
static int num_str_searches = 0;

typedef struct block_s {
	struct block_s *next;
	short refs;
	char *s;
} block_t;

#define	NEXT(x)	x->next
#define	REFS(x)	x->refs

/*
 * hash table - list of pointers to heads of string chains.
 * Each string in chain has a pointer to the next string and a
 * reference count (char *, int) stored just before the start of the string.
 * HTABLE_SIZE is in config.h, and should be a prime, probably between
 * 1000 and 5000.
 */

static block_t **base_table = (block_t **)0;

static void init_strings()
{
	int x;

	base_table = (block_t **) xalloc(sizeof(block_t *) * HTABLE_SIZE);
	overhead_bytes += (sizeof(block_t *) * HTABLE_SIZE);

	for (x=0; x < HTABLE_SIZE; x++)
		base_table[x] = 0;
}

/*
 * generic hash function.  This is probably overkill; I haven't checked the
 * stats for different prime numbers, etc.
 */

static int StrHash(s)
char * s;
{
	if (!base_table)
		init_strings();

	return hashstr(s, 20, HTABLE_SIZE);
}

/*
 * Looks for a string in the table.  If it finds it, returns a pointer to
 * the start of the string part, and moves the entry for the string to
 * the head of the pointer chain.  One thing (blech!) - puts the previous
 * pointer on the hash chain into fs_prev.
 */

block_t * findblock(s)
char *s;
{
	block_t *curr, *prev;
	int h = StrHash(s);

	curr = base_table[h];
	prev = 0;
	num_str_searches++;

	while (curr) {
		search_len++;
		if (*(curr->s) == *s && !strcmp(curr->s, s)) { /* found it */
			if (prev) { /* not at head of list */
				NEXT(prev) = NEXT(curr);
				NEXT(curr) = base_table[h];
				base_table[h] = curr;
			}
			return(curr);  /* pointer to string */
		}
		prev = curr;
		curr = NEXT(curr);
	}
	return((block_t *)0); /* not found */
}

char *findstring(s)
char *s;
{
	block_t *b;

	if (b = findblock(s))
		return b->s;
	else
		return((char *)0);
}

/*
 * Make a space for a string.  This is rather nasty, as we want to use
 * alloc/free, but malloc overhead is generally severe.  Later, we
 * will have to compact strings...
 */

static block_t *alloc_new_string(string,size)
char * string;
int size;
{
	int h = StrHash(string);
	block_t *b = (block_t *)xalloc(size);

	/* pointer arithmetic increments b by sizeof(block_t) */
	b->s = (char *)(b + 1);
	strcpy(b->s, string);
	REFS(b) = 0;
	NEXT(b) = base_table[h];
	base_table[h] = b;
	num_distinct_strings++;
	bytes_distinct_strings += size;
	overhead_bytes += sizeof(block_t);
	return(b);
}

char * make_shared_string(str)
char * str;
{
	block_t *b;
	int size = sizeof(block_t) + strlen(str) + 1;

	b = findblock(str);
	if (!b)
		b = alloc_new_string(str,size);
	REFS(b)++;
	allocd_strings++;
	allocd_bytes += size;
	return(b->s);
}

/*
   ref_string: this is the only routine that was slowed down by cleaning
   up the data structures.  and this routine is only currently called from
   two rarely called places anyway.
*/

char *ref_string(str)
char *str;
/* Doesn't do any good to call this unless the string is in fact shared */
{
	block_t *b;

	if (b = findblock(str)) {
		REFS(b)++;
		return b->s;
	}
	return str;
}

/*
 * free_string - reduce the ref count on a string.  Various sanity
 * checks applied.
 */

/*
 * function called on free_string detected errors; things return checked(s).
 */

static void checked(s, str)
char *s, *str;
{
	fprintf(stderr, "%s (\"%s\")\n", s, str);
	fatal(s); /* brutal - debugging */
}

void free_string(str)
char * str;
{
	block_t *b;

	allocd_strings--;
	allocd_bytes -= (strlen(str) + 1 + sizeof(block_t));

	b = findblock(str); /* moves it to head of table if found */
#ifndef	BUG_FREE
	if (!b) {
	    checked("Free string: not found in string table!", str);
	    return;
	}
	if (b->s != str) {
	    checked("Free string: string didnt hash to the same spot!", str);
	    return;
	}

	if (REFS(b) <= 0) {
	    checked("Free String: String refs zero or -ve!", str);
	    return;
	}
#endif	/* BUG_FREE */

	if (REFS(b) > MAXSHORT) return;
	REFS(b)--;
	if (REFS(b) > 0) return;

	/* It will be at the head of the hash chain */
	base_table[StrHash(str)] = NEXT(b);
	num_distinct_strings--;
	/* We know how much overhead malloc has */
	bytes_distinct_strings-= (sizeof(block_t) + strlen(str) + 1);
	overhead_bytes -= sizeof(block_t);
	FREE(b);

	return;
}

/*
 * you think this looks bad!  and we didn't even tell them about the
 * GNU malloc overhead!  tee hee!
 */

int add_string_status(verbose)
    int verbose;
{
    if (verbose) {
	add_message("\nShared string hash table:\n");
	add_message("-------------------------\t Strings    Bytes\n");
    }
    add_message("Strings malloced\t\t%8d %8d + %d overhead\n",
		num_distinct_strings, bytes_distinct_strings, overhead_bytes);
    if (verbose) {
	add_message("Total asked for\t\t\t%8d %8d\n",
		    allocd_strings, allocd_bytes);
	add_message("Space actually required/total string bytes %d%%\n",
		    (bytes_distinct_strings + overhead_bytes)*100 / allocd_bytes);
	add_message("Searches: %d    Average search length: %6.3f\n",
		    num_str_searches, (double)search_len / num_str_searches);
    }
    return(bytes_distinct_strings + overhead_bytes);
}
