#ifndef _SCRATCHPAD_H
#define _SCRATCHPAD_H

/* Tim Hollebeek
 *
 * The data must also consist of zero terminated chunks, with lengths in the
 * range 0 < len < 256.  Longer things can be handled, but they're just
 * malloc'ed.
 *
 * Designed to be used by the compile stack (if pointers get popped off due
 * to errors, we don't have to worry about them b/c we'll reclaim the space
 * when we throw away the scratchpad; this is another advantage), it could
 * be used by other things as well.
 */

#define SCRATCHPAD_SIZE  4096
#define SDEBUG(x) 
#define SDEBUG2(x) 

#define SCRATCH_MAGIC ((unsigned char)0xbb)

typedef struct sp_block_s {
    struct sp_block_s *next, *prev;
    char block[2]; /* block[0] must be nonzero, usually SCRATCH_MAGIC */
} sp_block_t;

#define scratch_free_last() \
    scr_tail = --scr_last; \
    scr_last -= *scr_tail; \
    while (!(*scr_last) && scr_tail != scr_last) { \
        /* check if the one before was already freed */ \
        scr_tail = --scr_last; \
        scr_last -= *scr_tail; \
    }

extern unsigned char *scr_last;
extern unsigned char *scr_tail;
extern unsigned char *scratch_end;

/*
 *  scratchpad.c
 */
void scratch_destroy (void);
char *scratch_copy (const char *);
char *scratch_alloc (int);
void scratch_free (char *);
char *scratch_join (char *, char *);
char *scratch_join2 (char *, char *);
char *scratch_realloc (char *, int);
char *scratch_copy_string (char *);
char *scratch_large_alloc (int);

#endif

