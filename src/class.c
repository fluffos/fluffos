#include "std.h"
#include "lpc_incl.h"

void dealloc_class P1(array_t *, p) {
    int i;

    for (i = p->size; i--;)
	free_svalue(&p->item[i], "dealloc_class");
    FREE((char *) p);
}

array_t *allocate_class P1(class_def_t *, cld) {
    array_t *p;
    int n = cld->size;

    p = (array_t *)DXALLOC(sizeof(array_t) + sizeof(svalue_t) * (n - 1), TAG_CLASS, "allocate_class");
    p->ref = 1;
    p->size = n;
    while (n--)
	p->item[n] = const0;
    return p;
}
