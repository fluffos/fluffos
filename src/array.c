#include "std.h"
#include "lpc_incl.h"
#include "comm.h"
#include "regexp.h"
#include "backend.h"
#include "qsort.h"
#include "array.h"

/*
 * This file contains functions used to manipulate arrays.
 * Some of them are connected to efuns, and some are only used internally
 * by the MudOS driver.
 */

int num_arrays;
int total_array_size;

INLINE static int builtin_sort_array_cmp_fwd PROT((struct svalue *, struct svalue *));
INLINE static int builtin_sort_array_cmp_rev PROT((struct svalue *, struct svalue *));
INLINE static int sort_array_cmp PROT((struct svalue *, struct svalue *));
static int deep_inventory_count PROT((struct object *));
static void deep_inventory_collect PROT((struct object *, struct vector *, int *));
INLINE static int alist_cmp PROT((struct svalue *, struct svalue *));

/*
 * Make an empty vector for everyone to use, never to be deallocated.
 * It is cheaper to reuse it, than to use MALLOC() and allocate.
 */

struct vector null_vector =
{
    1,				/* Ref count, which will ensure that it will
				 * never be deallocated */
#ifdef DEBUG
    1,                          /* extra ref */
#endif
    0,				/* size */
};

INLINE struct vector *
       null_array()
{
    null_vector.ref++;
    return &null_vector;
}

/*
 * Allocate an array of size 'n'.
 */
struct vector *allocate_array P1(int, n)
{
    struct vector *p;

    if (n < 0 || n > max_array_size)
	error("Illegal array size.\n");
    if (n == 0) {
	return null_array();
    }
    num_arrays++;
    total_array_size += sizeof(struct vector) + sizeof(struct svalue) *
	(n - 1);
    p = ALLOC_VECTOR(n);
    p->ref = 1;
    p->size = n;
#ifndef NO_MUDLIB_STATS
    if (current_object) {
	assign_stats(&p->stats, current_object);
	add_array_size(&p->stats, n);
    } else {
	null_stats(&p->stats);
    }
#endif
    while (n--)
	p->item[n] = const0;
    return p;
}

struct vector *allocate_empty_array P1(int, n)
{
    struct vector *p;
    
    if (n < 0 || n > max_array_size)
	error("Illegal array size.\n");
    if (!n) return null_array();
    num_arrays++;
    total_array_size += sizeof(struct vector) + sizeof(struct svalue) * (n-1);
    p = ALLOC_VECTOR(n);
    p->ref = 1;
    p->size = n;
#ifndef NO_MUDLIB_STATS
    if (current_object) {
	assign_stats(&p->stats, current_object);
	add_array_size(&p->stats, n);
    } else {
	null_stats(&p->stats);
    }
#endif
    return p;
}

void free_vector P1(struct vector *, p)
{
    int i;

    /*
     * don't keep track of the null vector since many muds reference it
     * enough times to overflow the ref count which is a short int.
     */
    if ((--(p->ref) > 0) || (p == &null_vector)) {
	return;
    }
    for (i = p->size; i--;)
	free_svalue(&p->item[i], "free_vector");
#ifndef NO_MUDLIB_STATS
    add_array_size(&p->stats, -((int)p->size));
#endif
    num_arrays--;
    total_array_size -= sizeof(struct vector) + sizeof(struct svalue) *
	(p->size - 1);
    FREE((char *) p);
}

void free_empty_vector P1(struct vector *, p)
{
    if ((--(p->ref) > 0) || (p == &null_vector)) {
        return;
      }
#ifndef NO_MUDLIB_STATS
    add_array_size(&p->stats, -((int)p->size));
#endif
    num_arrays--;
    total_array_size -= sizeof(struct vector) + sizeof(struct svalue) *
        (p->size - 1);
    FREE((char *) p);
}

struct vector *explode_string P2(char *, str, char *, del)
{
    char *p, *beg, *lastdel = (char *) NULL;
    int num, len, j, slen, limit;
    struct vector *ret;
    char *buff, *tmp;

    len = strlen(del);
    slen = strlen(str);

    if (!slen)
	return null_array();

    /* return an array of length strlen(str) -w- one character per element */
    if (len == 0) {
	if (slen > max_array_size) {
	    slen = max_array_size;
	}
	ret = allocate_empty_array(slen);
	for (j = 0; j < slen; j++) {
	    ret->item[j].type = T_STRING;
	    ret->item[j].subtype = STRING_MALLOC;
	    tmp = (char *) DXALLOC(2, TAG_STRING, "explode_string: tmp");
	    tmp[0] = str[j];
	    tmp[1] = '\0';
	    ret->item[j].u.string = tmp;
	}
	return ret;
    }
    if (len == 1) {
	char delimeter;

	delimeter = *del;

	/*
	 * Skip leading 'del' strings, if any.
	 */
	while (*str == delimeter) {
	    str++;
	    slen--;
	    if (str[0] == '\0') {
		return null_array();
	    }
#ifdef SANE_EXPLODE_STRING
	    break;
#endif
	}

	/*
	 * Find number of occurences of the delimiter 'del'.
	 */
	for (p = str, num = 0; *p;) {
	    if (*p == delimeter) {
		num++;
		lastdel = p;
	    }
	    p++;
	}

	/*
	 * Compute number of array items. It is either number of delimiters,
	 * or, one more.
	 */
	if ((slen == 1) || (lastdel != (str + slen - 1))) {
	    num++;
	}
	if (num > max_array_size) {
	    num = max_array_size;
	}
	ret = allocate_empty_array(num);
	limit = max_array_size - 1;	/* extra element can be added after
					 * loop */
	for (p = str, beg = str, num = 0; *p && (num < limit);) {
	    if (*p == delimeter) {
		DEBUG_CHECK(num >= ret->size, "Index out of bounds in explode!\n");
		ret->item[num].type = T_STRING;
		ret->item[num].subtype = STRING_MALLOC;
		ret->item[num].u.string = buff =
		    DXALLOC(p - beg + 1, TAG_STRING, "explode_string: buff");

		strncpy(buff, beg, p - beg);
		buff[p - beg] = '\0';
		num++;
		beg = ++p;
	    } else {
		p++;
	    }
	}

	/* Copy last occurence, if there was not a 'del' at the end. */
	if (*beg != '\0') {
	    ret->item[num].type = T_STRING;
	    ret->item[num].subtype = STRING_MALLOC;
	    ret->item[num].u.string = string_copy(beg, "explode_string");
	}
	return ret;
    }				/* len == 1 */
    /*
     * Skip leading 'del' strings, if any.
     */
    while (strncmp(str, del, len) == 0) {
	str += len;
	slen -= len;
	if (str[0] == '\0') {
	    return null_array();
	}
#ifdef SANE_EXPLODE_STRING
	break;
#endif
    }

    /*
     * Find number of occurences of the delimiter 'del'.
     */
    for (p = str, num = 0; *p;) {
	if (strncmp(p, del, len) == 0) {
	    num++;
	    lastdel = p;
	    p += len;
	} else {
	    p++;
	}
    }

    /*
     * Compute number of array items. It is either number of delimiters, or,
     * one more.
     */
    if ((slen <= len) || (lastdel != (str + slen - len))) {
	num++;
    }
    if (num > max_array_size) {
	num = max_array_size;
    }
    ret = allocate_empty_array(num);
    limit = max_array_size - 1;	/* extra element can be added after loop */
    for (p = str, beg = str, num = 0; *p && (num < limit);) {
	if (strncmp(p, del, len) == 0) {
	    if (num >= ret->size)
		fatal("Index out of bounds in explode!\n");

	    ret->item[num].type = T_STRING;
	    ret->item[num].subtype = STRING_MALLOC;
	    ret->item[num].u.string = buff =
		DXALLOC(p - beg + 1, TAG_STRING, "explode_string: buff");

	    strncpy(buff, beg, p - beg);
	    buff[p - beg] = '\0';
	    num++;
	    beg = p + len;
	    p = beg;
	} else {
	    p++;
	}
    }

    /* Copy last occurence, if there was not a 'del' at the end. */
    if (*beg != '\0') {
	ret->item[num].type = T_STRING;
	ret->item[num].subtype = STRING_MALLOC;
	ret->item[num].u.string = string_copy(beg, "explode_string");
    }
    return ret;
}

char *implode_string P2(struct vector *, arr, char *, del)
{
    int size, i, num;
    char *p, *q;
    int del_len;
    struct svalue *sv = arr->item;

    for (i = arr->size, size = 0, num = 0; i--;) {
	if (sv[i].type & T_STRING) {
	    size += strlen(sv[i].u.string);
	    num++;
	}
    }
    if (num == 0)
	return string_copy("", "implode_string");

    del_len = strlen(del);
    p = DXALLOC(size + (num - 1) * del_len + 1, TAG_STRING, "implode_string: p");
    q = p;
    for (i = 0, num = 0; i < arr->size; i++) {
	if (sv[i].type & T_STRING) {
	    if (num) {
		strncpy(p, del, del_len);
		p += del_len;
	    }
	    size = strlen(sv[i].u.string);
	    strncpy(p, sv[i].u.string, size);
	    p += size;
	    num++;
	}
    }
    *p = 0;
    return q;
}

struct vector *users()
{
    register struct object *ob;
    int i, j;
    int display_hidden = 0;
    struct vector *ret;

    if (num_hidden > 0) {
	if (current_object->flags & O_HIDDEN) {
	    display_hidden = 1;
	} else {
	    display_hidden = valid_hide(current_object);
	}
    }
    ret = allocate_empty_array(num_user - (display_hidden ? 0 : num_hidden));
    for (i = j = 0; i < MAX_USERS; i++) {
	if (!all_users[i]) {
	    continue;
	}
	ob = all_users[i]->ob;
	if (!display_hidden && (ob->flags & O_HIDDEN)) {
	    continue;
	}
	ret->item[j].type = T_OBJECT;
	ret->item[j].u.ob = ob;
	add_ref(ob, "users");
	j++;
    }
    return ret;
}

/*
 * Slice of an array.
 * It now frees the passed array
 */
struct vector *slice_array P3(struct vector *, p, int, from, int, to)
{
    int cnt;
    struct svalue *sv1, *sv2;

    if (from < 0)
	from = 0;
    if (to >= p->size)
	to = p->size - 1;
    if (from > to) {
	free_vector(p);
	return null_array();
    }

    if (!(--p->ref)){
#ifndef NO_MUDLIB_STATS
	add_array_size(&p->stats, -((int)p->size));
#endif
	total_array_size += (to - from + 1 - p->size) * sizeof(struct svalue);
	if (from){
	    sv1 = p->item + from;
	    cnt = from;
	    while (cnt--) free_svalue(--sv1, "slice_array:2");
	    cnt = to - from + 1;
	    sv1 = p->item;
	    sv2 = p->item + from;
	    while (cnt--) *sv1++ = *sv2++;
	    cnt = p->size - 1 - to;
	    while (cnt--) free_svalue(sv2++, "slice_array:3");
	}
	p = RESIZE_VECTOR(p, to-from+1);
#ifndef NO_MUDLIB_STATS
	if (current_object) {
	    assign_stats(&p->stats, current_object);
	    add_array_size(&p->stats, to - from + 1);
	} else null_stats(&p->stats);
#endif
	p->size = to-from+1;
	p->ref = 1;
	return p;
    } else {
	struct vector *d;

	d = allocate_empty_array(to - from + 1);
	sv1 = d->item - from;
	sv2 = p->item;
	for (cnt = from; cnt <= to; cnt++)
	  assign_svalue_no_free(sv1 + cnt, sv2 + cnt);
	return d;
    }
}

/*
 * Copy of an array
 */
struct vector *copy_array P1(struct vector *, p)
{
    struct vector *d;
    int n;
    struct svalue *sv1 = p->item, *sv2;

    d = allocate_empty_array(n = p->size);
    sv2 = d->item;
    while (n--)
	assign_svalue_no_free(sv2 + n, sv1 + n);
    return d;
}

#ifdef F_COMMANDS
struct vector *commands P1(struct object *, ob)
{
    struct sentence *s;
    struct vector *v, *p;
    int cnt = 0;
    struct svalue *sv;

    for (s = ob->sent; s && s->verb; s = s->next) {
	if (++cnt == max_array_size) break;
    }
    v = allocate_empty_array(cnt);
    sv = v->item;
    for (s = ob->sent; cnt-- && s && s->verb; s = s->next) {
	sv->type = T_POINTER;
	(sv++)->u.vec = p = allocate_empty_array(4);
	p->item[0].type = T_STRING;
	p->item[0].u.string = ref_string(s->verb);	/* the verb is shared */
	p->item[0].subtype = STRING_SHARED;
	p->item[1].type = T_NUMBER;
	p->item[1].u.number = s->flags;
	p->item[2].type = T_OBJECT;
	p->item[2].u.ob = s->ob;
	p->item[3].type = T_STRING;
        if (s->flags & V_FUNCTION) {
            p->item[3].u.string = "<function>";
            p->item[3].subtype = STRING_CONSTANT;
        } else {
            p->item[3].u.string = ref_string(s->function.s);
            p->item[3].subtype = STRING_SHARED;
        }
	add_ref(s->ob, "commands");
    }
    return v;
}
#endif

/* EFUN: filter_array

   Runs all elements of an array through ob->func()
   and returns an array holding those elements that ob->func
   returned 1 for.
   */

#ifdef F_FILTER
void
filter_array P2(struct svalue *, arg, int, num_arg)
{
    struct vector *vec = arg->u.vec, *r;
    int size;

    if ((size = vec->size) < 1) {
	pop_n_elems(num_arg - 1);
	return;
    }
    else {
	struct funp *fp;
	struct object *ob = 0;
	char *func;

	char *flags = DXALLOC(size+1, TAG_TEMPORARY, "filter: flags");
	struct svalue *extra, *v;
	int res = 0, cnt, numex = 0;

	push_malloced_string(flags);

	if ((arg+1)->type == T_FUNCTION){
	    fp = (arg+1)->u.fp;
	    if (num_arg > 2) extra = arg+2, numex = num_arg - 2;
	} else {    
	    func = (arg+1)->u.string;
	    if (num_arg < 3) ob = current_object;
	    else {
		if ((arg+2)->type == T_OBJECT) ob = (arg+2)->u.ob;
		else if ((arg+2)->type == T_STRING){
		    if ((ob = find_object(arg[2].u.string)) && !object_visible(ob)) ob = 0;
		}
		if (!ob) bad_argument(arg+2, T_STRING | T_OBJECT, 3, F_FILTER);
		if (num_arg > 3) extra = arg + 3, numex = num_arg - 3;
	    }
	}

	for (cnt = 0; cnt < size; cnt++){
	    push_svalue(vec->item + cnt);
	    if (numex) push_some_svalues(extra, numex);
	    v = ob ? apply(func, ob, 1 + numex, ORIGIN_EFUN)
		: call_function_pointer(fp, 1 + numex);
	    if (!IS_ZERO(v)){
		flags[cnt] = 1;
		res++;
	    } else flags[cnt] = 0;
	}
	r = allocate_empty_array(res);
	if (res){
	    while (cnt--) {
		if (flags[cnt])
		    assign_svalue_no_free(&r->item[--res], vec->item+cnt);
	    }
	}

	FREE(flags);
	sp--;
	pop_n_elems(num_arg - 1);
	free_vector(vec);
	sp->u.vec = r;
    }
}

#ifdef FILTER_ARRAY
void c_filter_array P5(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2, svalue *, s3)
{
    error("c_filter_array needs to be written.\n");
    /* TODO */
}
#endif
#endif

/* Unique maker

   These routines takes an array of objects and calls the function 'func'
   in them. The return values are used to decide which of the objects are
   unique. Then an array on the below form are returned:

   ({
   ({Same1:1, Same1:2, Same1:3, .... Same1:N }),
   ({Same2:1, Same2:2, Same2:3, .... Same2:N }),
   ({Same3:1, Same3:2, Same3:3, .... Same3:N }),
   ....
   ....
   ({SameM:1, SameM:2, SameM:3, .... SameM:N }),
   })

   i.e an array of arrays consisting of lists of objectpointers
   to all the nonunique objects for each unique set of objects.

   The basic purpose of this routine is to speed up the preparing of the
   array used for describing.

   */

/* nonstatic, is used in mappings too */
int sameval P2(struct svalue *, arg1, struct svalue *, arg2)
{
    DEBUG_CHECK(!arg1 || !arg2, "Null pointer passed to sameval.\n");

    switch (arg1->type | arg2->type) {
    case T_NUMBER:
	return arg1->u.number == arg2->u.number;
    case T_POINTER:
	return arg1->u.vec == arg2->u.vec;
    case T_STRING:
	return !strcmp(arg1->u.string, arg2->u.string);
    case T_OBJECT:
	return arg1->u.ob == arg2->u.ob;
    case T_MAPPING:
	return arg1->u.map == arg2->u.map;
    case T_FUNCTION:
	return arg1->u.fp == arg2->u.fp;
    case T_REAL:
	return arg1->u.real == arg2->u.real;
    case T_BUFFER:
	return arg1->u.buf == arg2->u.buf;
    }
    return 0;
}

#ifdef F_UNIQUE_ARRAY

struct unique {
    struct svalue mark;
    int count;
    struct unique *next;
    int *indices;
};

struct unique_list {
    struct unique *head;
    struct unique_list *next;
};

static struct unique_list *g_u_list = 0;

void unique_array_error_handler PROT((void)) {
    struct unique_list *unlist = g_u_list;
    struct unique *uptr = unlist->head, *nptr;

    g_u_list = g_u_list->next;
    while (uptr) {
	nptr = uptr->next;
	FREE((char *) uptr->indices);
	free_svalue(&uptr->mark, "unique_array_error_handler");
	FREE((char *) uptr);
	uptr = nptr;
    }
    FREE((char *)unlist);
}

void f_unique_array PROT((void)){
    struct vector *v, *ret;
    int size, i, numkeys = 0, *ind, num_arg = st_num_arg;
    struct svalue *skipval, *sv, *svp;
    struct unique_list *unlist;
    struct unique **head, *uptr, *nptr;
    struct funp *fp = 0;
    char *func;

    size = (v = (sp - num_arg + 1)->u.vec)->size;
    if (!size) {
	if (num_arg == 3) free_svalue(sp--, "f_unique_array");
	free_svalue(sp--, "f_unique_array");
	return;
    }

    if (num_arg == 3) {
	skipval = sp;
	if ((sp-1)->type & T_FUNCTION) fp = (sp-1)->u.fp;
	else func = (sp-1)->u.string;
    }
    else {
	skipval = &const0;
	if (sp->type & T_FUNCTION) fp = sp->u.fp;
	else func = sp->u.string;
    }

    unlist = ALLOCATE(struct unique_list, TAG_TEMPORARY, "f_unique_array:1");
    unlist->next = g_u_list;
    unlist->head = 0;
    head = &unlist->head;
    g_u_list = unlist;

    (++sp)->type = T_ERROR_HANDLER;
    sp->u.error_handler = unique_array_error_handler;
    
    for (i = 0; i < size; i++) {
	if (fp) {
	    push_svalue(v->item + i);
	    sv = call_function_pointer(fp, 1);
	} else if ((v->item + i)->type & T_OBJECT) {
	    sv = apply(func, (v->item + i)->u.ob, 0, ORIGIN_EFUN);
	} else sv = 0;

	if (sv && !sameval(sv, skipval)) {
	    uptr = *head;
	    while (uptr) {
		if (sameval(sv, &uptr->mark)) {
		    uptr->indices = RESIZE(uptr->indices, uptr->count + 1, int,
					   TAG_TEMPORARY, "f_unique_array:2");
		    uptr->indices[uptr->count++] = i;
		    break;
		}
		uptr = uptr->next;
	    }
	    if (!uptr) {
		numkeys++;
		uptr = ALLOCATE(struct unique, TAG_TEMPORARY, "f_unique_array:3");
		uptr->indices = ALLOCATE(int, TAG_TEMPORARY, "f_unique_array:4");
		uptr->count = 1;
		uptr->indices[0] = i;
		uptr->next = *head;
		assign_svalue_no_free(&uptr->mark, sv);
		*head = uptr;
	    }
	}
    }

    ret = allocate_empty_array(numkeys);
    uptr = *head;
    svp = v->item;
    while (numkeys--) {
	nptr = uptr->next;
	(sv = ret->item + numkeys)->type = T_POINTER;
	sv->u.vec = allocate_empty_array(i = uptr->count);
	skipval = sv->u.vec->item + i;
	ind = uptr->indices;
	while (i--) {
	    assign_svalue_no_free(--skipval, svp + ind[i]);
	}
	FREE((char *)ind);
	free_svalue(&uptr->mark, "f_unique_array");
	FREE((char *)uptr);
	uptr = nptr;
    }

    unlist = g_u_list->next;
    FREE((char *)g_u_list);
    g_u_list = unlist;
    sp--;
    pop_n_elems(num_arg - 1);
    free_vector(v);
    sp->u.vec = ret;
}

/*
 * End of Unique maker
 *************************
 */
#endif

/* Concatenation of two arrays into one
 */
struct vector *add_array P2(struct vector *, p, struct vector *, r)
{
    int cnt, res;
    struct vector *d;		/* destination */

    /*
     * have to be careful with size zero arrays because they could be
     * null_vector.  REALLOC(null_vector, ...) is bad :(
     */
    if (p->size == 0) {
	p->ref--;
	return r->ref > 1 ? (r->ref--, copy_array(r)) : r;
    }
    if (r->size == 0) {
	r->ref--;
	return p->ref > 1 ? (p->ref--, copy_array(p)) : p;
    }

    res = p->size + r->size;
    if (res < 0 || res > max_array_size)
      error("result of array addition is greater than maximum array size.\n");

    /* x += x */
    if ((p == r) && (p->ref == 2)) {
	d = RESIZE_VECTOR(p, res);
        if (!d)
	    fatal("Out of memory.\n");
        /* copy myself */
	for (cnt = d->size; cnt--;)
	    assign_svalue_no_free(&d->item[--res], &d->item[cnt]);
        total_array_size += sizeof(struct svalue) * (d->size);
#ifndef NO_MUDLIB_STATS
	/* mudlib_stats stuff */
	if (current_object) {
	    assign_stats(&d->stats, current_object);
	    add_array_size(&d->stats, d->size);
	} else {
	    null_stats(&d->stats);
	}
#endif
        d->ref = 1;
        d->size <<= 1;
	
        return d;
    }
    
    /* transfer svalues for ref 1 target array */
    if (p->ref == 1) {
	/*
	 * realloc(p) to try extending block; this will save an
	 * allocate_array(), copying the svalues over, and free()'ing p
	 */
	d = RESIZE_VECTOR(p, res);
	if (!d)
	    fatal("Out of memory.\n");

	total_array_size += sizeof(struct svalue) * (r->size);
	/* d->ref = 1;     d is p, and p's ref was already one -Beek */
	d->size = res;

#ifndef NO_MUDLIB_STATS
	/* mudlib_stats stuff */
	if (current_object) {
	    assign_stats(&d->stats, current_object);
	    add_array_size(&d->stats, r->size);
	} else {
	    null_stats(&d->stats);
	}
#endif
    } else {
	d = allocate_empty_array(res);

	for (cnt = p->size; cnt--;)
	    assign_svalue_no_free(&d->item[cnt], &p->item[cnt]);
	p->ref--;
    }

    /* transfer svalues from ref 1 source array */
    if (r->ref == 1) {
	for (cnt = r->size; cnt--;)
	    d->item[--res] = r->item[cnt];
#ifndef NO_MUDLIB_STATS
	add_array_size(&r->stats, -((int)r->size));
#endif
	num_arrays--;
	total_array_size -= sizeof(struct vector) +
	    sizeof(struct svalue) * (r->size - 1);
	FREE((char *) r);
    } else {
	for (cnt = r->size; cnt--;)
	    assign_svalue_no_free(&d->item[--res], &r->item[cnt]);
	r->ref--;
    }

    return d;
}

/* Returns an array of all objects contained in 'ob'
 */
struct vector *all_inventory P2(struct object *, ob, int, override)
{
    struct vector *d;
    struct object *cur;
    int cnt, res;
    int display_hidden;

    if (override) {
	display_hidden = 1;
    } else {
	display_hidden = -1;
    }
    cnt = 0;
    for (cur = ob->contains; cur; cur = cur->next_inv) {
	if (cur->flags & O_HIDDEN) {
	    if (display_hidden == -1) {
		display_hidden = valid_hide(current_object);
	    }
	    if (display_hidden)
		cnt++;
	} else
	    cnt++;
    }

    if (!cnt)
	return null_array();

    d = allocate_empty_array(cnt);
    cur = ob->contains;

    for (res = 0; res < cnt; res++) {
	if ((cur->flags & O_HIDDEN) && !display_hidden) {
	    cur = cur->next_inv;
	    res--;
	    continue;
	}
	d->item[res].type = T_OBJECT;
	d->item[res].u.ob = cur;
	add_ref(cur, "all_inventory");
	cur = cur->next_inv;
    }
    return d;
}


/* Runs all elements of an array through ob::func
   and replaces each value in arr by the value returned by ob::func
   */
#ifdef F_MAP
void
map_array P2(struct svalue *, arg, int, num_arg)
{
    struct vector *arr = arg->u.vec;
    struct vector *r;
    int size;

    if ((size = arr->size) < 1) r = null_array();
    else {
	struct funp *fp = 0;
	int numex = 0, cnt;
	struct object *ob;
	struct svalue *extra, *v;
	char *func;

	r = allocate_array(size);

	(++sp)->type = T_POINTER;
	sp->u.vec = r;

	if (arg[1].type == T_FUNCTION) {
	    fp = arg[1].u.fp;
	    if (num_arg > 2) extra = arg + 2, numex = num_arg - 2;
	}
	else {
	    func = arg[1].u.string;
	    if (num_arg < 3) ob = current_object;
	    else{
		if (arg[2].type == T_OBJECT) ob = arg[2].u.ob;
		else if (arg[2].type == T_STRING){
		    if ((ob = find_object(arg[2].u.string)) && !object_visible(ob))
			ob = 0;
		}
		if (num_arg > 3) extra = arg + 3, numex = num_arg - 3;
		if (!ob) error("Bad argument 3 to map_array.\n");
	    }
	}
	    
	for (cnt = 0; cnt < size; cnt++){
	    push_svalue(arr->item + cnt);
	    if (numex) push_some_svalues(extra, numex);
	    v = fp ? call_function_pointer(fp, numex + 1) : apply(func, ob, 1 + numex, ORIGIN_EFUN);
	    if (v) assign_svalue_no_free(&r->item[cnt], v);
	    else break;
	}
	sp--;
    }

    pop_n_elems(num_arg);
    (++sp)->type = T_POINTER;
    sp->u.vec = r;
}

void
map_string P2(struct svalue *, arg, int, num_arg)
{
    char *arr = arg->u.string;
    char *p;
    struct funp *fp = 0;
    int numex = 0;
    struct object *ob;
    struct svalue *extra, *v;
    char *func;

    /* get a modifiable string */
    /* do not use arg after this; it has been copied or freed.
       Put our result string on the stack where it belongs in case of an
       error (note it is also in the right spot for the return value).
     */
    if (arg->subtype != STRING_MALLOC) {
	arr = string_copy(arr, "map_string");
	free_string_svalue(arg);
	arg->subtype = STRING_MALLOC;
	arg->u.string = arr;
    }

    if (arg[1].type == T_FUNCTION) {
	fp = arg[1].u.fp;
	if (num_arg > 2) extra = arg + 2, numex = num_arg - 2;
    }
    else {
	func = arg[1].u.string;
	if (num_arg < 3) ob = current_object;
	else{
	    if (arg[2].type == T_OBJECT) ob = arg[2].u.ob;
	    else if (arg[2].type == T_STRING){
		if ((ob = find_object(arg[2].u.string)) && !object_visible(ob))
		    ob = 0;
	    }
	    if (num_arg > 3) extra = arg + 3, numex = num_arg - 3;
	    if (!ob) error("Bad argument 3 to map_string.\n");
	}
    }

    for (p = arr; *p; p++) {
	push_number((unsigned char)*p);
	if (numex) push_some_svalues(extra, numex);
	v = fp ? call_function_pointer(fp, numex + 1) : apply(func, ob, 1 + numex, ORIGIN_EFUN);
	/* no function or illegal return value becomes a ' '.
	 * Anyone got a better idea?  A few idea:
	 * (1) insert strings? - algorithm needs changing
	 * (2) ignore them? - again, could require a realloc, since size would
	 *                    change
	 */
	if (!v) break;
	if (v->type == T_NUMBER && v->u.number != 0) *p = ((unsigned char)(v->u.number));
    }

    pop_n_elems(num_arg - 1);
    /* return value on stack */
}
#endif

static struct funp *sort_array_cmp_funp;
static struct object *sort_array_cmp_ob;
static char *sort_array_cmp_func;

#define COMPARE_NUMS(x,y) (x < y ? -1 : (x > y ? 1 : 0))

struct vector *builtin_sort_array P2(struct vector *, inlist, int, dir)
{
    quickSort((char *) inlist->item, inlist->size, sizeof(inlist->item),
	      (dir<0) ? builtin_sort_array_cmp_rev : builtin_sort_array_cmp_fwd);

    return inlist;
}

INLINE static int builtin_sort_array_cmp_fwd P2(struct svalue *, p1, struct svalue *, p2)
{
    switch(p1->type | p2->type){
	case T_STRING:
	{
	    return strcmp(p1->u.string, p2->u.string);
	}

	case T_NUMBER:
	{
	    return COMPARE_NUMS(p1->u.number, p2->u.number);
	}

	case T_REAL:
	{
	    return COMPARE_NUMS(p1->u.real, p2->u.real);
	}

	case T_POINTER:
	{
	    struct vector *v1 = p1->u.vec, *v2 = p2->u.vec;
	    if (!v1->size  || !v2->size)
		error("Illegal to have empty array in array for sort_array()\n");


	    switch(v1->item->type | v2->item->type){
		case T_STRING:
		{
		    return strcmp(v1->item->u.string, v2->item->u.string);
		}

		case T_NUMBER:
		{
		    return COMPARE_NUMS(v1->item->u.number, v2->item->u.number);
		}

		case T_REAL:
		{
		    return COMPARE_NUMS(v1->item->u.real, v2->item->u.real);
		}
		default:
		{
		    /* Temp. long err msg till I can think of a better one - Sym */
		    error("sort_array() cannot handle arrays of arrays whose 1st elems\naren't strings/ints/floats\n");
		}
	    }
	}

    }
    error("built-in sort_array() can only handle homogeneous arrays of strings/ints/floats/arrays\n");
    return 0;
}

INLINE static int builtin_sort_array_cmp_rev P2(struct svalue *, p1, struct svalue *, p2)
{
    switch(p1->type | p2->type){
        case T_STRING:
        {
            return strcmp(p2->u.string, p1->u.string);
	}

        case T_NUMBER:
        {
            return COMPARE_NUMS(p2->u.number, p1->u.number);
	}

        case T_REAL:
        {
            return COMPARE_NUMS(p2->u.real, p1->u.real);
	}

        case T_POINTER:
        {
            struct vector *v1 = p1->u.vec, *v2 = p2->u.vec;
            if (!v1->size  || !v2->size)
                error("Illegal to have empty array in array for sort_array()\n");


            switch(v1->item->type | v2->item->type){
                case T_STRING:
                {
                    return strcmp(v2->item->u.string, v1->item->u.string);
		}

                case T_NUMBER:
                {
                    return COMPARE_NUMS(v2->item->u.number, v1->item->u.number);
		}

                case T_REAL:
                {
                    return COMPARE_NUMS(v2->item->u.real, v1->item->u.real);
		}
                default:
		{
		    /* Temp. long err msg till I can think of a better one - Sym */
		    error("sort_array() cannot handle arrays of arrays whose 1st elems\naren't strings/ints/floats\n");
		}
	    }
	}

    }
    error("built-in sort_array() can only handle homogeneous arrays of strings/ints/floats/arrays\n");
    return 0;
}

INLINE static
int sort_array_cmp P2(struct svalue *, p1, struct svalue *, p2) {
    struct svalue *d;

    push_svalue(p1);
    push_svalue(p2);

    if (sort_array_cmp_funp){
	d = call_function_pointer(sort_array_cmp_funp, 2);
    } else {	
	if (sort_array_cmp_ob->flags & O_DESTRUCTED)
	    error("object used by sort_array destructed\n");
	d = apply(sort_array_cmp_func, sort_array_cmp_ob, 2, ORIGIN_EFUN);
    }

    if (!d || d->type != T_NUMBER) {
	return 0;
    } else {
	return d->u.number;
    }
}

#ifdef F_SORT_ARRAY
void
f_sort_array PROT((void))
{
    struct svalue *arg = sp - st_num_arg + 1;
    struct vector *tmp = arg->u.vec;
    int num_arg = st_num_arg;

    check_for_destr(tmp);
    tmp = copy_array(tmp);

    switch(arg[1].type){
	case T_NUMBER:
        {
	    tmp = builtin_sort_array(tmp, arg[1].u.number);
	    break;
	}

	case T_FUNCTION:
        {
	    sort_array_cmp_funp = arg[1].u.fp;
	    quickSort((char *) tmp->item, tmp->size, sizeof(tmp->item), sort_array_cmp);
	    break;
	}

	case T_STRING:
        {
	    sort_array_cmp_funp = 0;
	    sort_array_cmp_ob = 0;
	    sort_array_cmp_func = (arg+1)->u.string;
	    if (num_arg == 2) {
		sort_array_cmp_ob = current_object;
		if (sort_array_cmp_ob->flags & O_DESTRUCTED) sort_array_cmp_ob = 0;
	    } else if (arg[2].type & T_OBJECT)
		sort_array_cmp_ob = arg[2].u.ob;
	    else if (arg[2].type & T_STRING) {
		sort_array_cmp_ob = find_object(arg[2].u.string);
		if (sort_array_cmp_ob && !object_visible(sort_array_cmp_ob)) sort_array_cmp_ob = 0;
	    }
	    if (!sort_array_cmp_ob) bad_arg(3, F_SORT_ARRAY);
	    quickSort((char *) tmp->item, tmp->size, sizeof(tmp->item), sort_array_cmp);
	    break;
	}

    }
    
    pop_n_elems(num_arg);
    (++sp)->type = T_POINTER;
    sp->u.vec = tmp;
}
#endif

/*
 * deep_inventory()
 *
 * This function returns the recursive inventory of an object. The returned
 * array of objects is flat, ie there is no structure reflecting the
 * internal containment relations.
 *
 * This is Robocoder's deep_inventory().  It uses two passes in order to
 * avoid costly temporary arrays (allocating, copying, adding, freeing, etc).
 * The recursive call routines are:
 *    deep_inventory_count() and deep_inventory_collect()
 */
static int valid_hide_flag;

static int deep_inventory_count P1(struct object *, ob)
{
    struct object *cur;
    int cnt;

    cnt = 0;

    /* step through object's inventory and count visible objects */
    for (cur = ob->contains; cur; cur = cur->next_inv) {
	if (cur->flags & O_HIDDEN) {
	    if (!valid_hide_flag)
		valid_hide_flag = 1 +
		    (valid_hide(current_object) ? 1 : 0);
	    if (valid_hide_flag & 2) {
		cnt++;
		cnt += deep_inventory_count(cur);
	    }
	} else {
	    cnt++;
	    cnt += deep_inventory_count(cur);
	}
    }

    return cnt;
}

static void deep_inventory_collect P3(struct object *, ob, struct vector *, inv, int *, i)
{
    struct object *cur;

    /* step through object's inventory and look for visible objects */
    for (cur = ob->contains; cur; cur = cur->next_inv) {
	if (cur->flags & O_HIDDEN) {
	    if (valid_hide_flag & 2) {
		inv->item[*i].type = T_OBJECT;
		inv->item[*i].u.ob = cur;
		(*i)++;
		add_ref(cur, "deep_inventory_collect");

		deep_inventory_collect(cur, inv, i);
	    }
	} else {
	    inv->item[*i].type = T_OBJECT;
	    inv->item[*i].u.ob = cur;
	    (*i)++;
	    add_ref(cur, "deep_inventory_collect");

	    deep_inventory_collect(cur, inv, i);
	}
    }
}

struct vector *deep_inventory P2(struct object *, ob, int, take_top)
{
    struct vector *dinv;
    int i;

    valid_hide_flag = 0;

    /*
     * count visible objects in an object's inventory, and in their
     * inventory, etc
     */
    i = deep_inventory_count(ob);
    if (take_top)
	i++;

    if (i == 0)
	return null_array();

    /*
     * allocate an array
     */
    dinv = allocate_empty_array(i);
    if (take_top) {
	dinv->item[0].type = T_OBJECT;
	dinv->item[0].u.ob = ob;
	add_ref(ob, "deep_inventory");
    }
    /*
     * collect visible inventory objects recursively
     */
    i = take_top;
    deep_inventory_collect(ob, dinv, &i);

    return dinv;
}

INLINE static int alist_cmp P2(struct svalue *, p1, struct svalue *, p2)
{
    register int d;

    if ((d = p1->u.number - p2->u.number))
	return d;
    if ((d = p1->type - p2->type))
	return d;
    return 0;
}

INLINE static struct svalue *alist_sort P1(struct vector *, inlist) {
    int size, j, curix, parix, child1, child2, flag;
    struct svalue *sv_tab, *tmp, *table, *sv_ptr, val;
    char *str;

    if (!(size = inlist->size)) return (struct svalue *)NULL;
    if (flag = (inlist->ref > 1)) {
	sv_tab = CALLOCATE(size, struct svalue, TAG_TEMPORARY, "alist_sort: sv_tab");
	sv_ptr = inlist->item;
	for (j = 0; j < size; j++) {
	    if (((tmp = (sv_ptr + j))->type & T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
		free_object(tmp->u.ob, "alist_sort");
		sv_tab[j] = *tmp = const0;
	    } else if ((tmp->type & T_STRING) && !(tmp->subtype & STRING_SHARED)) {
		sv_tab[j].u.string = make_shared_string(tmp->u.string);
		(tmp = sv_tab + j)->subtype = STRING_SHARED;
		tmp->type = T_STRING;
	    } else assign_svalue_no_free(sv_tab + j, tmp);

	    if (curix = j){
		val = *tmp;

		do {
		    parix = (curix - 1) >> 1;
		    if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
			sv_tab[curix] = sv_tab[parix];
			sv_tab[parix] = val;
		    }
		} while (curix = parix);
	    }
	}
    } else {
	sv_tab = inlist->item;
	for (j = 0; j < size; j++){
	    if (((tmp = (sv_tab + j))->type & T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
		free_object(tmp->u.ob, "alist_sort");
		*tmp = const0;
	    } else if ((tmp->type & T_STRING) && !(tmp->subtype & STRING_SHARED)) {
		str = make_shared_string(tmp->u.string);
		free_string_svalue(tmp);
		tmp->u.string = str;
		tmp->subtype = STRING_SHARED;
	    }

	    if (curix = j){
		val = *tmp;
	    
		do {
		    parix = (curix - 1) >> 1;
		    if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
			sv_tab[curix] = sv_tab[parix];
			sv_tab[parix] = val;
		    }
		} while (curix = parix);
	    }
	}
    }

    table = CALLOCATE(size, struct svalue, TAG_TEMPORARY, "alist_sort: table");

    for (j = 0; j < size; j++) {
	table[j] = sv_tab[0];

	for (curix = 0; ; ) {
	    child1 = (curix << 1) + 1;
	    child2 = child1 + 1;
	    
	    if (child2 < size && sv_tab[child2].type != T_INVALID &&
		(sv_tab[child1].type == T_INVALID ||
		 alist_cmp(sv_tab+child1, sv_tab+child2) > 0)) {
		child1 = child2;
	    }
	    if (child1 < size && sv_tab[child1].type != T_INVALID) {
		sv_tab[curix] = sv_tab[child1];
		curix = child1;
	    } else break;
	}

	sv_tab[curix].type = T_INVALID;
    }

    if (flag) FREE((char *)sv_tab);
    return table;
}

struct vector *subtract_array P2(struct vector *, minuend, struct vector *, subtrahend) {
    struct vector *difference;
    struct svalue *source, *dest, *svt;
    int i, size, o, d, l, h, msize;

    if (!(size = subtrahend->size)) {
	subtrahend->ref--;
	return minuend->ref > 1 ? (minuend->ref--, copy_array(minuend)) : minuend;
    }
    if (!(msize = minuend->size)) {
	free_vector(subtrahend);
	return &null_vector;
    }
    svt = alist_sort(subtrahend);
    difference = ALLOC_VECTOR(msize);
    for (source = minuend->item, dest = difference->item, i = msize;
	 i--; source++) {

	l = 0;
	o = (h = size - 1) >> 1;

	if ((source->type & T_OBJECT) && (source->u.ob->flags & O_DESTRUCTED)) {
	    free_object(source->u.ob, "subtract_array");
	    *source = const0;
	} else if ((source->type & T_STRING) && !(source->subtype & STRING_SHARED)) {
	    static struct svalue stmp = {T_STRING, STRING_SHARED};
	    
	    if (!(stmp.u.string = findstring(source->u.string))){
	        assign_svalue_no_free(dest++, source);
		continue;
	    }
	    while (d = alist_cmp(&stmp, svt + o)){
		if (d < 0) h = o - 1;
		else l = o + 1;
		if (l > h){
		    assign_svalue_no_free(dest++, source);
		    break;
		}
		o = (l + h) >> 1;
	    }
	    continue;
	}
	
	while (d = alist_cmp(source, svt + o)) {
	    if (d < 0) h = o - 1;
	    else l = o + 1;
	    if (l > h) {
		assign_svalue_no_free(dest++, source);
		break;
	    }
	    o = (l + h) >> 1;
	}

    }
    i = size;
    while (i--) free_svalue(svt + i, "subtract_array");
    FREE((char *) svt);
    if (subtrahend->ref > 1) {
	subtrahend->ref--;
    } else {
#ifndef NO_MUDLIB_STATS
	add_array_size(&subtrahend->stats, -size);
#endif
	num_arrays--;
	total_array_size -= sizeof(struct vector) + sizeof(struct svalue) *
	    (size - 1);
	FREE((char *) subtrahend);
    }
    free_vector(minuend);
    msize = dest - difference->item;
    if (!msize) {
	FREE((char *)difference);
	return null_array();
    }
    difference = RESIZE_VECTOR(difference, msize);
    difference->size = msize;
    difference->ref = 1;
    total_array_size += sizeof(struct vector) + sizeof(struct svalue[1]) * (msize - 1);
    num_arrays++;
#ifndef NO_MUDLIB_STATS
    if (current_object) {
	assign_stats(&difference->stats, current_object);
	add_array_size(&difference->stats, msize);
    } else {
	null_stats(&difference->stats);
    }
#endif
    return difference;
}

struct vector *intersect_array P2(struct vector *, a1, struct vector *, a2) {
    struct vector *a3;
    int d, l, j, i, a1s = a1->size, a2s = a2->size, flag;
    struct svalue *svt_1, *ntab, *sv_tab, *sv_ptr, val, *tmp;
    int curix, parix, child1, child2;
    
    if (!a1s || !a2s) {
	free_vector(a1);
	free_vector(a2);
	return null_array();
    }

    svt_1 = alist_sort(a1);
    if (flag = (a2->ref > 1)) {
	sv_tab = CALLOCATE(a2s, struct svalue, TAG_TEMPORARY, "intersect_array: sv2_tab");
	sv_ptr = a2->item;
	for (j = 0; j < a2s; j++) {
	    if (((tmp = (sv_ptr + j))->type & T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
		free_object(tmp->u.ob, "intersect_array");
		sv_tab[j] = *tmp = const0;
	    } else if ((tmp->type & T_STRING) && !(tmp->subtype & STRING_SHARED)) {
		sv_tab[j].u.string = make_shared_string(tmp->u.string);
		(tmp = sv_tab + j)->subtype = STRING_SHARED;
		tmp->type = T_STRING;
	    } else assign_svalue_no_free(sv_tab + j, tmp);
	    
	    if (curix = j){
		val = *tmp;

		do {
		    parix = (curix - 1) >> 1;
		    if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
			sv_tab[curix] = sv_tab[parix];
			sv_tab[parix] = val;
		    }
		} while (curix = parix);
	    } 
	}
    } else {
	char *str;

	sv_tab = a2->item;
	for (j = 0; j < a2s; j++) {
	    if (((tmp = (sv_tab + j))->type & T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
		free_object(tmp->u.ob, "alist_sort");
		*tmp = const0;
	    } else if ((tmp->type & T_STRING) && !(tmp->subtype & STRING_SHARED)) {
		str = make_shared_string(tmp->u.string);
		free_string_svalue(tmp);
		tmp->u.string = str;
		tmp->subtype = STRING_SHARED;
	    }

	    if (curix = j){
		val = *tmp;

		do {
		    parix = (curix - 1) >> 1;
		    if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
			sv_tab[curix] = sv_tab[parix];
			sv_tab[parix] = val;
		    }
		} while (curix = parix);
	    } 	    
	}
    }

    a3 = ALLOC_VECTOR(a2s);
    ntab = a3->item;

    l = i = 0;

    for (j = 0; j < a2s; j++) {
	val = sv_tab[0];

	while ((d = alist_cmp(&val, &svt_1[i])) > 0) {
	    if (++i >= a1s) goto settle_business;
	}

	if (!d) {
	    ntab[l++] = val;
	} else {
	    free_svalue(&val, "intersect_array");
	}

	for (curix = 0;; ) {
	    child1 = (curix << 1) + 1;
	    child2 = child1 + 1;
	    
	    if (child2 < a2s && sv_tab[child2].type != T_INVALID &&
		(sv_tab[child1].type == T_INVALID ||
		 alist_cmp(sv_tab + child1, sv_tab + child2) > 0)) {
		child1 = child2;
	    }
	    
	    if (child1 < a2s && sv_tab[child1].type != T_INVALID) {
		sv_tab[curix] = sv_tab[child1];
		curix = child1;
	    } else break;
	}
	
	sv_tab[curix].type = T_INVALID;
    }

 settle_business:
    
    curix = a2s;
    while (curix--){
	if (sv_tab[curix].type != T_INVALID) free_svalue(sv_tab + curix, "intersect_array:2");
    }

    i = a1s;
    while (i--) free_svalue(svt_1 + i, "intersect_array");
    FREE((char *)svt_1);

    if (a1->ref > 1) a1->ref--;
    else {
#ifndef NO_MUDLIB_STATS
	add_array_size(&a1->stats, -a1s);
#endif
	num_arrays--;
	total_array_size -= sizeof(struct vector) + sizeof(struct svalue) * (a1s - 1);
	FREE((char *) a1);
    }
    
    if (flag) {
	a2->ref--;
	FREE((char *) sv_tab);
    } else {
#ifndef NO_MUDLIB_STATS
	add_array_size(&a2->stats, -a2s);
#endif
	num_arrays--;
	total_array_size -= sizeof(struct vector) + sizeof(struct svalue) * (a2s - 1);
	FREE((char *) a2);
    }
    a3 = RESIZE_VECTOR(a3, l);
    a3->ref = 1;
    a3->size = l;
#ifndef NO_MUDLIB_STATS
    if (current_object) {
	assign_stats(&a3->stats, current_object);
	add_array_size(&a3->stats, l);
    } else null_stats(&a3->stats);
#endif
    total_array_size += sizeof(struct vector) + (l-1) * sizeof(struct svalue);
    num_arrays++;
    return a3;
}

struct vector *match_regexp P3(struct vector *, v, char *, pattern, int, flag) {
    struct regexp *reg;
    char *res;
    int num_match, size, match = !(flag & 2);
    struct vector *ret;
    struct svalue *sv1, *sv2;

    if (!(size = v->size)) return null_array();
    reg = regcomp(pattern, 0);
    if (!reg) return 0;
    res = (char *)DMALLOC(size, TAG_TEMPORARY, "match_regexp: res");
    sv1 = v->item + size;
    num_match = 0;
    while (size--){
        if (!((--sv1)->type & T_STRING) || (regexec(reg, sv1->u.string) != match)){
            res[size] = 0;
	} else {
	    res[size] = 1;
	    num_match++;
	}
    }
    
    flag &= 1;
    ret = allocate_empty_array(num_match << flag);
    sv2 = ret->item + (num_match << flag);
    size = v->size;
    while (size--){
        if (res[size]){
            if (flag) {
                (--sv2)->type = T_NUMBER;
                sv2->u.number = size + 1;
	    }
            (--sv2)->type = T_STRING;
            sv1 = v->item + size;
            switch(sv1->subtype){
		case STRING_MALLOC:
		    sv2->subtype = STRING_MALLOC;
		    sv2->u.string = string_copy(sv1->u.string, "match_regexp");
                    break;
	        case STRING_SHARED:
                    sv2->subtype = STRING_SHARED;
                    sv2->u.string = ref_string(sv1->u.string);
                    break;
	        case STRING_CONSTANT:
                    sv2->subtype = STRING_SHARED;
                    sv2->u.string = make_shared_string(sv1->u.string);
                    break;
		}
            if (!--num_match) break;
	}
    }
    FREE(res);
    FREE((char *) reg);
    return ret;
}

/*
 * Returns a list of all inherited files.
 *
 * Must be fixed so that any number of files can be returned, now max 256
 * (Sounds like a contradiction to me /Lars).
 */
struct vector *deep_inherit_list P1(struct object *, ob)
{
    struct vector *ret;
    struct program *pr, *plist[256];
    int il, il2, next, cur;

    plist[0] = ob->prog;
    next = 1;
    cur = 0;

    for (; cur < next && next < 256; cur++) {
	pr = plist[cur];
	for (il2 = 0; il2 < (int) pr->p.i.num_inherited; il2++)
	    plist[next++] = pr->p.i.inherit[il2].prog;
    }

    next--;
    ret = allocate_empty_array(next);

    for (il = 0; il < next; il++) {
	pr = plist[il + 1];
	ret->item[il].type = T_STRING;
	ret->item[il].subtype = STRING_SHARED;
	ret->item[il].u.string =
	    make_shared_string(pr->name);
    }
    return ret;
}

/*
 * Returns a list of the immediate inherited files.
 *
 */
struct vector *inherit_list P1(struct object *, ob)
{
    struct vector *ret;
    struct program *pr, *plist[256];
    int il, il2, next, cur;

    plist[0] = ob->prog;
    next = 1;
    cur = 0;

    pr = plist[cur];
    for (il2 = 0; il2 < (int) pr->p.i.num_inherited; il2++) {
	plist[next++] = pr->p.i.inherit[il2].prog;
    }

    next--;			/* don't count the file itself */
    ret = allocate_empty_array(next);

    for (il = 0; il < next; il++) {
	pr = plist[il + 1];
	ret->item[il].type = T_STRING;
	ret->item[il].subtype = STRING_SHARED;
	ret->item[il].u.string = make_shared_string(pr->name);
    }
    return ret;
}

struct vector *
       children P1(char *, str)
{
    int i, j;
    int t_sz;
    int sl, ol, needs_freed = 0;
    struct object *ob;
    struct object **tmp_children;
    struct vector *ret;
    int display_hidden;

    display_hidden = -1;
    /* Skip over leading '/' if any. */
    while (str[0] == '/') {
	str++;
    }
    /* Truncate possible .c in the object name. */
    sl = strlen(str);
    if ((sl > 2) && (str[sl - 2] == '.') &&
#ifndef LPC_TO_C
	(str[sl - 1] == 'c')) {
#else
	((str[sl - 1] == 'c') || (str[sl - 1] == 'C'))) {
#endif
	char *p;

	/*
	 * A new writable copy of the name is needed.
	 */
	/* (sl - 1) == minus ".c" plus "\0" */
	p = (char *) DMALLOC(sl - 1, TAG_TEMPORARY, "children: p");
	strncpy(p, str, sl - 2);
	p[sl - 2] = '\0';
	sl -= 2;		/* removed the ".c" */
	str = p;
	needs_freed = 1;
    }
    if (!(tmp_children = (struct object **)
	  DMALLOC(sizeof(struct object *) * (t_sz = 50),
		  TAG_TEMPORARY, "children: tmp_children"))) {
	if (needs_freed)
	    FREE(str);
	return null_array();	/* unable to malloc enough temp space */
    }
    for (i = 0, ob = obj_list; ob; ob = ob->next_all) {
	ol = strlen(ob->name);
	if (((ol == sl) || ((ol > sl) && (ob->name[sl] == '#')))
	    && !strncmp(str, ob->name, sl)) {
	    if (ob->flags & O_HIDDEN) {
		if (display_hidden == -1) {
		    display_hidden = valid_hide(current_object);
		}
		if (!display_hidden)
		    continue;
	    }
	    tmp_children[i] = ob;
	    if ((++i == t_sz) && (!(tmp_children
			= RESIZE(tmp_children, (t_sz += 50), struct object *,
				 TAG_TEMPORARY, "children: tmp_children: realloc")))) {
		/* unable to REALLOC more space 
		 * (tmp_children is now NULL) */
		if (needs_freed)
		    FREE(str);
		return null_array();
	    }
	}
    }
    if (i > max_array_size) {
	i = max_array_size;
    }
    ret = allocate_empty_array(i);
    for (j = 0; j < i; j++) {
	ret->item[j].type = T_OBJECT;
	ret->item[j].u.ob = tmp_children[j];
	add_ref(tmp_children[j], "children");
    }
    if (needs_freed) {
	FREE(str);
    }
    FREE((void *) tmp_children);
    return ret;
}

#ifdef F_LIVINGS
struct vector *livings()
{
    int nob, apply_valid_hide, hide_is_valid = 0;
    struct object *ob, **obtab;
    struct vector *vec;

    nob = 0;
    apply_valid_hide = 1;

    obtab = CALLOCATE(max_array_size, struct object *, TAG_TEMPORARY, "livings");

    for (ob = obj_list; ob != NULL; ob = ob->next_all) {
	if ((ob->flags & O_ENABLE_COMMANDS) == 0)
	    continue;
	if (ob->flags & O_HIDDEN) {
	    if (apply_valid_hide) {
		apply_valid_hide = 0;
		hide_is_valid = valid_hide(current_object);
	    }
	    if (hide_is_valid)
		continue;
	}
	if (nob == max_array_size)
	    break;
	obtab[nob++] = ob;
    }

    vec = allocate_empty_array(nob);
    while (--nob >= 0) {
	vec->item[nob].type = T_OBJECT;
	vec->item[nob].u.ob = obtab[nob];
	add_ref(obtab[nob], "livings");
    }

    FREE(obtab);

    return vec;
}
#endif

#ifdef F_OBJECTS
void f_objects PROT((void))
{
    char *func;
    struct object *ob, **tmp;
    struct vector *ret;
    struct funp *f = 0;
    int display_hidden = 0, t_sz, i,j, num_arg = st_num_arg;
    struct svalue *v;

    if (!num_arg) func = 0;
    else if (sp->type & T_FUNCTION) f = sp->u.fp;
    else func = sp->u.string;
	
    if (!(tmp = CALLOCATE(t_sz = 1000, struct object *, TAG_TEMPORARY, "objects: tmp")))
	fatal("Out of memory!\n");

    /* disguise DMALLOC()'d vector */
    push_malloced_string((char *) tmp);

    for (i = 0, ob = obj_list; ob; ob = ob->next_all) {
	if (ob->flags & O_HIDDEN) {
	    if (!display_hidden)
		display_hidden = 1 + !!valid_hide(current_object);
	    if (!(display_hidden & 2))
		continue;
	}
	if (f) {
	    push_object(ob);
	    v = call_function_pointer(f, 1);
	    if (!v){
		FREE((void *) tmp);
		sp--;
		free_svalue(sp,"f_objects");
		*sp = const0;
		return;
	    }
	    if (v->type == T_NUMBER && !v->u.number) continue;
	} else if (func){
	    push_object(ob);
	    v = apply(func, current_object, 1, ORIGIN_EFUN);
            if (!v){
                FREE((void *) tmp);
                sp--;
                free_svalue(sp,"f_objects");
                *sp = const0;
                return;
	    }
            if ((v->type & T_NUMBER) && !v->u.number) continue;
	}

	tmp[i] = ob;
	if (++i == t_sz) {
	    if (!(tmp = RESIZE(tmp, t_sz += 1000, struct object *,
			       TAG_TEMPORARY, "objects: tmp: realloc")))
		fatal("Out of memory!\n");
	    else
		sp->u.string = (char *) tmp;
	}
    }
    if (i > max_array_size)
	i = max_array_size;
    ret = allocate_empty_array(i);
    for (j = 0; j < i; j++) {
	ret->item[j].type = T_OBJECT;
	ret->item[j].u.ob = tmp[j];
	add_ref(tmp[j], "objects");
    }

    FREE((void *) tmp);
    sp--;
    pop_n_elems(num_arg);
    (++sp)->type = T_POINTER;
    sp->u.vec = ret;
}
#endif

#ifdef F_REG_ASSOC
/*
 * write(sprintf("%O", reg_assoc("testhahatest", ({ "haha", "te" }),
 *              ({ 2,3 }), 4)));

 * --------
 * ({
 *   ({ "", "te", "st", "haha", "", "te", "st" }),
 *   ({  4,    3,    4,      2,  4,    3,   4  })
 * })
 *
 */
struct vector *reg_assoc P4(char *, str, struct vector *, pat, struct vector *, tok, struct svalue *, def) {
    int i, size;
    char *tmp;
    struct vector *ret;
    
    if ((size = pat->size) != tok->size)
	error("Pattern and token array sizes must be identical.\n");
    
    for (i = 0; i < size; i++) {
	if (!(pat->item[i].type & T_STRING))
	    error("Non-string found in pattern array.\n");
    }
    
    ret = allocate_empty_array(2);
    
    if (size) {
	struct regexp **rgpp;
	struct reg_match {
 	    int tok_i;
	    char *begin, *end;
	    struct reg_match *next;
 	} *rmp = (struct reg_match *) 0, *rmph = (struct reg_match *) 0;
	int num_match = 0, length;
	struct svalue *sv1, *sv2, *sv;
 	int index;
	struct regexp *tmpreg;
 	char *laststart, *currstart;
 
	rgpp = CALLOCATE(size, struct regexp *, TAG_TEMPORARY, "reg_assoc : rgpp");
	for (i = 0; i < size; i++){
             if (!(rgpp[i] = regcomp(pat->item[i].u.string, 0))){
		 while (i--)
		     FREE((char *)rgpp[i]);
		 FREE((char *) rgpp);
		 return null_array();
	     }
 	}
 
         tmp = str;
         while (*tmp){
 
             /* Sigh - need a kludge here - Randor */
             /* In the future I may alter regexp.c to include branch info */
             /* so as to minimize checks here - Randor 5/30/94 */
 
 	    laststart = 0;
 	    index = -1;
 
	    for (i = 0; i < size; i++){
		if (regexec(tmpreg = rgpp[i], tmp)){
		    currstart = tmpreg->startp[0];
		    if (tmp == currstart){
			index = i;
			break;
 		    }
		    if (!laststart || currstart < laststart){
			laststart = currstart;
 			index = i;
 		    }
 		}
 	    }
 
            if (index >= 0){
		num_match++;
		if (rmp){
		    rmp->next = ALLOCATE(struct reg_match, 
					 TAG_TEMPORARY, "reg_assoc : rmp->next");
		    rmp = rmp->next;
 		}
		else rmph = rmp =
		    ALLOCATE(struct reg_match, TAG_TEMPORARY, "reg_assoc : rmp");
		tmpreg = rgpp[index];
		rmp->begin = tmpreg->startp[0];
		rmp->end = tmp = tmpreg->endp[0];
		rmp->tok_i = index; 
		rmp->next = (struct reg_match *) 0;
 	    }
	    else break;
 
	    /* The following is from regexplode, to prevent i guess infinite */
	    /* loops on "" patterns - Randor 5/29/94 */
	    if (rmp->begin == tmp && (!*++tmp)) break;
 	}
 
	sv = ret->item;
	sv->type = T_POINTER;
	sv1 = (sv->u.vec = allocate_empty_array(2*num_match + 1))->item;
	
	sv++;
	sv->type = T_POINTER;
	sv2 = (sv->u.vec = allocate_empty_array(2*num_match + 1))->item;
	
	rmp = rmph;
	
	tmp = str;
	
	while (num_match--) {
	    char *svtmp;
	    
	    length = rmp->begin - tmp;
	    sv1->type = T_STRING;
            sv1->subtype = STRING_MALLOC;
	    svtmp = sv1->u.string =
		(char *) DXALLOC(length + 1, TAG_STRING, "reg_assoc : sv1");
	    strncpy(svtmp, tmp, length);
	    svtmp[length] = 0;
	    sv1++;
	    assign_svalue_no_free(sv2++, def);
	    tmp += length;
	    length = rmp->end - rmp->begin;
	    sv1->type = T_STRING;
	    sv1->subtype = STRING_MALLOC;
	    svtmp = sv1->u.string =
		(char *) DXALLOC(length + 1, TAG_STRING, "reg_assoc : sv1");
	    strncpy(svtmp, tmp, length);
	    svtmp[length] = 0;
	    sv1++;
	    assign_svalue_no_free(sv2++, &tok->item[rmp->tok_i]);
	    tmp += length;
	    rmp = rmp->next;
	}
	sv1->type = T_STRING;
	sv1->subtype = STRING_MALLOC;
	sv1->u.string = string_copy(tmp, "reg_assoc");
	assign_svalue_no_free(sv2, def);
	for (i=0; i<size; i++)
	    FREE((char *)rgpp[i]);
	FREE((char *) rgpp);
	
	while (rmp = rmph){
	    rmph = rmp->next;
	    FREE((char *) rmp);
 	}
	return ret;
    }
    else { /* Default match */
	struct svalue *temp;
	struct svalue *sv;
	
	(sv = ret->item)->type = T_POINTER;
	temp = (sv->u.vec = allocate_empty_array(1))->item;
	temp->subtype = STRING_MALLOC;
	temp->type = T_STRING;
	temp->u.string = string_copy(str, "reg_assoc");
	sv = &ret->item[1];
	sv->type = T_POINTER;
	assign_svalue_no_free((sv->u.vec = allocate_empty_array(1))->item, def);
	return ret;
    }
}
#endif
