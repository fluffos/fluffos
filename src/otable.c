#include "std.h"
#include "otable.h"
#include "comm.h"
#include "hash.h"
#include "simul_efun.h"
#include "master.h"
#include "object.h"

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

static object_t *find_obj_n(const char *);

/*
 * Object hash function, ripped off from stralloc.c.
 */
#define ObjHash(s) whashstr(s) & otable_size_minus_one

/*
 * hash table - list of pointers to heads of object chains.
 * Each object in chain has a pointer, next_hash, to the next object.
 */

static object_t **obj_table = 0;
static object_t **ch_table = 0;

static char *basename(const char *full, int *size) {
	char *tmp;
	char *name= new_string(*size = strlen(full), "base_name: name");
	while (*full == '/') {
		full++;
		(*size)--;
	}
	strcpy(name, full);
	tmp = strchr(name, '#');
	if (tmp) {
		*size = tmp-name;
	}
	while (*size > 2&& name[*size-1] == 'c'&& name[*size-2] == '.')
		*size -= 2;
	name[*size] = 0;
	name = extend_string(name, *size);
	return name;
}

void init_otable() {
	int x, y;

	/* ensure that otable_size is a power of 2 */
	y = OTABLE_SIZE;
	for (otable_size = 1; otable_size < y; otable_size *= 2)
		;
	otable_size_minus_one = otable_size - 1;
	obj_table = CALLOCATE(otable_size, object_t *,
			TAG_OBJ_TBL, "init_otable");
	ch_table = CALLOCATE(otable_size, object_t *,
			TAG_OBJ_TBL, "init_ch_otable");

	for (x = 0; x < otable_size; x++) {
		obj_table[x] = 0;
		ch_table[x] = 0;
	}
}

/*
 * Looks for obj in table, moves it to head.
 */

static long obj_searches = 0, obj_probes = 0, objs_found = 0;

/* A global.  *shhhh* don't tell. */
static int h;
static int ch;

static object_t *find_obj_n(const char * s) {
	object_t *curr, *prev;
	h = ObjHash(s);
	curr = obj_table[h];
	prev = 0;

	obj_searches++;

	while (curr) {
		obj_probes++;
		if (!strcmp(curr->obname, s)) { /* found it */
			if (prev) { /* not at head of list */
				prev->next_hash = curr->next_hash;
				curr->next_hash = obj_table[h];
				obj_table[h] = curr;
			}
			objs_found++;
			return (curr); /* pointer to object */
		}
		prev = curr;
		curr = curr->next_hash;
	}

	return (0); /* not found */
}

array_t *children(const char * s) {
	object_t *curr;
	array_t *vec;
	int size;
	int count = 0;

	s = basename(s, &size);
	ch = ObjHash(s);
	curr = ch_table[ch];

	vec = allocate_empty_array(max_array_size);
	while (curr && count < max_array_size) {
		if (!strncmp(curr->obname, s, size)) { /* found one */
			vec->item[count].u.ob = curr;
			add_ref(curr, "children");
			vec->item[count].type = T_OBJECT;
			count++;
		}
		curr = curr->next_ch_hash;
	}
	FREE_MSTR(s);
	vec = resize_array(vec, count);
	return (vec);
}

/*
 * Add an object to the table - can't have duplicate names.
 * 
 * Exception: Precompiled objects have a dummy entry here, but it is
 * guaranteed to be behind the real entry if a real entry exists.
 */

static int objs_in_table = 0;

void enter_object_hash(object_t * ob) {
#ifdef DEBUG
	object_t *s;
#endif
	char *base;
	int dummy;
#ifndef DEBUG
	h = ObjHash(ob->obname);
#else
	s = find_obj_n(ob->obname); /* This sets h */
	/* when these reload, the new copy comes in before the old goes out */
	if (s != master_ob && s != simul_efun_ob) {
		DEBUG_CHECK1(s && s != ob,
				"Duplicate object \"/%s\" in object hash table",
				ob->obname);
	}
#endif

	ob->next_hash = obj_table[h];
	obj_table[h] = ob;
	objs_in_table++;

	//for children()
	base = basename(ob->obname, &dummy);

	ch = ObjHash(base);
	ob->next_ch_hash = ch_table[ch];
	ch_table[ch] = ob;
	FREE_MSTR(base);
	return;
}

/*
 * Remove an object from the table - generally called when it
 * is removed from the next_all list - i.e. in destruct.
 */

void remove_object_hash(object_t * ob) {
	int dummy;
	object_t *s;
	object_t *t = 0;
	char *base;

	s = find_obj_n(ob->obname); /* this sets h, and cycles the ob to the front */
	if (!s)
		fatal("couldn't find object %s in obj_table", ob->obname);

	DEBUG_CHECK1(s != ob, "Remove object \"/%s\": found a different object!",
			ob->obname);

	obj_table[h] = ob->next_hash;
	ob->next_hash = 0;
	objs_in_table--;

	base = basename(ob->obname, &dummy);
	ch = ObjHash(base);
	FREE_MSTR(base);
	s = ch_table[ch];
	while (s && s!=ob) {
		t = s;
		s = s->next_ch_hash;
	}
	if (!s)
		fatal("object not found in children list");
	if (!t)
		ch_table[ch] = s->next_ch_hash;
	else
		t->next_ch_hash = s->next_ch_hash;
	ob->next_ch_hash = 0;
	return;
}

/*
 * Lookup an object in the hash table; if it isn't there, return null.
 * This is only different to find_object_n in that it collects different
 * stats; more finds are actually done than the user ever asks for.
 */

static long user_obj_lookups = 0, user_obj_found = 0;

object_t *lookup_object_hash(const char * s) {
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

int show_otable_status(outbuffer_t * out, int verbose) {
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
	starts = (long) OTABLE_SIZE *sizeof(object_t *)+objs_in_table
			* sizeof(object_t);

	if (!verbose) {
		outbuf_addv(out, "Obj table overhead:\t\t%8d %8d\n", 
		OTABLE_SIZE * sizeof(object_t *), starts);
	}
	return starts;
}
