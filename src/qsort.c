/* median-3 variant of quicksort - coded by John Garnett.

   using this quicksort rather than the builtin one because most
   builtin implementations choke on non-deterministic compare functions
   (and we can't control what compare function is used since it is at
   the mudlib level).  Based on algorithm appearing in _Data Structures and
   Algorithm Analysis_ by Cawnthorpe.
*/

#include "std.h"
#include "qsort.h"

#define LEN sizeof(svalue_t)
#define MAX_LEN 1000

INLINE_STATIC void doSwap PROT((char *, char *, int));
static void qSort PROT((void *, int, int, int, int, int (*) ()));

INLINE_STATIC void doSwap P3(register char *, one, register char *, two,
                             register int, size)
{
    register char t;

    while (size--) {
	t = *one;
	*(one++) = *two;
	*(two++) = t;
    }
}

/* qsort adapted from page 87 of K&R 2nd edition */

static void qSort(v, left, right, size, rightmost, compar)
    void *v;
    int left, right, size, rightmost;
    int (*compar) PROT((void *, void *));
{
    int i, last, szleft;

    if ((left >= right) || (left < 0) || (right > rightmost) || (right < 0)) {
	return;
    }
    szleft = size * left;
    doSwap((char *) v + szleft, (char *) v + (size * ((left + right) / 2)), size);
    last = left;
    for (i = left + 1; i <= right; i++) {
	if ((*compar) ((char *) v + (size * i), (char *) v + szleft) < 0) {
	    doSwap((char *) v + (size * ++last), (char *) v + (size * i), size);
	}
    }
    doSwap((char *) v + szleft, (char *) v + (size * last), size);
    qSort(v, left, last - 1, size, rightmost, compar);
    qSort(v, last + 1, right, size, rightmost, compar);
}

void quickSort(a, nmemb, size, compar)
    void *a;
    int nmemb, size;
    int (*compar) PROT((void *, void *));
{
    if (nmemb < 2) {
	return;
    }
    qSort(a, 0, nmemb - 1, size, nmemb - 1, compar);
}
