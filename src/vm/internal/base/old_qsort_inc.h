/* median-3 variant of quicksort - coded by John Garnett.

   This implementation is retained to keep the broken sorting
   behavior, see SANE_SORTING in options.h for details.

   -sunyc@2013-04-27
*/

#ifndef _OLD_QSORT_INC_H
#define _OLD_QSORT_INC_H

#define LEN sizeof(svalue_t)
#define MAX_LEN 1000

static void doSwap(char *, char *, int);
static void qSort(void *, int, int, int, int, int (*)(const void *, const void *));
static void doSwap(char *one, char *two, int size) {
  char t;

  while (size--) {
    t = *one;
    *(one++) = *two;
    *(two++) = t;
  }
}

/* qsort adapted from page 87 of K&R 2nd edition */

static void qSort(void *v, int left, int right, int size, int rightmost,
                  int (*compar)(const void *, const void *)) {
  int i, last, szleft;

  if ((left >= right) || (left < 0) || (right > rightmost) || (right < 0)) {
    return;
  }
  szleft = size * left;
  doSwap((char *)v + szleft, (char *)v + (size * ((left + right) / 2)), size);
  last = left;
  for (i = left + 1; i <= right; i++) {
    if ((*compar)((char *)v + (size * i), (char *)v + szleft) < 0) {
      doSwap((char *)v + (size * ++last), (char *)v + (size * i), size);
    }
  }
  doSwap((char *)v + szleft, (char *)v + (size * last), size);
  qSort(v, left, last - 1, size, rightmost, compar);
  qSort(v, last + 1, right, size, rightmost, compar);
}

void old_quickSort(void *a, int nmemb, int size, int (*compar)(const void *, const void *)) {
  if (nmemb < 2) {
    return;
  }
  qSort(a, 0, nmemb - 1, size, nmemb - 1, compar);
}

#endif /* _OLD_QSORT_INC_H */
