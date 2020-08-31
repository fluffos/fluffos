#include "base/std.h"

#include "vm/internal/base/machine.h"

#include <stdlib.h>  // for qsort

#include "vm/internal/apply.h"
#include "vm/internal/simulate.h"

#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif
#ifndef SANE_SORTING
#include "vm/internal/base/old_qsort_inc.h"
#endif

/*
 * This file contains functions used to manipulate arrays.
 * Some of them are connected to efuns, and some are only used internally
 * by the MudOS driver.
 */

static int builtin_sort_array_cmp_fwd(const void * /*vp1*/, const void * /*vp2*/);
static int builtin_sort_array_cmp_rev(const void * /*vp1*/, const void * /*vp2*/);
static int sort_array_cmp(const void * /*vp1*/, const void * /*vp2*/);
static long alist_cmp(svalue_t * /*p1*/, svalue_t * /*p2*/);
/*
 * Make an empty array for everyone to use, never to be deallocated.
 * It is cheaper to reuse it, than to use MALLOC() and allocate.
 */

array_t the_null_array = {
    1, /* ref, always 1. */
#ifdef DEBUGMALLOC_EXTENSIONS
    1, /* extra_ref */
#endif
    0, /* size */
#ifdef PACKAGE_MUDLIB_STATS
    {nullptr}, /* statgroup_t stats */
#endif
    {{0, 0, {0}}}, /* svalue_t item[1] */
};

#ifdef PACKAGE_MUDLIB_STATS
static void ms_setup_stats(array_t *p) {
  if (current_object) {
    assign_stats(&p->stats, current_object);
    add_array_size(&p->stats, p->size);
  } else {
    null_stats(&p->stats);
  }
}

#define ms_remove_stats(p) add_array_size(&(p)->stats, -((p)->size))
#define ms_add_array_size(p, n) add_array_size(p, n)
#else
#define ms_setup_stats(x)
#define ms_remove_stats(x)
#define ms_add_array_size(p, n)
#endif

/* Array allocation routines:
 *
 * the prefix int_ indicates error checking is not performed.  It is up to
 * the caller to guarantee 0 < n <= max_array_size [note that the first
 * inequality is strict].
 *
 * _empty_ indicates that the internal svalues are not initialized to zero;
 * this saves some time if the calling routine is simply going to overwrite
 * the entries.  In the case of free_ and dealloc_, it means the entries
 * have already been moved somewhere else or freed, and should be ignored.
 *
 * Note that we rely a bit on gcc automatically inlining small routines.
 */
static array_t *int_allocate_empty_array(unsigned int n) {
  array_t *p;

  num_arrays++;
  total_array_size += sizeof(array_t) + sizeof(svalue_t) * (n - 1);

  p = ALLOC_ARRAY(n);
  p->ref = 1;
  p->size = n;
  ms_setup_stats(p);

  return p;
}

array_t *allocate_empty_array(int n) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  if (n < 0 || n > max_array_size) {
    error("Illegal array size.\n");
  }
  if (!n) {
    return &the_null_array;
  }

  return int_allocate_empty_array(n);
}

static array_t *int_allocate_array(unsigned int n) {
  array_t *p = int_allocate_empty_array(n);

  while (n--) {
    p->item[n] = const0;
  }

  return p;
}

array_t *allocate_array(int n) {
  array_t *p = allocate_empty_array(n);

  while (n--) {
    p->item[n] = const0;
  }

  return p;
}

array_t *allocate_array2(int n, svalue_t *svp) {
  int i;
  array_t *ret;

  if (svp->type == T_FUNCTION) {
    ret = allocate_array(n);

    for (i = 0; i < n; i++) {
      svalue_t *r;

      push_number(i);
      r = call_function_pointer(svp->u.fp, 1);
      ret->item[i] = *r;
      r->type = T_NUMBER;
    }
  } else {
    ret = allocate_empty_array(n);

    for (i = 0; i < n; i++) {
      assign_svalue_no_free(ret->item + i, svp);
    }
  }

  return ret;
}

static void dealloc_empty_array(array_t *p) {
  ms_remove_stats(p);

  num_arrays--;
  total_array_size -= sizeof(array_t) + sizeof(svalue_t) * (p->size - 1);

  FREE((char *)p);
}

void dealloc_array(array_t *p) {
  int i;

  for (i = p->size; i--;) {
    free_svalue(&p->item[i], "free_array");
  }
  dealloc_empty_array(p);
}

void free_array(array_t *p) {
  if (--(p->ref) > 0 || (p == &the_null_array)) {
    return;
  }

  dealloc_array(p);
}

void free_empty_array(array_t *p) {
  if ((--(p->ref) > 0) || (p == &the_null_array)) {
    return;
  }

  dealloc_empty_array(p);
}

/* Finish setting up an array allocated with ALLOC_ARRAY, resizing it to
   size n */
static array_t *fix_array(array_t *p, unsigned int n) {
  if (n) {
    num_arrays++;
    total_array_size += sizeof(array_t) + sizeof(svalue_t) * (n - 1);

    p->size = n;
    p->ref = 1;
    ms_setup_stats(p);
    //      if(n>65535)
    //          fatal("big array2");
    return RESIZE_ARRAY(p, n);
  }
  if (p != &the_null_array) {
    FREE(p);
  }
  return &the_null_array;
}

array_t *resize_array(array_t *p, unsigned int n) {
  // it is possible that n < p->size, be careful to not upgrade the result to unsigned.
  total_array_size += ((int)n - p->size) * int(sizeof(svalue_t));
  if (n) {
    ms_remove_stats(p);
    p = RESIZE_ARRAY(p, n);
    //    if(n>65535)
    //              fatal("big array3");

    if (!p) {
      fatal("Out of memory.\n");
    }
    p->size = n;
    ms_setup_stats(p);
  } else {
    if (p != &the_null_array) {
      FREE(p);
    }
    return &the_null_array;
  }
  return p;
}

array_t *explode_string(const char *str, int slen, const char *del, int len) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  const char *p, *beg, *lastdel = nullptr;
  int num, j, limit;
  array_t *ret;
  char *buff, *tmp;
  int sz;

  if (!slen) {
    return &the_null_array;
  }

  /* return an array of length strlen(str) -w- one character per element */
  if (len == 0) {
    sz = 1;

    if (slen > max_array_size) {
      slen = max_array_size;
    }
    ret = int_allocate_empty_array(slen);
    for (j = 0; j < slen; j++) {
      ret->item[j].type = T_STRING;
      ret->item[j].subtype = STRING_MALLOC;
      ret->item[j].u.string = tmp = new_string(1, "explode_string: tmp");
      tmp[0] = str[j];
      tmp[1] = '\0';
    }
    return ret;
  }
  if (len == 1) {
    char delimeter;

    delimeter = *del;

    if (!CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
      /*
       * Skip leading 'del' strings, if any.
       */
      while (*str == delimeter) {
        str++;
        slen--;
        if (str[0] == '\0') {
          return &the_null_array;
        }
        if (CONFIG_INT(__RC_SANE_EXPLODE_STRING__) != 0) {
          break;
        }
      }
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
    limit = max_array_size;
    if (CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
      num++;
      limit--;
    } else {
      if (lastdel != (str + slen - 1)) {
        num++;
        limit--;
      }
    }
    if (num > max_array_size) {
      num = max_array_size;
    }
    ret = int_allocate_empty_array(num);
    for (p = str, beg = str, num = 0; *p && (num < limit);) {
      if (*p == delimeter) {
        DEBUG_CHECK(num >= ret->size, "Index out of bounds in explode!\n");
        sz = p - beg;
        ret->item[num].type = T_STRING;
        ret->item[num].subtype = STRING_MALLOC;
        ret->item[num].u.string = buff = new_string(sz, "explode_string: buff");

        strncpy(buff, beg, sz);
        buff[sz] = '\0';
        num++;
        beg = ++p;
      } else {
        p++;
      }
    }

    if (CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
      ret->item[num].type = T_STRING;
      ret->item[num].subtype = STRING_MALLOC;
      ret->item[num].u.string = string_copy(beg, "explode_string: last, len == 1");
    } else {
      /* Copy last occurence, if there was not a 'del' at the end. */
      if (*beg != '\0' && num != limit) {
        ret->item[num].type = T_STRING;
        ret->item[num].subtype = STRING_MALLOC;
        ret->item[num].u.string = string_copy(beg, "explode_string: last, len == 1");
      }
    }
    return ret;
  } /* len == 1 */

  if (!CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
    /*
     * Skip leading 'del' strings, if any.
     */
    while (strncmp(str, del, len) == 0) {
      str += len;
      slen -= len;
      if (str[0] == '\0') {
        return &the_null_array;
      }
      if (CONFIG_INT(__RC_SANE_EXPLODE_STRING__)) {
        break;
      }
    }
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
  if (CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
    num++;
  } else {
    if (lastdel != (str + slen - len)) {
      num++;
    }
  }
  if (num > max_array_size) {
    num = max_array_size;
  }
  ret = int_allocate_empty_array(num);
  limit = max_array_size - 1; /* extra element can be added after loop */
  for (p = str, beg = str, num = 0; *p && (num < limit);) {
    if (strncmp(p, del, len) == 0) {
      if (num >= ret->size) {
        fatal("Index out of bounds in explode!\n");
      }

      ret->item[num].type = T_STRING;
      ret->item[num].subtype = STRING_MALLOC;
      ret->item[num].u.string = buff = new_string(p - beg, "explode_string: buff");

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
  if (CONFIG_INT(__RC_REVERSIBLE_EXPLODE_STRING__)) {
    ret->item[num].type = T_STRING;
    ret->item[num].subtype = STRING_MALLOC;
    ret->item[num].u.string = string_copy(beg, "explode_string: last, len != 1");
  } else {
    if (*beg != '\0' && num != limit) {
      ret->item[num].type = T_STRING;
      ret->item[num].subtype = STRING_MALLOC;
      ret->item[num].u.string = string_copy(beg, "explode_string: last, len != 1");
    }
  }
  return ret;
}

char *implode_string(array_t *arr, const char *del, int del_len) {
  int size, i, num;
  char *p, *q;

  for (i = arr->size, size = 0, num = 0; i--;) {
    if (arr->item[i].type == T_STRING) {
      size += SVALUE_STRLEN(&arr->item[i]);
      num++;
    }
  }
  if (num == 0) {
    return string_copy("", "implode_string");
  }

  p = new_string(size + (num - 1) * del_len, "implode_string: p");
  q = p;
  for (i = 0, num = 0; i < arr->size; i++) {
    if (arr->item[i].type == T_STRING) {
      if (num) {
        strncpy(p, del, del_len);
        p += del_len;
      }
      size = SVALUE_STRLEN(&arr->item[i]);
      strncpy(p, arr->item[i].u.string, size);
      p += size;
      num++;
    }
  }
  *p = 0;
  return q;
}

void implode_array(funptr_t *fptr, array_t *arr, svalue_t *dest, int first_on_stack) {
  int i = 0, n;
  svalue_t *v;

  if (first_on_stack) {
    if (!(n = arr->size)) {
      *dest = *sp--;
      return;
    }
  } else {
    if (!(n = arr->size)) {
      *dest = const0;
      return;
    } else if (n == 1) {
      assign_svalue_no_free(dest, &arr->item[0]);
      free_array(arr);
      return;
    }
  }

  if (!first_on_stack) {
    push_svalue(&arr->item[i++]);
  }

  while (true) {
    push_svalue(&arr->item[i++]);
    v = call_function_pointer(fptr, 2);
    if (!v) {
      *dest = const0;
      return;
    }
    if (i < n) {
      push_svalue(v);
    } else {
      break;
    }
  }
  assign_svalue_no_free(dest, v);
  free_array(arr);
}

/*
 * Slice of an array.
 * It now frees the passed array
 */
array_t *slice_array(array_t *p, int from, int to) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  int cnt;
  svalue_t *sv1, *sv2;

  if (from < 0) {
    from = 0;
  }
  if (to >= p->size) {
    to = p->size - 1;
  }
  if (from > to) {
    free_array(p);
    return &the_null_array;
  }

  if (!(--p->ref)) {
    if (from) {
      sv1 = p->item + from;
      cnt = from;
      while (cnt--) {
        free_svalue(--sv1, "slice_array:2");
      }
      cnt = to - from + 1;
      sv1 = p->item;
      sv2 = p->item + from;
      while (cnt--) {
        *sv1++ = *sv2++;
      }
    } else {
      sv2 = p->item + to + 1;
    }
    cnt = (p->size - 1) - to;
    while (cnt--) {
      free_svalue(sv2++, "slice_array:3");
    }
    if (to - from + 1 > max_array_size) {
      error("array slice too big");  // can't happen in theory
    }
    p = resize_array(p, to - from + 1);
    p->ref = 1;
    return p;
  } else {
    array_t *d;
    if (to - from + 1 > max_array_size) {
      error("array slice too big");  // can't happen in theory
    }
    d = int_allocate_empty_array(to - from + 1);
    sv1 = d->item - from;
    sv2 = p->item;
    for (cnt = from; cnt <= to; cnt++) {
      assign_svalue_no_free(sv1 + cnt, sv2 + cnt);
    }
    return d;
  }
}

/*
 * Copy of an array
 */
array_t *copy_array(array_t *p) {
  array_t *d;
  int n;
  svalue_t *sv1 = p->item, *sv2;

  d = allocate_empty_array(n = p->size);
  sv2 = d->item;
  while (n--) {
    assign_svalue_no_free(&sv2[n], &sv1[n]);
  }
  return d;
}

#ifdef F_COMMANDS
array_t *commands(object_t *ob) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  sentence_t *s;
  array_t *v, *p;
  int cnt = 0;
  svalue_t *sv;

  for (s = ob->sent; s && s->verb; s = s->next) {
    if (++cnt == max_array_size) {
      break;
    }
  }
  v = allocate_empty_array(cnt);
  sv = v->item;
  for (s = ob->sent; cnt-- && s && s->verb; s = s->next) {
    sv->type = T_ARRAY;
    (sv++)->u.arr = p = int_allocate_empty_array(4);
    p->item[0].type = T_STRING;
    p->item[0].u.string = ref_string(s->verb); /* the verb is shared */
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
void filter_array(svalue_t *arg, int num_arg) {
  array_t *vec = arg->u.arr, *r;
  int size;

  if ((size = vec->size) < 1) {
    pop_n_elems(num_arg - 1);
    return;
  } else {
    svalue_t *v;
    int res = 0, cnt;
    function_to_call_t ftc;

    process_efun_callback(1, &ftc, F_FILTER);

    /* allocate a full size array and push it onto the stack so that if an
     * error occurs, it'll get cleaned up.  can't use empty array because
     * if an error occurs, it'll contain garbage and crash the driver
     */
    r = allocate_array(size);
    push_refed_array(r);

    for (cnt = 0; cnt < size; cnt++) {
      push_svalue(&vec->item[cnt]);
      v = call_efun_callback(&ftc, 1);
      if (!IS_ZERO(v)) {
        assign_svalue_no_free(&r->item[res++], &vec->item[cnt]);
      }
    }

    sp--; /* pull the work array off the stack without freeing it */
    if (res) {
      r = resize_array(r, res);
    } else {
      free_array(r);
      r = &the_null_array;
    }

    pop_n_elems(num_arg - 1);
    free_array(vec);
    sp->u.arr = r;
  }
}

void filter_string(svalue_t *arg, int num_arg) {
  if (arg->u.string[0] == 0) {
    pop_n_elems(num_arg - 1);
    return;
  } else {
    int size;
    svalue_t *v;
    int idx = 0, cnt;
    function_to_call_t ftc;
    char *str;

    unlink_string_svalue(arg);
    size = SVALUE_STRLEN(arg);
    str = const_cast<char *>(arg->u.string);

    process_efun_callback(1, &ftc, F_FILTER);

    for (cnt = 0; cnt < size; cnt++) {
      push_number(str[cnt]);
      v = call_efun_callback(&ftc, 1);
      if (!IS_ZERO(v)) {
        str[idx++] = str[cnt];
      }
    }
    str[idx] = 0;

    if (idx != cnt) {
      arg->u.string = extend_string(arg->u.string, idx);
    }

    pop_n_elems(num_arg - 1);
  }
}
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
int sameval(svalue_t *arg1, svalue_t *arg2) {
  DEBUG_CHECK(!arg1 || !arg2, "Null pointer passed to sameval.\n");

  switch (arg1->type | arg2->type) {
    case T_NUMBER:
      return arg1->u.number == arg2->u.number;
    case T_ARRAY:
    case T_CLASS:
      return arg1->u.arr == arg2->u.arr;
    case T_STRING:
      if (SVALUE_STRLEN_DIFFERS(arg1, arg2)) {
        return 0;
      }
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

typedef struct unique_s {
  svalue_t mark;
  int count;
  struct unique_s *next;
  int *indices;
} unique_t;

typedef struct unique_list_s {
  unique_t *head;
  struct unique_list_s *next;
} unique_list_t;

static unique_list_t *g_u_list = nullptr;

static void unique_array_error_handler(void) {
  unique_list_t *unlist = g_u_list;
  unique_t *uptr = unlist->head, *nptr;

  g_u_list = g_u_list->next;
  while (uptr) {
    nptr = uptr->next;
    FREE((char *)uptr->indices);
    free_svalue(&uptr->mark, "unique_array_error_handler");
    FREE((char *)uptr);
    uptr = nptr;
  }
  FREE((char *)unlist);
}

void f_unique_array(void) {
  array_t *v, *ret;
  long size, i, numkeys = 0, num_arg = st_num_arg;
  int *ind;
  svalue_t *skipval, *sv, *svp;
  unique_list_t *unlist;
  unique_t **head, *uptr, *nptr;
  funptr_t *fptr = nullptr;
  const char *func;

  size = (v = (sp - num_arg + 1)->u.arr)->size;
  if (!size) {
    if (num_arg == 3) {
      free_svalue(sp--, "f_unique_array");
    }
    free_svalue(sp--, "f_unique_array");
    return;
  }

  if (num_arg == 3) {
    skipval = sp;
    if ((sp - 1)->type == T_FUNCTION) {
      fptr = (sp - 1)->u.fp;
    } else {
      func = (sp - 1)->u.string;
    }
  } else {
    skipval = &const0;
    if (sp->type == T_FUNCTION) {
      fptr = sp->u.fp;
    } else {
      func = sp->u.string;
    }
  }

  unlist = reinterpret_cast<unique_list_t *>(
      DMALLOC(sizeof(unique_list_t), TAG_TEMPORARY, "f_unique_array:1"));
  unlist->next = g_u_list;
  unlist->head = nullptr;
  head = &unlist->head;
  g_u_list = unlist;

  STACK_INC;
  sp->type = T_ERROR_HANDLER;
  sp->u.error_handler = unique_array_error_handler;

  for (i = 0; i < size; i++) {
    if (fptr) {
      push_svalue(v->item + i);
      sv = call_function_pointer(fptr, 1);
    } else if (v->item[i].type == T_OBJECT) {
      sv = apply(func, v->item[i].u.ob, 0, ORIGIN_EFUN);
    } else {
      sv = nullptr;
    }

    if (sv && !sameval(sv, skipval)) {
      uptr = *head;
      while (uptr) {
        if (sameval(sv, &uptr->mark)) {
          uptr->indices =
              RESIZE(uptr->indices, uptr->count + 1, int, TAG_TEMPORARY, "f_unique_array:2");
          uptr->indices[uptr->count++] = i;
          break;
        }
        uptr = uptr->next;
      }
      if (!uptr) {
        numkeys++;
        uptr = reinterpret_cast<unique_t *>(
            DMALLOC(sizeof(unique_t), TAG_TEMPORARY, "f_unique_array:3"));
        uptr->indices =
            reinterpret_cast<int *>(DMALLOC(sizeof(int), TAG_TEMPORARY, "f_unique_array:4"));
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
    (sv = ret->item + numkeys)->type = T_ARRAY;
    sv->u.arr = allocate_empty_array(i = uptr->count);
    skipval = sv->u.arr->item + i;
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
  free_array(v);
  sp->u.arr = ret;
}

/*
 * End of Unique maker
 *************************
 */
#endif

/* Concatenation of two arrays into one
 */
array_t *add_array(array_t *p, array_t *r) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  int cnt, res;
  array_t *d; /* destination */

  /*
   * have to be careful with size zero arrays because they could be
   * the_null_array.  REALLOC(the_null_array, ...) is bad :(
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
  if (res < 0 || res > max_array_size) {
    error("result of array addition is greater than maximum array size.\n");
  }

  /* x += x */
  if ((p == r) && (p->ref == 2)) {
    int osize = p->size;

    p->ref = 1;
    d = resize_array(p, res);

    /* copy myself */
    for (cnt = osize; cnt--;) {
      assign_svalue_no_free(&d->item[--res], &d->item[cnt]);
    }

    return d;
  }

  /* transfer svalues for ref 1 target array */
  if (p->ref == 1) {
    d = resize_array(p, res);
    /* note that d->ref is already 1, which is right */
  } else {
    d = int_allocate_empty_array(res);

    for (cnt = p->size; cnt--;) {
      assign_svalue_no_free(&d->item[cnt], &p->item[cnt]);
    }
    p->ref--;
  }

  /* transfer svalues from ref 1 source array */
  if (r->ref == 1) {
    for (cnt = r->size; cnt--;) {
      d->item[--res] = r->item[cnt];
    }
    dealloc_empty_array(r);
  } else {
    for (cnt = r->size; cnt--;) {
      assign_svalue_no_free(&d->item[--res], &r->item[cnt]);
    }
    r->ref--;
  }

  return d;
}

#ifndef NO_ENVIRONMENT
/* Returns an array of all objects contained in 'ob' */
array_t *all_inventory(object_t *ob, int override) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  array_t *d;
  object_t *cur;
  int cnt, res;
  int display_hidden;

  if (override) {
    display_hidden = 1;
  } else {
    display_hidden = -1;
  }
  cnt = 0;
  for (cur = ob->contains; cur; cur = cur->next_inv) {
#ifdef F_SET_HIDE
    if (cur->flags & O_HIDDEN) {
      if (display_hidden == -1) {
        display_hidden = valid_hide(current_object);
      }
      if (display_hidden) {
        cnt++;
      }
    } else {
#endif
      cnt++;
    }
  }

  if (!cnt) {
    return &the_null_array;
  }

  if (cnt > max_array_size) {
    cnt = max_array_size;
  }

  d = int_allocate_empty_array(cnt);
  cur = ob->contains;

  for (res = 0; res < cnt; res++) {
#ifdef F_SET_HIDE
    if ((cur->flags & O_HIDDEN) && !display_hidden) {
      cur = cur->next_inv;
      res--;
      continue;
    }
#endif
    d->item[res].type = T_OBJECT;
    d->item[res].u.ob = cur;
    add_ref(cur, "all_inventory");
    cur = cur->next_inv;
  }
  return d;
}
#endif

/* Runs all elements of an array through ob::func
   and replaces each value in arr by the value returned by ob::func
   */
#ifdef F_MAP
void map_array(svalue_t *arg, int num_arg) {
  array_t *arr = arg->u.arr;
  array_t *r;
  int size;

  if ((size = arr->size) == 0) {
    r = &the_null_array;
  } else {
    function_to_call_t ftc;
    int cnt;
    svalue_t *v;

    process_efun_callback(1, &ftc, F_MAP);

    r = int_allocate_array(size);

    push_refed_array(r);

    for (cnt = 0; cnt < size; cnt++) {
      push_svalue(arr->item + cnt);
      v = call_efun_callback(&ftc, 1);
      if (v) {
        assign_svalue_no_free(&r->item[cnt], v);
      } else {
        break;
      }
    }
    sp--;
  }

  pop_n_elems(num_arg);
  push_refed_array(r);
}

void map_string(svalue_t *arg, int num_arg) {
  char *arr;
  char *p;
  funptr_t *fptr = nullptr;
  int numex = 0;
  object_t *ob = nullptr;
  svalue_t *extra, *v;
  const char *func;

  /* get a modifiable string */
  /* do not use arg after this; it has been copied or freed.
     Put our result string on the stack where it belongs in case of an
     error (note it is also in the right spot for the return value).
   */
  unlink_string_svalue(arg);
  arr = const_cast<char *>(arg->u.string);

  if (arg[1].type == T_FUNCTION) {
    fptr = arg[1].u.fp;
    if (num_arg > 2) {
      extra = arg + 2;
      numex = num_arg - 2;
    }
  } else {
    func = arg[1].u.string;
    if (num_arg < 3) {
      ob = current_object;
    } else {
      if (arg[2].type == T_OBJECT) {
        ob = arg[2].u.ob;
      } else if (arg[2].type == T_STRING) {
        if ((ob = find_object(arg[2].u.string)) && !object_visible(ob)) {
          ob = nullptr;
        }
      }
      if (num_arg > 3) {
        extra = arg + 3;
        numex = num_arg - 3;
      }
      if (!ob) {
        error("Bad argument 3 to map_string.\n");
      }
    }
  }

  for (p = arr; *p; p++) {
    push_number(static_cast<unsigned char>(*p));
    if (numex) {
      push_some_svalues(extra, numex);
    }
    v = fptr ? call_function_pointer(fptr, numex + 1) : apply(func, ob, 1 + numex, ORIGIN_EFUN);
    /* no function or illegal return value is unaltered.
     * Anyone got a better idea?  A few idea:
     * (1) insert strings? - algorithm needs changing
     * (2) ignore them? - again, could require a realloc, since size would
     *                    change
     * (3) become ' ' or something
     */
    if (!v) {
      break;
    }
    if (v->type == T_NUMBER && v->u.number != 0) {
      *p = (static_cast<char>(v->u.number));
    }
  }

  pop_n_elems(num_arg - 1);
  /* return value on stack */
}
#endif

#ifdef F_SORT_ARRAY
static function_to_call_t *sort_array_ftc;

array_t *builtin_sort_array(array_t *inlist, int dir) {
  qsort(reinterpret_cast<char *>(inlist->item), inlist->size, sizeof(inlist->item),
        (dir < 0) ? builtin_sort_array_cmp_rev : builtin_sort_array_cmp_fwd);

  return inlist;
}

static int builtin_sort_array_cmp_fwd(const void *vp1, const void *vp2) {
  auto *p1 = (svalue_t *)vp1;
  auto *p2 = (svalue_t *)vp2;
  switch (p1->type | p2->type) {
    case T_STRING: {
      return strcmp(p1->u.string, p2->u.string);
    }

    case T_NUMBER: {
      return COMPARE_NUMS(p1->u.number, p2->u.number);
    }

    case T_REAL: {
      return COMPARE_NUMS(p1->u.real, p2->u.real);
    }

    case T_ARRAY: {
      array_t *v1 = p1->u.arr, *v2 = p2->u.arr;
      if (!v1->size || !v2->size) {
        error("Illegal to have empty array in array for sort_array()\n");
      }

      switch (v1->item[0].type | v2->item[0].type) {
        case T_STRING: {
          return strcmp(v1->item[0].u.string, v2->item[0].u.string);
        }

        case T_NUMBER: {
          return COMPARE_NUMS(v1->item[0].u.number, v2->item[0].u.number);
        }

        case T_REAL: {
          return COMPARE_NUMS(v1->item[0].u.real, v2->item[0].u.real);
        }
        default: {
          /* Temp. long err msg till I can think of a better one - Sym */
          error(
              "sort_array() cannot handle arrays of arrays whose 1st "
              "elems\naren't strings/ints/floats\n");
        }
      }
    }
  }
  error(
      "built-in sort_array() can only handle homogeneous arrays of "
      "strings/ints/floats/arrays\n");
  return 0;
}

static int builtin_sort_array_cmp_rev(const void *vp1, const void *vp2) {
  return builtin_sort_array_cmp_fwd(vp2, vp1);
}

static int sort_array_cmp(const void *vp1, const void *vp2) {
  auto *p1 = (svalue_t *)vp1;
  auto *p2 = (svalue_t *)vp2;
  svalue_t *d;

  push_svalue(p1);
  push_svalue(p2);

  d = call_efun_callback(sort_array_ftc, 2);
  if (!d || d->type != T_NUMBER) {
    return 0;
  } else {
    LPC_INT n = d->u.number;
    // sanitize the result.
    if (n) {
      n = n > 0 ? 1 : -1;
    }
    return n;
  }
}

void f_sort_array(void) {
  svalue_t *arg = sp - st_num_arg + 1;
  array_t *tmp = arg->u.arr;
  int num_arg = st_num_arg;

  check_for_destr(tmp);

  switch (arg[1].type) {
    case T_NUMBER: {
      tmp = builtin_sort_array(copy_array(tmp), arg[1].u.number);
      break;
    }

    case T_FUNCTION:
    case T_STRING: {
      /*
       * We use a global to communicate with the comparison function,
       * so we have to be careful to make sure we can recurse (the
       * callback might call sort_array itself).  For this reason, the
       * ftc structure is on the stack, and we just keep a pointer
       * to it in a global, being careful to save and restore the old
       * value.
       */
      function_to_call_t ftc, *old_ptr;

      old_ptr = sort_array_ftc;
      sort_array_ftc = &ftc;
      process_efun_callback(1, &ftc, F_SORT_ARRAY);

      tmp = copy_array(tmp);
      push_refed_array(tmp);
      if (CONFIG_INT(__RC_SANE_SORTING__)) {
        qsort(reinterpret_cast<char *>(tmp->item), tmp->size, sizeof(tmp->item), sort_array_cmp);
      } else {
        old_quickSort((char *)tmp->item, tmp->size, sizeof(tmp->item), sort_array_cmp);
      }
      sort_array_ftc = old_ptr;
      sp--;  // remove tmp from stack, but we don't want to free it!
      break;
    }
  }
  pop_n_elems(num_arg);
  push_refed_array(tmp);  // put it back
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
 *
 * 20100824 Tiz added function parameter to allow filtering of result.
 */
#ifndef NO_ENVIRONMENT
static int valid_hide_flag;

static int deep_inventory_count(object_t *ob) {
  object_t *cur;
  int cnt;

  cnt = 0;

  /* step through object's inventory and count visible objects */
  for (cur = ob->contains; cur; cur = cur->next_inv) {
#ifdef F_SET_HIDE
    if (cur->flags & O_HIDDEN) {
      if (!valid_hide_flag) valid_hide_flag = 1 + (valid_hide(current_object) ? 1 : 0);
      if (valid_hide_flag & 2) {
        cnt++;
        cnt += deep_inventory_count(cur);
      }
    } else {
#endif
      cnt++;
      cnt += deep_inventory_count(cur);
#ifdef F_SET_HIDE
    }
#endif
  }

  return cnt;
}

static void deep_inventory_collect(object_t *ob, array_t *inv, int *i, int max, funptr_t *fp) {
  object_t *cur, *next;
  svalue_t *fp_result;

  /* step through object's inventory and look for visible objects */
  for (cur = ob->contains; cur && *i < max; cur = next) {
    next = cur->next_inv;
    if (fp) {
      push_object(cur);
      fp_result = call_function_pointer(fp, 1);
      if (!fp_result || (current_object->flags & O_DESTRUCTED)) {
        *i = 0;
        return;
      }
    }
    if (!fp || (fp && fp_result->type == T_NUMBER && fp_result->u.number > 0 &&
                !(cur->flags & O_DESTRUCTED))) {
#ifdef F_SET_HIDE
      if (cur->flags & O_HIDDEN) {
        if (valid_hide_flag & 2) {
          if (!fp || (fp && fp_result->type == T_NUMBER && fp_result->u.number != 3)) {
            inv->item[*i].type = T_OBJECT;
            inv->item[*i].u.ob = cur;
            (*i)++;
            add_ref(cur, "deep_inventory_collect");
          }
          if (!fp || (fp && fp_result->type == T_NUMBER &&
                      (fp_result->u.number == 1 || fp_result->u.number == 3))) {
            deep_inventory_collect(cur, inv, i, max, fp);
          }
        }
      } else {
#endif
        if (!fp || (fp && fp_result->type == T_NUMBER && fp_result->u.number != 3)) {
          inv->item[*i].type = T_OBJECT;
          inv->item[*i].u.ob = cur;
          (*i)++;
          add_ref(cur, "deep_inventory_collect");
        }
        if (!fp || (fp && fp_result->type == T_NUMBER &&
                    (fp_result->u.number == 1 || fp_result->u.number == 3))) {
          deep_inventory_collect(cur, inv, i, max, fp);
        }
#ifdef F_SET_HIDE
      }
#endif
    }
  }
}

array_t *deep_inventory(object_t *ob, int take_top, funptr_t *fp) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  array_t *dinv;
  int i, o;
  svalue_t *fp_result;

  valid_hide_flag = 0;

  /*
   * count visible objects in an object's inventory, and in their
   * inventory, etc
   */
  i = deep_inventory_count(ob);
  if (take_top) {
    i++;
  }

  if (i == 0) {
    return &the_null_array;
  }

  if (i > max_array_size) {
    i = max_array_size;
  }

  /*
   * allocate an array
   */
  dinv = int_allocate_array(o = i);
  push_refed_array(dinv);
  /*
   * collect visible inventory objects recursively
   */
  if (take_top) {
    if (fp) {
      push_object(ob);
      fp_result = call_function_pointer(fp, 1);
      if (!fp_result || (current_object->flags & O_DESTRUCTED)) {
        pop_stack();  // dinv
        return &the_null_array;
      }
    }
    if (!fp || (fp && fp_result->type == T_NUMBER && fp_result->u.number > 0 &&
                !(ob->flags & O_DESTRUCTED))) {
      if (!fp || (fp && fp_result->type == T_NUMBER && fp_result->u.number != 3)) {
        dinv->item[0].type = T_OBJECT;
        dinv->item[0].u.ob = ob;
        add_ref(ob, "deep_inventory");
        i = 1;
      }
      if (!fp || (fp && fp_result->type == T_NUMBER &&
                  (fp_result->u.number == 1 || fp_result->u.number == 3))) {
        deep_inventory_collect(ob, dinv, &i, o, fp);
      }
    }
  } else {
    i = 0;
    deep_inventory_collect(ob, dinv, &i, o, fp);
  }

  // resize the array if we have filtered out values
  if (fp && i < o) {
    dinv = resize_array(dinv, i);
  }
  sp--;  // get dinv off the stack, but don't free it;
  return dinv;
}

array_t *deep_inventory_array(array_t *arr, int take_top, funptr_t *fp) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  array_t *dinv;
  int i, o, c;
  svalue_t *fp_result;

  valid_hide_flag = 0;

  /*
   * count visible objects in an object's inventory, and in their
   * inventory, etc
   */
  i = 0;
  for (c = 0; c < arr->size; c++) {
    if (arr->item[c].type == T_OBJECT) {
      i += deep_inventory_count(arr->item[c].u.ob);
      if (take_top) {
        i++;
      }
    }
  }

  if (i == 0) {
    return &the_null_array;
  }

  if (i > max_array_size) {
    i = max_array_size;
  }

  /*
   * allocate an array
   */
  dinv = int_allocate_array(o = i);
  push_refed_array(dinv);
  /*
   * collect visible inventory objects recursively
   */
  i = 0;
  for (c = 0; i < o && c < arr->size; c++) {
    if (arr->item[c].type == T_OBJECT) {
      if (take_top) {
        if (fp) {
          push_object(arr->item[c].u.ob);
          fp_result = call_function_pointer(fp, 1);
          if (!fp_result || (current_object->flags & O_DESTRUCTED)) {
            pop_stack();  // free dinv
            return &the_null_array;
          }
        }
        if (!fp || (fp && fp_result->type == T_NUMBER && fp_result->u.number > 0 &&
                    !(arr->item[c].u.ob->flags & O_DESTRUCTED))) {
          if (!fp || (fp && fp_result->type == T_NUMBER && fp_result->u.number != 3)) {
            dinv->item[i].type = T_OBJECT;
            dinv->item[i].u.ob = arr->item[c].u.ob;
            add_ref(arr->item[c].u.ob, "deep_inventory");
            i++;
          }
          if (!fp || (fp && fp_result->type == T_NUMBER &&
                      (fp_result->u.number == 1 || fp_result->u.number == 3))) {
            deep_inventory_collect(arr->item[c].u.ob, dinv, &i, o, fp);
          }
        }
      } else {
        deep_inventory_collect(arr->item[c].u.ob, dinv, &i, o, fp);
      }
    }
  }

  // resize the array if we have filtered out values
  if (fp && i < o) {
    dinv = resize_array(dinv, i);
  }
  sp--;  // get dinv of the stack but don't free it
  return dinv;
}
#endif

static long alist_cmp(svalue_t *p1, svalue_t *p2) {
  long d;

  if ((d = p1->u.number - p2->u.number)) {
    if (d == LONG_MIN) {
      d = p1->u.number > p2->u.number;
    }
    return d;
  }
  if ((d = p1->type - p2->type)) {
    return d;
  }
  return 0;
}

static svalue_t *alist_sort(array_t *inlist) {
  long size, j, curix, parix, child1, child2, flag;
  svalue_t *sv_tab, *tmp, *table, *sv_ptr, val;
  char *str;

  if (!(size = inlist->size)) {
    return (svalue_t *)nullptr;
  }
  if ((flag = (inlist->ref > 1))) {
    sv_tab = reinterpret_cast<svalue_t *>(
        DCALLOC(size, sizeof(svalue_t), TAG_TEMPORARY, "alist_sort: sv_tab"));
    sv_ptr = inlist->item;
    for (j = 0; j < size; j++) {
      if (((tmp = (sv_ptr + j))->type == T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&tmp->u.ob, "alist_sort");
        sv_tab[j] = *tmp = const0u;
      } else if ((tmp->type == T_STRING) && !(tmp->subtype == STRING_SHARED)) {
        sv_tab[j].u.string = make_shared_string(tmp->u.string);
        (tmp = sv_tab + j)->subtype = STRING_SHARED;
        tmp->type = T_STRING;
      } else {
        assign_svalue_no_free(sv_tab + j, tmp);
      }

      if ((curix = j)) {
        val = *tmp;

        do {
          parix = (curix - 1) >> 1;
          if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
            sv_tab[curix] = sv_tab[parix];
            sv_tab[parix] = val;
          }
        } while ((curix = parix));
      }
    }
  } else {
    sv_tab = inlist->item;
    for (j = 0; j < size; j++) {
      if (((tmp = (sv_tab + j))->type == T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&tmp->u.ob, "alist_sort");
        *tmp = const0u;
      } else if ((tmp->type == T_STRING) && !(tmp->subtype == STRING_SHARED)) {
        str = make_shared_string(tmp->u.string);
        free_string_svalue(tmp);
        tmp->u.string = str;
        tmp->subtype = STRING_SHARED;
      }

      if ((curix = j)) {
        val = *tmp;

        do {
          parix = (curix - 1) >> 1;
          if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
            sv_tab[curix] = sv_tab[parix];
            sv_tab[parix] = val;
          }
        } while ((curix = parix));
      }
    }
  }

  table = reinterpret_cast<svalue_t *>(
      DCALLOC(size, sizeof(svalue_t), TAG_TEMPORARY, "alist_sort: table"));

  for (j = 0; j < size; j++) {
    table[j] = sv_tab[0];

    for (curix = 0;;) {
      child1 = (curix << 1) + 1;
      child2 = child1 + 1;

      if (child2 < size && sv_tab[child2].type != T_INVALID &&
          (sv_tab[child1].type == T_INVALID || alist_cmp(sv_tab + child1, sv_tab + child2) > 0)) {
        child1 = child2;
      }
      if (child1 < size && sv_tab[child1].type != T_INVALID) {
        sv_tab[curix] = sv_tab[child1];
        curix = child1;
      } else {
        break;
      }
    }

    sv_tab[curix].type = T_INVALID;
  }

  if (flag) {
    FREE((char *)sv_tab);
  }
  return table;
}

array_t *subtract_array(array_t *minuend, array_t *subtrahend) {
  array_t *difference;
  svalue_t *source, *dest, *svt;
  long i, size, o, d, l, h, msize;

  if (!(size = subtrahend->size)) {
    subtrahend->ref--;
    return minuend->ref > 1 ? (minuend->ref--, copy_array(minuend)) : minuend;
  }
  if (!(msize = minuend->size)) {
    free_array(subtrahend);
    return &the_null_array;
  }
  svt = alist_sort(subtrahend);
  difference = ALLOC_ARRAY(msize);
  for (source = minuend->item, dest = difference->item, i = msize; i--; source++) {
    l = 0;
    o = (h = size - 1) >> 1;

    if ((source->type == T_OBJECT) && (source->u.ob->flags & O_DESTRUCTED)) {
      free_object(&source->u.ob, "subtract_array");
      *source = const0u;
    } else if ((source->type == T_STRING) && !(source->subtype == STRING_SHARED)) {
      svalue_t stmp = {T_STRING, STRING_SHARED};

      if (!(stmp.u.string = findstring(source->u.string))) {
        assign_svalue_no_free(dest++, source);
        continue;
      }
      while ((d = alist_cmp(&stmp, svt + o))) {
        if (d < 0) {
          h = o - 1;
        } else {
          l = o + 1;
        }
        if (l > h) {
          assign_svalue_no_free(dest++, source);
          break;
        }
        o = (l + h) >> 1;
      }
      continue;
    }

    while ((d = alist_cmp(source, svt + o))) {
      if (d < 0) {
        h = o - 1;
      } else {
        l = o + 1;
      }
      if (l > h) {
        assign_svalue_no_free(dest++, source);
        break;
      }
      o = (l + h) >> 1;
    }
  }
  i = size;
  while (i--) {
    free_svalue(svt + i, "subtract_array");
  }
  FREE((char *)svt);
  free_empty_array(subtrahend);
  free_array(minuend);
  msize = dest - difference->item;
  return fix_array(difference, msize);
}

array_t *intersect_array(array_t *a1, array_t *a2) {
  array_t *a3;
  long d, l, j, i, a1s = a1->size, a2s = a2->size, flag;
  svalue_t *svt_1, *ntab, *sv_tab, *sv_ptr, val, *tmp;
  long curix, parix, child1, child2;

  if (!a1s || !a2s) {
    free_array(a1);
    free_array(a2);
    return &the_null_array;
  }

  svt_1 = alist_sort(a1);
  if ((flag = (a2->ref > 1))) {
    sv_tab = reinterpret_cast<svalue_t *>(
        DCALLOC(a2s, sizeof(svalue_t), TAG_TEMPORARY, "intersect_array: sv2_tab"));
    sv_ptr = a2->item;
    for (j = 0; j < a2s; j++) {
      if (((tmp = (sv_ptr + j))->type == T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&tmp->u.ob, "intersect_array");
        sv_tab[j] = *tmp = const0u;
      } else if ((tmp->type == T_STRING) && !(tmp->subtype == STRING_SHARED)) {
        sv_tab[j].u.string = make_shared_string(tmp->u.string);
        (tmp = sv_tab + j)->subtype = STRING_SHARED;
        tmp->type = T_STRING;
      } else {
        assign_svalue_no_free(sv_tab + j, tmp);
      }

      if ((curix = j)) {
        val = *tmp;

        do {
          parix = (curix - 1) >> 1;
          if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
            sv_tab[curix] = sv_tab[parix];
            sv_tab[parix] = val;
          }
        } while ((curix = parix));
      }
    }
  } else {
    char *str;

    sv_tab = a2->item;
    for (j = 0; j < a2s; j++) {
      if (((tmp = (sv_tab + j))->type == T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&tmp->u.ob, "alist_sort");
        *tmp = const0u;
      } else if ((tmp->type == T_STRING) && !(tmp->subtype == STRING_SHARED)) {
        str = make_shared_string(tmp->u.string);
        free_string_svalue(tmp);
        tmp->u.string = str;
        tmp->subtype = STRING_SHARED;
      }

      if ((curix = j)) {
        val = *tmp;

        do {
          parix = (curix - 1) >> 1;
          if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
            sv_tab[curix] = sv_tab[parix];
            sv_tab[parix] = val;
          }
        } while ((curix = parix));
      }
    }
  }

  a3 = ALLOC_ARRAY(a2s);
  ntab = a3->item;

  l = i = 0;

  for (j = 0; j < a2s; j++) {
    val = sv_tab[0];

    while ((d = alist_cmp(&val, &svt_1[i])) > 0) {
      if (++i >= a1s) {
        goto settle_business;
      }
    }

    if (!d) {
      ntab[l++] = val;
    } else {
      free_svalue(&val, "intersect_array");
    }

    for (curix = 0;;) {
      child1 = (curix << 1) + 1;
      child2 = child1 + 1;

      if (child2 < a2s && sv_tab[child2].type != T_INVALID &&
          (sv_tab[child1].type == T_INVALID || alist_cmp(sv_tab + child1, sv_tab + child2) > 0)) {
        child1 = child2;
      }

      if (child1 < a2s && sv_tab[child1].type != T_INVALID) {
        sv_tab[curix] = sv_tab[child1];
        curix = child1;
      } else {
        break;
      }
    }

    sv_tab[curix].type = T_INVALID;
  }

settle_business:

  curix = a2s;
  while (curix--) {
    if (sv_tab[curix].type != T_INVALID) {
      free_svalue(sv_tab + curix, "intersect_array:2");
    }
  }

  i = a1s;
  while (i--) {
    free_svalue(svt_1 + i, "intersect_array");
  }
  FREE((char *)svt_1);

  free_empty_array(a1);

  if (flag) {
    a2->ref--;
    FREE((char *)sv_tab);
  } else {
    dealloc_empty_array(a2);
  }

  return fix_array(a3, l);
}

array_t *union_array(array_t *a1, array_t *a2) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  int a1s = a1->size, a2s = a2->size;
  long d, l, j, i, cnt, flag;
  array_t *a3; /* destination */
  svalue_t *svt_1, *ntab, *sv_tab, *tmp, *sv_ptr, val;
  long curix, parix, child1, child2;

  if (a1s == 0) {
    a1->ref--;
    return a2->ref > 1 ? (a2->ref--, copy_array(a2)) : a2;
  }

  if (a2s == 0 || a1 == a2) {
    a2->ref--;
    return a1->ref > 1 ? (a1->ref--, copy_array(a1)) : a1;
  }
  /* allocating a new place, a1s + a2s must be enough */
  d = a1s + a2s;
  if (d < 0 || d > max_array_size) {
    error("result of array union could be greater than maximum array size.\n");
  }
  a3 = int_allocate_empty_array(d);
  tmp = a1->item;
  ntab = a3->item;
  l = cnt = a1s;
  while (cnt--) {
    assign_svalue_no_free(ntab + cnt, tmp + cnt);
  }

  svt_1 = alist_sort(a1);
  if ((flag = (a2->ref > 1))) {
    sv_tab = reinterpret_cast<svalue_t *>(
        DCALLOC(a2s, sizeof(svalue_t), TAG_TEMPORARY, "union_array: sv2_tab"));
    sv_ptr = a2->item;
    for (j = 0; j < a2s; j++) {
      if (((tmp = (sv_ptr + j))->type == T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&tmp->u.ob, "union_array");
        sv_tab[j] = *tmp = const0u;
      } else if ((tmp->type == T_STRING) && !(tmp->subtype == STRING_SHARED)) {
        sv_tab[j].u.string = make_shared_string(tmp->u.string);
        (tmp = sv_tab + j)->subtype = STRING_SHARED;
        tmp->type = T_STRING;
      } else {
        assign_svalue_no_free(sv_tab + j, tmp);
      }

      if ((curix = j)) {
        val = *tmp;

        do {
          parix = (curix - 1) >> 1;
          if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
            sv_tab[curix] = sv_tab[parix];
            sv_tab[parix] = val;
          }
        } while ((curix = parix));
      }
    }
  } else {
    char *str;

    sv_tab = a2->item;
    for (j = 0; j < a2s; j++) {
      if (((tmp = (sv_tab + j))->type == T_OBJECT) && (tmp->u.ob->flags & O_DESTRUCTED)) {
        free_object(&tmp->u.ob, "union_array");
        *tmp = const0u;
      } else if ((tmp->type == T_STRING) && !(tmp->subtype == STRING_SHARED)) {
        str = make_shared_string(tmp->u.string);
        free_string_svalue(tmp);
        tmp->u.string = str;
        tmp->subtype = STRING_SHARED;
      }

      if ((curix = j)) {
        val = *tmp;

        do {
          parix = (curix - 1) >> 1;
          if (alist_cmp(sv_tab + parix, sv_tab + curix) > 0) {
            sv_tab[curix] = sv_tab[parix];
            sv_tab[parix] = val;
          }
        } while ((curix = parix));
      }
    }
  }

  i = 0;

  for (j = 0; j < a2s; j++) {
    val = sv_tab[0];

    while (i < a1s && (d = alist_cmp(&val, &svt_1[i])) > 0) {
      i++;
    }

    if (d) {
      ntab[l++] = val;
    } else {
      free_svalue(&val, "union_array");
    }

    for (curix = 0;;) {
      child1 = (curix << 1) + 1;
      child2 = child1 + 1;

      if (child2 < a2s && sv_tab[child2].type != T_INVALID &&
          (sv_tab[child1].type == T_INVALID || alist_cmp(sv_tab + child1, sv_tab + child2) > 0)) {
        child1 = child2;
      }

      if (child1 < a2s && sv_tab[child1].type != T_INVALID) {
        sv_tab[curix] = sv_tab[child1];
        curix = child1;
      } else {
        break;
      }
    }

    sv_tab[curix].type = T_INVALID;
  }

  curix = a2s;
  while (curix--) {
    if (sv_tab[curix].type != T_INVALID) {
      free_svalue(sv_tab + curix, "union_array:2");
    }
  }

  i = a1s;
  while (i--) {
    free_svalue(svt_1 + i, "union_array");
  }
  FREE((char *)svt_1);

  free_empty_array(a1);

  if (flag) {
    a2->ref--;
    FREE((char *)sv_tab);
  } else {
    dealloc_empty_array(a2);
  }
  a3 = resize_array(a3, l);
  a3->ref = 1;

  return a3;
}

/*
 * Returns a list of all inherited files.
 *
 * Must be fixed so that any number of files can be returned, now max 256
 * (Sounds like a contradiction to me /Lars).
 */
array_t *deep_inherit_list(object_t *ob) {
  array_t *ret;
  program_t *pr, *plist[256];
  int il, il2, next, cur;

  plist[0] = ob->prog;
  next = 1;
  cur = 0;

  for (; cur < next && next < 256; cur++) {
    pr = plist[cur];
    for (il2 = 0; il2 < pr->num_inherited; il2++) {
      plist[next++] = pr->inherit[il2].prog;
    }
  }

  next--;
  ret = allocate_empty_array(next);

  for (il = 0; il < next; il++) {
    pr = plist[il + 1];
    ret->item[il].type = T_STRING;
    ret->item[il].subtype = STRING_MALLOC;
    ret->item[il].u.string = add_slash(pr->filename);
  }
  return ret;
}

/*
 * Returns a list of the immediate inherited files.
 *
 */
array_t *inherit_list(object_t *ob) {
  array_t *ret;
  program_t *pr, *plist[256];
  int il, il2, next, cur;

  plist[0] = ob->prog;
  next = 1;
  cur = 0;

  pr = plist[cur];
  for (il2 = 0; il2 < pr->num_inherited; il2++) {
    plist[next++] = pr->inherit[il2].prog;
  }

  next--; /* don't count the file itself */
  ret = allocate_empty_array(next);

  for (il = 0; il < next; il++) {
    pr = plist[il + 1];
    ret->item[il].type = T_STRING;
    ret->item[il].subtype = STRING_MALLOC;
    ret->item[il].u.string = add_slash(pr->filename);
  }
  return ret;
}

#ifdef F_LIVINGS
static int livings_filter(object_t *ob, void *data) { return (ob->flags & O_ENABLE_COMMANDS); }

array_t *livings() {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  int count;
  object_t **list;
  array_t *ret;

  get_objects(&list, &count, livings_filter, nullptr);

  if (count > max_array_size) {
    count = max_array_size;
  }
  ret = allocate_empty_array(count);
  while (count--) {
    ret->item[count].type = T_OBJECT;
    ret->item[count].u.ob = list[count];
    add_ref(list[count], "livings");
  }

  pop_stack();
  return ret;
}
#endif

#ifdef F_OBJECTS
void f_objects(void) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  int count, i;
  const char *func = nullptr;
  object_t **list;
  array_t *ret;
  funptr_t *f = nullptr;

  int num_arg = st_num_arg;

  if (!num_arg) {
    func = nullptr;
  } else if (sp->type == T_FUNCTION) {
    f = sp->u.fp;
  } else {
    func = sp->u.string;
  }

  get_objects(&list, &count, nullptr, nullptr);
  if (f || func) {
    /* NOTE: If an object's hidden status changes during a callback, that
     * change will NOT be reflected in the returned array.  If the caller
     * is destructed in a callback or the callback function does not exist,
     * abort and return the_null_array.
     */
    for (i = 0; i < count; i++) {
      svalue_t *v;

      push_object(list[i]);
      if (f) {
        v = call_function_pointer(f, 1);
      } else {
        v = apply(func, current_object, 1, ORIGIN_EFUN);
      }
      if (!v || (current_object->flags & O_DESTRUCTED)) {
        pop_n_elems(num_arg + 1);
        push_refed_array(&the_null_array);
        return;
      }
      if (v->type == T_NUMBER && !v->u.number) {
        list[i] = nullptr;
      }
    }
    for (i = 0; i < count; i++) {
      if (!list[i] || (list[i]->flags & O_DESTRUCTED)) {
        list[i--] = list[--count];
      }
    }
  }

  if (count > max_array_size) {
    count = max_array_size;
  }
  ret = allocate_empty_array(count);
  for (i = 0; i < count; i++) {
    ret->item[i].type = T_OBJECT;
    ret->item[i].u.ob = list[i];
    add_ref(list[i], "f_objects");
  }

  pop_n_elems(num_arg + 1); /* include our temporary 'string' */
  push_refed_array(ret);
}
#endif
