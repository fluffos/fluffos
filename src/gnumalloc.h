/* malloc.h - declarations for the allocator.
   Copyright (c) 1989, 1993  Michael J. Haertel
   You may redistribute this library under the terms of the
   GNU Library General Public License (version 2 or any later
   version) as published by the Free Software Foundation.
   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
   WARRANTY.  IN PARTICULAR, THE AUTHOR MAKES NO REPRESENTATION OR
   WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
   SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE. */

/* Underlying allocation function; successive calls should return
   contiguous pieces of memory. */

#ifndef GNU_MALLOC_H
#define GNU_MALLOC_H
extern void *(*_morecore)(long);

/* Default value of previous. */
extern void *_default_morecore(long);

/* The allocator divides the heap into blocks of fixed size; large
   requests receive one or more whole blocks, and small requests
   receive a fragment of a block.  Fragment sizes are powers of two,
   and all fragments of a block are the same size.  When all the
   fragments in a block have been freed, the block itself is freed.
   WARNING: BLOCKSIZE must be set greater than or equal to the
   machine's page size for valloc() to work correctly.  The default
   definition here is 4096 bytes. */
#define INT_BIT (CHAR_BIT * sizeof (int))
#define BLOCKLOG (INT_BIT > 16 ? 12 : 9)
#define BLOCKSIZE (1 << BLOCKLOG)
#define BLOCKIFY(SIZE) (((SIZE) + BLOCKSIZE - 1) / BLOCKSIZE)

/* Determine the amount of memory spanned by the initial heap table
   (not an absolute limit). */
#define HEAP (INT_BIT > 16 ? 4194304 : 65536)

/* Number of contiguous free blocks allowed to build up at the end of
   memory before they will be returned to the system. */
#define FINAL_FREE_BLOCKS 8

/* Data structure giving per-block information. */
union info {
    struct {
	int type;		/* Zero for a large block, or positive
				   giving the logarithm to the base two
				   of the fragment size. */
	union {
	    struct {
		int nfree;	/* Free fragments in a fragmented block. */
		int first;	/* First free fragment of the block. */
	    } frag;
	    int size;		/* Size (in blocks) of a large cluster. */
	} info;
    } busy;
    struct {
	int size;		/* Size (in blocks) of a free cluster. */
	int next;		/* Index of next free cluster. */
	int prev;		/* Index of previous free cluster. */
    } free;
};

/* Pointer to first block of the heap. */
extern char *_heapbase;

/* Table indexed by block number giving per-block information. */
extern union info *_heapinfo;

/* Address to block number and vice versa. */
#define BLOCK(A) (((char *) (A) - _heapbase) / BLOCKSIZE + 1)
#define ADDRESS(B) ((void *) (((B) - 1) * BLOCKSIZE + _heapbase))

/* Current search index for the heap table. */
extern int _heapindex;

/* Limit of valid info table indices. */
extern int _heaplimit;

/* Doubly linked lists of free fragments. */
struct list {
    struct list *next;
    struct list *prev;
};

/* Count of blocks for each fragment size. */
extern int _fragblocks[];

/* Free list headers for each fragment size. */
extern struct list _fraghead[];

void *gnumalloc(size_t);
void gnufree(void *);
void *gnurealloc(void *, size_t);
void *gnucalloc(size_t,size_t);

#endif
