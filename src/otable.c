#include "std.h"
#include "lpc_incl.h"
#include "otable.h"
#include "comm.h"
#include "hash.h"
#include "simul_efun.h"

/*
 * Object name hash table.  Object names are unique, so no special
 * problems - like stralloc.c.  For non-unique hashed names, we need
 * a better package (if we want to be able to get at them all) - we
 * cant move them to the head of the hash chain, for example.
 *
 * Note: if you change an object name, you must remove it and reenter it.
 */

static int otable_size;
static int otable_size_minus_one;

static object_t *find_obj_n PROT((char *));

/*
 * Object hash function, ripped off from stralloc.c.
 */
#define ObjHash(s) whashstr(s, 40) & otable_size_minus_one

/*
 * hash table - list of pointers to heads of object chains.
 * Each object in chain has a pointer, next_hash, to the next object.
 */

static object_t **obj_table = 0;

void init_otable()
{
    int x, y;

    /* ensure that otable_size is a power of 2 */
    y = OTABLE_SIZE;
    for (otable_size = 1; otable_size < y; otable_size *= 2)
	;
    otable_size_minus_one = otable_size - 1;
    obj_table = CALLOCATE(otable_size, object_t *, 
			  TAG_OBJ_TBL, "init_otable");

    for (x = 0; x < otable_size; x++)
	obj_table[x] = 0;
}

/*
 * Looks for obj in table, moves it to head.
 */

static int obj_searches = 0, obj_probes = 0, objs_found = 0;

/* A global.  *shhhh* don't tell. */
static int h;

static object_t *find_obj_n P1(char *, s)
{
    object_t *curr, *prev;

    h = ObjHash(s);
    curr = obj_table[h];
    prev = 0;

    obj_searches++;

    while (curr) {
	obj_probes++;
	if (!strcmp(curr->name, s)) {	/* found it */
	    if (prev) {		/* not at head of list */
		prev->next_hash = curr->next_hash;
		curr->next_hash = obj_table[h];
		obj_table[h] = curr;
	    }
	    objs_found++;
	    return (curr);	/* pointer to object */
	}
	prev = curr;
	curr = curr->next_hash;
    }

    return (0);			/* not found */
}

/*
 * Add an object to the table - can't have duplicate names.
 * 
 * Exception: Precompiled objects have a dummy entry here, but it is
 * guaranteed to be behind the real entry if a real entry exists.
 */

static int objs_in_table = 0;

void enter_object_hash P1(object_t *, ob)
{
    object_t *s;

    s = find_obj_n(ob->name); /* This sets h */

#ifdef DEBUG
    /* when these reload, the new copy comes in before the old goes out */
    if (s != master_ob && s != simul_efun_ob) {
#ifdef LPC_TO_C
    DEBUG_CHECK1(s && s != ob && (!(s->flags & O_COMPILED_PROGRAM)),
		 "Duplicate object \"/%s\" in object hash table",
		 ob->name);
#else
    DEBUG_CHECK1(s && s != ob,
		 "Duplicate object \"/%s\" in object hash table",
		 ob->name);
#endif
    }
#endif

    ob->next_hash = obj_table[h];
    obj_table[h] = ob;
    objs_in_table++;
    return;
}

/* for adding a precompiled entry (dynamic loading) since it is possible
 * that the real object exists.
 */ 
void enter_object_hash_at_end P1(object_t *, ob)
{
    object_t *s;
    object_t **op;
    
    s = find_obj_n(ob->name); /* This sets h */

    ob->next_hash = 0;

    op = &obj_table[h];
    while (*op)
	op = &((*op)->next_hash);
    *op = ob;
    objs_in_table++;
    return;
}

/*
 * Remove an object from the table - generally called when it
 * is removed from the next_all list - i.e. in destruct.
 */

void remove_object_hash P1(object_t *, ob)
{
    object_t *s;

    s = find_obj_n(ob->name); /* this sets h, and cycles the ob to the front */

    DEBUG_CHECK1(s != ob, "Remove object \"/%s\": found a different object!",
		 ob->name);

    obj_table[h] = ob->next_hash;
    ob->next_hash = 0;
    objs_in_table--;
    return;
}

#ifdef LPC_TO_C
void remove_precompiled_hashes P1(char *, name) {
    int h = ObjHash(name);
    object_t **p;
    object_t *curr;
    
    p = &obj_table[h];
    
    while (*p) {
	if (((*p)->flags & O_COMPILED_PROGRAM) && !strcmp((*p)->name, name)) {
	    curr = *p;
	    FREE(curr->name);
	    FREE(curr);
	    (*p) = (*p)->next_hash;
	} else
	    p = &((*p)->next_hash);
    }
}
#endif

/*
 * Lookup an object in the hash table; if it isn't there, return null.
 * This is only different to find_object_n in that it collects different
 * stats; more finds are actually done than the user ever asks for.
 */

static int user_obj_lookups = 0, user_obj_found = 0;

object_t *lookup_object_hash P1(char *, s)
{
    object_t *ob = find_obj_n(s);

    user_obj_lookups++;
    if (ob)
	user_obj_found++;
    return (ob);
}

/*
 * Print stats, returns the total size of the object table.  All objects
 * are in table, so their size is included as well.
 */

static char sbuf[100];

int show_otable_status P2(outbuffer_t *, out, int, verbose)
{
    int starts;

    if (verbose == 1) {
	outbuf_add(out, "Object name hash table status:\n");
	outbuf_add(out, "------------------------------\n");
	sprintf(sbuf, "%10.2f", objs_in_table / (float) OTABLE_SIZE);
	outbuf_addv(out, "Average hash chain length:       %s\n", sbuf);
	sprintf(sbuf, "%10.2f", (float) obj_probes / obj_searches);
	outbuf_addv(out, "Average search length:           %s\n", sbuf);
	outbuf_addv(out, "Internal lookups (succeeded):    %lu (%lu)\n",
		    obj_searches - user_obj_lookups, objs_found - user_obj_found);
	outbuf_addv(out, "External lookups (succeeded):    %lu (%lu)\n",
		    user_obj_lookups, user_obj_found);
    }
    starts = (int) OTABLE_SIZE *sizeof(object_t *) +
                objs_in_table * sizeof(object_t);

    if (!verbose) {
	outbuf_addv(out, "Obj table overhead:\t\t%8d %8d\n",
		    OTABLE_SIZE * sizeof(object_t *), starts);
    }
    return starts;
}
