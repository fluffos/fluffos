/*
 * mudlib_stats.c
 * created by: Erik Kay
 * last modified: 11/1/92
 * this file is a replacement for wiz_list.c and all of its purposes
 * the idea is that it will be more domain based, rather than user based
 * and will be a little more general purpose than wiz_list was
 */

#ifdef LATTICE
#include "/lpc_incl.h"
#include "/backend.h"
#include "/md.h"
#include "/master.h"
#else
#include "../lpc_incl.h"
#include "../backend.h"
#include "../md.h"
#include "../master.h"
#include "../efun_protos.h"
#endif

#include "mudlib_stats.h"

#ifdef F_DOMAIN_STATS
void
f_domain_stats (void)
{
    mapping_t *m;

    if (st_num_arg) {
        m = get_domain_stats(sp->u.string);
        free_string_svalue(sp--);
    } else {
        m = get_domain_stats(0);
    }
    if (!m) {
        push_number(0);
    } else {
        /* ref count is properly decremented by get_domain_stats */
        push_mapping(m);
    }
}
#endif

#ifdef F_SET_AUTHOR
void
f_set_author (void)
{
    set_author(sp->u.string);
    free_string_svalue(sp--);
}
#endif

#ifdef F_AUTHOR_STATS
void
f_author_stats (void)
{
    mapping_t *m;

    if (st_num_arg) {
        m = get_author_stats(sp->u.string);
        free_string_svalue(sp--);
    } else {
        m = get_author_stats(0);
    }
    if (!m) {
        push_number(0);
    } else {
        /* ref count is properly decremented by get_author_stats */
        push_mapping(m);
    }
}
#endif

/* Support functions */
static mudlib_stats_t *domains = 0;
static mudlib_stats_t *backbone_domain = 0;
static mudlib_stats_t *authors = 0;
static mudlib_stats_t *master_author = 0;

static mudlib_stats_t *find_stat_entry (const char *, mudlib_stats_t *);
static mudlib_stats_t *add_stat_entry (const char *, mudlib_stats_t **);
static void init_author_for_ob (object_t *);
static const char *author_for_file (const char *);
static void init_domain_for_ob (object_t *);
static const char *domain_for_file (const char *);
static void save_stat_list (const char *, mudlib_stats_t *);
static void restore_stat_list (const char *, mudlib_stats_t **);
static mapping_t *get_info (mudlib_stats_t *);
static mapping_t *get_stats (const char *, mudlib_stats_t *);
static mudlib_stats_t *insert_stat_entry (mudlib_stats_t *, mudlib_stats_t **);

#ifdef DEBUGMALLOC_EXTENSIONS
/* debugging */
int check_valid_stat_entry (mudlib_stats_t * se) {
    mudlib_stats_t *tmp;

    tmp = domains;
    while (tmp) {
	if (tmp == se) return 1;
	tmp = tmp->next;
    }
    tmp = authors;
    while (tmp) {
	if (tmp == se) return 1;
	tmp = tmp->next;
    }
    return 0;
}
#endif

/**************************
 * stat list manipulation
 **************************/

static mudlib_stats_t *insert_stat_entry (mudlib_stats_t * entry, mudlib_stats_t ** list)
{
    entry->next = *list;
    *list = entry;
    return *list;
}

/*
 * Return the data for an individual domain, if it exists.
 * this uses a simple linear search.  it's a good thing that most muds
 * will have a relatively small number of domains
 */
static mudlib_stats_t *find_stat_entry (const char * name, mudlib_stats_t * list)
{
    int length;

    length = strlen(name);
    for (; list; list = list->next)
	if (list->length == length && strcmp(list->name, name) == 0)
	    return list;
    return 0;
}

/*
 * add a new domain to the domain list.  If it exists, do nothing.
 */
static mudlib_stats_t *add_stat_entry (const char * str, mudlib_stats_t ** list)
{
    mudlib_stats_t *entry;

    if ((entry = find_stat_entry(str, *list)))
	return entry;
    entry = ALLOCATE(mudlib_stats_t, TAG_MUDLIB_STATS, "add_stat_entry");
    entry->name = make_shared_string(str);
    entry->length = strlen(str);
    entry->moves = 0;
    entry->heart_beats = 0;
    entry->errors = 0;
    entry->objects = 0;
    entry->next = NULL;
    entry->size_array = 0;
    insert_stat_entry(entry, list);
    return entry;
}


/*************************************
 * general stat modifying accessor functions
 **************************************/


void assign_stats (statgroup_t * st, object_t * ob)
{
    st->domain = ob->stats.domain;
    st->author = ob->stats.author;
}

void null_stats (statgroup_t * st)
{
    if (st) {
	st->domain = NULL;
	st->author = NULL;
    }
}

void init_stats_for_object (object_t * ob)
{
    init_domain_for_ob(ob);
    init_author_for_ob(ob);
}


/*
 * Add moves to an existing domain.
 */
void add_moves (statgroup_t * st, int moves)
{
    if (st) {
	if (st->domain)
	    st->domain->moves += moves;
	if (st->author)
	    st->author->moves += moves;
    }
}

INLINE void add_heart_beats (statgroup_t * st, int hbs)
{
    if (st) {
	if (st->domain)
	    st->domain->heart_beats += hbs;
	if (st->author)
	    st->author->heart_beats += hbs;
    }
}

void add_array_size (statgroup_t * st, int size)
{
    if (st) {
	if (st->domain)
	    st->domain->size_array += size;
	if (st->author)
	    st->author->size_array += size;
    }
}

void add_errors (statgroup_t * st, int errors)
{
    if (st) {
	if (st->domain)
	    st->domain->errors += errors;
	if (st->author)
	    st->author->errors += errors;
    }
}

void add_errors_for_file (const char * file, int errors)
{
    mudlib_stats_t *entry;
    const char *name;

    name = domain_for_file(file);
    if (name && domains) {
	entry = find_stat_entry(name, domains);
	if (entry)
	    entry->errors += errors;
    }
    name = author_for_file(file);
    if (name && authors) {
	entry = find_stat_entry(name, authors);
	if (entry)
	    entry->errors += errors;
    }
}

void add_objects (statgroup_t * st, int objects)
{
    if (st) {
	if (st->domain)
	    st->domain->objects += objects;
	if (st->author)
	    st->author->objects += objects;
    }
}

/*
 * Basically the "scores" are averaged over time by having them decay
 * gradually at each reset.
 * Here's how the decay breaks down:
 *    moves -= 1%
 *    heart_beats -= 10%
 */
void mudlib_stats_decay()
{
    mudlib_stats_t *dl;
    static int next_time;

    /* Perform this once every hour. */
    if (next_time > current_time)
	return;
    next_time = current_time + 60 * 60;
    for (dl = domains; dl; dl = dl->next) {
	dl->moves = dl->moves * 99 / 100;
	dl->heart_beats = dl->heart_beats * 9 / 10;
    }
    for (dl = authors; dl; dl = dl->next) {
	dl->moves = dl->moves * 99 / 100;
	dl->heart_beats = dl->heart_beats * 9 / 10;
    }
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_mudlib_stats() {
    mudlib_stats_t *dl;

    for (dl = domains; dl; dl = dl->next) {
	DO_MARK(dl, TAG_MUDLIB_STATS);
	EXTRA_REF(BLOCK(dl->name))++;
    }
    for (dl = authors; dl; dl = dl->next) {
	DO_MARK(dl, TAG_MUDLIB_STATS);
	EXTRA_REF(BLOCK(dl->name))++;
    }
}
#endif

/*************************
 Author specific functions
 *************************/

static void init_author_for_ob (object_t * ob)
{
    svalue_t *ret;

    push_malloced_string(add_slash(ob->obname));
    ret = apply_master_ob(APPLY_AUTHOR_FILE, 1);
    if (ret == (svalue_t *)-1) {
	ob->stats.author = master_author;
    } else if (!ret || ret->type != T_STRING) {
	ob->stats.author = NULL;
    } else {
	ob->stats.author = add_stat_entry(ret->u.string, &authors);
    }
}

void set_author (const char *name)
{
    object_t *ob;

    if (!current_object)
	return;
    ob = current_object;
    if (master_ob == (object_t *)-1) {
	ob->stats.author = NULL;
	return;
    }
    if (ob->stats.author) {
	ob->stats.author->objects--;
    }
    ob->stats.author = add_stat_entry(name, &authors);
    if (ob->stats.author) {
	ob->stats.author->objects++;
    }
}

mudlib_stats_t *set_master_author (const char * str)
{
    mudlib_stats_t *author;

    author = add_stat_entry(str, &authors);
    if (author)
	master_author = author;
    return author;
}

static const char *author_for_file (const char * file)
{
    svalue_t *ret;
    static char buff[50];

    copy_and_push_string(file);
    ret = apply_master_ob(APPLY_AUTHOR_FILE, 1);
    if (ret == 0 || ret == (svalue_t*)-1 || ret->type != T_STRING)
	return 0;
    strcpy(buff, ret->u.string);
    return buff;
}


/*************************
 Domain specific functions
 *************************/

static void init_domain_for_ob (object_t * ob)
{
    svalue_t *ret;
    const char *domain_name;

    if (!current_object
#ifdef PACKAGE_UIDS
	|| !current_object->uid
#endif
	) {
	/*
	 * Only for the master and void object. Note that you can't ask for
	 * the backbone or root domain here since we're in the process of
	 * loading the master object.
	 */
	ob->stats.domain = add_stat_entry("NONAME", &domains);
	return;
    }
    /*
     * Ask master object who the creator of this object is.
     */
    push_malloced_string(add_slash(ob->obname));

    if (master_ob)
	ret = apply_master_ob(APPLY_DOMAIN_FILE, 1);
    else
	ret = apply(applies_table[APPLY_DOMAIN_FILE], ob, 1, ORIGIN_DRIVER);

    if (IS_ZERO(ret)) {
	ob->stats.domain = current_object->stats.domain;
	return;
    }
    if (ret->type != T_STRING)
	error("'domain_file' in the master object must return a string!\n");
    domain_name = ret->u.string;
    if (strcmp(current_object->stats.domain->name, domain_name) == 0) {
	ob->stats.domain = current_object->stats.domain;
	return;
    }
    if (strcmp(backbone_domain->name, domain_name) == 0) {
	/*
	 * The object is loaded from backbone. We give domain ownership to
	 * the creator rather than backbone.
	 */
	ob->stats.domain = current_object->stats.domain;
	return;
    }
    /*
     * The object isn't loaded from backbone or from the same domain as the
     * creator, so we need to lookup the domain, and add it if it isnt
     * present.
     */
    ob->stats.domain = add_stat_entry(domain_name, &domains);
    return;
}

mudlib_stats_t *set_backbone_domain (const char * str)
{
    mudlib_stats_t *dom;

    dom = add_stat_entry(str, &domains);
    if (dom)
	backbone_domain = dom;
    return dom;
}


/*
 * Argument is a file name, which we want to get the domain of.
 * Ask the master object.
 */
static const char *domain_for_file (const char * file)
{
    svalue_t *ret;
    static char buff[512];

    share_and_push_string(file);
    ret = apply_master_ob(APPLY_DOMAIN_FILE, 1);
    if (ret == 0 || ret == (svalue_t*)-1 || ret->type != T_STRING)
	return 0;
    strcpy(buff, ret->u.string);
    return buff;
}


/************************************
 * save and restore stats to a file *
 ************************************/

static void save_stat_list (const char * file, mudlib_stats_t * list)
{
    FILE *f;
    char fname_buf[MAXPATHLEN];
    char *fname = fname_buf;

    if (file) {
	if (strchr(file, '/')) {
	    if (file[0] == '/')
		file++;
	    f = fopen(file, "w");
	} else {
	    sprintf(fname, "%s/%s", LOG_DIR, file);
	    if (fname[0] == '/')
		fname++;
	    f = fopen(fname, "w");
	}
    } else {
	debug_message("*Warning: call to save_stat_list with null filename\n");
	return;
    }
    if (!f) {
	debug_message("*Error: unable to open stat file %s for writing.\n",
		file);
	return;
    }
    while (list) {
	fprintf(f, "%s %d %d\n", list->name,
		list->moves, list->heart_beats);
	list = list->next;
    }
    fclose(f);
}

static void restore_stat_list (const char * file, mudlib_stats_t ** list)
{
    FILE *f;
    char fname_buf[MAXPATHLEN];
    char *fname = fname_buf;
    mudlib_stats_t *entry;

    if (file) {
	if (strchr(file, '/')) {
	    if (file[0] == '/')
		file++;
	    f = fopen(file, "r");
	} else {
	    sprintf(fname, "%s/%s", LOG_DIR, file);
	    if (fname[0] == '/')
		fname++;
	    f = fopen(fname, "r");
	}
    } else {
	debug_message("*Warning: call to save_stat_list with null filename\n");
	return;
    }
    if (!f) {
	debug_message("*Warning: unable to open stat file %s for reading.\n",
		file);
	return;
    }
    while (fscanf(f, "%s", fname) != EOF) {
	entry = add_stat_entry(fname, list);
	fscanf(f, "%d %d\n", &entry->moves, &entry->heart_beats);
    }
    fclose(f);
}


void save_stat_files()
{
    save_stat_list(DOMAIN_STATS_FILE_NAME, domains);
    save_stat_list(AUTHOR_STATS_FILE_NAME, authors);
}

void restore_stat_files()
{
    restore_stat_list(DOMAIN_STATS_FILE_NAME, &domains);
    restore_stat_list(AUTHOR_STATS_FILE_NAME, &authors);
}


/*************************************
 * The following functions are the interface for efuns to get mappings
 * that describe the statistics for authors and domains.
 **************************************/

static mapping_t *
        get_info (mudlib_stats_t * dl)
{
    mapping_t *ret;

    ret = allocate_mapping(8);
    add_mapping_pair(ret, "moves", dl->moves);
    add_mapping_pair(ret, "errors", dl->errors);
    add_mapping_pair(ret, "heart_beats", dl->heart_beats);
    add_mapping_pair(ret, "array_size", dl->size_array);
    add_mapping_pair(ret, "objects", dl->objects);
    return ret;
}

static mapping_t *
        get_stats (const char * str, mudlib_stats_t * list)
{
    mudlib_stats_t *dl;
    mapping_t *m;
    svalue_t lv, *s;

    if (str) {
	for (dl = list; dl; dl = dl->next) {
	    if (!strcmp(str, dl->name))	/* are these both shared strings? */
		break;
	}
	if (dl) {
	    mapping_t *tmp;

	    tmp = get_info(dl);
	    tmp->ref--;
	    return tmp;
	} else {
	    return 0;
	}
    }
    m = allocate_mapping(8);
    for (dl = list; dl; dl = dl->next) {
	lv.type = T_STRING;
	lv.subtype = STRING_SHARED;
	lv.u.string = dl->name;  /* find_for_insert() adds a ref */
	s = find_for_insert(m, &lv, 1);
	s->type = T_MAPPING;
	s->subtype = 0;
	s->u.map = get_info(dl);
    }
    m->ref--;
    return m;
}

mapping_t *get_domain_stats (const char * str){
    return get_stats(str, domains);
}

mapping_t *get_author_stats (const char * str){
    return get_stats(str, authors);
}
