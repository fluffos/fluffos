#include "std.h"
#include "file_incl.h"
#include "lpc_incl.h"

void debug_message P1V(char *, fmt)
{
    static char deb_buf[256];
    static char *deb = deb_buf;
    static FILE *debug_message_fp = 0;
    va_list args;
    V_DCL(char *fmt);

    if (!debug_message_fp) {
	/* check whether config file specified this option */
	if (*DEBUG_LOG_FILE)
	    sprintf(deb, "%s/%s", LOG_DIR, DEBUG_LOG_FILE);
	else
	    sprintf(deb, "%s/debug.log", LOG_DIR);
	while (*deb == '/')
	    deb++;
	debug_message_fp = fopen(deb, "w");
	if (!debug_message_fp) {
	    /* darn.  We're in trouble */
	    perror(deb);
	    abort();
	}
    }

    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vfprintf(debug_message_fp, fmt, args);
    va_end(args);
    fflush(debug_message_fp);

    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

/*
 * open file for writing, creating intermediate directories if needed
 * Moved from binaries.c - idea from CD, Darin Johnson
 */
FILE *crdir_fopen P1(char *, file_name)
{
    char *p;
    struct stat st;
    FILE *ret;

    /*
     * Beek - These directories probably exist most of the time, so let's
     * optimize by trying the fopen first
     */
    if ((ret = fopen(file_name, "wb")) != NULL)
	return ret;

    for (p = file_name;  *p && (p = strchr(p, '/'));  *p++ = '/') {
	*p = 0;
	if (stat(file_name, &st) == -1) {
	    /* make this dir */
	    if (OS_mkdir(file_name, 0770) == -1) {
		*p = '/';
		return NULL;
	    }
	}
    }

    return fopen(file_name, "wb");
}

/*
 * median-3 variant of quicksort - coded by John Garnett.
 *
 * using this quicksort rather than the builtin one because most
 * builtin implementations choke on non-deterministic compare functions
 * (and we can't control what compare function is used since it is at
 * the mudlib level).  Based on algorithm appearing in _Data Structures and
 * Algorithm Analysis_ by Cawnthorpe.
 */

INLINE_STATIC void doSwap P3(register char *, one, register char *, two, register int, size)
{
    register char t;

    while (size--) {
	t = *one;
	*(one++) = *two;
	*(two++) = t;
    }
}

/* qsort adapted from page 87 of K&R 2nd edition */
static void qSort P6(void *, v, int, left, int, right, int, size, int, rightmost, qsort_comparefn_t, compar)
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

void quickSort P4(void *, a, int, nmemb, int, size, qsort_comparefn_t, compar)
{
    if (nmemb >= 2)
	qSort(a, 0, nmemb - 1, size, nmemb - 1, compar);
}

void outbuf_zero P1(outbuffer_t *, outbuf)
{
    outbuf->real_size = 0;
    outbuf->buffer = 0;
}

int outbuf_extend P2(outbuffer_t *, outbuf, int, l)
{
    int limit;

    DEBUG_CHECK(l < 0, "Negative length passed to outbuf_extend.\n");

    if (outbuf->buffer) {
	limit = MSTR_SIZE(outbuf->buffer);
	if (outbuf->real_size + l > limit) {
	    if (outbuf->real_size == USHRT_MAX) return 0; /* TRUNCATED */

	    /* assume it's going to grow some more */
	    limit = (outbuf->real_size + l) * 2;
	    if (limit > USHRT_MAX) {
		limit = outbuf->real_size + l;
		if (limit > USHRT_MAX) {
		    outbuf->buffer = extend_string(outbuf->buffer, USHRT_MAX);
		    return USHRT_MAX - outbuf->real_size;
		}
	    }
	    outbuf->buffer = extend_string(outbuf->buffer, limit);
	}
    } else {
	outbuf->buffer = new_string(l, "outbuf_add");
	outbuf->real_size = 0;
    }
    return l;
}

void outbuf_add P2(outbuffer_t *, outbuf, char *, str)
{
    int l, limit;

    if (!outbuf || !(l = strlen(str)))
	return;
    if (!(limit = outbuf_extend(outbuf, l)))
	return;

    strncpy(outbuf->buffer + outbuf->real_size, str, limit);
    outbuf->real_size += limit;
    *(outbuf->buffer + outbuf->real_size) = 0;
}

void outbuf_addchar P2(outbuffer_t *, outbuf, char, c)
{
    int limit;

    if (!outbuf) return;
    if (!(limit = outbuf_extend(outbuf, 1)))
	return;

    *(outbuf->buffer + outbuf->real_size++) = c;
    *(outbuf->buffer + outbuf->real_size) = 0;
}

void outbuf_addv P2V(outbuffer_t *, outbuf, char *, format)
{
    char buf[LARGEST_PRINTABLE_STRING + 1];
    va_list args;

    V_DCL(char *format);
    V_DCL(outbuffer_t *outbuf);

    V_START(args, format);
    V_VAR(outbuffer_t *, outbuf, args);
    V_VAR(char *, format, args);

    vsprintf(buf, format, args);
    va_end(args);

    if (!outbuf) return;
    outbuf_add(outbuf, buf);
}

void outbuf_fix P1(outbuffer_t *, outbuf)
{
    if (outbuf && outbuf->buffer)
	outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);
}

void outbuf_push P1(outbuffer_t *, outbuf)
{
    STACK_INC;
    sp->type = T_STRING;
    if (outbuf && outbuf->buffer) {
	sp->subtype = STRING_MALLOC;
	sp->u.string = extend_string(outbuf->buffer, outbuf->real_size);
    } else {
	sp->subtype = STRING_CONSTANT;
	sp->u.string = "";
    }
}

void outbuf_free P1(outbuffer_t *, outbuf)
{
    if (outbuf && outbuf->buffer)
	FREE_MSTR(outbuf->buffer);
    outbuf_zero(outbuf);
}
