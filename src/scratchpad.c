#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "lpc_incl.h"
#include "scratchpad.h"
#include "compiler.h"

/*
 * This is a first attempt at beating malloc() for allocation of strings
 * during compilation.  It's pretty general, and could probably be done
 * better.
 * 
 * Here's what we can assume:
 * Things are going to conform to LIFO order, more or less.
 * Strings will be of the long variety and the short variety.
 * A realloc on strings might be nice, for "a" "b" "c" ... etc
 * 
 * Although in rare cases (errors) certain items won't come off until the
 * table is destroyed.
 */
/* Here is what is currently being used:
 *
 * <0> <0> string1 <len1> string2 <len2>
 *                        ^          ^
 *                        last      tail
 *
 * len1 is the length of string1 including the zero at the end
 *
 * Note: "" looks a heck of a lot like a interior freed string.  Currently,
 * we ignore the problem.  In some cases, it could be left dangling, but
 * I don't think that can happen with the present grammar/use of the
 * scratchpad.
 */
/*
 *  Todo: This algorithm might be faster if we aligned to 2 byte
 *  boundaries and used shorts for lengths.  We wouldn't have to
 *  worry about the 256 byte limit then
 */
/*
 * Within this file, a capitalized identifier is that var cast to an
 * unsigned type.  It makes things easier to read than having casts
 * all over the place since we go back and forth a lot.  strict ANSI
 * requires casts from (unsigned char *) to (char *) and back, but
 * we want to deal with strings as unsigned since we keep the length
 * in them.
 */
#define Str ((unsigned char *)str)
#define Ptr ((unsigned char *)ptr)
#define Res ((unsigned char *)res)
#define S1 ((unsigned char *)s1)
#define S2 ((unsigned char *)s2)
#define Scratch_large_alloc(x) ((unsigned char *)scratch_large_alloc(x))
#define Strlen(x) (strlen((char *)x))
#define Strcpy(x, y) (strcpy((char *)x, (char *)y))
#define Strncpy(x, y, z) (strncpy((char *)x, (char *)y, z))

/* not strictly ANSI, but should always work ... */
#define HDR_SIZE ((char *)&scratch_head.block[2] - (char *)&scratch_head)
#define FIND_HDR(x) ((sp_block_t *)(x - HDR_SIZE))
#define SIZE_WITH_HDR(x) (x + HDR_SIZE)

static unsigned char scratchblock[SCRATCHPAD_SIZE];
static sp_block_t scratch_head = { 0, 0 };
unsigned char *scr_last = &scratchblock[2], *scr_tail = &scratchblock[2];
unsigned char *scratch_end = scratchblock + SCRATCHPAD_SIZE;

#if 0
static void scratch_summary (void);

static void scratch_summary() {
    unsigned char *p = scratchblock;
    int i;
    
    while (p<=scr_tail) {
        if (*p == 0) printf("0");
        else if (*p < 32 || *p > 127) printf("*");
        else printf("%c", *p);
        p++;
    }
    printf("\n");
    i = scr_last - scratchblock;
    while (i--) printf(" ");
    printf("l\n");
    i = scr_tail - scratchblock;
    while (i--) printf(" ");
    printf("t\n");
}
#endif

void scratch_destroy() {
    sp_block_t *next, *thisb = scratch_head.next;

    SDEBUG(printf("scratch_destroy\n"));

    while (thisb) {
        next = thisb->next;
        FREE(thisb);
        thisb = next;
    }
    scratch_head.next = 0;
    scr_last = &scratchblock[2];
    scr_tail = &scratchblock[2];
}


char *scratch_copy (const char * str) {
    unsigned char *from, *to, *end;

    SDEBUG2(printf("scratch_copy(%s):", str));

    /* first, take a wild guess that there is room and save a strlen() :) */
    from = Str;
    to = scr_tail+1;
    end = scratch_end - 2; /* room for zero and len */
    if (end > to + 255) end = to + 255;
    while (*from && to < end)
        *to++ = *from++;
    if (!(*from)) {
        SDEBUG2(printf(" on scratchpad\n"));

        scr_last = scr_tail + 1;
        *to++ = 0;
        scr_tail = to;
        *to = to - scr_last;
        return (char *)scr_last;
    }
    SDEBUG(printf(" mallocing ... "));

    /* ACK! no room. strlen(str) == (from - str) + strlen(from) */
    to = Scratch_large_alloc((from - Str) + Strlen(from) + 1);
    Strcpy(to, str);
    return (char *)to;
}

void scratch_free (char * ptr) {
    /* how do we know what this is?  first we check if it's the last string
       we made.  Otherwise, take advantage of the fact that things on the
       scratchpad have a zero two before them.  Things not on it wont
       (we make sure of this) */

    SDEBUG2(printf("scratch_free(%s): ", ptr));

    if (Ptr == scr_last) {
        SDEBUG2(printf("last freed\n"));
        scratch_free_last();
    } else if (*(Ptr - 2)) {
        sp_block_t *sbt;

        DEBUG_CHECK(*(Ptr - 2) != SCRATCH_MAGIC, "scratch_free called on non-scratchpad string.\n");
        SDEBUG(printf("block freed\n"));
        sbt = FIND_HDR(ptr);
        if (sbt->prev)
            sbt->prev->next = sbt->next;
        if (sbt->next)
            sbt->next->prev = sbt->prev;
        FREE(sbt);
    } else {
        SDEBUG(printf("interior free\n"));
        *ptr = 0; /* mark it as freed */
    }
}

char *scratch_large_alloc (int size) {
    sp_block_t *spt;

    SDEBUG(printf("scratch_large_alloc(%i)\n", size));

    spt = (sp_block_t *)DMALLOC(SIZE_WITH_HDR(size), TAG_COMPILER, "scratch_alloc");
    if ((spt->next = scratch_head.next))
        spt->next->prev = spt;
    spt->prev = (sp_block_t *)&scratch_head;
    spt->block[0] = SCRATCH_MAGIC;
    scratch_head.next = spt;
    return (char *)&spt->block[2];
}

/* warning: unlike REALLOC(), this one only allows increases */
char *scratch_realloc (char * ptr, int size) {
    SDEBUG(printf("scratch_realloc(%s): ", ptr));

     if (Ptr == scr_last) {
         if (size < 256 && (scr_last + size) < scratch_end) {
             SDEBUG(printf("on scratchpad\n"));
             scr_tail = scr_last + size;
             *scr_tail = size;
             return ptr;
         } else {
             char *res;
             SDEBUG(printf("copy off ... "));
             res = scratch_large_alloc(size);
             strcpy(res, ptr);
             scratch_free_last();
             return res;
         }
     } else if (*(Ptr - 2)) {
         sp_block_t *sbt, *newsbt;

        DEBUG_CHECK(*(Ptr - 2) != SCRATCH_MAGIC, "scratch_realloc on non-scratchpad string.\n");
         SDEBUG(printf("block\n"));
         sbt = FIND_HDR(ptr);
         newsbt = (sp_block_t *)DREALLOC(sbt, SIZE_WITH_HDR(size),
                                         TAG_COMPILER, "scratch_realloc");
         newsbt->prev->next = newsbt;
         if (newsbt->next)
             newsbt->next->prev = newsbt;
         return (char *)&newsbt->block[2];
     } else {
         char *res;

         SDEBUG(printf("interior ... "));
         /* ACK!! it's in the middle. */
         if (size < 256 && (scr_tail + size + 1) < scratch_end) {
             SDEBUG(printf("move to end\n"));
             scr_last = scr_tail + 1;
             Strcpy(scr_last, ptr);
             scr_tail = scr_last + size;
             *scr_tail = size;
             res = (char *)scr_last;
         } else {
             SDEBUG(printf("copy off ... "));
             res = scratch_large_alloc(size);
             strcpy(res, ptr);
         }
         *ptr = 0; /* free the old version */
         return res;
     }
}

/* the routines above are better than this */
char *scratch_alloc (int size) {
    SDEBUG(printf("scratch_alloc(%i)\n", size));
    if (size < 256 && (scr_tail + size + 1) < scratch_end) {
        scr_last = scr_tail + 1;
        scr_tail = scr_last + size;
        *scr_tail = size;
        return (char *)scr_last;
    } else
        return scratch_large_alloc(size);
}

char *scratch_join (char * s1, char * s2) {
    char *res;
    int tmp;

    SDEBUG(printf("scratch_join\n"));
    if (*(s1-2) || *(s2-2)) {
        int l = strlen(s1);

        DEBUG_CHECK(*(S1 - 2) && *(S1 - 2) != SCRATCH_MAGIC, "argument 1 to scratch_join was not a scratchpad string.\n");
        DEBUG_CHECK(*(S2 - 2) && *(S2 - 2) != SCRATCH_MAGIC, "argument 2 to scratch_join was not a scratchpad string.\n");

        res = scratch_realloc(s1, l + strlen(s2) + 1);
        strcpy(res + l, s2);
        scratch_free(s2);
        return res;
    } else {
        /* This assumes that S1 and S2 were the last two things allocated.
           Make sure this is true */
        DEBUG_CHECK(S2 != scr_last, "Argument 2 to scratch_join was not the last allocated string.\n");
        DEBUG_CHECK(S1 != (scr_last - 1 - (*(scr_last - 1))), "Argument 1 to scratch_join was not the second to last allocated string.\n");

        if ((tmp = ((scr_tail - S1) - 2)) < 256) {
            scr_tail = scr_last - 2;
            do {
                *scr_tail = *(scr_tail + 2);
            } while (*scr_tail++);
            *scr_tail = tmp;
            scr_last = S1;
            return s1;
        } else {
            char *ret = scratch_large_alloc(tmp);
            strcpy(ret, s1);
            strcpy(ret + (scr_last - S1) - 2, s2);
            scratch_free(s1);
            scratch_free(s2);
            return ret;
        }
    }
}

char *scratch_copy_string (char *s) {
    int l;
    register unsigned char *to = scr_tail + 1;
    char *res;

    SDEBUG2(printf("scratch_copy_string\n"));
    l = scratch_end - to;

    if (l > 255) l = 255;
    s++;
    while (l--) {
        if (*s == '\\') {
            switch (*++s) {
            case 'n': *to++ = '\n'; break;
            case 't': *to++ = '\t'; break;
            case 'r': *to++ = '\r'; break;
            case 'b': *to++ = '\b'; break;
            case '"':
            case '\\': *to++ = *s; break;
            default:
                *to++ = *s;
                yywarn("Unknown \\x char.");
            }
            s++;
        } else if (*s == '"') {
            *to++ = 0;
            if (!l && (to == scratch_end)) {
                res = scratch_large_alloc(to - scr_tail - 1);
                Strcpy(res, scr_tail + 1);
                return res;
            }
            scr_last = scr_tail + 1;
            scr_tail = to;
            *to = to - scr_last;
            return (char *)scr_last;
        } else
            *to++ = *s++;
    }
    /* estimate the length we need */
    /* Note that the last char is we read is ", not \0 - Sym */
    res = scratch_large_alloc(to - scr_tail + strlen(s) - 1);
    Strncpy(res, (scr_tail + 1), (to - scr_tail) - 1);
    to = Res + (to - scr_tail) - 1;
    for (;;) {
        if (*s == '\\') {
            switch (*++s) {
            case 'n': *to++ = '\n'; break;
            case 't': *to++ = '\t'; break;
            case 'r': *to++ = '\r'; break;
            case 'b': *to++ = '\b'; break;
            case '"':
            case '\\': *to++ = *s; break;
            default:
                *to++ = *s;
                yywarn("Unknown \\x char.");
            }
            s++;
        } else if (*s == '"') {
            *to++ = 0;
            return res;
        } else
            *to++ = *s++;
    }
}



