/* median-3 variant of quicksort - coded by John Garnett.

   using this quicksort rather than the builtin one because most
   builtin implementations choke on non-deterministic compare functions
   (and we can't control what compare function is used since it is at
   the mudlib level).  Based on algorithm appearing in _Data Structures and
   Algorithm Analysis_ by Cawnthorpe.
*/

#include "config.h"
#include "lint.h"
#include "mudlib_stats.h"
#include "interpret.h"

#define LEN sizeof(struct svalue)

/* define cutoff point at which insertion sort is used */
#define CUTOFF (10 * LEN)

INLINE static void
insertionSort(data, n, size, compar)
char *data;
int n, size;
int (*compar) PROT((void *, void *));
{
	static char x[LEN];
	int i, j;

	for (i = 0; i < n; i += size) {
		j = i;
		memcpy(x, data + j, size);
		while ((j >= size) && (compar(x, data + j - size) < 0)) {
			memcpy(data + j, data + j - size, size);
			j -= size;
		}
		memcpy(data + j, x, size);
	}
}

INLINE static void
doSwap(one, two, size)
void *one, *two;
int size;
{
	static char buf[LEN];

	memcpy(buf, one, size);
	memcpy(one, two, size);
	memcpy(two, buf, size);
}

INLINE static void
median3(a, left, right, pivot, size, compar)
char *a;
int left, right;
void **pivot;
int size;
int (*compar) PROT((void *, void *));
{
	int center;

	center = (left + right) / 2;
	/* round down to nearest multiple of size */
	center -= (center % size);
	if (compar(a + left, a + center) > 0) {
		doSwap(a + left, a + center, size);
	}
	if (compar(a + left, a + right) > 0) {
		doSwap(a + left, a + right, size);
	}
	if (compar(a + center, a + right) > 0) {
		doSwap(a + center, a + right, size);
	}
	*pivot = a + center;
	doSwap(a + center, a + right - size, size);
}

void
qSort(a, left, right, size, compar)
char *a;
int left, right, size;
int (*compar) PROT((void *, void *));
{
	char *pivot, *temp;
	int i, j;

	if ((left + CUTOFF) <= right) {
		median3(a, left, right, &pivot, size, compar);
		i = left;
		j = right - size;
		while (1) {
			while (1) {
				temp = a + (i += size);
				/* protect against non-determistic compare */
				if (i >= right) {
					return;
				}
				if (compar(temp, pivot) >= 0) {
					break;
				}
			}
			while (1) {
				temp = a + (j -= size);
				/* protect against non-determistic compare */
				if (j < 0) {
					return;
				}
				if (compar(temp, pivot) <= 0) {
					break;
				}
			}
			if (j <= i) {
				break;
			}
			doSwap(a + i, a + j, size);
		}
		doSwap(a + i, a + right - size, size);
		qSort(a, left, i - size, size, compar);
		qSort(a, i + size, right, size, compar);
	}
}

void
quickSort(a, nmemb, size, compar)
void *a;
int nmemb, size;
int (*compar) PROT((void *, void *));
{
	if (nmemb < 2) {
		return;
	}
	qSort(a, 0, size * (nmemb - 1), size, compar);
	insertionSort(a, size * nmemb, size, compar);
}
